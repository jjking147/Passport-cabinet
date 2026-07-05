#include "led.h"

void LED_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOF);

    gpio_mode_set(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO_PIN_9 | GPIO_PIN_10);
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9 | GPIO_PIN_10);

    gpio_bit_set(GPIOF, GPIO_PIN_9 | GPIO_PIN_10);
}
