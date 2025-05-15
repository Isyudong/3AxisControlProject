#include <Arduino.h>

// 定义一个结构体用于存储 ROI 数据
struct ROIData {
  int roiIndex;  // ROI 区域索引
  float cx;      // 中心 X 坐标
  float cy;      // 中心 Y 坐标
};

// 定义一个数组用于存储多个 ROI 数据
const int MAX_ROI_COUNT = 400; // 最大支持的 ROI 数量
ROIData roiDataArray[MAX_ROI_COUNT];
int roiCount = 0; // 当前已接收的 ROI 数量

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
    float cx = 0, cy = 0;

    // 使用 sscanf 解析字符串
    if (sscanf(inputString.c_str(), "ROI%d,X%f,Y%f", &roiIndex, &cx, &cy) == 3) {
      // 将解析结果存储到数组中
      if (roiCount < MAX_ROI_COUNT) {
        roiDataArray[roiCount].roiIndex = roiIndex;
        roiDataArray[roiCount].cx = cx;
        roiDataArray[roiCount].cy = cy;
        roiCount++;

        // 向上位机发送 "ACK" 确认
        Serial.println("ACK");
      } else {
        Serial.println("ROI 数据已满，无法存储更多！");
      }
    } else {
      Serial.println("解析失败，数据格式错误！");
    }

    // 清空字符串和标志位
    inputString = "";
    stringComplete = false;
  }

  // 检查是否接收到所有数据
  if (roiCount == MAX_ROI_COUNT) {
    // 所有数据接收完成后，执行像素坐标到实际移动距离的转换
    Serial.println("所有数据接收完成，开始转换坐标...");

    for (int i = 0; i < roiCount; i++) {
      // TODO: 添加像素坐标到实际移动距离的转换逻辑
      // 示例：假设每像素对应 0.1 mm
      float actualX = roiDataArray[i].cx * 0.1; // 转换为实际 X 距离
      float actualY = roiDataArray[i].cy * 0.1; // 转换为实际 Y 距离

      // 打印转换结果（可选）
      // Serial.print("ROI ");
      // Serial.print(roiDataArray[i].roiIndex);
      // Serial.print(": X=");
      // Serial.print(actualX);
      // Serial.print(" mm, Y=");
      // Serial.print(actualY);
      // Serial.println(" mm");
    }

    // 清空 ROI 数据以便接收下一组
    for (int i = 0; i < MAX_ROI_COUNT; i++) {
      roiDataArray[i] = {0, 0.0, 0.0}; // 清空临时变量
    }
    roiCount = 0;

    Serial.println("坐标转换完成，准备接收下一组数据！");
  }
}