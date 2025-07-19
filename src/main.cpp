#include <Arduino.h>
#include "config.h"
#include "stepper_control.h"
#include "command_handler.h"
#include "serial_communication.h"
#include "roi_processor.h"

/*
main.cpp - 三轴步进电机控制系统主程序
-----------------------------------
- 负责系统初始化、模块调度、主循环。
- 支持两种工作模式：
  1. 自动批量处理（与上位机协议同步，推荐生产环境）
  2. 手动调试模式（开发/维护调试专用）
- 主循环持续处理串口数据和电机运行，保证实时性。
- 高内聚低耦合架构，便于维护和扩展。
*/

/*
目标功能：三轴步进电机控制系统主程序 - ROI数据流处理系统
 * setup() - 系统初始化函数
 *   - 初始化步进电机控制 (StepperControl)
 *   - 初始化串口通信 (115200波特率)
 *   - 默认启动自动模式 (AutoMode)
 *   - 显示系统启动信息和ROI数据格式
 *   - 各模块状态确认
 * 
 * loop() - 主循环函数
 *   - 处理串口接收数据 (ROI数据流和调试命令)
 *   - 持续运行电机控制 (保证电机正常运转)
 *   - 微秒级延时避免CPU过度占用
 * 
 * 系统架构:
 * StepperControl - 底层电机控制，三轴独立控制
 * CommandHandler - 调试模式支持，单轴和多轴操作
 * ROIProcessor - ROI批量处理器（兼容性保留）
 * SerialCommunication - 串口协议处理，ROI流处理和数据分发
 * 
 * 工作模式:
 * AutoMode (默认) - 自动模式，实时处理ROI数据流
 * HandMode (调试) - 手动调试模式，支持单步控制和参数调整
 * 
 * 依赖关系:
 * main -> SerialCommunication -> {CommandHandler, ROIProcessor} -> StepperControl
 * 
 * 系统特性:
 * - 默认启动: 自动进入ROI处理模式，无需手动切换
 * - 流处理: 接收即处理，支持2000+个ROI数据
 * - 低内存: 仅120字节缓冲区，避免内存溢出
 * - 实时性: 非阻塞式数据处理和电机控制
 * - 调试友好: 随时可切换到手动模式进行三轴调试
*/

// 全局对象 - 高内聚低耦合设计
StepperControl stepperControl;
CommandHandler commandHandler(&stepperControl);
ROIProcessor roiProcessor(&stepperControl);
SerialCommunication serialComm(&commandHandler, &roiProcessor, &stepperControl);

void setup() // 初始化函数
{
  // 初始化步进电机控制
  stepperControl.init();

  // 初始化串口通信
  serialComm.init(115200);
  
  // 默认设置为自动模式
  commandHandler.setHandMode(false);
  
  // 系统启动信息
  Serial.println(F("========================================"));
  Serial.println(F("  三轴步进电机控制系统 v2.0"));
  Serial.println(F("  ROI Data Stream Processing Ready"));
  Serial.println(F("========================================"));
  Serial.println(F("System modules initialized:"));
  Serial.println(F("  ✓ StepperControl"));
  Serial.println(F("  ✓ CommandHandler"));
  Serial.println(F("  ✓ ROIProcessor"));
  Serial.println(F("  ✓ SerialCommunication"));
  Serial.println(F("========================================"));
  Serial.println(F("Mode: AutoMode (ROI Processing Active)"));
  Serial.println(F("Ready to receive ROI data..."));
  Serial.println(F("========================================"));
  Serial.println(F("Debug Commands:"));
  Serial.println(F("  HandMode - Enter manual debug mode"));
  Serial.println(F("  AutoMode - Return to automatic mode"));
  Serial.println(F("  ROI_STATS - Show processing statistics"));
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
