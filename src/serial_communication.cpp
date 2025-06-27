#include "serial_communication.h"
#include "command_handler.h"

// 构造函数
SerialCommunication::SerialCommunication(CommandHandler* handler) 
    : commandHandler(handler), serialPort(&Serial), baudRate(115200), isInitialized(false) {
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
    if (isDataAvailable()) {
        String input = readCommand();
        input.trim(); // 移除前后空白字符
        
        if (input.length() > 0) {
            // 数据验证
            if (validateData(input)) {
                // 尝试自定义协议处理
                processCustomProtocol(input);
                
                // 标准命令处理
                commandHandler->processCommand(input);
            } else {
                handleError("Invalid data format: " + input);
            }
        }
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

// 自定义协议处理接口（预留，可被子类重写）
void SerialCommunication::processCustomProtocol(const String& data) {
    // 默认实现为空，子类可以重写此方法来实现自定义协议
    // 例如：JSON协议、二进制协议等
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
