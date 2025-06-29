#ifndef STEPPER_CONTROL_H
#define STEPPER_CONTROL_H

#include <AccelStepper.h>
#include <MultiStepper.h>
#include "config.h"

// 步进电机控制类
class StepperControl {
private:
    AccelStepper stepperX_;
    AccelStepper stepperY_;
    AccelStepper stepperZ_;
    int currentStepperNumber_;

public:
    // 构造函数
    StepperControl();
    
    // 初始化函数
    void init();
    
    // 设置当前控制的电机编号 (0:全部, 1:X轴, 2:Y轴, 3:Z轴)
    void setCurrentStepper(int stepperNumber);
    int getCurrentStepper() const;
    
    // 电机移动控制
    void moveTo(int data);
    void move(int data);
    void runToNewPosition(int data);
    void setCurrentPosition(int data);
    
    // 电机参数设置
    void setAcceleration(int accelerationValue);
    void setMaxSpeed(int maxSpeedValue);
    
    // 获取电机位置信息
    void printPositions();
    
    // 检查电机是否在运行
    bool isAnyRunning();
    
    // 运行电机（需要在循环中调用）
    void run();
    
    // 设置零点
    void setZeroPosition();
    
    // 单独控制各轴
    void moveXAxisTo(int position);
    void moveYAxisTo(int position);
    void moveZAxisTo(int position);
    
    // 等待电机到达目标位置
    void waitForCompletion();
};

#endif
