# LiSanSmall_Rotate STM32 → GD32 移植总结

## 项目信息
- **源项目**: LiSanSmall_Rotate (STM32F407VG)
- **目标**: GD32F407VE
- **参考项目**: LiSanBigRotate_V1.1_gd32, Screw_ClawV1.1

---

## 移植过程中遇到的问题

### 1. USART_FLAG_TBE 未定义
**错误**: `USART_DENT_ENABLE is undefined`

**原因**: GD32 的 USART DMA 发送使能宏名不同

**解决**: 将 `USART_DENT_ENABLE` 改为 `USART_TRANSMIT_DMA_ENABLE`

```c
// 错误
usart_dma_transmit_config(USART0, USART_DENT_ENABLE);

// 正确
usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE);
```

---

### 2. motor.c 与 bll_motor.c 符号重复定义
**错误**: `L6200E: Symbol Motor_Enable multiply defined (by bll_motor.o and motor.o)`

**原因**: `motor.c` 是旧版电机驱动，`bll_motor.c` 是新版，两者定义了相同的函数名

**解决**: 从 Keil 工程文件中移除 `motor.c`，只保留 `bll_motor.c`

---

### 3. USART1_SendWithDMA 未定义
**错误**: `L6218E: Undefined symbol USART1_SendWithDMA`

**原因**: STM32 USART1 → GD32 USART0，函数名从 `USART1_SendWithDMA` 改为 `USART0_SendWithDMA`

**解决**: 在 `usart.h` 中添加兼容宏
```c
#define USART1_SendWithDMA  USART0_SendWithDMA
```

---

### 4. Flash Download 失败 - Flash Device Description
**错误**: `Cannot Load Flash Device Description! STM32F4xx_1024.FLM`

**原因**: Keil 工程文件中的 Flash 算法配置错误

**解决**:
1. 修改 `FlashDriverDll` 中的算法文件名：`GD32F4xx_512.FLM` → `GD32F4xx_512KB.FLM`
2. 修改 `DriverSelection`：`4096` → `4099`
3. 在 Keil 的 Utilities → Settings 中手动添加正确的 Flash 算法

---

### 5. GPIO_Pin_x 未定义
**错误**: `GPIO_Pin_4 is undefined` 等

**原因**: GD32 使用 `GPIO_PIN_x`（大写），STM32 使用 `GPIO_Pin_x`（混合大小写）

**解决**: 在 `sys.h` 中添加兼容宏
```c
#define GPIO_Pin_0   GPIO_PIN_0
#define GPIO_Pin_1   GPIO_PIN_1
// ... 以此类推
```

---

### 6. CRC 字节顺序问题
**现象**: Modbus 通信无响应

**原因**: 项目使用非标准 CRC 字节顺序（高字节在前，低字节在后）

**解决**: 发送 Modbus 命令时，CRC 使用非标准格式
```
// 标准 Modbus（低位在前）: 06 03 00 00 00 01 85 BD
// 本项目（高位在前）:      06 03 00 00 00 01 BD 85
```

---

### 7. SWD 调试接口被破坏
**错误**: `ST-LINK USB communication error`

**原因**: 测试程序把所有 GPIO（包括 PA13/PA14 SWD 引脚）设置为输出

**解决**: 按住复位按钮 → 点击下载 → 看到进度条时松开复位按钮

---

### 8. EEPROM 初始化阻塞
**现象**: 程序启动后 Modbus 无响应

**原因**: EEPROM 不存在时，I2C 通信超时等待

**解决**: 确保 EEPROM 硬件连接正确，或适当调整 `EEPROM_TIMEOUT` 值

---

## USART 编号映射

| STM32 | GD32 | 功能 | 引脚 |
|-------|------|------|------|
| USART1 | USART0 | Modbus 从站 | PA9(TX), PA10(RX) |
| USART2 | USART1 | Modbus 主站 | PA2(TX), PA3(RX) |

## 定时器映射

| STM32 | GD32 | 功能 |
|-------|------|------|
| TIM5 | TIMER4 | 延时计数器 |
| TIM6 | TIMER5 | Modbus 从站超时 |
| TIM7 | TIMER6 | Modbus 主站超时 |
| TIM9 | TIMER9 | 调度器（50ms） |

## 中断处理函数映射

| STM32 | GD32 |
|-------|------|
| USART1_IRQHandler | USART0_IRQHandler |
| USART2_IRQHandler | USART1_IRQHandler |
| TIM6_DAC_IRQHandler | TIMER5_IRQHandler |
| TIM7_IRQHandler | TIMER6_IRQHandler |
| TIM1_BRK_TIM9_IRQHandler | TIMER0_UP_TIMER10_IRQHandler |
| DMA2_Stream7_IRQHandler | DMA1_Channel7_IRQHandler |

## 关键配置

- **HXTAL_VALUE**: 8000000 (8MHz)
- **系统时钟**: 168MHz
- **Modbus 从站地址**: 6
- **波特率**: 115200
- **Flash 算法**: GD32F4xx_512KB.FLM
- **DFP 包版本**: GigaDevice.GD32F4xx_DFP.3.0.3

---

## 测试命令

```
# 读取寄存器地址 0x0000
06 03 00 00 00 01 BD 85

# 读取 Version（地址 0x0002）
06 03 00 02 00 01 BD 85

# 读取 DrvType（地址 0x0003）
06 03 00 03 00 01 BD 85
```

---

*移植完成日期: 2026-06-17*
