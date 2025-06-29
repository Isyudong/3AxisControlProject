#include "serial_communication.h"
#include "command_handler.h"

// 构造函数 - 内存优化版本
SerialCommunication::SerialCommunication(CommandHandler* handler) 
    : commandHandler(handler), serialPort(&Serial), baudRate(115200), isInitialized(false),
      roiCount(0), bufferIndex(0), stringComplete(false) {
    // 初始化字符缓冲区
    memset(inputBuffer, 0, sizeof(inputBuffer));
    
    // 初始化 ROI 数据数组
    for (size_t i = 0; i < MAX_ROI_COUNT; i++) {
        roiDataArray[i] = {0, 0.0, 0.0};
    }
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

// 处理接收到的命令
void SerialCommunication::processReceivedData() {
    // 处理 ROI 数据接收（这个方法会处理所有串口输入）
    processROIData();
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

// 数据验证接口 - 内存优化版本
bool SerialCommunication::validateData(const char* data) {
    // 默认验证：非空且长度合理
    return (data != nullptr && strlen(data) > 0 && strlen(data) < 128);
}

// 错误处理接口 - 内存优化版本
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

// ROI 数据处理方法 - 内存优化版本
void SerialCommunication::processROIData() {
    // 检查是否有数据可读
    while (serialPort->available()) {
        char inChar = (char)serialPort->read(); // 读取一个字符
        
        // 防止缓冲区溢出 - 修复类型警告
        if (bufferIndex < (sizeof(inputBuffer) - 1)) {
            inputBuffer[bufferIndex] = inChar;
            bufferIndex++;
        }

        // 检查是否接收到换行符，表示一行数据结束
        if (inChar == '\n') {
            inputBuffer[bufferIndex] = '\0'; // 添加字符串结束符
            stringComplete = true;
            break;
        }
    }

    // 如果接收到完整的一行数据，开始解析
    if (stringComplete) {
        // 移除换行符
        if (bufferIndex > 0 && inputBuffer[bufferIndex-1] == '\n') {
            inputBuffer[bufferIndex-1] = '\0';
            bufferIndex--;
        }
        
        if (bufferIndex > 0) {
            ROIData roiData;
            
            // 尝试解析 ROI 数据
            if (parseROIString(inputBuffer, roiData)) {
                // 这是 ROI 数据，进行处理
                if (roiCount < MAX_ROI_COUNT) {
                    roiDataArray[roiCount] = roiData;
                    roiCount++;

                    // 向上位机发送 "ACK" 确认
                    sendLine(F("ACK"));
                } else {
                    sendLine(F("ROI full!"));
                }
            } else {
                // 不是 ROI 数据格式，当作普通命令处理
                if (validateData(inputBuffer)) {
                    // 标准命令处理
                    commandHandler->processCommand(inputBuffer);
                } else {
                    handleError("Invalid data");
                }
            }
        }

        // 清空缓冲区和标志位
        memset(inputBuffer, 0, sizeof(inputBuffer));
        bufferIndex = 0;
        stringComplete = false;
    }

    // 检查是否接收到所有数据 - 内存优化
    if (roiCount == MAX_ROI_COUNT) {
        // 所有数据接收完成后，执行像素坐标到实际移动距离的转换
        sendLine(F("All ROI data received"));
        convertROICoordinates();
        sendLine(F("Coordinate conversion complete"));
        clearROIData(); // 处理完后清空数据
    }
}

// 解析 ROI 字符串 - 内存优化版本
bool SerialCommunication::parseROIString(const char* input, ROIData& roiData) {
    int roiIndex = 0;
    float cx = 0, cy = 0;

    // 使用 sscanf 解析字符串
    if (sscanf(input, "ROI%d,X%f,Y%f", &roiIndex, &cx, &cy) == 3) {
        roiData.roiIndex = roiIndex;
        roiData.cx = cx;
        roiData.cy = cy;
        return true;
    }
    return false;
}

// 转换 ROI 坐标
void SerialCommunication::convertROICoordinates() {
    for (size_t i = 0; i < roiCount; i++) {
        // 像素坐标到实际移动距离的转换
        const float PIXEL_TO_MM_X = 0.1;
        const float PIXEL_TO_MM_Y = 0.1;

        // 直接使用现有的电机控制方法 - 内存优化版本
        if (commandHandler) {
            // 计算实际坐标
            int actualX = (int)(roiDataArray[i].cx * PIXEL_TO_MM_X);
            int actualY = (int)(roiDataArray[i].cy * PIXEL_TO_MM_Y);
            
            // 使用字符数组构造命令，避免String对象
            char moveCommand[32];
            snprintf(moveCommand, sizeof(moveCommand), "X%dY%d", actualX, actualY);
            commandHandler->processCommand(moveCommand);
            
            // 调试信息 - 内存优化版本
            Serial.print(F("Moving to ROI "));
            Serial.print(roiDataArray[i].roiIndex);
            Serial.print(F(": X="));
            Serial.print(actualX);
            Serial.print(F("mm, Y="));
            Serial.print(actualY);
            Serial.println(F("mm"));
        }
    }
}

// 清空 ROI 数据
void SerialCommunication::clearROIData() {
    for (size_t i = 0; i < MAX_ROI_COUNT; i++) {
        roiDataArray[i] = {0, 0.0, 0.0};
    }
    roiCount = 0;
}
