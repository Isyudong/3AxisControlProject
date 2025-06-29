#ifndef SERIAL_COMMUNICATION_H
#define SERIAL_COMMUNICATION_H

#include <Arduino.h>

// 前向声明避免循环依赖
class CommandHandler;

// 定义一个结构体用于存储 ROI 数据
struct ROIData {
    int roiIndex;  // ROI 区域索引
    float cx;      // 中心 X 坐标
    float cy;      // 中心 Y 坐标
};

// 串口通讯接口类 - 为后续自定义串口通讯功能预留
class SerialCommunication {
private:
    CommandHandler* commandHandler;
    HardwareSerial* serialPort;
    unsigned long baudRate;
    bool isInitialized;
    
    // ROI 数据处理相关成员 - 内存优化
    static const int MAX_ROI_COUNT = 10; // 优化：从400减少到10，节省内存
    ROIData roiDataArray[MAX_ROI_COUNT];  // 现在只用120字节（12*10）
    size_t roiCount; // 改为size_t类型，保持一致性
    
    // 串口数据接收缓冲区 - 内存优化
    char inputBuffer[128]; // 优化：使用固定大小字符数组代替String
    size_t bufferIndex;    // 改为size_t类型，避免signed/unsigned警告
    bool stringComplete;

public:
    // 构造函数
    SerialCommunication(CommandHandler* handler);
    
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
    
    // 处理接收到的命令
    void processReceivedData();
    
    // 获取串口状态
    bool isReady() const;
    
    // 设置波特率
    void setBaudRate(unsigned long baud);
    
    // 数据验证接口（预留） - 内存优化
    virtual bool validateData(const char* data);  // 改为const char*
    
    // 错误处理接口（预留） - 内存优化  
    virtual void handleError(const char* errorMsg);  // 改为const char*
    
    // ROI 数据处理相关方法 - 内存优化
    void processROIData();
    bool parseROIString(const char* input, ROIData& roiData);  // 改为const char*
    void convertROICoordinates();
    void clearROIData();
    
private:
    // 数据缓冲区清理
    void clearBuffer();
};

#endif
