#include "stepper_control.h"
/*
目标功能：三轴步进电机底层控制模块
 * init() - 初始化三轴步进电机，设置引脚、速度和加速度参数
 * run() - 持续运行电机控制，必须在主循环中调用以保证电机正常运转
 * moveTo(data) - 移动当前选中电机到绝对位置，非阻塞式
 * move(data) - 移动当前选中电机相对距离，非阻塞式
 * runToNewPosition(data) - 移动到绝对位置，阻塞式，等待到达后才返回
 * setCurrentPosition(data) - 设置当前位置值，用于零点校准
 * setAcceleration(data) - 设置当前选中电机的加速度
 * setMaxSpeed(data) - 设置当前选中电机的最大速度
 * setCurrentStepper(num) - 选择操作的电机 (0=全部, 1=X轴, 2=Y轴, 3=Z轴)
 * moveXTo(pos), moveYTo(pos), moveZTo(pos) - 直接控制特定轴移动到位置
 * isAnyRunning() - 检查是否有任何电机正在运行
 * waitForCompletion() - 等待所有电机运动完成
 * printPositions() - 打印所有轴的当前位置信息
 * getCurrentStepper() - 获取当前选中的电机编号
 * 
 * 硬件配置 (config.h):
 * X轴: STEP_PIN=2, DIR_PIN=3
 * Y轴: STEP_PIN=4, DIR_PIN=5  
 * Z轴: STEP_PIN=6, DIR_PIN=7
 * 运动参数: OneMM=40步/毫米, MAX_SPEED=1000, ACCELERATION=500
*/

#include <Arduino.h>

// 构造函数
StepperControl::StepperControl() 
    : stepperX(1, xstepPin, xdirPin),
      stepperY(1, ystepPin, ydirPin),
      stepperZ(1, zstepPin, zdirPin),
      currentStepperNum(0)
{
}

// 初始化函数
void StepperControl::init() {
    // 设置引脚状态
    pinMode(xstepPin, OUTPUT);
    pinMode(xdirPin, OUTPUT);
    pinMode(ystepPin, OUTPUT);
    pinMode(ydirPin, OUTPUT);
    pinMode(zstepPin, OUTPUT);
    pinMode(zdirPin, OUTPUT);
    
    pinMode(enablePin, OUTPUT);
    digitalWrite(enablePin, LOW); // 使能电机驱动板
    
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);
    
    // 设置步进电机参数
    stepperX.setMaxSpeed(STEPPER_X_MAX_SPEED);
    stepperX.setAcceleration(STEPPER_X_ACCELERATION);
    
    stepperY.setMaxSpeed(STEPPER_Y_MAX_SPEED);
    stepperY.setAcceleration(STEPPER_Y_ACCELERATION);
    
    stepperZ.setMaxSpeed(STEPPER_Z_MAX_SPEED);
    stepperZ.setAcceleration(STEPPER_Z_ACCELERATION);
    
    // 设置零点
    setZeroPosition();
}

// 设置当前控制的电机编号
void StepperControl::setCurrentStepper(int stepperNum) {
    if (stepperNum >= 0 && stepperNum <= 3) {
        currentStepperNum = stepperNum;
    }
}

int StepperControl::getCurrentStepper() const {
    return currentStepperNum;
}

// 电机移动控制
void StepperControl::moveTo(int data) {
    switch (currentStepperNum) {
        case 1:
            stepperX.moveTo(data * OneMM);
            break;
        case 2:
            stepperY.moveTo(data * OneMM);
            break;
        case 3:
            stepperZ.moveTo(data * OneMM);
            break;
        case 0:
            stepperX.moveTo(data * OneMM);
            stepperY.moveTo(data * OneMM);
            stepperZ.moveTo(data * OneMM);
            break;
    }
}

