#include "command_handler.h"
#include "config.h"
/*
CommandHandler - 手动调试命令处理模块
-----------------------------------
- 仅在手动调试模式下生效，支持三轴单步/多步运动、参数设置、帮助等。
- 支持HandMode/AutoMode切换，调试命令丰富。
- 与自动批量协议互不干扰，便于开发和维护。
- 高内聚低耦合，便于扩展。
*/

/*
三轴联动与串口通讯控制命令说明：
 * 'O'  查询当前三轴电机输出轴位置，通过串口输出
 * 'V'  moveTo：非阻塞移动到指定坐标（如 v1024）
 * 'M'  move：非阻塞相对移动指定步数（如 m1024）
 * 'R'  runToNewPosition：阻塞移动到指定坐标（如 r1024），到达前程序暂停
 * 'S'  setCurrentPosition：设置当前位置为指定值（如 s0）
 * 'A'  setAcceleration：设置加速度（如 a100，建议≥500减少震动）
 * 'X'  setMaxSpeed：设置最大速度（如 x500）
 * 'D'  选择控制电机（d1=X轴，d2=Y轴，d3=Z轴，d0=全部）
 * 'H'  显示帮助信息
 * 'T'  测试所有轴正反运动
 *
 * 自动模式下ROI批量处理由ROI流处理器负责，所有ROI处理完毕后三轴自动归零。
 * 手动模式仅用于调试，不影响自动归零逻辑。
*/

// 构造函数
CommandHandler::CommandHandler(StepperControl* stepper) 
    : stepperControl_(stepper),
      isHandMode_(false) {
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
        if (command == "H" || command == "HELP") {
            printHelpMessage();
        }
        else if (command == "TEST") {
            testAllAxes();
        }
        else if (command == "O") {
            // 允许在自动模式下查询位置
            Serial.println(F("=== Current Positions (Auto Mode) ==="));
            stepperControl_->printPositions();
        }
        else if (command.startsWith("ROI")) {
            // ROI命令提示 - 应该由SerialCommunication处理
            Serial.println(F("Note: ROI data is handled by ROI stream processor"));
        }
        else {
            // 未知命令
            Serial.print(F("AutoMode: Unknown command '"));
            Serial.print(command);
            Serial.println(F("' - Use 'H' for help"));
        }
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
