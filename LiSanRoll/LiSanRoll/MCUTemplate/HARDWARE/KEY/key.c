#include "key.h"
#include "delay.h"

void KEY_Init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOE);

    /* KEY0-KEY2: PE2, PE3, PE4 - input with pull-up */
    gpio_mode_set(GPIOE, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4);

    /* WK_UP: PA0 - input with pull-down */
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_0);
}

u8 KEY_Scan(u8 mode)
{
    static u8 key_up = 1;
    if(mode) key_up = 1;
    if(key_up && (KEY0 == 0 || KEY1 == 0 || KEY2 == 0 || WK_UP == 1))
    {
        delay_ms(10);
        key_up = 0;
        if(KEY0 == 0) return 1;
        else if(KEY1 == 0) return 2;
        else if(KEY2 == 0) return 3;
        else if(WK_UP == 1) return 4;
    } else if(KEY0 == 1 && KEY1 == 1 && KEY2 == 1 && WK_UP == 0) key_up = 1;
    return 0;
}
