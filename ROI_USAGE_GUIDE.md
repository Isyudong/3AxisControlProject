# ROI系统使用指南

## 系统启动

### 默认模式
- **系统启动后自动进入AutoMode（自动模式）**
- **无需手动切换，立即可以接收ROI数据**
- **专为ROI数据流处理设计**

### 启动信息
```
========================================
  三轴步进电机控制系统 v2.0
  ROI Data Stream Processing Ready
========================================
System modules initialized:
  ✓ StepperControl
  ✓ CommandHandler
  ✓ ROIProcessor
  ✓ SerialCommunication
========================================
Mode: AutoMode (ROI Processing Active)
Ready to receive ROI data...
Format: ROI{index},X{coord},Y{coord}
Example: ROI1,X100.5,Y200.3
========================================
```

## 工作模式

### AutoMode（自动模式 - 默认）
- **系统启动后的默认模式**
- **用途**：处理ROI数据流，执行自动化任务
- **特点**：实时处理，无需等待批次完成

### HandMode（手动调试模式）
- **用途**：仅用于三轴系统调试和故障排除
- **激活命令**：`HandMode`
- **返回自动模式**：`AutoMode`

## ROI数据处理

### 数据格式
```
ROI1,X10.0,Y20.0
ROI2,X30.0,Y40.0
ROI3,X50.0,Y60.0
```

### 处理流程
1. 系统接收ROI数据
2. 立即解析和验证
3. 执行电机移动（G1X10.0Y20.0）
4. 发送确认应答

### 应答格式
- **成功**：`ACK_ROI1`
- **失败**：`NACK_ROI1`
- **调试信息**：
  ```
  处理ROI 1: X=10.00 mm, Y=20.00 mm
  已处理ROI: 1 总计: 1
  ```

## 使用示例

### 1. 系统启动（自动进入ROI处理模式）
```
系统自动启动到AutoMode
无需发送任何模式切换命令
```

### 2. 直接发送ROI数据
```
发送: ROI1,X100.5,Y200.3
接收: 处理ROI 1: X=100.50 mm, Y=200.30 mm
接收: 已处理ROI: 1 总计: 1
接收: ACK_ROI1
```

### 3. 批量发送ROI数据
```
发送: ROI1,X10.0,Y20.0
发送: ROI2,X30.0,Y40.0
发送: ROI3,X50.0,Y60.0

接收: 处理ROI 1: X=10.00 mm, Y=20.00 mm
接收: 已处理ROI: 1 总计: 1
接收: ACK_ROI1
接收: 处理ROI 2: X=30.00 mm, Y=40.00 mm
接收: 已处理ROI: 2 总计: 2
接收: ACK_ROI2
接收: 处理ROI 3: X=50.00 mm, Y=60.00 mm
接收: 已处理ROI: 3 总计: 3
接收: ACK_ROI3
```

### 4. 查询处理统计
```
发送: ROI_STATS
接收: Total ROI processed: 3
```

### 5. 调试模式（仅在需要时使用）
```
发送: HandMode
接收: HandMode Activated

发送: O
接收: === Current Positions ===
接收: [位置信息]

发送: V1000
接收: Command V with data 1000: moveTo

发送: AutoMode
接收: AutoMode Activated
[返回ROI处理模式]
```

## 错误处理

### 格式错误
```
发送: ROI1,X,Y20.0
接收: NACK_ROI0
接收: ERROR: Invalid ROI format
```

### 系统状态
- 自动模式下，非ROI命令会显示帮助信息
- 手动模式下，ROI数据会被忽略
- 未知命令会有明确的错误提示

## 性能特性

- **内存占用**：120字节（极低）
- **处理能力**：支持2000+个ROI
- **响应时间**：<1ms（解析和应答）
- **可靠性**：每个ROI都有独立的应答确认

## 注意事项

1. **应答等待**：建议上位机等待每个ROI的应答后再发送下一个
2. **坐标系统**：发送的坐标应该是实际的物理坐标（毫米）
3. **模式切换**：切换模式后系统会自动发送确认消息
4. **错误恢复**：单个ROI处理失败不会影响后续数据
5. **调试输出**：所有操作都有详细的串口调试信息
