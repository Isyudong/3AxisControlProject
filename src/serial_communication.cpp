#include "serial_communication.h"
#include "command_handler.h"

// 构造函数
SerialCommunication::SerialCommunication(CommandHandler* handler) 
    : commandHandler(handler), serialPort(&Serial), baudRate(115200), isInitialized(false),
      roiCount(0), inputBuffer(""), stringComplete(false) {
    // 初始化 ROI 数据数组
    for (int i = 0; i < MAX_ROI_COUNT; i++) {
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

// 数据验证接口（预留，可被子类重写）
bool SerialCommunication::validateData(const String& data) {
    // 默认验证：非空且长度合理
    return data.length() > 0 && data.length() < 256;
}

// 错误处理接口（预留，可被子类重写）
void SerialCommunication::handleError(const String& errorMsg) {
    // 默认错误处理：输出错误信息
    sendLine("ERROR: " + errorMsg);
}

// 数据缓冲区清理
void SerialCommunication::clearBuffer() {
    if (isInitialized) {
        while (serialPort->available() > 0) {
            serialPort->read();
        }
    }
}

// ROI 数据处理方法
void SerialCommunication::processROIData() {
    // 检查是否有数据可读
    while (serialPort->available()) {
        char inChar = (char)serialPort->read(); // 读取一个字符
        inputBuffer += inChar; // 将字符添加到字符串中

        // 检查是否接收到换行符，表示一行数据结束
        if (inChar == '\n') {
            stringComplete = true;
            break;
        }
    }

    // 如果接收到完整的一行数据，开始解析
    if (stringComplete) {
        inputBuffer.trim(); // 移除前后空白字符
        
        if (inputBuffer.length() > 0) {
            ROIData roiData;
            
            // 尝试解析 ROI 数据
            if (parseROIString(inputBuffer, roiData)) {
                // 这是 ROI 数据，进行处理
                if (roiCount < MAX_ROI_COUNT) {
                    roiDataArray[roiCount] = roiData;
                    roiCount++;

                    // 向上位机发送 "ACK" 确认
                    sendLine("ACK");
                } else {
                    sendLine("ROI 数据已满，无法存储更多！");
                }
            } else {
                // 不是 ROI 数据格式，当作普通命令处理
                if (validateData(inputBuffer)) {
                    // 标准命令处理
                    commandHandler->processCommand(inputBuffer);
                } else {
                    handleError("Invalid data format: " + inputBuffer);
                }
            }
        }

        // 清空字符串和标志位
        inputBuffer = "";
        stringComplete = false;
    }

    // 检查是否接收到所有数据
    if (roiCount == MAX_ROI_COUNT) {
        // 所有数据接收完成后，执行像素坐标到实际移动距离的转换
        sendLine("所有数据接收完成，开始转换坐标...");
        convertROICoordinates();
        sendLine("坐标转换完成，准备接收下一组数据！");
    }
}

// 解析 ROI 字符串
bool SerialCommunication::parseROIString(const String& input, ROIData& roiData) {
    int roiIndex = 0;
    float cx = 0, cy = 0;

    // 使用 sscanf 解析字符串
    if (sscanf(input.c_str(), "ROI%d,X%f,Y%f", &roiIndex, &cx, &cy) == 3) {
        roiData.roiIndex = roiIndex;
        roiData.cx = cx;
        roiData.cy = cy;
        return true;
    }
    return false;
}

// 转换 ROI 坐标
void SerialCommunication::convertROICoordinates() {
    for (int i = 0; i < roiCount; i++) {
        // TODO: 添加像素坐标到实际移动距离的转换逻辑
        // 示例：假设每像素对应 0.1 mm
        float actualX = roiDataArray[i].cx * 0.1; // 转换为实际 X 距离
        float actualY = roiDataArray[i].cy * 0.1; // 转换为实际 Y 距离

        // 打印转换结果（可选）
        // Serial.print("ROI ");
        // Serial.print(roiDataArray[i].roiIndex);
        // Serial.print(": X=");
        // Serial.print(actualX);
        // Serial.print(" mm, Y=");
        // Serial.print(actualY);
        // Serial.println(" mm");
    }
    
    // 转换完成后清空数据
    clearROIData();
}

// 清空 ROI 数据
void SerialCommunication::clearROIData() {
    for (int i = 0; i < MAX_ROI_COUNT; i++) {
        roiDataArray[i] = {0, 0.0, 0.0};
    }
    roiCount = 0;
}
