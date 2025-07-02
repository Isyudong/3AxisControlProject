#include "serial_communication.h"
#include "command_handler.h"
#include "roi_processor.h"

/*
SerialCommunication - 串口通讯协议处理模块
----------------------------------------
- 自动模式：严格遵循上位机批量协议（BATCH_START、TARGET、BATCH_COMPLETE等），实时应答。
- 手动模式：转发所有命令到CommandHandler，支持调试命令。
- 负责串口数据收发、协议解析、错误处理。
- 支持自动/手动模式随时切换，开发生产两不误。
- 高内聚低耦合，便于维护和扩展。
*/

// 构造函数：设置默认值
SerialCommunication::SerialCommunication(CommandHandler* handler, ROIProcessor* processor, StepperControl* stepper) 
    : commandHandler_(handler), 
      roiProcessor_(processor),
      stepperControl_(stepper),
      serialPort_(&Serial),          // 设置默认串口指针
      baudRate_(115200),             // 设置默认波特率
      isInitialized_(false),
      bufferIndex_(0),
      isStringComplete_(false),
      batch_in_progress_(false),
      expected_target_count_(0),
      processed_target_count_(0)
{
    memset(inputBuffer_, 0, sizeof(inputBuffer_));
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

// 批次状态重置
void SerialCommunication::resetBatchState() {
    this->batch_in_progress_ = false;
    this->expected_target_count_ = 0;
    this->processed_target_count_ = 0;
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
                    this->resetBatchState();
                    this->expected_target_count_ = count;
                    this->batch_in_progress_ = true;
                    sendLine("BATCH_READY");
                } else {
                    sendLine("ERROR: Invalid target count");
                }
            } else {
                sendLine("ERROR: Invalid BATCH_START format");
            }
        } else if (msg.startsWith("TARGET")) {
            if (!this->batch_in_progress_ || this->expected_target_count_ == 0) {
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
                        this->processed_target_count_++;
                        sendLine("TARGET_DONE,SEQ=" + String(seq));
                    } else {
                        sendLine("ERROR: Invalid target format");
                    }
                } else {
                    sendLine("ERROR: Invalid TARGET format");
                }
            }
        } else if (msg.startsWith("BATCH_COMPLETE")) {
            if (!this->batch_in_progress_) {
                sendLine("ERROR: Batch not started");
            } else {
                // 归零三轴
                if (stepperControl_) {
                    stepperControl_->homeAllAxes();
                }
                sendLine("BATCH_FINISHED");
                this->resetBatchState();
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