void StepperControl::move(int data) {
    switch (currentStepperNum) {
        case 1:
            stepperX.move(data * OneMM);
            break;
        case 2:
            stepperY.move(data * OneMM);
            break;
        case 3:
            stepperZ.move(data * OneMM);
            break;
        case 0:
            stepperX.move(data * OneMM);
            stepperY.move(data * OneMM);
            stepperZ.move(data * OneMM);
            break;
    }
}

void StepperControl::runToNewPosition(int data) {
    switch (currentStepperNum) {
        case 1:
            stepperX.runToNewPosition(data * OneMM);
            break;
        case 2:
            stepperY.runToNewPosition(data * OneMM);
            break;
        case 3:
            stepperZ.runToNewPosition(data * OneMM);
            break;
        case 0:
            stepperX.runToNewPosition(data * OneMM);
            stepperY.runToNewPosition(data * OneMM);
            stepperZ.runToNewPosition(data * OneMM);
            break;
    }
}

void StepperControl::setCurrentPosition(int data) {
    switch (currentStepperNum) {
        case 1:
            stepperX.setCurrentPosition(data * OneMM);
            break;
        case 2:
            stepperY.setCurrentPosition(data * OneMM);
            break;
        case 3:
            stepperZ.setCurrentPosition(data * OneMM);
            break;
        case 0:
            stepperX.setCurrentPosition(data * OneMM);
            stepperY.setCurrentPosition(data * OneMM);
            stepperZ.setCurrentPosition(data * OneMM);
            break;
    }
}

// 电机参数设置
void StepperControl::setAcceleration(int data) {
    switch (currentStepperNum) {
        case 1:
            stepperX.setAcceleration(data);
            break;
        case 2:
            stepperY.setAcceleration(data);
            break;
        case 3:
            stepperZ.setAcceleration(data);
            break;
        case 0:
            stepperX.setAcceleration(data);
            stepperY.setAcceleration(data);
            stepperZ.setAcceleration(data);
            break;
    }
}

void StepperControl::setMaxSpeed(int data) {
    switch (currentStepperNum) {
        case 1:
            stepperX.setMaxSpeed(data);
            break;
        case 2:
            stepperY.setMaxSpeed(data);
            break;
        case 3:
            stepperZ.setMaxSpeed(data);
            break;
        case 0:
            stepperX.setMaxSpeed(data);
            stepperY.setMaxSpeed(data);
            stepperZ.setMaxSpeed(data);
            break;
    }
}

// 获取电机位置信息
void StepperControl::printPositions() {
    Serial.print(F("stepperX Position: "));
    Serial.println(stepperX.currentPosition() / OneMM);
    Serial.print(F("stepperY Position: "));
    Serial.println(stepperY.currentPosition() / OneMM);
    Serial.print(F("stepperZ Position: "));
    Serial.println(stepperZ.currentPosition() / OneMM);
    Serial.print(F("Current Running Motor: "));
    
    if (currentStepperNum == 1 || currentStepperNum == 2 || currentStepperNum == 3) {
        Serial.print(F("Motor# "));
        Serial.println(currentStepperNum);
    } else if (currentStepperNum == 0) {
        Serial.println(F("All Motors"));
    }
}

// 检查电机是否在运行
bool StepperControl::isAnyRunning() {
    return stepperX.isRunning() || stepperY.isRunning() || stepperZ.isRunning();
}

// 运行电机
void StepperControl::run() {
    stepperX.run();
    stepperY.run();
    stepperZ.run();
}

// 设置零点
void StepperControl::setZeroPosition() {
    stepperX.setCurrentPosition(0);
    stepperY.setCurrentPosition(0);
    stepperZ.setCurrentPosition(0);
}

// 单独控制各轴
void StepperControl::moveXTo(int position) {
    stepperX.moveTo(position * OneMM);
}

void StepperControl::moveYTo(int position) {
    stepperY.moveTo(position * OneMM);
}

void StepperControl::moveZTo(int position) {
    stepperZ.moveTo(position * OneMM);
}

// 等待电机到达目标位置
void StepperControl::waitForCompletion() {
    while (isAnyRunning()) {
        run();
    }
}
