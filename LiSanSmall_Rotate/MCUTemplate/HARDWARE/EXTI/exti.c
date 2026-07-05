#include "exti.h"
#include "delay.h"
#include "gpio.h"
#include "bll_tocase.h"

void NVIC_Config(u8 num, u8 edge)
{
    rcu_periph_clock_enable(RCU_SYSCFG);

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
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN2);
            exti_init(EXTI_2, EXTI_INTERRUPT, trigger_edge);
            nvic_irq_enable(EXTI2_IRQn, 0, 2);
        break;

        case 2:
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN3);
            exti_init(EXTI_3, EXTI_INTERRUPT, trigger_edge);
            nvic_irq_enable(EXTI3_IRQn, 2, 3);
        break;

        case 3:
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN4);
            exti_init(EXTI_4, EXTI_INTERRUPT, trigger_edge);
            nvic_irq_enable(EXTI4_IRQn, 0, 2);
        break;

        case 4:
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN5);
            exti_init(EXTI_5, EXTI_INTERRUPT, trigger_edge);
            nvic_irq_enable(EXTI5_9_IRQn, 0, 2);
        break;

        case 5:
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN7);
            exti_init(EXTI_7, EXTI_INTERRUPT, trigger_edge);
            nvic_irq_enable(EXTI5_9_IRQn, 0, 2);
        break;

        case 6:
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN8);
            exti_init(EXTI_8, EXTI_INTERRUPT, trigger_edge);
            nvic_irq_enable(EXTI5_9_IRQn, 0, 2);
        break;

        case 7:
            syscfg_exti_line_config(EXTI_SOURCE_GPIOC, EXTI_SOURCE_PIN9);
            exti_init(EXTI_9, EXTI_INTERRUPT, trigger_edge);
            nvic_irq_enable(EXTI5_9_IRQn, 0, 2);
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

void My_EXTI_Cmd(uint8_t line, uint8_t cmd)
{
    if (cmd) {
        exti_init(exti_lines[line], EXTI_INTERRUPT, EXTI_TRIG_RISING);
        exti_interrupt_flag_clear(exti_lines[line]);
    } else {
        exti_interrupt_flag_clear(exti_lines[line]);
    }
}

void EXTI0_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_0) != RESET) {
        GPIO_INT_HANDLER(1);
        exti_interrupt_flag_clear(EXTI_0);
    }
}

void EXTI1_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_1) != RESET) {
        exti_interrupt_flag_clear(EXTI_1);
    }
}

void EXTI2_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_2) != RESET) {
        BLL_ToCase_EXTIHandler();
        exti_interrupt_flag_clear(EXTI_2);
    }
}

void EXTI3_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_3) != RESET) {
        exti_interrupt_flag_clear(EXTI_3);
    }
}

void EXTI4_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_4) != RESET) {
        exti_interrupt_flag_clear(EXTI_4);
    }
}

void EXTI5_9_IRQHandler(void)
{
    if (exti_interrupt_flag_get(EXTI_5) != RESET) {
        GPIO_INT_HANDLER(6);
        exti_interrupt_flag_clear(EXTI_5);
    }

    if (exti_interrupt_flag_get(EXTI_6) != RESET) {
        GPIO_INT_HANDLER(7);
        exti_interrupt_flag_clear(EXTI_6);
    }

    if (exti_interrupt_flag_get(EXTI_7) != RESET) {
        GPIO_INT_HANDLER(8);
        exti_interrupt_flag_clear(EXTI_7);
    }

    if (exti_interrupt_flag_get(EXTI_8) != RESET) {
        GPIO_INT_HANDLER(9);
        exti_interrupt_flag_clear(EXTI_8);
    }

    if (exti_interrupt_flag_get(EXTI_9) != RESET) {
        GPIO_INT_HANDLER(10);
        exti_interrupt_flag_clear(EXTI_9);
    }
}
