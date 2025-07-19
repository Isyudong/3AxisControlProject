#ifndef CONFIG_H
#define CONFIG_H

// 运动参数宏定义
#define ONE_MM_STEPS 40 // 每毫米对应的步数

// 使能控制引脚
const int ENABLE_PIN = 8;

// X轴引脚定义
const int X_DIR_PIN = 5;   // X轴方向控制引脚
const int X_STEP_PIN = 2;  // X轴步进控制引脚

// Y轴引脚定义
const int Y_DIR_PIN = 6;   // Y轴方向控制引脚
const int Y_STEP_PIN = 3;  // Y轴步进控制引脚

// Z轴引脚定义
// const int Z_DIR_PIN = 7;   // Z轴方向控制引脚
// const int Z_STEP_PIN = 4;  // Z轴步进控制引脚

// 继电器控制引脚
const int RELAY_PIN = 9;

// 串口通讯参数
const int MAX_COORDINATES = 40; // 最大坐标数量

// X轴电机参数
const int X_AXIS_MAX_SPEED = 3000;
const int X_AXIS_ACCELERATION = 3000;

// Y轴电机参数
const int Y_AXIS_MAX_SPEED = 3000;
const int Y_AXIS_ACCELERATION = 3000;

// Z轴电机参数
// const int Z_AXIS_MAX_SPEED = 1000;
// const int Z_AXIS_ACCELERATION = 500;

#endif
