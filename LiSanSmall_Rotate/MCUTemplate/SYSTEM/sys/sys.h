#ifndef __SYS_H
#define __SYS_H
#include "gd32f4xx.h"
#include <stdint.h>

/* STM32�������Ͷ��� */
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef int8_t    s8;
typedef int16_t   s16;
typedef int32_t   s32;
typedef volatile int8_t   vs8;
typedef volatile int16_t  vs16;
typedef volatile int32_t  vs32;
typedef volatile uint8_t  vu8;
typedef volatile uint16_t vu16;
typedef volatile uint32_t vu32;

/* BitAction���ݶ��� (GD32�ٷ�ͷ�ļ��Ѷ���FlagStatus/FunctionalState/ErrorStatus) */
typedef enum {Bit_RESET = 0, Bit_SET = 1} BitAction;

/* STM32���ݺ궨�� */
#define GPIOC_BASE  GPIOC
#define GPIOD_BASE  GPIOD
#define GPIOA_BASE  GPIOA
#define GPIOB_BASE  GPIOB
#define GPIOE_BASE  GPIOE
#define GPIOF_BASE  GPIOF

/* STM32 Timer���ݺ��� */
#define TIM_Cmd(timer, state)           do { if(state) timer_enable(timer); else timer_disable(timer); } while(0)
#define TIM_SetCounter(timer, val)      timer_counter_value_config(timer, val)
#define TIM_GetCounter(timer)           timer_counter_read(timer)

/* STM32 USART���ݺ��� */
#define USART_SendData(usart, data)     usart_data_transmit(usart, data)
#define USART_GetFlagStatus(usart, flag) usart_flag_get(usart, flag)
#define USART_ClearFlag(usart, flag)    usart_flag_clear(usart, flag)
#define USART_GetITStatus(usart, it)    usart_interrupt_flag_get(usart, it)
#define USART_ClearITPendingBit(usart, it) usart_interrupt_flag_clear(usart, it)

/* STM32 USART��־λ���ݶ��� */
/* USART_FLAG_TC (Transmission Complete) �� USART_FLAG_TBE (Transmit Buffer Empty) �ǲ�ͬ�ı�־λ,
   GD32 ԭ�������� USART_FLAG_TC, ����ӳ�䵽 TBE, ���� modbus_master ���ֽڷ��ͻ���� */
#define USART_FLAG_TXE   USART_FLAG_TBE
#define USART_FLAG_RXNE  USART_FLAG_RBNE
#define USART_IT_RXNE    USART_INT_RBNE

/* STM32 GPIO���ݺ��� */
#define GPIO_ReadInputDataBit(port, pin) gpio_input_bit_get(port, pin)

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

//0,��֧��ucos
//1,֧��ucos
#define SYSTEM_SUPPORT_OS		0		//����ϵͳ�ļ��Ƿ�֧��UCOS


//λ������,ʵ��51���Ƶ�GPIO���ƹ���
//ʵ��˼��,�ο�<<CM3Ȩ��ָ��>>������(87ҳ~92ҳ).M4ͬM3,ֻ�ǼĴ�����ַ����.
//IO�ڲ����궨��
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr))
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))
//IO�ڵ�ַӳ��
#define GPIOA_ODR_Addr    (GPIOA_BASE+20) //0x40020014
#define GPIOB_ODR_Addr    (GPIOB_BASE+20) //0x40020414
#define GPIOC_ODR_Addr    (GPIOC_BASE+20) //0x40020814
#define GPIOD_ODR_Addr    (GPIOD_BASE+20) //0x40020C14
#define GPIOE_ODR_Addr    (GPIOE_BASE+20) //0x40021014
#define GPIOF_ODR_Addr    (GPIOF_BASE+20) //0x40021414
#define GPIOG_ODR_Addr    (GPIOG_BASE+20) //0x40021814
#define GPIOH_ODR_Addr    (GPIOH_BASE+20) //0x40021C14
#define GPIOI_ODR_Addr    (GPIOI_BASE+20) //0x40022014

#define GPIOA_IDR_Addr    (GPIOA_BASE+16) //0x40020010
#define GPIOB_IDR_Addr    (GPIOB_BASE+16) //0x40020410
#define GPIOC_IDR_Addr    (GPIOC_BASE+16) //0x40020810
#define GPIOD_IDR_Addr    (GPIOD_BASE+16) //0x40020C10
#define GPIOE_IDR_Addr    (GPIOE_BASE+16) //0x40021010
#define GPIOF_IDR_Addr    (GPIOF_BASE+16) //0x40021410
#define GPIOG_IDR_Addr    (GPIOG_BASE+16) //0x40021810
#define GPIOH_IDR_Addr    (GPIOH_BASE+16) //0x40021C10
#define GPIOI_IDR_Addr    (GPIOI_BASE+16) //0x40022010

//IO�ڲ���,ֻ�Ե�һ��IO��!
//ȷ��n��ֵС��16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //���
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //����

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //���
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //����

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //���
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //����

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //���
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //����

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //���
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //����

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //���
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //����

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //���
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //����

#define PHout(n)   BIT_ADDR(GPIOH_ODR_Addr,n)  //���
#define PHin(n)    BIT_ADDR(GPIOH_IDR_Addr,n)  //����

#define PIout(n)   BIT_ADDR(GPIOI_ODR_Addr,n)  //���
#define PIin(n)    BIT_ADDR(GPIOI_IDR_Addr,n)  //����

//����Ϊ��ຯ��
void WFI_SET(void);		//ִ��WFIָ��
void INTX_DISABLE(void);//�ر������ж�
void INTX_ENABLE(void);	//���������ж�
void MSR_MSP(u32 addr);	//���ö�ջ��ַ
#endif
