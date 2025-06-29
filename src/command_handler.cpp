#include "command_handler.h"
#include "config.h"
/*
目标功能：三轴联动，通讯控制。
 * 'O' 利用currentPosition获取当前电机输出轴位置并通过串口监视器输出该信息
 * 'V' 利用moveTo函数使电机运行到用户指定坐标位置，moveTo函数不会“block”程序。
 *     例：v1024 - 将电机moveTo到1024位置
 * 'M' 利用move函数使电机运行相应步数。move函数不会“block”程序。
 *     例：m1024 - 使电机运行1024步 
 * 'R' 利用runToNewPosition函数让电机运行到用户指定坐标位置。runToNewPosition函数功能
 *     与moveTo函数功能基本相同。唯一区别是runToNewPosition函数将“block”程序运行。
 *     即电机没有到达目标位置前，Arduino将不会继续执行后续程序内容。
 *     这一点很像Arduino官方Stepper库中的step函数。
 *     例：r1024 - 将电机runToNewPosition到1024位置。Arduino在电机达到1024位置前将停止
 *     其它程序任务的执行。
 * 'S' 利用setCurrentPosition函数设置当前位置为用户指定位置值。
 *     例：s0 - 设置当前位置为0
 * 'A' 利用setAcceleration函数设置加速度 
 *     例：a100 - 设置电机运行的加速度为100,对于目前使用电机，加速度请设置最低500，以减少震动。 
 * 'X' 利用setMaxSpeed函数设置最大速度 
 *     例：x500 - 设置电机运行的最大速度为500 
 * 'D' 用户通过此指令可指定哪一个电机进行工作  
 *     例：d1 一号电机工作，d2 二号电机工作,d3 一号电机工作 d0 电机同时工作
*/
// 构造函数
CommandHandler::CommandHandler(StepperControl* stepper) 
    : stepperControl_(stepper), isHandMode_(false) {
}

// 设置和获取模式
void CommandHandler::setHandMode(bool mode) {
    isHandMode_ = mode;
}

bool CommandHandler::isHandMode() const {
    return isHandMode_;
}

// 处理用户命令
void CommandHandler::processCommand(const String& command) {
    if (command == "HandMode") {
        setHandMode(true);
        Serial.println("HandMode Activated");
    }
    else if (command == "AutoMode") {
        setHandMode(false);
        Serial.println("AutoMode Activated");
    }
    else if (isHandMode_) {
        // 手动模式下处理调试命令
        processManualCommand(command);
        stepperControl_->waitForCompletion();
    }
    else {
        // 自动模式下的其他命令处理
        Serial.println("AutoMode: Command received but not processed by CommandHandler");
        Serial.println("Note: ROI data should be handled by ROIProcessor");
    }
}

// 处理手动模式命令 - 用于三轴系统调试
void CommandHandler::processManualCommand(const String& command) {
    char commandChar = getCommandChar(command);
    int data = getCommandData(command);
    
    switch (commandChar) {
        case 'O': // 获取当前位置
            Serial.println(F("=== Current Positions ==="));
            stepperControl_->printPositions();
            break;
            
        case 'V': // moveTo - 移动到绝对位置
            printCommandResponse(commandChar, data, "moveTo");
            stepperControl_->moveTo(data);
            break;
            
        case 'M': // move - 相对移动
            printCommandResponse(commandChar, data, "move");
            stepperControl_->move(data);
            break;
            
        case 'R': // runToNewPosition - 立即移动到位置
            printCommandResponse(commandChar, data, "runToNewPosition");
            stepperControl_->runToNewPosition(data);
            break;
            
        case 'S': // setCurrentPosition - 设置当前位置
            printCommandResponse(commandChar, data, "setCurrentPosition");
            stepperControl_->setCurrentPosition(data);
            break;
            
        case 'A': // setAcceleration - 设置加速度
            printCommandResponse(commandChar, data, "setAcceleration");
            stepperControl_->setAcceleration(data);
            break;
            
        case 'X': // setMaxSpeed - 设置最大速度
            printCommandResponse(commandChar, data, "setMaxSpeed");
            stepperControl_->setMaxSpeed(data);
            break;
            
        case 'D': // 设置控制电机
            if (data >= 0 && data <= 3) {
                stepperControl_->setCurrentStepper(data);
                if (data == 0) {
                    Serial.println(F("=== Running All Motors ==="));
                } else {
                    Serial.print(F("=== Running Motor "));
                    Serial.print(data);
                    Serial.println(F(" ==="));
                }
            } else {
                Serial.println(F("ERROR: Motor Number Wrong (0-3 allowed)"));
            }
            break;
            
        case 'H': // 帮助命令
            printHelpMessage();
            break;
            
        case 'T': // 测试所有轴
            testAllAxes();
            break;
            
        default:
            Serial.print(F("ERROR: Unknown Command '"));
            Serial.print(commandChar);
            Serial.println(F("' - Type 'H' for help"));
            break;
    }
}

