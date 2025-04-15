#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>

// 宏定义
#define OneMM 40 // 电机一圈的脉冲数

// 全局变量
int stepperNum; // 控制电机编号
bool HandMode = false; // 调试模式标志位

//串口通讯常量、变量定义
const int numCoordinates = 40; // 串口通讯数据长度
int xCoord[numCoordinates]; // 串口通讯数据
int yCoord = 0; // 串口通讯数据索引

//引脚定义
const int enablePin = 8; // 使能控制引脚

const int xdirPin = 5;  // x方向引脚定义
const int xstepPin = 2; // x驱动引脚定义

const int ydirPin = 6;  // y方向引脚定义
const int ystepPin = 3; // y驱动引脚定义

const int zdirPin = 7;  // z方向引脚定义
const int zstepPin = 4; // z驱动引脚定义

const int relayPin = 9; // 继电器引脚定义

// 定义三个步进电机对象
AccelStepper stepperX(1, xstepPin, xdirPin); // 使用TMC2209时，一步为0.225度，一圈为1600脉冲, 实际一圈OneMMmm，则OneMM个脉冲为1mm
AccelStepper stepperY(1, ystepPin, ydirPin);
AccelStepper stepperZ(1, zstepPin, zdirPin);

void runUsrCmd(String CMD);
void stepperRunOne();

void setup() // 初始化函数
{
  // 设置引脚状态
  pinMode(xstepPin, OUTPUT); // Arduino控制x步进引脚为输出模式
  pinMode(xdirPin, OUTPUT);  // Arduino控制x方向引脚为输出模式
  pinMode(ystepPin, OUTPUT); // Arduino控制y步进引脚为输出模式
  pinMode(ydirPin, OUTPUT);  // Arduino控制y方向引脚为输出模式
  //pinMode(zstepPin, OUTPUT); // Arduino控制z步进引脚为输出模式
  //pinMode(zdirPin, OUTPUT);  // Arduino控制z方向引脚为输出模式

  pinMode(enablePin, OUTPUT);   // Arduino控制使能引脚为输出模式
  digitalWrite(enablePin, LOW); // 将使能控制引脚设置为低电平从而让电机驱动板进入工作状态

  pinMode(relayPin, OUTPUT); // 继电器引脚设置为输出模式
  digitalWrite(relayPin, LOW); // 继电器引脚设置为低电平

  // 设置步进电机初始速度模式、速度参数
  stepperX.setMaxSpeed(2000);
  stepperX.setAcceleration(1000);

  stepperY.setMaxSpeed(2000);
  stepperY.setAcceleration(1000);

  stepperZ.setMaxSpeed(1000);
  stepperZ.setAcceleration(500);

  // 串口通信相关设置
  Serial.begin(115200);
  Serial.println("v");
  stepperRunOne(); // 开机设置当前位置为零点
}

