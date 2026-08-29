/*!
    \file    main.c
    \brief   OTA 演示 App (运行于 Bank A 或 Bank B)

    要点:
      1. 启动后重定位向量表 SCB->VTOR = APP_ADDR (A/B 任意槽可用)
      2. 正常运行后调用 bootmgr_confirm() 标记本槽为已确认
      3. 串口收到字符 'O' -> 请求进入 OTA 并复位到 Bootloader
*/

#include "gd32f4xx.h"
#include "delay.h"
#include "ota_log.h"
#include "bootmgr.h"

/* 置 1 模拟"坏固件": 启动后不确认, 观察 Bootloader 的回滚机制 */
#define APP_SIMULATE_BAD   0

/* ALIENTEK GD32F407 开发板 DS0 = PF9 */
static void led_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOF);
    gpio_mode_set(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_9);
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_bit_set(GPIOF, GPIO_PIN_9);
}

#define LED_TOGGLE()  gpio_bit_toggle(GPIOF, GPIO_PIN_9)

static void relocate_vector_table(void)
{
    /* A/B 双槽运行的关键: 让中断向量指向本镜像所在槽 */
    SCB->VTOR = APP_ADDR;
}

int main(void)
{
    char bank = (APP_ADDR == BANK_A_ADDR) ? 'A' : 'B';

    relocate_vector_table();
    SystemInit();
    delay_init(168);
    ota_log_init();
    led_init();

    ota_log_print("\r\n===== OTA APP-");
    ota_log_send_byte((uint8_t)bank);
    ota_log_print(" =====\r\n");
    ota_log_print("  link @ ");
    ota_log_print_hex(APP_ADDR);
    ota_log_print("\r\n");

#if APP_SIMULATE_BAD
    ota_log_print("SIMULATE_BAD: never confirm -> rollback expected.\r\n");
#else
    if(bootmgr_confirm() == 0) {
        ota_log_print("Boot confirmed.\r\n");
    } else {
        ota_log_print("Boot confirm FAILED!\r\n");
    }
#endif

    ota_log_print("Type 'O' to request OTA upgrade.\r\n");

    uint32_t tick = 0;
    for(;;) {
        LED_TOGGLE();
        delay_ms(500);

        /* 收到 'O' 触发升级 */
        if(ota_log_recv_byte(1) == 'O') {
            ota_log_print("OTA requested, reboot to bootloader...\r\n");
            bootmgr_request_update();
            NVIC_SystemReset();
        }

        /* 每 2 秒打印一次心跳, 用日志持续标识当前 Bank (替代看不到的 LED) */
        tick++;
        if(tick >= 4) {                     /* 4 x 500ms = 2s */
            tick = 0;
            ota_log_print("[APP-");
            ota_log_send_byte((uint8_t)bank);
            ota_log_print("] alive @0x");
            ota_log_print_hex(APP_ADDR);
            ota_log_print("\r\n");
        }
    }
}
