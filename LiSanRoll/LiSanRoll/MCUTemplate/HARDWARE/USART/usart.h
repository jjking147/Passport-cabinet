#ifndef __USART_H
#define __USART_H
#include "stdio.h"
#include "gd32f4xx_libopt.h"
#include "sys.h"

#define USART_REC_LEN  		20

// GD32 USART mapping:
// STM32 USART2 -> GD32 USART1 (Master/Motor)
// STM32 USART1 -> GD32 USART0 (Slave/Host)
#define MOTOR_USART         USART1
#define MOTOR_USART_IRQn	USART1_IRQn

#define MOTOR_USART_RX	    GPIO_PIN_3
#define MOTOR_USART_TX	    GPIO_PIN_2
#define MOTOR_USART_GPIO    GPIOA

#define MOTOR_USART_CLK_EN	     rcu_periph_clock_enable(RCU_USART1)
#define MOTOR_USART_GPIO_CLK_EN rcu_periph_clock_enable(RCU_GPIOA)

extern u8  USART_RX_BUF[USART_REC_LEN];
extern u16 USART_RX_STA;

void Master_USART_Init(u32 bound);
void Slave_USART_Init(u32 bound);
void USART0_Tx_InitDMA(void);
void USART0_SendByte(u8 byte);
void USART0_SendBytes(u8 *bytes, u32 len);
void USART0_SendString(u8 *str);
void USART0_SendWithDMA(u8* data, u32 len);
void WriteToFlash(u32* dataArray, u16 len, u8 sector, u8* flashAddr);

/* Compatibility macro: STM32 USART1 -> GD32 USART0 */
#define USART1_SendWithDMA  USART0_SendWithDMA
#define USART1_SendByte     USART0_SendByte
#define USART1_SendBytes    USART0_SendBytes
#define USART1_SendString   USART0_SendString

#endif
