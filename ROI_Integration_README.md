# ROI 数据处理功能说明

## 概述
已将 `testSerial.cpp` 中的 ROI 数据处理功能成功整合到 `src/serial_communication.cpp` 中。

## 功能特性

### 1. ROI 数据接收
- 支持接收格式为 `ROI{index},X{x_coord},Y{y_coord}` 的数据
- 例如：`ROI1,X123.45,Y678.90`
- 最大支持 400 个 ROI 数据点

### 2. 数据处理流程
1. 串口接收数据逐字符处理
2. 遇到换行符时完成一行数据的接收
3. 尝试解析为 ROI 数据格式
4. 如果是 ROI 数据，存储并发送 "ACK" 确认
5. 如果不是 ROI 数据，当作普通命令处理（直接发送给命令处理器）

### 3. 坐标转换
- 当接收到所有 400 个 ROI 数据后，自动执行坐标转换
- 目前使用示例转换：每像素 = 0.1mm
- 转换完成后自动清空数据，准备接收下一组

### 4. 错误处理
- 数据格式错误时发送错误信息
- ROI 数据已满时发送警告信息
- 保持与原有命令处理系统的兼容性
- 移除了自定义协议处理，简化了代码结构

## 使用方法

### 发送 ROI 数据
```
ROI1,X100.5,Y200.3
ROI2,X150.2,Y180.7
...
ROI400,X300.1,Y400.9
```

### 预期响应
- 每接收到一个有效的 ROI 数据，返回 "ACK"
- 接收完所有数据后，返回处理状态信息

## 集成说明

### 修改的文件
1. `include/serial_communication.h` - 添加了 ROI 数据结构和相关方法声明
2. `src/serial_communication.cpp` - 实现了 ROI 数据处理逻辑

### 新增的类成员
- `ROIData roiDataArray[MAX_ROI_COUNT]` - ROI 数据存储数组
- `int roiCount` - 当前接收的 ROI 数量
- `String inputBuffer` - 串口输入缓冲区
- `bool stringComplete` - 数据接收完成标志

### 新增的方法
- `processROIData()` - 主要的 ROI 数据处理方法
- `parseROIString()` - ROI 字符串解析方法
- `convertROICoordinates()` - 坐标转换方法
- `clearROIData()` - 清空 ROI 数据方法

## 注意事项
1. ROI 数据处理与普通命令处理是兼容的
2. 坐标转换算法需要根据实际硬件参数调整
3. 当前设置为接收 400 个数据点后自动处理，可根据需要调整
