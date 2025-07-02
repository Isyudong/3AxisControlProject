#include "stepper_control.h"
/*
目标功能：三轴步进电机底层控制模块
 * init() - 初始化三轴步进电机，设置引脚、速度和加速度参数
 * run() - 持续运行电机控制，必须在主循环中调用以保证电机正常运转
 * moveTo(data) - 移动当前选中电机到绝对位置，非阻塞式
 * move(data) - 移动当前
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
    : stepperX_(1, X_STEP_PIN, X_DIR_PIN),
      stepperY_(1, Y_STEP_PIN, Y_DIR_PIN),
      stepperZ_(1, Z_STEP_PIN, Z_DIR_PIN),
      currentStepperNumber_(0)
{
}

// 初始化函数
void StepperControl::init() {
    // 设置引脚状态
    pinMode(X_STEP_PIN, OUTPUT);
    pinMode(X_DIR_PIN, OUTPUT);
    pinMode(Y_STEP_PIN, OUTPUT);
    pinMode(Y_DIR_PIN, OUTPUT);
    pinMode(Z_STEP_PIN, OUTPUT);
    pinMode(Z_DIR_PIN, OUTPUT);
    
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, LOW); // 使能电机驱动板
    
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    
    // 设置步进电机参数
    stepperX_.setMaxSpeed(X_AXIS_MAX_SPEED);
    stepperX_.setAcceleration(X_AXIS_ACCELERATION);
    
    stepperY_.setMaxSpeed(Y_AXIS_MAX_SPEED);
    stepperY_.setAcceleration(Y_AXIS_ACCELERATION);
    
    stepperZ_.setMaxSpeed(Z_AXIS_MAX_SPEED);
    stepperZ_.setAcceleration(Z_AXIS_ACCELERATION);
    
    // 设置零点
    setZeroPosition();
}

// 设置当前控制的电机编号
void StepperControl::setCurrentStepper(int stepperNumber) {
    if (stepperNumber >= 0 && stepperNumber <= 3) {
        currentStepperNumber_ = stepperNumber;
    }
}

int StepperControl::getCurrentStepper() const {
    return currentStepperNumber_;
}

// 电机移动控制
void StepperControl::moveTo(int data) {
    switch (currentStepperNumber_) {
        case 1:
            stepperX_.moveTo(data * ONE_MM_STEPS);
            break;
        case 2:
            stepperY_.moveTo(data * ONE_MM_STEPS);
            break;
        case 3:
            stepperZ_.moveTo(data * ONE_MM_STEPS);
            break;
        case 0:
            stepperX_.moveTo(data * ONE_MM_STEPS);
            stepperY_.moveTo(data * ONE_MM_STEPS);
            stepperZ_.moveTo(data * ONE_MM_STEPS);
            break;
    }
}

void StepperControl::move(int data) {
    switch (currentStepperNumber_) {
        case 1:
            stepperX_.move(data * ONE_MM_STEPS);
            break;
        case 2:
            stepperY_.move(data * ONE_MM_STEPS);
            break;
        case 3:
            stepperZ_.move(data * ONE_MM_STEPS);
            break;
        case 0:
            stepperX_.move(data * ONE_MM_STEPS);
            stepperY_.move(data * ONE_MM_STEPS);
            stepperZ_.move(data * ONE_MM_STEPS);
            break;
    }
}

void StepperControl::runToNewPosition(int data) {
    switch (currentStepperNumber_) {
        case 1:
            stepperX_.runToNewPosition(data * ONE_MM_STEPS);
            break;
        case 2:
            stepperY_.runToNewPosition(data * ONE_MM_STEPS);
            break;
        case 3:
            stepperZ_.runToNewPosition(data * ONE_MM_STEPS);
            break;
        case 0:
            stepperX_.runToNewPosition(data * ONE_MM_STEPS);
            stepperY_.runToNewPosition(data * ONE_MM_STEPS);
            stepperZ_.runToNewPosition(data * ONE_MM_STEPS);
            break;
    }
}

void StepperControl::setCurrentPosition(int data) {
    switch (currentStepperNumber_) {
        case 1:
            stepperX_.setCurrentPosition(data * ONE_MM_STEPS);
            break;
        case 2:
            stepperY_.setCurrentPosition(data * ONE_MM_STEPS);
            break;
        case 3:
            stepperZ_.setCurrentPosition(data * ONE_MM_STEPS);
            break;
        case 0:
            stepperX_.setCurrentPosition(data * ONE_MM_STEPS);
            stepperY_.setCurrentPosition(data * ONE_MM_STEPS);
            stepperZ_.setCurrentPosition(data * ONE_MM_STEPS);
            break;
    }
}

// 电机参数设置
void StepperControl::setAcceleration(int accelerationValue) {
    switch (currentStepperNumber_) {
        case 1:
            stepperX_.setAcceleration(accelerationValue);
            break;
        case 2:
            stepperY_.setAcceleration(accelerationValue);
            break;
        case 3:
            stepperZ_.setAcceleration(accelerationValue);
            break;
        case 0:
            stepperX_.setAcceleration(accelerationValue);
            stepperY_.setAcceleration(accelerationValue);
            stepperZ_.setAcceleration(accelerationValue);
            break;
    }
}

void StepperControl::setMaxSpeed(int maxSpeedValue) {
    switch (currentStepperNumber_) {
        case 1:
            stepperX_.setMaxSpeed(maxSpeedValue);
            break;
        case 2:
            stepperY_.setMaxSpeed(maxSpeedValue);
            break;
        case 3:
            stepperZ_.setMaxSpeed(maxSpeedValue);
            break;
        case 0:
            stepperX_.setMaxSpeed(maxSpeedValue);
            stepperY_.setMaxSpeed(maxSpeedValue);
            stepperZ_.setMaxSpeed(maxSpeedValue);
            break;
    }
}

// 获取电机位置信息
void StepperControl::printPositions() {
    Serial.print(F("stepperX Position: "));
    Serial.println(stepperX_.currentPosition() / ONE_MM_STEPS);
    Serial.print(F("stepperY Position: "));
    Serial.println(stepperY_.currentPosition() / ONE_MM_STEPS);
    Serial.print(F("stepperZ Position: "));
    Serial.println(stepperZ_.currentPosition() / ONE_MM_STEPS);
    Serial.print(F("Current Running Motor: "));
    
    if (currentStepperNumber_ == 1 || currentStepperNumber_ == 2 || currentStepperNumber_ == 3) {
        Serial.print(F("Motor# "));
        Serial.println(currentStepperNumber_);
    } else if (currentStepperNumber_ == 0) {
        Serial.println(F("All Motors"));
    }
}

// 检查电机是否在运行
bool StepperControl::isAnyRunning() {
    return stepperX_.isRunning() || stepperY_.isRunning() || stepperZ_.isRunning();
}

// 运行电机
void StepperControl::run() {
    stepperX_.run();
    stepperY_.run();
    stepperZ_.run();
}

// 设置零点
void StepperControl::setZeroPosition() {
    stepperX_.setCurrentPosition(0);
    stepperY_.setCurrentPosition(0);
    stepperZ_.setCurrentPosition(0);
}

// 单独控制各轴
void StepperControl::moveXAxisTo(int position) {
    stepperX_.moveTo(position * ONE_MM_STEPS);
}

void StepperControl::moveYAxisTo(int position) {
    stepperY_.moveTo(position * ONE_MM_STEPS);
}

void StepperControl::moveZAxisTo(int position) {
    stepperZ_.moveTo(position * ONE_MM_STEPS);
}

// 等待电机到达目标位置
void StepperControl::waitForCompletion() {
    while (isAnyRunning()) {
        run();
    }
}

// 归零所有轴：移动到0位置并等待完成
void StepperControl::homeAllAxes() {
    moveXAxisTo(0);
    moveYAxisTo(0);
    moveZAxisTo(0);
    waitForCompletion();
    setZeroPosition(); // 可选：归零后重置当前位置为0
}
