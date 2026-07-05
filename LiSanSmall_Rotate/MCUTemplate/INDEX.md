# LiSanSmall_Rotate MCUTemplate 项目索引

## 项目概述
GD32F407 移植版本 - LiSanSmall_Rotate 项目

---

## 目录结构

### 📁 BLL/ - 业务逻辑层 (Business Logic Layer)
| 文件 | 说明 |
|------|------|
| [bll_access.c](BLL/bll_access.c) / [.h](BLL/bll_access.h) | 访问控制逻辑 |
| [bll_adapter.c](BLL/bll_adapter.c) / [.h](BLL/bll_adapter.h) | 适配器逻辑 |
| [bll_backzero.c](BLL/bll_backzero.c) / [.h](BLL/bll_backzero.h) | 回零逻辑 |
| [bll_claw.c](BLL/bll_claw.c) / [.h](BLL/bll_claw.h) | 爪子控制逻辑 |
| [bll_eeprom.c](BLL/bll_eeprom.c) / [.h](BLL/bll_eeprom.h) | EEPROM 存储逻辑 |
| [bll_main.c](BLL/bll_main.c) / [.h](BLL/bll_main.h) | 主业务逻辑 |
| [bll_motor.c](BLL/bll_motor.c) / [.h](BLL/bll_motor.h) | 电机控制逻辑 |
| [bll_tocase.c](BLL/bll_tocase.c) / [.h](BLL/bll_tocase.h) | 归位/装箱逻辑 |

---

### 📁 CORE/ - ARM Cortex-M4 核心文件
| 文件 | 说明 |
|------|------|
| [core_cm4.h](CORE/core_cm4.h) | Cortex-M4 核心头文件 |
| [core_cm4_simd.h](CORE/core_cm4_simd.h) | SIMD 指令支持 |
| [core_cmFunc.h](CORE/core_cmFunc.h) | 核心功能函数 |
| [core_cmInstr.h](CORE/core_cmInstr.h) | 核心指令支持 |
| [startup_gd32f407.s](CORE/startup_gd32f407.s) | 启动汇编文件 |

---

### 📁 FWLIB/ - GD32F4xx 固件库
#### inc/ - 头文件
| 文件 | 说明 |
|------|------|
| [gd32f4xx_adc.h](FWLIB/inc/gd32f4xx_adc.h) | ADC 模块 |
| [gd32f4xx_can.h](FWLIB/inc/gd32f4xx_can.h) | CAN 总线 |
| [gd32f4xx_crc.h](FWLIB/inc/gd32f4xx_crc.h) | CRC 校验 |
| [gd32f4xx_ctc.h](FWLIB/inc/gd32f4xx_ctc.h) | CTC 模块 |
| [gd32f4xx_dac.h](FWLIB/inc/gd32f4xx_dac.h) | DAC 模块 |
| [gd32f4xx_dbg.h](FWLIB/inc/gd32f4xx_dbg.h) | 调试支持 |
| [gd32f4xx_dci.h](FWLIB/inc/gd32f4xx_dci.h) | DCI 模块 |
| [gd32f4xx_dma.h](FWLIB/inc/gd32f4xx_dma.h) | DMA 控制 |
| [gd32f4xx_enet.h](FWLIB/inc/gd32f4xx_enet.h) | 以太网 |
| [gd32f4xx_exmc.h](FWLIB/inc/gd32f4xx_exmc.h) | 外部存储控制器 |
| [gd32f4xx_exti.h](FWLIB/inc/gd32f4xx_exti.h) | 外部中断 |
| [gd32f4xx_fmc.h](FWLIB/inc/gd32f4xx_fmc.h) | Flash 存储控制器 |
| [gd32f4xx_fwdgt.h](FWLIB/inc/gd32f4xx_fwdgt.h) | 独立看门狗 |
| [gd32f4xx_gpio.h](FWLIB/inc/gd32f4xx_gpio.h) | GPIO 控制 |
| [gd32f4xx_i2c.h](FWLIB/inc/gd32f4xx_i2c.h) | I2C 总线 |
| [gd32f4xx_ipa.h](FWLIB/inc/gd32f4xx_ipa.h) | IPA 模块 |
| [gd32f4xx_iref.h](FWLIB/inc/gd32f4xx_iref.h) | 内部参考 |
| [gd32f4xx_misc.h](FWLIB/inc/gd32f4xx_misc.h) | 杂项功能 |
| [gd32f4xx_pmu.h](FWLIB/inc/gd32f4xx_pmu.h) | 电源管理 |
| [gd32f4xx_rcu.h](FWLIB/inc/gd32f4xx_rcu.h) | 时钟控制 |
| [gd32f4xx_rtc.h](FWLIB/inc/gd32f4xx_rtc.h) | 实时时钟 |
| [gd32f4xx_sdio.h](FWLIB/inc/gd32f4xx_sdio.h) | SDIO 接口 |
| [gd32f4xx_spi.h](FWLIB/inc/gd32f4xx_spi.h) | SPI 总线 |
| [gd32f4xx_syscfg.h](FWLIB/inc/gd32f4xx_syscfg.h) | 系统配置 |
| [gd32f4xx_timer.h](FWLIB/inc/gd32f4xx_timer.h) | 定时器 |
| [gd32f4xx_tli.h](FWLIB/inc/gd32f4xx_tli.h) | TLI 模块 |
| [gd32f4xx_trng.h](FWLIB/inc/gd32f4xx_trng.h) | 真随机数生成 |
| [gd32f4xx_usart.h](FWLIB/inc/gd32f4xx_usart.h) | USART 串口 |
| [gd32f4xx_wwdgt.h](FWLIB/inc/gd32f4xx_wwdgt.h) | 窗口看门狗 |