// 继电器控制 - 预留接口
void CommandHandler::activateRelay() {
    Serial.println(F("Relay activation - To be implemented"));
}

// 解析命令参数
char CommandHandler::getCommandChar(const String& command) {
    return command.charAt(0);
}

int CommandHandler::getCommandData(const String& command) {
    return command.substring(1).toInt();
}

// 打印命令响应
void CommandHandler::printCommandResponse(char commandChar, int data, const String& action) {
    int stepperNumber = stepperControl_->getCurrentStepper();
    
    Serial.print(F("[DEBUG] "));
    if (stepperNumber == 1) {
        Serial.print(F("Motor1"));
    }
    else if (stepperNumber == 2) {
        Serial.print(F("Motor2"));
    }
    else if (stepperNumber == 3) {
        Serial.print(F("Motor3"));
    }
    else if (stepperNumber == 0) {
        Serial.print(F("All Motors"));
    }
    
    Serial.print(F(" executing '"));
    Serial.print(action);
    Serial.print(F("' with value: "));
    Serial.println(data);
}

// 打印帮助信息
void CommandHandler::printHelpMessage() {
    Serial.println(F("=== Manual Mode Commands ==="));
    Serial.println(F("O      - Get current positions"));
    Serial.println(F("V<num> - MoveTo absolute position"));
    Serial.println(F("M<num> - Move relative distance"));
    Serial.println(F("R<num> - RunToNewPosition immediately"));
    Serial.println(F("S<num> - SetCurrentPosition"));
    Serial.println(F("A<num> - SetAcceleration"));
    Serial.println(F("X<num> - SetMaxSpeed"));
    Serial.println(F("D<num> - Select motor (0=all, 1=X, 2=Y, 3=Z)"));
    Serial.println(F("T      - Test all axes"));
    Serial.println(F("H      - Show this help"));
    Serial.println(F("Example: D1 (select X motor), V100 (move to position 100)"));
}

// 测试所有轴
void CommandHandler::testAllAxes() {
    Serial.println(F("=== Testing All Axes ==="));
    
    // 保存当前电机选择
    int originalMotor = stepperControl_->getCurrentStepper();
    
    // 测试X轴
    Serial.println(F("Testing X-axis..."));
    stepperControl_->setCurrentStepper(1);
    stepperControl_->move(100);
    stepperControl_->waitForCompletion();
    stepperControl_->move(-100);
    stepperControl_->waitForCompletion();
    
    // 测试Y轴
    Serial.println(F("Testing Y-axis..."));
    stepperControl_->setCurrentStepper(2);
    stepperControl_->move(100);
    stepperControl_->waitForCompletion();
    stepperControl_->move(-100);
    stepperControl_->waitForCompletion();
    
    // 测试Z轴
    Serial.println(F("Testing Z-axis..."));
    stepperControl_->setCurrentStepper(3);
    stepperControl_->move(100);
    stepperControl_->waitForCompletion();
    stepperControl_->move(-100);
    stepperControl_->waitForCompletion();
    
    // 恢复原来的电机选择
    stepperControl_->setCurrentStepper(originalMotor);
    Serial.println(F("=== All Axes Test Complete ==="));
}
