#include "exti.h"
#include "delay.h" 
#include "gpio.h"
#include "bll_claw.h"
extern void BLL_FlyOut_EXTIHandler(void);	// X2/PC3 护照脱出

void NVIC_Config(u8 num, u8 edge)	
{
    rcu_periph_clock_enable(RCU_SYSCFG);  // 使能SYSCFG时钟
    
    exti_trig_type_enum trigger_edge = EXTI_TRIG_RISING;
    
    if(edge == 0)
    {
        trigger_edge = EXTI_TRIG_RISING;	
    }
    else
    {
        trigger_edge = EXTI_TRIG_FALLING;	
    }

    switch(num)
    {
        case 1:
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN2);  // PC2连接到中断线2
            exti_init(EXTI_2, EXTI_INTERRUPT, trigger_edge);  // 直接传3个参数
            nvic_irq_enable(EXTI2_IRQn, 0, 2);  // 抢占优先级0，子优先级2
        break;
        
        case 2:
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN3);  // PC3连接到中断线3
            exti_init(EXTI_3, EXTI_INTERRUPT, trigger_edge);
            nvic_irq_enable(EXTI3_IRQn, 2, 3);  // 抢占优先级2，子优先级3
        break;
        
        case 3:
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN4);  // PC4连接到中断线4
            exti_init(EXTI_4, EXTI_INTERRUPT, trigger_edge);
            nvic_irq_enable(EXTI4_IRQn, 0, 2);  // 抢占优先级0，子优先级2
        break;
        
        case 4:
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN5);  // PC5连接到中断线5
            exti_init(EXTI_5, EXTI_INTERRUPT, trigger_edge);
            nvic_irq_enable(EXTI5_9_IRQn, 0, 2);  // 抢占优先级0，子优先级2
        break;
        
        case 5:
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN7);  // PC7连接到中断线7
            exti_init(EXTI_7, EXTI_INTERRUPT, trigger_edge);
            nvic_irq_enable(EXTI5_9_IRQn, 0, 2);  // 抢占优先级0，子优先级2
        break;
        
        case 6:
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN8);  // PC8连接到中断线8
            exti_init(EXTI_8, EXTI_INTERRUPT, trigger_edge);
            nvic_irq_enable(EXTI5_9_IRQn, 0, 2);  // 抢占优先级0，子优先级2
        break;
 
        case 7:
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN9);  // PC9连接到中断线9
            exti_init(EXTI_9, EXTI_INTERRUPT, trigger_edge);
            nvic_irq_enable(EXTI5_9_IRQn, 0, 2);  // 抢占优先级0，子优先级2
        break;
        
        default:
            break;
    }
}

static const uint32_t exti_lines[] = {
    EXTI_2,
    EXTI_3,
    EXTI_4,
    EXTI_5,
    EXTI_7,
    EXTI_8,
    EXTI_9
};

// 注意：GD32 的初始化通常需要传入触发类型，这里假设默认上升沿触发
// cmd: 0-禁用, 1-使能
void My_EXTI_Cmd(uint8_t line, uint8_t cmd)
{
    if (cmd) {
        exti_init(exti_lines[line], EXTI_INTERRUPT, EXTI_TRIG_RISING);
        exti_interrupt_flag_clear(exti_lines[line]);
    } else {
        // GD32 标准库中没有直接的 disable 单条线的宏，通常通过清除使能位实现
        // 若需要严格禁用，可直接操作寄存器或根据实际需求调整
        exti_interrupt_flag_clear(exti_lines[line]); 
    }
}

// EXTI0 中断服务函数
void EXTI0_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_0) != RESET) {
        GPIO_INT_HANDLER(1);
        exti_interrupt_flag_clear(EXTI_0); // 清除 LINE0 上的中断标志位
    }
}

// EXTI1 中断服务函数
void EXTI1_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_1) != RESET) {
        exti_interrupt_flag_clear(EXTI_1); // 清除 LINE1 上的中断标志位
    }
}

// EXTI2 中断服务函数
void EXTI2_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_2) != RESET) {
        BLL_ToCase_EXTIHandler();
        exti_interrupt_flag_clear(EXTI_2); // 清除 LINE2 上的中断标志位
    }
}

// EXTI3 中断服务函数 这里就是护照脱出的中断
void EXTI3_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_3) != RESET) {
        BLL_FlyOut_EXTIHandler();
        exti_interrupt_flag_clear(EXTI_3); // 清除 LINE3 上的中断标志位
    }
}

// EXTI4 中断服务函数
void EXTI4_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_4) != RESET) {
        exti_interrupt_flag_clear(EXTI_4); // 清除 LINE4 上的中断标志位
    }
}

// GD32F4 中 EXTI5~9 共用一个中断向量
void EXTI5_9_IRQHandler(void)
{
    // 检查并处理 EXTI_Line5 的中断
    if (exti_interrupt_flag_get(EXTI_5) != RESET) {
        GPIO_INT_HANDLER(6);
        exti_interrupt_flag_clear(EXTI_5);
    }
    
    // 检查并处理 EXTI_Line6 的中断
    if (exti_interrupt_flag_get(EXTI_6) != RESET) {
        GPIO_INT_HANDLER(7);
        exti_interrupt_flag_clear(EXTI_6);
    }
    
    // 检查并处理 EXTI_Line7 的中断
    if (exti_interrupt_flag_get(EXTI_7) != RESET) {
        GPIO_INT_HANDLER(8);
        exti_interrupt_flag_clear(EXTI_7);
    }
    
    // 检查并处理 EXTI_Line8 的中断
    if (exti_interrupt_flag_get(EXTI_8) != RESET) {
        GPIO_INT_HANDLER(9);
        exti_interrupt_flag_clear(EXTI_8);
    }
    
    // 检查并处理 EXTI_Line9 的中断
    if (exti_interrupt_flag_get(EXTI_9) != RESET) {
        GPIO_INT_HANDLER(10);
        exti_interrupt_flag_clear(EXTI_9);
    }
}