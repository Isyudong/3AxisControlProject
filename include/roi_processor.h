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

// ROI处理器类 - 仅处理单点ROI
class ROIProcessor {
private:
    StepperControl* stepperControl_;
    
    // 坐标转换参数
    static constexpr float PIXEL_TO_MM_X = 0.1f;
    static constexpr float PIXEL_TO_MM_Y = 0.1f;

public:
    // 构造函数
    explicit ROIProcessor(StepperControl* stepper);
    
    // ROI数据处理
    bool parseROIString(const char* input, ROIData& roiData);
    void executeROIMovement(const ROIData& roiData);
    
    // 坐标转换
    void convertPixelToMM(float pixelX, float pixelY, int& mmX, int& mmY) const;
    
private:
    // 等待电机完成移动
    void waitForMovementComplete();
};

#endif
