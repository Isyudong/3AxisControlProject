#ifndef SERIAL_COMMUNICATION_H
#define SERIAL_COMMUNICATION_H

#include <Arduino.h>

// 前向声明避免循环依赖
class CommandHandler;
class ROIProcessor;
class StepperControl;

// 串口通信专用的ROI数据结构
struct SerialROIData {
    int roiIndex;  // ROI 区域索引
    float cx;      // 中心 X 坐标
    float cy;      // 中心 Y 坐标
};

// 串口通讯接口类 - 专注于串口通讯功能
class SerialCommunication {
private:
    // 依赖注入的组件指针 - 用于处理不同类型的命令
    CommandHandler* commandHandler_;    // 手动模式命令处理器指针，处理调试和手动控制命令
    ROIProcessor* roiProcessor_;        // ROI数据处理器指针，处理视觉系统传来的感兴趣区域数据
    StepperControl* stepperControl_;    // 步进电机控制器指针，用于直接控制电机移动
    
    // 硬件串口抽象接口
    HardwareSerial* serialPort_;        // 串口硬件接口指针，可指向Serial、Serial1等不同串口
    
    // 串口通信配置参数
    unsigned long baudRate_;            // 串口波特率设置，默认115200bps，影响数据传输速度
    
    // 串口状态管理标志
    bool isInitialized_;               // 串口初始化状态标志，防止未初始化时进行串口操作
    
    // ROI数据流处理相关成员
    static const int BATCH_BUFFER_SIZE = 10; // 每批最多缓存10个ROI数据
    SerialROIData batchBuffer_[BATCH_BUFFER_SIZE];
    int currentBatchCount_;            // 当前批次已接收的ROI数量
    int totalProcessedCount_;          // 总共已处理的ROI数量
    
    // 新增：批次ROI数量与计数
    int expectedROICount_ = 0;   // 本批次应处理ROI数量
    int processedROICount_ = 0;  // 本批次已处理ROI数量
    
    // 串口数据接收缓冲区
    char inputBuffer_[128];
    size_t bufferIndex_;
    bool isStringComplete_;

public:
    // 构造函数
    SerialCommunication(CommandHandler* handler, ROIProcessor* processor, StepperControl* stepper);
    
    // 初始化串口
    void init(unsigned long baudRate = 115200);
    void init(HardwareSerial* port, unsigned long baudRate = 115200);
    
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
    void setBaudRate(unsigned long baudRate);
    
    // 数据验证接口
    virtual bool validateData(const char* data);
    
    // 错误处理接口
    virtual void handleError(const char* errorMsg);
    
    // ROI数据流处理方法
    bool parseROIString(const String& input, SerialROIData& roiData);
    void processIndividualROI(const SerialROIData& roiData);
    void sendACK(int roiIndex, bool success);
    void clearBatchBuffer();
    
    // 新增：重置批次计数
    void resetROICount();
    
    // 批次状态重置
    void resetBatchState();

private:
    // 数据缓冲区清理
    void clearBuffer();
    
    // 处理接收到的完整命令行
    void processCompleteLine();
};

#endif
