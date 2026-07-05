#include "ji2c.h"
#include "gd32f4xx.h"

static JI2C_Message_Type JI2C_Message = { .Status = JI2C_State_Idle };

JI2C_State_Type JI2C_GetState(void)
{
    return JI2C_Message.Status;
}

static void JI2C_IO_Init(void)
{
    // 开启时钟
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_I2C1);

    // 配置 PB6, PB7 为 I2C1 复用开漏输出
    gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
}

static void JI2C_IIC_Init(uint8_t ownaddr, uint32_t speed)
{
    i2c_deinit(I2C1);
    
    // 1. 使用新的统一配置函数
    // 参数说明：外设, 模式(I2C/SMBus), 地址位数(7/10位), 地址值
    i2c_mode_addr_config(I2C1, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, ownaddr);

	//i2c_clock_config(I2C1, speed, I2C_DTCY_2); 
    // 2. 以下配置保持不变
    i2c_ack_config(I2C1, I2C_ACK_ENABLE);       // 开启应答
    i2c_ackpos_config(I2C1, I2C_ACKPOS_CURRENT); // 应答位置
    i2c_enable(I2C1);                           // 使能 I2C

    // 3. 禁用时钟延展 (注意：直接操作寄存器以兼容旧逻辑)
    // 根据你上传的头文件，I2C_CTL0_SS 宏定义存在，可以直接使用
    I2C_CTL0(I2C1) |= I2C_CTL0_SS; 
}

static void JI2C_Interrupt_Init(void)
{
    nvic_irq_enable(I2C1_EV_IRQn, 0U, 0U);
    nvic_irq_enable(I2C1_ER_IRQn, 0U, 0U);
    
    // 使能错误中断、事件中断和缓冲中断
    i2c_interrupt_enable(I2C1, I2C_INT_ERR);
    i2c_interrupt_enable(I2C1, I2C_INT_BUF);
    i2c_interrupt_enable(I2C1, I2C_INT_EV);
}

void JI2C_Init(uint32_t speed)
{
    JI2C_IO_Init();
    JI2C_IIC_Init(JI2C_OWNADDR, speed);
    JI2C_Interrupt_Init();
}

// 异步读数据
void JI2C_ReadData_Async(uint8_t slav_addr, uint16_t bytes2read, uint8_t* buffer, JI2C_TRANSFINISH_CB callback)
{
    JI2C_Message.Status = JI2C_State_Start_R;
    JI2C_Message.SlaveAddr.Addr8 = slav_addr;
    JI2C_Message.Index = 0;
    JI2C_Message.Buffer = buffer;
    JI2C_Message.Len = bytes2read;
    JI2C_Message.TransType = JI2C_TType_Read;
    JI2C_Message.Callback = callback;

    i2c_start_on_bus(I2C1); // 触发起始条件
}

// 异步写寄存器(8位地址)
void JI2C_WriteReg8_Async(uint8_t slav_addr, uint8_t regaddr, uint16_t bytes2write, uint8_t* buffer, JI2C_TRANSFINISH_CB callback)
{
    JI2C_Message.Status = JI2C_State_Start;
    JI2C_Message.SlaveAddr.Addr8 = slav_addr;
    JI2C_Message.Index = 0;
    JI2C_Message.Buffer = buffer;
    JI2C_Message.Len = bytes2write;
    JI2C_Message.RegAddr.Hi8 = regaddr;
    JI2C_Message.TransType = JI2C_TType_Write;
    JI2C_Message.Callback = callback;

    i2c_start_on_bus(I2C1);
}

// 异步读寄存器(8位地址)
void JI2C_ReadReg8_Async(uint8_t slav_addr, uint8_t regaddr, uint16_t bytes2read, uint8_t* buffer, JI2C_TRANSFINISH_CB callback)
{
    JI2C_Message.Status = JI2C_State_Start;
    JI2C_Message.SlaveAddr.Addr8 = slav_addr;
    JI2C_Message.Index = 0;
    JI2C_Message.Buffer = buffer;
    JI2C_Message.Len = bytes2read;
    JI2C_Message.RegAddr.Hi8 = regaddr;
    JI2C_Message.TransType = JI2C_TType_Read;
    JI2C_Message.Callback = callback;

    i2c_start_on_bus(I2C1);
}

