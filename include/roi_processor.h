#ifndef ROI_PROCESSOR_H
#define ROI_PROCESSOR_H

#include <Arduino.h>

// 前向声明避免循环依赖
class StepperControl;

// ROI数据结构
struct ROIData {
    int roiIndex;     // ROI区域索引
    float centerX;    // 中心X坐标
    float centerY;    // 中心Y坐标
};

// ROI处理器类 - 专门处理上位机传来的ROI数据
class ROIProcessor {
private:
    StepperControl* stepperControl_;
    
    // ROI数据存储
    static const int MAX_ROI_COUNT = 10;
    ROIData roiDataArray_[MAX_ROI_COUNT];
    size_t roiCount_;
    
    // 坐标转换参数
    static constexpr float PIXEL_TO_MM_X = 0.1f;
    static constexpr float PIXEL_TO_MM_Y = 0.1f;

public:
    // 构造函数
    explicit ROIProcessor(StepperControl* stepper);
    
    // ROI数据处理
    bool parseROIString(const char* input, ROIData& roiData);
    bool addROIData(const ROIData& roiData);
    void processAllROI();
    void clearROIData();
    
    // 状态查询
    bool isFull() const;
    size_t getROICount() const;
    bool isEmpty() const;
    
    // 坐标转换
    void convertPixelToMM(float pixelX, float pixelY, int& mmX, int& mmY) const;
    
private:
    // 执行单个ROI的移动
    void executeROIMovement(const ROIData& roiData);
    
    // 等待电机完成移动
    void waitForMovementComplete();
};

#endif
