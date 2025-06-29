#include <Arduino.h>
#include "config.h"
#include "stepper_control.h"
#include "command_handler.h"
#include "serial_communication.h"
#include "roi_processor.h"

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
