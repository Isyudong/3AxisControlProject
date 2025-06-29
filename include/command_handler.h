#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "stepper_control.h"

// 命令处理类 - 专注于手动模式调试
class CommandHandler {
private:
    StepperControl* stepperControl;
    bool handMode;

public:
    // 构造函数
    CommandHandler(StepperControl* stepper);
    
    // 模式设置和获取
    void setHandMode(bool mode);
    bool isHandMode() const;
    
    // 处理用户命令
    void processCommand(const String& cmd);
    
    // 处理手动模式命令 - 用于三轴系统调试
    void processManualCommand(const String& cmd);
    
    // 继电器控制 - 预留接口
    void activateRelay();
    
private:
    // 解析命令参数
    char getCommandChar(const String& cmd);
    int getCommandData(const String& cmd);
    
    // 打印命令响应
    void printCommandResponse(char cmd, int data, const String& action);
    
    // 帮助和测试功能
    void printHelpMessage();
    void testAllAxes();
};

#endif
