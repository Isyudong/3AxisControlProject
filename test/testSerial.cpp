#include <Arduino.h>

void setup() {
  Serial.begin(115200); // 初始化串口通信
}

void loop() {
  static String inputString = ""; // 用于存储接收到的字符串
  static bool stringComplete = false; // 标记是否接收到完整的一行数据

  // 检查是否有数据可读
  while (Serial.available()) {
    char inChar = (char)Serial.read(); // 读取一个字符
    inputString += inChar; // 将字符添加到字符串中

    // 检查是否接收到换行符，表示一行数据结束
    if (inChar == '\n') {
      stringComplete = true;
      break;
    }
  }

  // 如果接收到完整的一行数据，开始解析
  if (stringComplete) {
    int roiIndex = 0;
    float cx = 0, cy = 0, r = 0;

    // 使用 sscanf 解析字符串
    if (sscanf(inputString.c_str(), "ROI%d,X%f,Y%f,R%f", &roiIndex, &cx, &cy, &r) == 4) {
      // 打印解析结果
      Serial.print("ROI Index: ");
      Serial.println(roiIndex);
      Serial.print("X: ");
      Serial.println(cx);
      Serial.print("Y: ");
      Serial.println(cy);
      Serial.print("R: ");
      Serial.println(r);
    } else {
      Serial.println("解析失败，数据格式错误！");
    }

    // 清空字符串和标志位
    inputString = "";
    stringComplete = false;
  }
}