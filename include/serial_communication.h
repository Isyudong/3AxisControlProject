#ifndef SERIAL_COMMUNICATION_H
#define SERIAL_COMMUNICATION_H

#include <Arduino.h>

// 前向声明避免循环依赖
class CommandHandler;
class ROIProcessor;

// 串口通讯接口类 - 专注于串口通讯功能
class SerialCommunication {
private:
    CommandHandler* commandHandler;
    ROIProcessor* roiProcessor;
    HardwareSerial* serialPort;
    unsigned long baudRate;
    bool isInitialized;
    
    // 串口数据接收缓冲区
    char inputBuffer[128];
    size_t bufferIndex;
    bool stringComplete;

public:
    // 构造函数
    SerialCommunication(CommandHandler* handler, ROIProcessor* processor);
    
    // 初始化串口
    void init(unsigned long baud = 115200);
    void init(HardwareSerial* port, unsigned long baud = 115200);
    
    // 检查串口是否有数据可读
    bool isDataAvailable();
    
    // 读取串口数据
    String readCommand();
    
    // 发送数据到串口
    void sendMessage(const String& message);
    void sendLine(const String& message);
    
    // 处理接收到的数据
    void processReceivedData();
    
    // 获取串口状态
    bool isReady() const;
    
    // 设置波特率
    void setBaudRate(unsigned long baud);
    
    // 数据验证接口
    virtual bool validateData(const char* data);
    
    // 错误处理接口
    virtual void handleError(const char* errorMsg);

private:
    // 数据缓冲区清理
    void clearBuffer();
    
    // 处理接收到的完整命令行
    void processCompleteLine();
};

#endif
