#include "serial_communication.h"
#include "command_handler.h"
#include "roi_processor.h"

/*
目标功能：串口通讯协议处理模块 - 专注于数据收发和协议解析
 * init(baud) - 初始化串口通信，默认115200波特率，发送"v"确认信号
 * processReceivedData() - 处理接收到的串口数据，逐字符缓冲直到收到完整命令行
 * sendMessage(msg) - 发送消息到串口，不添加换行符
 * sendLine(msg) - 发送消息到串口，自动添加换行符
 * isDataAvailable() - 检查串口是否有数据可读
 * isReady() - 检查串口是否已初始化完成
 * 
 * 支持的数据类型和命令:
 * 1. ROI数据格式: "ROI1,X123.45,Y67.89" - 自动转发给ROIProcessor处理
 * 2. ROI管理命令: 
 *    - "PROCESS_ROI" - 处理所有已接收的ROI数据
 *    - "CLEAR_ROI" - 清空ROI数据缓存
 * 3. 手动调试命令: 转发给CommandHandler处理
 * 
 * 响应消息:
 * "ACK" - ROI数据接收确认
 * "ROI_FULL" - ROI缓存已满
 * "ROI_PROCESSED" - ROI处理完成  
 * "ROI_CLEARED" - ROI数据已清空
 * "ERROR: xxx" - 错误信息
 * 
 * 安全特性:
 * - 缓冲区溢出保护 (128字节限制)
 * - 数据格式验证
 * - 空指针检查
 * - 模块可用性检查
*/

// 构造函数：设置默认值
SerialCommunication::SerialCommunication(CommandHandler* handler, ROIProcessor* processor, StepperControl* stepper) 
    : commandHandler_(handler), 
      roiProcessor_(processor),
      stepperControl_(stepper),
      serialPort_(&Serial),          // 设置默认串口指针
      baudRate_(115200),             // 设置默认波特率
      isInitialized_(false),
      currentBatchCount_(0),         // 初始化批次计数
      totalProcessedCount_(0),       // 初始化总处理计数
      bufferIndex_(0),
      isStringComplete_(false)
{
    memset(inputBuffer_, 0, sizeof(inputBuffer_));
    // 初始化批次缓冲区
    for (int i = 0; i < BATCH_BUFFER_SIZE; i++) {
        batchBuffer_[i] = {0, 0.0, 0.0};
    }
}

// init函数：实际的初始化
void SerialCommunication::init(unsigned long baudRate) {
    baudRate_ = baudRate;              // 直接设置用户指定的波特率
    serialPort_->begin(baudRate_);     // 启动硬件
    isInitialized_ = true;             // 标记已初始化
    sendLine("v");                     // 发送确认信号
}

void SerialCommunication::init(HardwareSerial* port, unsigned long baudRate) {
    serialPort_ = port;            
    baudRate_ = baudRate;
    serialPort_->begin(baudRate_);
    isInitialized_ = true;
    sendLine("v"); // 发送初始化完成信号
}

// 检查串口是否有数据可读
bool SerialCommunication::isDataAvailable() {
    return isInitialized_ && serialPort_->available() > 0;
}

// 读取串口数据
String SerialCommunication::readCommand() {
    if (!isDataAvailable()) {
        return "";
    }
    return serialPort_->readStringUntil('\n');
}

// 发送数据到串口
void SerialCommunication::sendMessage(const String& message) {
    if (isInitialized_) {
        serialPort_->print(message);
    }
}

void SerialCommunication::sendLine(const String& message) {
    if (isInitialized_) {
        serialPort_->println(message);
    }
}

// 处理接收到的数据
void SerialCommunication::processReceivedData() {
    // 逐字符读取并缓冲
    while (serialPort_->available()) {
        char inChar = (char)serialPort_->read();
        
        // 防止缓冲区溢出
        if (bufferIndex_ < (sizeof(inputBuffer_) - 1)) {
            inputBuffer_[bufferIndex_] = inChar;
            bufferIndex_++;
        }

        // 检查是否接收到换行符，表示一行数据结束
        if (inChar == '\n') {
            inputBuffer_[bufferIndex_] = '\0';
            isStringComplete_ = true;
            break;
        }
    }

    // 如果接收到完整的一行数据，开始处理
    if (isStringComplete_) {
        processCompleteLine();
    }
}

// 获取串口状态
bool SerialCommunication::isReady() const {
    return isInitialized_;
}

// 设置波特率
void SerialCommunication::setBaudRate(unsigned long baudRate) {
    baudRate_ = baudRate;
    if (isInitialized_) {
        serialPort_->end();
        serialPort_->begin(baudRate_);
    }
}

// 数据验证接口
bool SerialCommunication::validateData(const char* data) {
    // 默认验证：非空且长度合理
    return (data != nullptr && strlen(data) > 0 && strlen(data) < 128);
}

// 错误处理接口
void SerialCommunication::handleError(const char* errorMsg) {
    // 默认错误处理：输出错误信息
    sendMessage(F("ERROR: "));
    sendLine(errorMsg);
}

// 数据缓冲区清理
void SerialCommunication::clearBuffer() {
    if (isInitialized_) {
        while (serialPort_->available() > 0) {
            serialPort_->read();
        }
    }
}

// 批次状态变量
bool batch_in_progress_ = false;
int expected_target_count_ = 0;
int processed_target_count_ = 0;

// 批次状态重置
void SerialCommunication::resetBatchState() {
    batch_in_progress_ = false;
    expected_target_count_ = 0;
    processed_target_count_ = 0;
}

