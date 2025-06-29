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
    
    // ROI 数据处理相关成员
    static const int MAX_ROI_COUNT = 400; // 最大支持的 ROI 数量
    ROIData roiDataArray[MAX_ROI_COUNT];
    int roiCount; // 当前已接收的 ROI 数量
    
    // 串口数据接收缓冲区
    String inputBuffer;
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
    
    // 数据验证接口（预留）
    virtual bool validateData(const String& data);
    
    // 错误处理接口（预留）
    virtual void handleError(const String& errorMsg);
    
    // ROI 数据处理相关方法
    void processROIData();
    bool parseROIString(const String& input, ROIData& roiData);
    void convertROICoordinates();
    void clearROIData();
    
private:
    // 数据缓冲区清理
    void clearBuffer();
};

#endif