#### src/ - 源文件
| 文件 | 说明 |
|------|------|
| [gd32f4xx_adc.c](FWLIB/src/gd32f4xx_adc.c) | ADC 实现 |
| [gd32f4xx_can.c](FWLIB/src/gd32f4xx_can.c) | CAN 实现 |
| [gd32f4xx_crc.c](FWLIB/src/gd32f4xx_crc.c) | CRC 实现 |
| [gd32f4xx_ctc.c](FWLIB/src/gd32f4xx_ctc.c) | CTC 实现 |
| [gd32f4xx_dac.c](FWLIB/src/gd32f4xx_dac.c) | DAC 实现 |
| [gd32f4xx_dbg.c](FWLIB/src/gd32f4xx_dbg.c) | 调试实现 |
| [gd32f4xx_dci.c](FWLIB/src/gd32f4xx_dci.c) | DCI 实现 |
| [gd32f4xx_dma.c](FWLIB/src/gd32f4xx_dma.c) | DMA 实现 |
| [gd32f4xx_enet.c](FWLIB/src/gd32f4xx_enet.c) | 以太网实现 |
| [gd32f4xx_exmc.c](FWLIB/src/gd32f4xx_exmc.c) | 外部存储实现 |
| [gd32f4xx_exti.c](FWLIB/src/gd32f4xx_exti.c) | 外部中断实现 |
| [gd32f4xx_fmc.c](FWLIB/src/gd32f4xx_fmc.c) | Flash 控制实现 |
| [gd32f4xx_fwdgt.c](FWLIB/src/gd32f4xx_fwdgt.c) | 独立看门狗实现 |
| [gd32f4xx_gpio.c](FWLIB/src/gd32f4xx_gpio.c) | GPIO 实现 |
| [gd32f4xx_i2c.c](FWLIB/src/gd32f4xx_i2c.c) | I2C 实现 |
| [gd32f4xx_ipa.c](FWLIB/src/gd32f4xx_ipa.c) | IPA 实现 |
| [gd32f4xx_iref.c](FWLIB/src/gd32f4xx_iref.c) | 内部参考实现 |
| [gd32f4xx_misc.c](FWLIB/src/gd32f4xx_misc.c) | 杂项功能实现 |
| [gd32f4xx_pmu.c](FWLIB/src/gd32f4xx_pmu.c) | 电源管理实现 |
| [gd32f4xx_rcu.c](FWLIB/src/gd32f4xx_rcu.c) | 时钟控制实现 |
| [gd32f4xx_rtc.c](FWLIB/src/gd32f4xx_rtc.c) | RTC 实现 |
| [gd32f4xx_sdio.c](FWLIB/src/gd32f4xx_sdio.c) | SDIO 实现 |
| [gd32f4xx_spi.c](FWLIB/src/gd32f4xx_spi.c) | SPI 实现 |
| [gd32f4xx_syscfg.c](FWLIB/src/gd32f4xx_syscfg.c) | 系统配置实现 |
| [gd32f4xx_timer.c](FWLIB/src/gd32f4xx_timer.c) | 定时器实现 |
| [gd32f4xx_tli.c](FWLIB/src/gd32f4xx_tli.c) | TLI 实现 |
| [gd32f4xx_trng.c](FWLIB/src/gd32f4xx_trng.c) | 随机数实现 |
| [gd32f4xx_usart.c](FWLIB/src/gd32f4xx_usart.c) | USART 实现 |
| [gd32f4xx_wwdgt.c](FWLIB/src/gd32f4xx_wwdgt.c) | 窗口看门狗实现 |

---