// 处理完整命令行（新协议）
void SerialCommunication::processCompleteLine() {
    // 移除换行符
    if (bufferIndex_ > 0 && inputBuffer_[bufferIndex_-1] == '\n') {
        inputBuffer_[bufferIndex_-1] = '\0';
        bufferIndex_--;
    }
    if (bufferIndex_ > 0) {
        String msg(inputBuffer_);
        msg.trim();
        if (msg.startsWith("BATCH_START")) {
            int count_index = msg.indexOf("COUNT=");
            if (count_index != -1) {
                int count = msg.substring(count_index + 6).toInt();
                if (count > 0 && count <= 2000) {
                    resetBatchState();
                    expected_target_count_ = count;
                    batch_in_progress_ = true;
                    sendLine("BATCH_READY");
                } else {
                    sendLine("ERROR: Invalid target count");
                }
            } else {
                sendLine("ERROR: Invalid BATCH_START format");
            }
        } else if (msg.startsWith("TARGET")) {
            if (!batch_in_progress_ || expected_target_count_ == 0) {
                sendLine("ERROR: Batch not started");
            } else {
                // 解析SEQ、X、Y
                int seq_index = msg.indexOf("SEQ=");
                int x_index = msg.indexOf("X");
                int y_index = msg.indexOf("Y");
                int comma1 = msg.indexOf(',', seq_index);
                int comma2 = msg.indexOf(',', x_index);
                if (seq_index != -1 && x_index != -1 && y_index != -1 && comma1 != -1 && comma2 != -1) {
                    int seq = msg.substring(seq_index + 4, comma1).toInt();
                    float x = msg.substring(x_index + 1, comma2).toFloat();
                    float y = msg.substring(y_index + 1).toFloat();
                    if (seq > 0) {
                        // 执行目标处理
                        if (stepperControl_) {
                            int mmX = (int)x;
                            int mmY = (int)y;
                            stepperControl_->moveYAxisTo(mmY);
                            while (stepperControl_->isAnyRunning()) stepperControl_->run();
                            stepperControl_->moveXAxisTo(mmX);
                            while (stepperControl_->isAnyRunning()) stepperControl_->run();
                        }
                        processed_target_count_++;
                        sendLine("TARGET_DONE,SEQ=" + String(seq));
                    } else {
                        sendLine("ERROR: Invalid target format");
                    }
                } else {
                    sendLine("ERROR: Invalid TARGET format");
                }
            }
        } else if (msg.startsWith("BATCH_COMPLETE")) {
            if (!batch_in_progress_) {
                sendLine("ERROR: Batch not started");
            } else {
                // 归零三轴
                if (stepperControl_) {
                    stepperControl_->homeAllAxes();
                }
                sendLine("BATCH_FINISHED");
                resetBatchState();
            }
        } else {
            // 新增：转发到手动调试命令处理
            if (commandHandler_) {
                commandHandler_->processCommand(inputBuffer_);
            } else {
                sendLine("ERROR: Unknown command");
            }
        }
    }
    // 清空缓冲区和标志位
    memset(inputBuffer_, 0, sizeof(inputBuffer_));
    bufferIndex_ = 0;
    isStringComplete_ = false;
}

// ROI数据流处理方法实现

// 解析 ROI 字符串
bool SerialCommunication::parseROIString(const String& input, SerialROIData& roiData) {
    // 检查基本格式
    if (!input.startsWith("ROI")) {
        return false;
    }
    
    // 查找分隔符位置
    int firstComma = input.indexOf(',');
    int xPos = input.indexOf('X');
    int secondComma = input.indexOf(',', firstComma + 1);
    int yPos = input.indexOf('Y');
    
    if (firstComma == -1 || xPos == -1 || secondComma == -1 || yPos == -1) {
        return false;
    }
    
    // 提取各部分
    String roiStr = input.substring(3, firstComma); // 跳过"ROI"
    String xStr = input.substring(xPos + 1, secondComma); // 跳过"X"
    String yStr = input.substring(yPos + 1); // 跳过"Y"
    
    // 转换为数值
    roiData.roiIndex = roiStr.toInt();
    roiData.cx = xStr.toFloat();
    roiData.cy = yStr.toFloat();
    
    // 验证转换结果
    if (roiData.roiIndex <= 0) {
        return false;
    }
    
    return true;
}

// 立即处理单个ROI数据（流处理模式）
void SerialCommunication::processIndividualROI(const SerialROIData& roiData) {
    float targetX = roiData.cx;
    float targetY = roiData.cy;
    if (stepperControl_) {
        int stepX = (int)targetX;
        int stepY = (int)targetY;
        stepperControl_->moveXAxisTo(stepX);
        stepperControl_->moveYAxisTo(stepY);
    }
}

// 发送应答消息给上位机
void SerialCommunication::sendACK(int roiIndex, bool success) {
    if (success) {
        sendLine("ACK_ROI" + String(roiIndex));
    } else {
        sendLine("NACK_ROI" + String(roiIndex));
    }
}

// 清空批次缓冲区
void SerialCommunication::clearBatchBuffer() {
    for (int i = 0; i < BATCH_BUFFER_SIZE; i++) {
        batchBuffer_[i] = {0, 0.0, 0.0};
    }
    currentBatchCount_ = 0;
}

// 新增：重置批次计数
void SerialCommunication::resetROICount() {
    expectedROICount_ = 0;
    processedROICount_ = 0;
}
