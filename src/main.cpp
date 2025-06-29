#include <Arduino.h>
#include "config.h"
#include "stepper_control.h"
#include "command_handler.h"
#include "serial_communication.h"

// 全局对象
StepperControl stepperControl;
CommandHandler commandHandler(&stepperControl);
SerialCommunication serialComm(&commandHandler);

void setup() // 初始化函数
{
  // 初始化步进电机控制
  stepperControl.init();

  // 初始化串口通信
  serialComm.init(115200);
  Serial.println("系统已启动")
}

void loop() // 循环函数
{
  // 处理串口通讯数据
  serialComm.processReceivedData();
  
  // 运行电机（确保电机持续运行）
  stepperControl.run();
}
