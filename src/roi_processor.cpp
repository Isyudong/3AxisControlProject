#include "roi_processor.h"
#include "stepper_control.h"
#include <Arduino.h>

// 构造函数
ROIProcessor::ROIProcessor(StepperControl* stepper) 
    : stepperControl(stepper), roiCount(0) {
    // 初始化ROI数据数组
    for (size_t i = 0; i < MAX_ROI_COUNT; i++) {
        roiDataArray[i] = {0, 0.0, 0.0};
    }
}

// 解析ROI字符串
bool ROIProcessor::parseROIString(const char* input, ROIData& roiData) {
    if (!input) return false;
    
    int roiIndex = 0;
    float cx = 0.0f, cy = 0.0f;
    
    // 使用sscanf解析"ROI1,X123.45,Y67.89"格式
    if (sscanf(input, "ROI%d,X%f,Y%f", &roiIndex, &cx, &cy) == 3) {
        roiData.roiIndex = roiIndex;
        roiData.cx = cx;
        roiData.cy = cy;
        return true;
    }
    return false;
}

// 添加ROI数据
bool ROIProcessor::addROIData(const ROIData& roiData) {
    if (roiCount >= MAX_ROI_COUNT) {
        Serial.println(F("ROI buffer full!"));
        return false;
    }
    
    roiDataArray[roiCount] = roiData;
    roiCount++;
    
    Serial.print(F("Added ROI "));
    Serial.print(roiData.roiIndex);
    Serial.print(F(": X="));
    Serial.print(roiData.cx);
    Serial.print(F(", Y="));
    Serial.println(roiData.cy);
    
    return true;
}

// 处理所有ROI数据
void ROIProcessor::processAllROI() {
    if (roiCount == 0) {
        Serial.println(F("No ROI data to process"));
        return;
    }
    
    Serial.print(F("Processing "));
    Serial.print(roiCount);
    Serial.println(F(" ROI points"));
    
    for (size_t i = 0; i < roiCount; i++) {
        executeROIMovement(roiDataArray[i]);
    }
    
    Serial.println(F("All ROI processing complete"));
}

// 执行单个ROI的移动
void ROIProcessor::executeROIMovement(const ROIData& roiData) {
    if (!stepperControl) {
        Serial.println(F("Error: StepperControl not initialized"));
        return;
    }
    
    // 转换像素坐标到实际坐标
    int actualX, actualY;
    convertPixelToMM(roiData.cx, roiData.cy, actualX, actualY);
    
    Serial.print(F("Moving to ROI "));
    Serial.print(roiData.roiIndex);
    Serial.print(F(": X="));
    Serial.print(actualX);
    Serial.print(F("mm, Y="));
    Serial.print(actualY);
    Serial.println(F("mm"));
    
    // 执行移动：先移动Y轴，再移动X轴
    stepperControl->moveYTo(actualY);
    waitForMovementComplete();
    
    stepperControl->moveXTo(actualX);
    waitForMovementComplete();
    
    // 在到达位置后可以触发继电器或其他操作
    Serial.print(F("Reached ROI "));
    Serial.println(roiData.roiIndex);
}

// 等待电机完成移动
void ROIProcessor::waitForMovementComplete() {
    while (stepperControl && stepperControl->isAnyRunning()) {
        stepperControl->run();
        // 可以添加小延时避免占用过多CPU
        delay(1);
    }
}

// 坐标转换：像素到毫米
void ROIProcessor::convertPixelToMM(float pixelX, float pixelY, int& mmX, int& mmY) const {
    mmX = static_cast<int>(pixelX * PIXEL_TO_MM_X);
    mmY = static_cast<int>(pixelY * PIXEL_TO_MM_Y);
}

// 清空ROI数据
void ROIProcessor::clearROIData() {
    for (size_t i = 0; i < MAX_ROI_COUNT; i++) {
        roiDataArray[i] = {0, 0.0, 0.0};
    }
    roiCount = 0;
    Serial.println(F("ROI data cleared"));
}

// 状态查询方法
bool ROIProcessor::isFull() const {
    return roiCount >= MAX_ROI_COUNT;
}

size_t ROIProcessor::getROICount() const {
    return roiCount;
}

bool ROIProcessor::isEmpty() const {
    return roiCount == 0;
}
