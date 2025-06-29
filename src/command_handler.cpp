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
        Serial.println("HandMode");
    }
    else if (cmd == "AutoMode") {
        setHandMode(false);
        Serial.println("AutoMode");
    }
    else if (handMode) {
        processManualCommand(cmd);
        stepperControl->waitForCompletion();
    }
    else if (cmd.startsWith("Y") && cmd.indexOf('X') != -1) {
        processCoordinateData(cmd);
    }
}

// 处理手动模式命令
void CommandHandler::processManualCommand(const String& cmd) {
    char command = getCommandChar(cmd);
    int data = getCommandData(cmd);
    
    switch (command) {
        case 'O': // 获取当前位置
            stepperControl->printPositions();
            break;
            
        case 'V': // moveTo
            printCommandResponse(command, data, "moveTo");
            stepperControl->moveTo(data);
            break;
            
        case 'M': // move
            printCommandResponse(command, data, "move");
            stepperControl->move(data);
            break;
            
        case 'R': // runToNewPosition
            printCommandResponse(command, data, "runToNewPosition");
            stepperControl->runToNewPosition(data);
            break;
            
        case 'S': // setCurrentPosition
            printCommandResponse(command, data, "setCurrentPosition");
            stepperControl->setCurrentPosition(data);
            break;
            
        case 'A': // setAcceleration
            printCommandResponse(command, data, "setAcceleration");
            stepperControl->setAcceleration(data);
            break;
            
        case 'X': // setMaxSpeed
            printCommandResponse(command, data, "setMaxSpeed");
            stepperControl->setMaxSpeed(data);
            break;
            
        case 'D': // 设置控制电机
            if (data >= 0 && data <= 3) {
                stepperControl->setCurrentStepper(data);
                if (data == 0) {
                    Serial.println(F("Running All Motors"));
                } else {
                    Serial.print(F("Running Motor "));
                    Serial.println(data);
                }
            } else {
                Serial.println(F("Motor Number Wrong."));
            }
            break;
            
        default:
            Serial.println(F("Unknown Command"));
            break;
    }
}

// 处理自动模式的坐标数据
void CommandHandler::processCoordinateData(const String& input) {
    int xCoord[numCoordinates];
    
    // 读取y坐标数据
    int yCoord = input.substring(1, 4).toInt();
    
    // 读取x坐标数据,并存入数组
    int xStartIndex = input.indexOf('X') + 1;
    int coordCount = 0;
    for (int j = xStartIndex; j < static_cast<int>(input.length()); j += 3) {
        if (coordCount < numCoordinates) {
            xCoord[coordCount] = input.substring(j, j + 3).toInt();
            coordCount++;
        } else {
            break;
        }
    }
    
    // 移动Y轴到指定位置
    stepperControl->moveYTo(yCoord);
    while (stepperControl->isAnyRunning()) {
        stepperControl->run();
    }
    
    // 逐个执行X轴坐标点
    for (int j = 0; j < coordCount; j++) {
        stepperControl->moveXTo(xCoord[j]);
        while (stepperControl->isAnyRunning()) {
            stepperControl->run();
        }
        activateRelay();
    }
}

// 继电器控制
// void CommandHandler::activateRelay() {
//     digitalWrite(relayPin, HIGH);
//     delay(500);
//     digitalWrite(relayPin, LOW);
//     delay(100);
// }

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
    
    if (stepperNum == 1) {
        Serial.print(F("Motor1 '"));
        Serial.print(action);
        Serial.print(F("' "));
        Serial.println(data);
    }
    else if (stepperNum == 2) {
        Serial.print(F("Motor2 '"));
        Serial.print(action);
        Serial.print(F("' "));
        Serial.println(data);
    }
    else if (stepperNum == 3) {
        Serial.print(F("Motor3 '"));
        Serial.print(action);
        Serial.print(F("' "));
        Serial.println(data);
    }
    else if (stepperNum == 0) {
        Serial.print(F("All Motors '"));
        Serial.print(action);
        Serial.print(F("' "));
        Serial.println(data);
    }
}
