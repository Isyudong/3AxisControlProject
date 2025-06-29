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

// 构造函数
SerialCommunication::SerialCommunication(CommandHandler* handler, ROIProcessor* processor) 
    : commandHandler(handler), 
      roiProcessor(processor),
      serialPort(&Serial), 
      baudRate(115200),
      isInitialized(false),
      bufferIndex(0),
      stringComplete(false)
{
    // 初始化字符缓冲区
    memset(inputBuffer, 0, sizeof(inputBuffer));
}

// 初始化串口
void SerialCommunication::init(unsigned long baud) {
    baudRate = baud;
    serialPort = &Serial;
    serialPort->begin(baudRate);
    isInitialized = true;
    sendLine("v"); // 发送初始化完成信号
}

void SerialCommunication::init(HardwareSerial* port, unsigned long baud) {
    baudRate = baud;
    serialPort = port;
    serialPort->begin(baudRate);
    isInitialized = true;
    sendLine("v"); // 发送初始化完成信号
}

// 检查串口是否有数据可读
bool SerialCommunication::isDataAvailable() {
    return isInitialized && serialPort->available() > 0;
}

// 读取串口数据
String SerialCommunication::readCommand() {
    if (!isDataAvailable()) {
        return "";
    }
    return serialPort->readStringUntil('\n');
}

// 发送数据到串口
void SerialCommunication::sendMessage(const String& message) {
    if (isInitialized) {
        serialPort->print(message);
    }
}

void SerialCommunication::sendLine(const String& message) {
    if (isInitialized) {
        serialPort->println(message);
    }
}

// 处理接收到的数据
void SerialCommunication::processReceivedData() {
    // 逐字符读取并缓冲
    while (serialPort->available()) {
        char inChar = (char)serialPort->read();
        
        // 防止缓冲区溢出
        if (bufferIndex < (sizeof(inputBuffer) - 1)) {
            inputBuffer[bufferIndex] = inChar;
            bufferIndex++;
        }

        // 检查是否接收到换行符，表示一行数据结束
        if (inChar == '\n') {
            inputBuffer[bufferIndex] = '\0';
            stringComplete = true;
            break;
        }
    }

    // 如果接收到完整的一行数据，开始处理
    if (stringComplete) {
        processCompleteLine();
    }
}

// 获取串口状态
bool SerialCommunication::isReady() const {
    return isInitialized;
}

// 设置波特率
void SerialCommunication::setBaudRate(unsigned long baud) {
    baudRate = baud;
    if (isInitialized) {
        serialPort->end();
        serialPort->begin(baudRate);
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
    if (isInitialized) {
        while (serialPort->available() > 0) {
            serialPort->read();
        }
    }
}

// 处理接收到的完整命令行
void SerialCommunication::processCompleteLine() {
    // 移除换行符
    if (bufferIndex > 0 && inputBuffer[bufferIndex-1] == '\n') {
        inputBuffer[bufferIndex-1] = '\0';
        bufferIndex--;
    }
    
    if (bufferIndex > 0) {
        // 检查是否为ROI数据
        if (strncmp(inputBuffer, "ROI", 3) == 0) {
            // 处理ROI数据
            if (roiProcessor) {
                ROIData roiData;
                if (roiProcessor->parseROIString(inputBuffer, roiData)) {
                    if (roiProcessor->addROIData(roiData)) {
                        sendLine(F("ACK"));
                    } else {
                        sendLine(F("ROI_FULL"));
                    }
                } else {
                    handleError("Invalid ROI format");
                }
            } else {
                handleError("ROI processor not available");
            }
        } else if (strcmp(inputBuffer, "PROCESS_ROI") == 0) {
            // 处理所有ROI数据的命令
            if (roiProcessor) {
                roiProcessor->processAllROI();
                sendLine(F("ROI_PROCESSED"));
            }
        } else if (strcmp(inputBuffer, "CLEAR_ROI") == 0) {
            // 清空ROI数据的命令
            if (roiProcessor) {
                roiProcessor->clearROIData();
                sendLine(F("ROI_CLEARED"));
            }
        } else {
            // 普通命令处理
            if (validateData(inputBuffer)) {
                if (commandHandler) {
                    commandHandler->processCommand(inputBuffer);
                }
            } else {
                handleError("Invalid command format");
            }
        }
    }

    // 清空缓冲区和标志位
    memset(inputBuffer, 0, sizeof(inputBuffer));
    bufferIndex = 0;
    stringComplete = false;
}
