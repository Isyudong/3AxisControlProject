#include "roi_processor.h"
#include "stepper_control.h"
#include <Arduino.h>

/*
目标功能：ROI视觉数据处理模块 - 专门处理上位机传来的ROI坐标数据
 * parseROIString(input, roiData) - 解析ROI字符串格式 "ROI1,X123.45,Y67.89"
 * addROIData(roiData) - 添加ROI数据到缓冲区，最多存储10个ROI点
 * processAllROI() - 处理所有已存储的ROI数据，执行坐标移动
 * clearROIData() - 清空ROI数据缓冲区
 * convertPixelToMM(pixelX, pixelY, mmX, mmY) - 像素坐标转换为实际毫米坐标
 * 
 * ROI数据格式:
 * 输入: "ROI1,X123.45,Y67.89" (ROI编号, X像素坐标, Y像素坐标)
 * 解析: roiIndex=1, cx=123.45, cy=67.89
 * 转换: 像素坐标 × 转换系数 = 实际毫米坐标
 * 
 * 坐标转换参数:
 * PIXEL_TO_MM_X = 0.1 (X轴像素到毫米转换系数)
 * PIXEL_TO_MM_Y = 0.1 (Y轴像素到毫米转换系数)
 * 
 * 执行流程:
 * 1. 移动Y轴到目标位置
 * 2. 移动X轴到目标位置
 * 3. 到达ROI位置 (可扩展触发继电器等操作)
 * 
 * 状态查询:
 * isFull() - 检查ROI缓冲区是否已满
 * isEmpty() - 检查ROI缓冲区是否为空
 * getROICount() - 获取当前存储的ROI数量
*/

// 构造函数
ROIProcessor::ROIProcessor(StepperControl* stepper) 
    : stepperControl_(stepper), roiCount_(0) {
    // 初始化ROI数据数组
    for (size_t i = 0; i < MAX_ROI_COUNT; i++) {
        roiDataArray_[i] = {0, 0.0f, 0.0f};
    }
}

// 解析ROI字符串
bool ROIProcessor::parseROIString(const char* input, ROIData& roiData) {
    if (!input) return false;
    
    int roiIndex = 0;
    float centerX = 0.0f, centerY = 0.0f;
    
    // 使用sscanf解析"ROI1,X123.45,Y67.89"格式
    if (sscanf(input, "ROI%d,X%f,Y%f", &roiIndex, &centerX, &centerY) == 3) {
        roiData.roiIndex = roiIndex;
        roiData.centerX = centerX;
        roiData.centerY = centerY;
        return true;
    }
    return false;
}

// 添加ROI数据
bool ROIProcessor::addROIData(const ROIData& roiData) {
    if (roiCount_ >= MAX_ROI_COUNT) {
        Serial.println(F("ROI buffer full!"));
        return false;
    }
    
    roiDataArray_[roiCount_] = roiData;
    roiCount_++;
    
    Serial.print(F("Added ROI "));
    Serial.print(roiData.roiIndex);
    Serial.print(F(": X="));
    Serial.print(roiData.centerX);
    Serial.print(F(", Y="));
    Serial.println(roiData.centerY);
    
    return true;
}

// 处理所有ROI数据
void ROIProcessor::processAllROI() {
    if (roiCount_ == 0) {
        Serial.println(F("No ROI data to process"));
        return;
    }
    
    Serial.print(F("Processing "));
    Serial.print(roiCount_);
    Serial.println(F(" ROI points"));
    
    for (size_t i = 0; i < roiCount_; i++) {
        executeROIMovement(roiDataArray_[i]);
    }
    
    Serial.println(F("All ROI processing complete"));
}

// 执行单个ROI的移动
void ROIProcessor::executeROIMovement(const ROIData& roiData) {
    if (!stepperControl_) {
        Serial.println(F("Error: StepperControl not initialized"));
        return;
    }
    
    // 转换像素坐标到实际坐标
    int actualX, actualY;
    convertPixelToMM(roiData.centerX, roiData.centerY, actualX, actualY);
    
    Serial.print(F("Moving to ROI "));
    Serial.print(roiData.roiIndex);
    Serial.print(F(": X="));
    Serial.print(actualX);
    Serial.print(F("mm, Y="));
    Serial.print(actualY);
    Serial.println(F("mm"));
    
    // 执行移动,同步进行！
    stepperControl_->moveYAxisTo(actualY);
    // waitForMovementComplete();
    stepperControl_->moveXAxisTo(actualX);
    waitForMovementComplete();
    
    // 在到达位置后可以触发继电器或其他操作
    Serial.print(F("Reached ROI "));
    Serial.println(roiData.roiIndex);
}

// 等待电机完成移动
void ROIProcessor::waitForMovementComplete() {
    while (stepperControl_ && stepperControl_->isAnyRunning()) {
        stepperControl_->run();
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
        roiDataArray_[i] = {0, 0.0f, 0.0f};
    }
    roiCount_ = 0;
    Serial.println(F("ROI data cleared"));
}

// 状态查询方法
bool ROIProcessor::isFull() const {
    return roiCount_ >= MAX_ROI_COUNT;
}

size_t ROIProcessor::getROICount() const {
    return roiCount_;
}

bool ROIProcessor::isEmpty() const {
    return roiCount_ == 0;
}
