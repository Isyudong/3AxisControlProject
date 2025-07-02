#include "roi_processor.h"
#include "stepper_control.h"
#include <Arduino.h>

/*
ROIProcessor - ROI视觉数据处理模块
-----------------------------------
- 主要职责：负责单点ROI坐标的解析、像素到毫米转换、运动执行。
- 典型流程：
  1. 解析ROI字符串（如 "ROI1,X123.45,Y67.89"）
  2. 坐标转换（像素→毫米）
  3. 运动顺序：先Y轴，后X轴，运动完成可扩展触发动作
- 适配高内聚低耦合架构，便于维护和扩展
*/

/*
目标功能：ROI视觉数据处理模块 - 专门处理上位机传来的ROI坐标数据
 * parseROIString(input, roiData) - 解析ROI字符串格式 "ROI1,X123.45,Y67.89"
 * executeROIMovement(roiData) - 执行单个ROI的运动
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
 * 2. 等待Y轴运动完成
 * 3. 移动X轴到目标位置
 * 4. 等待X轴运动完成
 * 5. 到达ROI位置 (可扩展触发继电器等操作)
 * 
 * 状态查询:
 * isFull() - 检查ROI缓冲区是否已满
 * isEmpty() - 检查ROI缓冲区是否为空
 * getROICount() - 获取当前存储的ROI数量
*/

// 构造函数
ROIProcessor::ROIProcessor(StepperControl* stepper) 
    : stepperControl_(stepper) {
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

// 单点ROI运动
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
    
    // 执行移动：先移动Y轴，再移动X轴
    stepperControl_->moveYAxisTo(actualY);
    waitForMovementComplete();
    
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

// // 状态查询方法
// bool ROIProcessor::isFull() const {
//     return roiCount_ >= MAX_ROI_COUNT;
// }

// size_t ROIProcessor::getROICount() const {
//     return roiCount_;
// }

// bool ROIProcessor::isEmpty() const {
//     return roiCount_ == 0;
// }
