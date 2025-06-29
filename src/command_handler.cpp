#include "command_handler.h"
#include "config.h"

// 构造函数
CommandHandler::CommandHandler(StepperControl* stepper) 
    : stepperControl(stepper), handMode(false) {
}

// 设置和获取模式
void CommandHandler::setHandMode(bool mode) {
    handMode = mode;
}

bool CommandHandler::isHandMode() const {
    return handMode;
}

// 处理用户命令
void CommandHandler::processCommand(const String& cmd) {
    if (cmd == "HandMode") {
        setHandMode(true);
        Serial.println("HandMode Activated");
    }
    else if (cmd == "AutoMode") {
        setHandMode(false);
        Serial.println("AutoMode Activated");
    }
    else if (handMode) {
        // 手动模式下处理调试命令
        processManualCommand(cmd);
        stepperControl->waitForCompletion();
    }
    else {
        // 自动模式下的其他命令处理
        Serial.println("AutoMode: Command received but not processed by CommandHandler");
        Serial.println("Note: ROI data should be handled by ROIProcessor");
    }
}

// 处理手动模式命令 - 用于三轴系统调试
void CommandHandler::processManualCommand(const String& cmd) {
    char command = getCommandChar(cmd);
    int data = getCommandData(cmd);
    
    switch (command) {
        case 'O': // 获取当前位置
            Serial.println(F("=== Current Positions ==="));
            stepperControl->printPositions();
            break;
            
        case 'V': // moveTo - 移动到绝对位置
            printCommandResponse(command, data, "moveTo");
            stepperControl->moveTo(data);
            break;
            
        case 'M': // move - 相对移动
            printCommandResponse(command, data, "move");
            stepperControl->move(data);
            break;
            
        case 'R': // runToNewPosition - 立即移动到位置
            printCommandResponse(command, data, "runToNewPosition");
            stepperControl->runToNewPosition(data);
            break;
            
        case 'S': // setCurrentPosition - 设置当前位置
            printCommandResponse(command, data, "setCurrentPosition");
            stepperControl->setCurrentPosition(data);
            break;
            
        case 'A': // setAcceleration - 设置加速度
            printCommandResponse(command, data, "setAcceleration");
            stepperControl->setAcceleration(data);
            break;
            
        case 'X': // setMaxSpeed - 设置最大速度
            printCommandResponse(command, data, "setMaxSpeed");
            stepperControl->setMaxSpeed(data);
            break;
            
        case 'D': // 设置控制电机
            if (data >= 0 && data <= 3) {
                stepperControl->setCurrentStepper(data);
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
            Serial.print(command);
            Serial.println(F("' - Type 'H' for help"));
            break;
    }
}

// 继电器控制 - 预留接口
void CommandHandler::activateRelay() {
    Serial.println(F("Relay activation - To be implemented"));
}

// 解析命令参数
char CommandHandler::getCommandChar(const String& cmd) {
    return cmd.charAt(0);
}

int CommandHandler::getCommandData(const String& cmd) {
    return cmd.substring(1).toInt();
}

// 打印命令响应
void CommandHandler::printCommandResponse(char cmd, int data, const String& action) {
    int stepperNum = stepperControl->getCurrentStepper();
    
    Serial.print(F("[DEBUG] "));
    if (stepperNum == 1) {
        Serial.print(F("Motor1"));
    }
    else if (stepperNum == 2) {
        Serial.print(F("Motor2"));
    }
    else if (stepperNum == 3) {
        Serial.print(F("Motor3"));
    }
    else if (stepperNum == 0) {
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
    int originalMotor = stepperControl->getCurrentStepper();
    
    // 测试X轴
    Serial.println(F("Testing X-axis..."));
    stepperControl->setCurrentStepper(1);
    stepperControl->move(100);
    stepperControl->waitForCompletion();
    stepperControl->move(-100);
    stepperControl->waitForCompletion();
    
    // 测试Y轴
    Serial.println(F("Testing Y-axis..."));
    stepperControl->setCurrentStepper(2);
    stepperControl->move(100);
    stepperControl->waitForCompletion();
    stepperControl->move(-100);
    stepperControl->waitForCompletion();
    
    // 测试Z轴
    Serial.println(F("Testing Z-axis..."));
    stepperControl->setCurrentStepper(3);
    stepperControl->move(100);
    stepperControl->waitForCompletion();
    stepperControl->move(-100);
    stepperControl->waitForCompletion();
    
    // 恢复原来的电机选择
    stepperControl->setCurrentStepper(originalMotor);
    Serial.println(F("=== All Axes Test Complete ==="));
}
