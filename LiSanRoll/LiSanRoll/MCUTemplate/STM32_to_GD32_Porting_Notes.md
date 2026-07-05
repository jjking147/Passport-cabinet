# LiSanRoll STM32 → GD32 移植总结

## 项目信息
- **源项目**: LiSanRoll (STM32F407VETx)
- **目标**: GD32F407VE
- **参考项目**: LiSanBigRotate_V1.1_gd32, LiSanSmall_Rotate
- **Modbus 从站地址**: 11 (0x0B)
- **波特率**: 115200

---

## 移植过程中遇到的问题

### 1. GPIO_Pin_x 未定义
**错误**: `GPIO_Pin_4 is undefined`、`GPIO_Pin_3 is undefined` 等

**原因**: GD32 使用 `GPIO_PIN_x`（全大写），STM32 使用 `GPIO_Pin_x`（混合大小写）

**解决**: 在 `sys.h` 中添加兼容宏
```c
/* STM32 GPIO_Pin_x -> GD32 GPIO_PIN_x */
#define GPIO_Pin_0   GPIO_PIN_0
#define GPIO_Pin_1   GPIO_PIN_1
#define GPIO_Pin_2   GPIO_PIN_2
#define GPIO_Pin_3   GPIO_PIN_3
#define GPIO_Pin_4   GPIO_PIN_4
#define GPIO_Pin_5   GPIO_PIN_5
#define GPIO_Pin_6   GPIO_PIN_6
#define GPIO_Pin_7   GPIO_PIN_7
#define GPIO_Pin_8   GPIO_PIN_8
#define GPIO_Pin_9   GPIO_PIN_9
#define GPIO_Pin_10  GPIO_PIN_10
#define GPIO_Pin_11  GPIO_PIN_11
#define GPIO_Pin_12  GPIO_PIN_12
#define GPIO_Pin_13  GPIO_PIN_13
#define GPIO_Pin_14  GPIO_PIN_14
#define GPIO_Pin_15  GPIO_PIN_15
#define GPIO_Pin_All GPIO_PIN_ALL
```

---

### 2. motor.c 与 bll_motor.c 符号重复定义
**错误**: `L6200E: Symbol Motor_Enable multiply defined (by bll_motor.o and motor.o)`

**原因**: `motor.c`（CiA 402 风格）和 `bll_motor.c`（另一种驱动）定义了相同的函数名：
- `Motor_Enable`
- `Stop`
- `Check_Status`
- `Check_Fault`

**解决**: 删除 `motor.c` 中重复的函数，只保留 `motor.c` 独有的函数（如 `Set_Speed_Mode`、`Set_Acce` 等）

---

### 3. swtich_flag 被定义为 static
**错误**: `L6218E: Undefined symbol swtich_flag`

**原因**: `bll_claw.c` 中 `swtich_flag` 被定义为 `static`，导致 `bll_access.c` 无法访问

**解决**: 移除 `static` 关键字
```c
// 修改前
static vu8 swtich_flag;

// 修改后
vu8 swtich_flag;
```

---

### 4. zero_flag 和 photoelectric_sign 未定义
**错误**: `L6218E: Undefined symbol zero_flag`、`L6218E: Undefined symbol photoelectric_sign`

**原因**: 这些变量在 `bll_access.c` 中被声明为 `extern`，但没有在任何地方定义

**解决**: 在 `bll_access.c` 中添加变量定义
```c
// 修改前
extern u8 zero_flag;
extern u8 photoelectric_sign;

// 修改后
u8 zero_flag = 0;
u8 photoelectric_sign = 0;
```

---

### 5. Set_Abso_Position 拼写错误
**错误**: `L6218E: Undefined symbol Set_Abso_Position`

**原因**: `bll_access.c` 中调用了 `Set_Abso_Position()`，但函数名应该是 `Set_Abso_Position_Mode()`

**解决**: 修正函数名
```c
// 修改前
Set_Abso_Position();

// 修改后
Set_Abso_Position_Mode();
```

---

## 关键配置

| 配置项 | 值 |
|--------|-----|
| HXTAL_VALUE | 8000000 (8MHz) |
| 系统时钟 | 168MHz |
| Modbus 从站地址 | 11 (0x0B) |
| 波特率 | 115200 |
| Flash 算法 | GD32F4xx_512KB.FLM |
| DriverSelection | 4099 |
| DFP 包版本 | GigaDevice.GD32F4xx_DFP.3.0.3 |

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

---

## 测试命令

```
# 读取寄存器地址 0x0000（从站地址 11）
0B 03 00 00 00 01 A0 84

# 读取 Version（地址 0x0002）
0B 03 00 02 00 01 A0 84

# 读取 DrvType（地址 0x0003）
0B 03 00 03 00 01 A0 84
```

**注意**: CRC 格式是非标准的（高字节在前，低字节在后）

---

*移植完成日期: 2026-06-17*
