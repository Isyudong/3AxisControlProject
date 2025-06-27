#ifndef CONFIG_H
#define CONFIG_H

// 宏定义
#define OneMM 40 // 电机一圈的脉冲数（恢复原始值）

// 引脚定义
const int enablePin = 8; // 使能控制引脚

const int xdirPin = 5;  // x方向引脚定义
const int xstepPin = 2; // x驱动引脚定义

const int ydirPin = 6;  // y方向引脚定义
const int ystepPin = 3; // y驱动引脚定义

const int zdirPin = 7;  // z方向引脚定义
const int zstepPin = 4; // z驱动引脚定义

const int relayPin = 9; // 继电器引脚定义

// 串口通讯常量定义
const int numCoordinates = 40; // 串口通讯数据长度

// 电机参数配置
const int STEPPER_X_MAX_SPEED = 2000;
const int STEPPER_X_ACCELERATION = 1000;

const int STEPPER_Y_MAX_SPEED = 2000;
const int STEPPER_Y_ACCELERATION = 1000;

const int STEPPER_Z_MAX_SPEED = 1000;
const int STEPPER_Z_ACCELERATION = 500;

#endif