void loop() // 循环函数
{
  if (Serial.available() > 0)
  {
    // 读取串口数据遇到换行符结束
    String input = Serial.readStringUntil('\n');

    // 解析指令
    if (input == "HandMode")
    {
      HandMode = true;
      Serial.println("HandMode");
    }
    else if (input == "AutoMode")
    {
      HandMode = false;
      Serial.println("AutoMode");
    }
    else if (HandMode)
    {
      runUsrCmd(input);
      // 持续移动步进电机到目标位置
      while(stepperX.isRunning() || stepperY.isRunning() || stepperZ.isRunning())
      {
        stepperX.run();
        stepperY.run();
        stepperZ.run();
      }
    }
    else if (input.startsWith("Y") && input.indexOf('X') != -1)
    {
      // 读取y坐标数据
      yCoord = input.substring(1, 4).toInt();

      // 读取x坐标数据,并存入数组(最多41个)
      int xStartIndex = input.indexOf('X') + 1;
      int i = 0;
      for (int j = xStartIndex; j < static_cast<int>(input.length()); j += 3)
      {
        if (i < numCoordinates)
        {
          xCoord[i] = input.substring(j, j + 3).toInt();
          i++;
        }
        else
        {
          break;
        }
      }
      
      stepperY.moveTo(yCoord * OneMM);
      while(stepperY.isRunning())
      {
        stepperY.run();
      }

      for (int j = 0; j < i; j++)
      {
        // 设置步进电机目标位置
        stepperX.moveTo(xCoord[j] * OneMM); // 传进来的数表示移动多少毫米。
        // 等待步进电机到达目标位置
        while (stepperX.isRunning())
        {
          stepperX.run();
        }
        digitalWrite(relayPin, HIGH); // 继电器引脚设置为高电平
        delay(500);                  // 等待0.5秒
        digitalWrite(relayPin, LOW);  // 继电器引脚设置为低电平
        delay(100);
      }
      // while(stepperX.currentPosition() != 0)
      // {
      //   stepperX.moveTo(0);
      //   stepperY.run();
      // }
    }

  }
}
// 电机初始化设置,通电之前将电机移动到指定位置，通电时会读取当时位置并设置为坐标原点。
void stepperRunOne()
{
  // Serial.print(F("stepperX Position: "));
  // Serial.println(stepperX.currentPosition());

  // Serial.print(F("stepperY Position: "));
  // Serial.println(stepperY.currentPosition());

  // Serial.print(F("stepperZ Position: "));
  // Serial.println(stepperZ.currentPosition());

  stepperX.setCurrentPosition(0);
  stepperY.setCurrentPosition(0);
  stepperZ.setCurrentPosition(0);
}
// 指令读取运行函数
/*
 * 'o' 利用currentPosition获取当前电机输出轴位置并通过串口监视器输出该信息
 * 'v' 利用moveTo函数使电机运行到用户指定坐标位置，moveTo函数不会“block”程序。
 *     例：v1024 - 将电机moveTo到1024位置
 * 'm' 利用move函数使电机运行相应步数。move函数不会“block”程序。
 *     例：m1024 - 使电机运行1024步
 * 'r' 利用runToNewPosition函数让电机运行到用户指定坐标位置。runToNewPosition函数功能
 *     与moveTo函数功能基本相同。唯一区别是runToNewPosition函数将“block”程序运行。
 *     即电机没有到达目标位置前，Arduino将不会继续执行后续程序内容。
 *     这一点很像Arduino官方Stepper库中的step函数。
 *     例：r1024 - 将电机runToNewPosition到1024位置。Arduino在电机达到1024位置前将停止
 *     其它程序任务的执行。
 * 's' 利用setCurrentPosition函数设置当前位置为用户指定位置值。
 *     例：s0 - 设置当前位置为0
 * 'a' 利用setAcceleration函数设置加速度
 *     例：a100 - 设置电机运行的加速度为100,对于目前使用电机，加速度请设置最低500，以减少震动。
 * 'x' 利用setMaxSpeed函数设置最大速度
 *     例：x500 - 设置电机运行的最大速度为500
 * 'd' 用户通过此指令可指定哪一个电机进行工作
 *     例：d1 一号电机工作，d2 二号电机工作,d3 一号电机工作 d0 电机同时工作
 */
