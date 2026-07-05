#include "ji2c.h"
#include "gd32f4xx.h"

static JI2C_Message_Type JI2C_Message = { .Status = JI2C_State_Idle };

JI2C_State_Type JI2C_GetState(void)
{
    return JI2C_Message.Status;
}

static void JI2C_IO_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_I2C1);

    gpio_af_set(GPIOB, GPIO_AF_4, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_6 | GPIO_PIN_7);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7);
}

static void JI2C_IIC_Init(uint8_t ownaddr, uint32_t speed)
{
    i2c_deinit(I2C1);

    i2c_mode_addr_config(I2C1, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, ownaddr);

    i2c_ack_config(I2C1, I2C_ACK_ENABLE);
    i2c_ackpos_config(I2C1, I2C_ACKPOS_CURRENT);
    i2c_enable(I2C1);

    /* Enable clock stretching */
    I2C_CTL0(I2C1) |= I2C_CTL0_SS;
}

static void JI2C_Interrupt_Init(void)
{
    nvic_irq_enable(I2C1_EV_IRQn, 0U, 0U);
    nvic_irq_enable(I2C1_ER_IRQn, 0U, 0U);

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

void JI2C_ReadData_Async(uint8_t slav_addr, uint16_t bytes2read, uint8_t* buffer, JI2C_TRANSFINISH_CB callback)
{
    JI2C_Message.Status = JI2C_State_Start_R;
    JI2C_Message.SlaveAddr.Addr8 = slav_addr;
    JI2C_Message.Index = 0;
    JI2C_Message.Buffer = buffer;
    JI2C_Message.Len = bytes2read;
    JI2C_Message.TransType = JI2C_TType_Read;
    JI2C_Message.Callback = callback;

    i2c_start_on_bus(I2C1);
}

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

/* I2C1 Error interrupt handler */
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

/* I2C1 Event interrupt handler */
void I2C1_EV_IRQHandler(void)
{
    uint32_t stat0 = I2C_STAT0(I2C1);
    uint32_t stat1 = I2C_STAT1(I2C1);

    /* Start condition sent (SBSEND) */
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
    /* Address sent (ADDSEND) */
    else if (stat0 & I2C_STAT0_ADDSEND) {
        i2c_flag_clear(I2C1, I2C_FLAG_ADDSEND);

        if (JI2C_Message.Status == JI2C_State_Address) {
            i2c_data_transmit(I2C1, JI2C_Message.RegAddr.Hi8);
        } else if (JI2C_Message.Status == JI2C_State_Writing) {
            i2c_data_transmit(I2C1, JI2C_Message.Buffer[JI2C_Message.Index++]);
            if (JI2C_Message.Index == JI2C_Message.Len) {
                i2c_stop_on_bus(I2C1);
                JI2C_Message.Status = JI2C_State_Finished;
            }
        }

        if (JI2C_Message.Len == 1 && JI2C_Message.Status == JI2C_State_Reading) {
            i2c_ack_config(I2C1, I2C_ACK_DISABLE);
        }
    }
    /* Byte transfer complete (BTC) / Transmit buffer empty (TBE) */
    else if ((stat0 & I2C_STAT0_BTC) && (stat0 & I2C_STAT0_TBE)) {
        if (JI2C_Message.TransType == JI2C_TType_Read && JI2C_Message.Status == JI2C_State_Address) {
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
    /* Receive buffer not empty (RBNE) */
    else if (stat0 & I2C_STAT0_RBNE) {
        JI2C_Message.Buffer[JI2C_Message.Index++] = i2c_data_receive(I2C1);

        if (JI2C_Message.Index == JI2C_Message.Len - 1) {
            i2c_ack_config(I2C1, I2C_ACK_DISABLE);
        }
        if (JI2C_Message.Index == JI2C_Message.Len) {
            i2c_stop_on_bus(I2C1);
            JI2C_Message.Status = JI2C_State_Finished;
        }
    }

    /* Transfer complete callback */
    if (JI2C_Message.Status == JI2C_State_Finished) {
        i2c_interrupt_disable(I2C1, I2C_INT_BUF | I2C_INT_EV);

        if (JI2C_Message.Callback != NULL) {
            (*JI2C_Message.Callback)(true, JI2C_Message.Buffer);
        }
        JI2C_Message.Status = JI2C_State_Idle;
    }
}
