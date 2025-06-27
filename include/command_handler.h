#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "stepper_control.h"

// 命令处理类
class CommandHandler {
private:
    StepperControl* stepperControl;
    bool handMode;

public:
    // 构造函数
    CommandHandler(StepperControl* stepper);
    
    // 设置和获取模式
    void setHandMode(bool mode);
    bool isHandMode() const;
    
    // 处理用户命令
    void processCommand(const String& cmd);
    
    // 处理手动模式命令
    void processManualCommand(const String& cmd);
    
    // 处理自动模式的坐标数据
    void processCoordinateData(const String& input);
    
    // 继电器控制
    void activateRelay();
    
private:
    // 解析命令参数
    char getCommandChar(const String& cmd);
    int getCommandData(const String& cmd);
    
    // 打印命令响应
    void printCommandResponse(char cmd, int data, const String& action);
};

#endif