void runUsrCmd(String CMD)
{
  char cmd; // 电机指令字符
  int data; // 电机指令参数
  // 解析指令和参数
  cmd = CMD.charAt(0);
  data = CMD.substring(1).toInt();
  switch (cmd)
  {
  case 'O': // 利用currentPosition获取当前电机输出轴位置并通过串口监视器输出该信息

    Serial.print(F("stepperX Position: "));
    Serial.println(stepperX.currentPosition() / OneMM);
    Serial.print(F("stepperY Position: "));
    Serial.println(stepperY.currentPosition() / OneMM);
    Serial.print(F("stepperZ Position: "));
    Serial.println(stepperZ.currentPosition() / OneMM);
    Serial.print(F("Current Running Motor: "));

    if (stepperNum == 1 || stepperNum == 2 || stepperNum == 3)
    {
      Serial.print(F("Motor# "));
      Serial.println(stepperNum);
    }
    else if (stepperNum == 0)
    {
      Serial.println(F("Both Motors"));
    }
    break;

  case 'V': // 利用moveTo函数使电机运行到用户指定坐标位置，moveTo函数不会“block”程序。

    if (stepperNum == 1)
    {
      Serial.print(F("Motor1 'moveTo' "));
      Serial.println(data);
      stepperX.moveTo(data * OneMM);
    }
    else if (stepperNum == 2)
    {
      Serial.print(F("Motor2 'moveTo' "));
      Serial.println(data);
      stepperY.moveTo(data * OneMM);
    }
    else if (stepperNum == 3)
    {
      Serial.print(F("Motor3 'moveTo' "));
      Serial.println(data);
      stepperZ.moveTo(data * OneMM);
    }
    else if (stepperNum == 0)
    {
      Serial.print(F("Both Motors 'moveTo' "));
      Serial.println(data);
      stepperX.moveTo(data * OneMM);
      stepperY.moveTo(data * OneMM);
      stepperZ.moveTo(data * OneMM);
    }
    break;

  case 'M': // 利用move函数使电机运行相应步数。move函数不会“block”程序。

    if (stepperNum == 1)
    {
      Serial.print(F("Motor1 'move'  "));
      Serial.println(data);
      stepperX.move(data * OneMM);
    }
    else if (stepperNum == 2)
    {
      Serial.print(F("Motor2 'move'  "));
      Serial.println(data);
      stepperY.move(data * OneMM);
    }
    else if (stepperNum == 3)
    {
      Serial.print(F("Motor3 'move'  "));
      Serial.println(data);
      stepperZ.move(data * OneMM);
    }
    else if (stepperNum == 0)
    {
      Serial.print(F("Both Motors 'move'  "));
      Serial.println(data);
      stepperX.move(data * OneMM);
      stepperY.move(data * OneMM);
      stepperZ.move(data * OneMM);
    }
    break;

  case 'R': // 利用runToNewPosition函数让电机运行到用户指定位置值。
    if (stepperNum == 1)
    {
      Serial.print(F("Motor1 'runToNewPosition' "));
      Serial.println(data);
      stepperX.runToNewPosition(data * OneMM);
    }
    else if (stepperNum == 2)
    {
      Serial.print(F("Motor2 'runToNewPosition' "));
      Serial.println(data);
      stepperY.runToNewPosition(data * OneMM);
    }
    else if (stepperNum == 3)
    {
      Serial.print(F("Motor3 'runToNewPosition' "));
      Serial.println(data);
      stepperZ.runToNewPosition(data * OneMM);
    }
    else if (stepperNum == 0)
    {
      Serial.print(F("Both Motors 'runToNewPosition' "));
      Serial.println(data);
      stepperX.runToNewPosition(data * OneMM);
      stepperY.runToNewPosition(data * OneMM);
      stepperZ.runToNewPosition(data * OneMM);
    }
    break;

  case 'S': // 利用setCurrentPosition函数设置当前位置为用户指定位置值。
    if (stepperNum == 1)
    {
      Serial.print(F("Set stepperX Current Position to "));
      Serial.println(data);
      stepperX.setCurrentPosition(data * OneMM);
    }
    else if (stepperNum == 2)
    {
      Serial.print(F("Set stepperY Current Position to "));
      Serial.println(data);
      stepperY.setCurrentPosition(data * OneMM);
    }
    else if (stepperNum == 3)
    {
      Serial.print(F("Set stepperZ Current Position to "));
      Serial.println(data);
      stepperZ.setCurrentPosition(data * OneMM);
    }
    else if (stepperNum == 0)
    {
      Serial.print(F("Set both steppers' Current Position to "));
      Serial.println(data);
      stepperX.setCurrentPosition(data * OneMM);
      stepperY.setCurrentPosition(data * OneMM);
      stepperZ.setCurrentPosition(data * OneMM);
    }
    break;

  case 'A': // 利用setAcceleration函数设置加速度
    if (stepperNum == 1)
    {
      Serial.print(F("Motor1 'setAcceleration' "));
      Serial.println(data);
      stepperX.setAcceleration(data);
    }
    else if (stepperNum == 2)
    {
      Serial.print(F("Motor2 'setAcceleration' "));
      Serial.println(data);
      stepperY.setAcceleration(data);
    }
    else if (stepperNum == 3)
    {
      Serial.print(F("Motor3 'setAcceleration' "));
      Serial.println(data);
      stepperZ.setAcceleration(data);
    }
    else if (stepperNum == 0)
    {
      Serial.print(F("Both Motors 'setAcceleration' "));
      Serial.println(data);
      stepperX.setAcceleration(data);
      stepperY.setAcceleration(data);
      stepperZ.setAcceleration(data);
    }
    break;

  case 'X': // 利用setMaxSpeed函数设置最大速度
    if (stepperNum == 1)
    {
      Serial.print(F("Motor1 'setMaxSpeed' "));
      Serial.println(data);
      stepperX.setMaxSpeed(data);
    }
    else if (stepperNum == 2)
    {
      Serial.print(F("Motor2 'setMaxSpeed' "));
      Serial.println(data);
      stepperY.setMaxSpeed(data);
    }
    else if (stepperNum == 3)
    {
      Serial.print(F("Motor3 'setMaxSpeed' "));
      Serial.println(data);
      stepperZ.setMaxSpeed(data);
    }
    else if (stepperNum == 0)
    {
      Serial.print(F("Both Motors 'setMaxSpeed' "));
      Serial.println(data);
      stepperX.setMaxSpeed(data);
      stepperY.setMaxSpeed(data);
      stepperZ.setMaxSpeed(data);
    }
    break;

  case 'D': // 用户通过此指令可指定哪一个电机进行工作
    if (data == 1 || data == 2 || data == 3)
    {
      stepperNum = data;
      Serial.print(F("Running Motor "));
      Serial.println(stepperNum);
    }
    else if (data == 0)
    {
      stepperNum = data;
      Serial.println(F("Running Both Motors "));
    }
    else
    {
      Serial.print(F("Motor Number Wrong."));
    }
    break;

  default: // 未知指令
    Serial.println(F("Unknown Command"));
  }
}