// I2C1 错误中断服务函数
void I2C1_ER_IRQHandler(void)
{
    if (i2c_flag_get(I2C1, I2C_FLAG_AERR)) {
        i2c_flag_clear(I2C1, I2C_FLAG_AERR);
    }
    if (i2c_flag_get(I2C1, I2C_FLAG_BERR)) {
        i2c_flag_clear(I2C1, I2C_FLAG_BERR);
    }
    if (i2c_flag_get(I2C1, I2C_FLAG_OUERR)) {
        i2c_flag_clear(I2C1, I2C_FLAG_OUERR);
    }
    if (i2c_flag_get(I2C1, I2C_FLAG_LOSTARB)) {
        i2c_flag_clear(I2C1, I2C_FLAG_LOSTARB);
    }
    
    JI2C_Message.Status = JI2C_State_Error;
    if (JI2C_Message.Callback != NULL) {
        (*JI2C_Message.Callback)(false, JI2C_Message.Buffer);
    }
    JI2C_Message.Status = JI2C_State_Idle;
}

// I2C1 事件中断服务函数
void I2C1_EV_IRQHandler(void)
{
    uint32_t stat0 = I2C_STAT0(I2C1);
    uint32_t stat1 = I2C_STAT1(I2C1);

    // 起始条件已发送 (SBSEND)
    if (stat0 & I2C_STAT0_SBSEND) {
        if (JI2C_Message.Status == JI2C_State_Start || JI2C_Message.Status == JI2C_State_Start_W) {
            i2c_master_addressing(I2C1, JI2C_Message.SlaveAddr.Addr8, I2C_TRANSMITTER);
            JI2C_Message.Status = JI2C_State_Address;
        } else if (JI2C_Message.Status == JI2C_State_Start_R) {
            i2c_ack_config(I2C1, I2C_ACK_ENABLE);
            i2c_master_addressing(I2C1, JI2C_Message.SlaveAddr.Addr8, I2C_RECEIVER);
            JI2C_Message.Status = JI2C_State_Reading;
        }
    }
    // 地址发送完成 (ADDSEND)
    else if (stat0 & I2C_STAT0_ADDSEND) {
        i2c_flag_clear(I2C1, I2C_FLAG_ADDSEND); // 清除ADDSEND
        
        if (JI2C_Message.Status == JI2C_State_Address) {
            i2c_data_transmit(I2C1, JI2C_Message.RegAddr.Hi8);
        } else if (JI2C_Message.Status == JI2C_State_Writing) {
            i2c_data_transmit(I2C1, JI2C_Message.Buffer[JI2C_Message.Index++]);
            if (JI2C_Message.Index == JI2C_Message.Len) {
                i2c_stop_on_bus(I2C1);
                JI2C_Message.Status = JI2C_State_Finished;
            }
        }
        
        // 如果只读1个字节，在地址发送后立刻关闭ACK
        if (JI2C_Message.Len == 1 && JI2C_Message.Status == JI2C_State_Reading) {
            i2c_ack_config(I2C1, I2C_ACK_DISABLE);
        }
    }
    // 字节传输完成 (BTC) / 发送缓冲区空 (TBE)
    else if ((stat0 & I2C_STAT0_BTC) && (stat0 & I2C_STAT0_TBE)) {
        if (JI2C_Message.TransType == JI2C_TType_Read && JI2C_Message.Status == JI2C_State_Address) {
            // 寄存器地址发完，产生重复起始条件进行读取
            i2c_start_on_bus(I2C1);
            JI2C_Message.Status = JI2C_State_Start_R;
        } else if (JI2C_Message.TransType == JI2C_TType_Write) {
            if (JI2C_Message.Index < JI2C_Message.Len) {
                i2c_data_transmit(I2C1, JI2C_Message.Buffer[JI2C_Message.Index++]);
            }
            if (JI2C_Message.Index == JI2C_Message.Len) {
                i2c_stop_on_bus(I2C1);
                JI2C_Message.Status = JI2C_State_Finished;
            }
        }
    }
    // 接收缓冲区非空 (RBNE)
    else if (stat0 & I2C_STAT0_RBNE) {
        JI2C_Message.Buffer[JI2C_Message.Index++] = i2c_data_receive(I2C1);
        
        // 倒数第二个字节接收完时关闭ACK
        if (JI2C_Message.Index == JI2C_Message.Len - 1) {
            i2c_ack_config(I2C1, I2C_ACK_DISABLE);
        }
        // 最后一个字节接收完毕，发送停止条件
        if (JI2C_Message.Index == JI2C_Message.Len) {
            i2c_stop_on_bus(I2C1);
            JI2C_Message.Status = JI2C_State_Finished;
        }
    }

    // 传输完成回调处理
    if (JI2C_Message.Status == JI2C_State_Finished) {
        // 关闭相关中断防止反复进入
        i2c_interrupt_disable(I2C1, I2C_INT_BUF | I2C_INT_EV);
        
        if (JI2C_Message.Callback != NULL) {
            (*JI2C_Message.Callback)(true, JI2C_Message.Buffer);
        }
        JI2C_Message.Status = JI2C_State_Idle;
    }
}