### 📁 HARDWARE/ - 硬件驱动层
| 文件 | 说明 |
|------|------|
| [EXTI/exti.c](HARDWARE/EXTI/exti.c) / [.h](HARDWARE/EXTI/exti.h) | 外部中断驱动 |
| [GPIO/gpio.c](HARDWARE/GPIO/gpio.c) / [.h](HARDWARE/GPIO/gpio.h) | GPIO 驱动 |
| [IWDG/iwdg.c](HARDWARE/IWDG/iwdg.c) / [.h](HARDWARE/IWDG/iwdg.h) | 独立看门狗驱动 |
| [ji2c.c](HARDWARE/ji2c.c) / [.h](HARDWARE/ji2c.h) | 软件 I2C 驱动 |
| [KEY/key.c](HARDWARE/KEY/key.c) / [.h](HARDWARE/KEY/key.h) | 按键驱动 |
| [LED/led.c](HARDWARE/LED/led.c) / [.h](HARDWARE/LED/led.h) | LED 驱动 |
| [MOTOR/motor.c](HARDWARE/MOTOR/motor.c) / [.h](HARDWARE/MOTOR/motor.h) | 电机驱动 |
| [TIMER/timer.c](HARDWARE/TIMER/timer.c) / [.h](HARDWARE/TIMER/timer.h) | 定时器驱动 |
| [USART/usart.c](HARDWARE/USART/usart.c) / [.h](HARDWARE/USART/usart.h) | 串口驱动 |

---

### 📁 MODBUS/ - Modbus 通信协议
| 文件 | 说明 |
|------|------|
| [modbus_master.c](MODBUS/modbus_master.c) / [.h](MODBUS/modbus_master.h) | Modbus 主站 |
| [modbus_slave.c](MODBUS/modbus_slave.c) / [.h](MODBUS/modbus_slave.h) | Modbus 从站 |
| [modbus_save_data.c](MODBUS/modbus_save_data.c) / [.h](MODBUS/modbus_save_data.h) | Modbus 数据存储 |

---

### 📁 SYSTEM/ - 系统基础模块
| 文件 | 说明 |
|------|------|
| [delay/delay.c](SYSTEM/delay/delay.c) / [.h](SYSTEM/delay/delay.h) | 延时函数 |
| [sys/sys.c](SYSTEM/sys/sys.c) / [.h](SYSTEM/sys/sys.h) | 系统配置 |

---

### 📁 USER/ - 用户主程序
| 文件 | 说明 |
|------|------|
| [main.c](USER/main.c) | 主程序入口 |
| [common.h](USER/common.h) | 公共定义 |
| [config.h](USER/config.h) | 系统配置 |
| [gd32f4xx.h](USER/gd32f4xx.h) | GD32F4xx 主头文件 |
| [gd32f4xx_it.c](USER/gd32f4xx_it.c) / [.h](USER/gd32f4xx_it.h) | 中断处理 |
| [gd32f4xx_libopt.h](USER/gd32f4xx_libopt.h) | 库选项配置 |
| [jexception.h](USER/jexception.h) | 异常处理 |
| [system_gd32f4xx.c](USER/system_gd32f4xx.c) / [.h](USER/system_gd32f4xx.h) | 系统初始化 |

---

### 📁 其他目录
| 文件 | 说明 |
|------|------|
| [OBJ/](OBJ/) | 编译输出目录 |
| [README/](README/) | 说明文档 |
| [VSUSER/RTE/RTE_Components.h](VSUSER/RTE/RTE_Components.h) | VS 开发环境配置 |

---

## 快速参考

### 关键文件入口
- **主程序**: [USER/main.c](USER/main.c)
- **业务逻辑入口**: [BLL/bll_main.c](BLL/bll_main.c)
- **系统配置**: [USER/config.h](USER/config.h)
- **中断处理**: [USER/gd32f4xx_it.c](USER/gd32f4xx_it.c)

### 外设驱动
- **电机**: [HARDWARE/MOTOR/motor.c](HARDWARE/MOTOR/motor.c)
- **串口**: [HARDWARE/USART/usart.c](HARDWARE/USART/usart.c)
- **GPIO**: [HARDWARE/GPIO/gpio.c](HARDWARE/GPIO/gpio.c)
- **定时器**: [HARDWARE/TIMER/timer.c](HARDWARE/TIMER/timer.c)
- **按键**: [HARDWARE/KEY/key.c](HARDWARE/KEY/key.c)
- **LED**: [HARDWARE/LED/led.c](HARDWARE/LED/led.c)

### 通信协议
- **Modbus 主站**: [MODBUS/modbus_master.c](MODBUS/modbus_master.c)
- **Modbus 从站**: [MODBUS/modbus_slave.c](MODBUS/modbus_slave.c)

---

## 文档
- [STM32_to_GD32_Porting_Notes.md](STM32_to_GD32_Porting_Notes.md) - STM32 到 GD32 移植说明
- [readme.txt](readme.txt) - 项目说明

---

*最后更新: 2026-06-18*