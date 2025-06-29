#include <Arduino.h>
#include "config.h"
#include "stepper_control.h"
#include "command_handler.h"
#include "serial_communication.h"
#include "roi_processor.h"

/*
目标功能：三轴步进电机控制系统主程序 - 高内聚低耦合架构
 * setup() - 系统初始化函数
 *   - 初始化步进电机控制 (StepperControl)
 *   - 初始化串口通信 (115200波特率)
 *   - 显示系统启动信息和可用命令
 *   - 各模块状态确认
 * 
 * loop() - 主循环函数
 *   - 处理串口接收数据 (命令和ROI数据)
 *   - 持续运行电机控制 (保证电机正常运转)
 *   - 微秒级延时避免CPU过度占用
 * 
 * 系统架构:
 * StepperControl - 底层电机控制，三轴独立控制
 * CommandHandler - 手动调试模式，支持单轴和多轴操作
 * ROIProcessor - ROI视觉数据处理，像素坐标转换和批量执行
 * SerialCommunication - 串口协议处理，数据分发和错误处理
 * 
 * 工作模式:
 * HandMode - 手动调试模式，支持单步控制和参数调整
 * AutoMode - 自动模式，处理ROI数据和批量执行
 * 
 * 依赖关系:
 * main -> SerialCommunication -> {CommandHandler, ROIProcessor} -> StepperControl
 * 
 * 系统特性:
 * - 高内聚: 每个模块专注单一职责
 * - 低耦合: 通过依赖注入实现模块间通信  
 * - 实时性: 非阻塞式数据处理和电机控制
 * - 安全性: 缓冲区保护和错误处理机制
*/

// 全局对象 - 高内聚低耦合设计
StepperControl stepperControl;
CommandHandler commandHandler(&stepperControl);
ROIProcessor roiProcessor(&stepperControl);
SerialCommunication serialComm(&commandHandler, &roiProcessor);

void setup() // 初始化函数
{
  // 初始化步进电机控制
  stepperControl.init();

  // 初始化串口通信
  serialComm.init(115200);
  
  // 系统启动信息
  Serial.println(F("========================================"));
  Serial.println(F("  三轴步进电机控制系统 v2.0"));
  Serial.println(F("  High Cohesion & Low Coupling Design"));
  Serial.println(F("========================================"));
  Serial.println(F("System modules initialized:"));
  Serial.println(F("  ✓ StepperControl"));
  Serial.println(F("  ✓ CommandHandler (Manual Debug Mode)"));
  Serial.println(F("  ✓ ROIProcessor (Vision Data Processing)"));
  Serial.println(F("  ✓ SerialCommunication (Protocol Handler)"));
  Serial.println(F("========================================"));
  Serial.println(F("Commands:"));
  Serial.println(F("  HandMode - Enter manual debug mode"));
  Serial.println(F("  AutoMode - Enter automatic mode"));
  Serial.println(F("  Type 'H' in HandMode for help"));
  Serial.println(F("========================================"));
}

void loop() // 循环函数
{
  // 处理串口通讯数据（包括命令和ROI数据）
  serialComm.processReceivedData();
  
  // 运行电机（确保电机持续运行）
  stepperControl.run();
  
  // 小延时避免过度占用CPU
  delayMicroseconds(10);
}
