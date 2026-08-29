/*!
    \file    main.c
    \brief   OTA Bootloader (IAP 引导器)

    流程:
      1. 读启动记录 (A/B 槽状态)
      2. 若收到升级请求 (REQ_UPDATE) 或无有效 App -> 进入 YMODEM 接收
      3. 否则校验活动槽 CRC 并跳转; 连续 TRY_LIMIT 次未确认则回滚
*/

#include "gd32f4xx.h"
#include "delay.h"
#include "ota_log.h"
#include "ota_dl.h"
#include "ota_flash.h"
#include "ota_crc.h"
#include "bootmgr.h"
#include "ymodem.h"

/* 调试开关: 置 1 = PA2/PA3 串口回显测试, 测试完改回 0 再重新编译烧录 */
#define UART_ECHO_TEST  0

/* 状态指示灯: ALIENTEK GD32F407 开发板 DS0 = PF9 */
static void led_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOF);
    gpio_mode_set(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_9);
    gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_bit_set(GPIOF, GPIO_PIN_9);
}

#define LED_TOGGLE()  gpio_bit_toggle(GPIOF, GPIO_PIN_9)

/* 上电心跳: 快闪几次表示 Bootloader 已运行 (闪烁, 与 LED 极性无关) */
static void led_blink(uint32_t times, uint32_t period_ms)
{
    uint32_t i;
    for(i = 0; i < times; i++) {
        gpio_bit_toggle(GPIOF, GPIO_PIN_9);
        delay_ms(period_ms);
        gpio_bit_toggle(GPIOF, GPIO_PIN_9);
        delay_ms(period_ms);
    }
}

/* YMODEM 写回调上下文: 目标槽基地址 */
static uint32_t g_target_addr = 0;

static int flash_write_cb(uint32_t offset, const uint8_t *data, uint32_t len, void *ctx)
{
    (void)ctx;
    if((offset + len) > APP_MAX_SIZE) {
        return -1;
    }
    return ota_flash_write(g_target_addr + offset, data, len);
}

static void print_slot_state(uint32_t slot, const boot_record_t *r)
{
    ota_log_print("  Bank ");
    ota_log_send_byte((slot == SLOT_A) ? 'A' : 'B');
    ota_log_print(" @");
    ota_log_print_hex(bootmgr_slot_addr(slot));
    ota_log_print(" state=");
    ota_log_print_hex(r->state[slot]);
    ota_log_print(" size=");
    ota_log_print_hex(r->app_size[slot]);
    ota_log_print(" crc=");
    ota_log_print_hex(r->app_crc[slot]);
    ota_log_print("\r\n");
}

/* 选择本次 OTA 的目标槽: 总是保留可运行版本; 两槽都无好版本(首次)时从 Bank A 起步 */
static uint32_t ota_target_slot(const boot_record_t *rec)
{
    int a_ok = (rec->state[SLOT_A] == ST_CONFIRMED);
    int b_ok = (rec->state[SLOT_B] == ST_CONFIRMED);

    /* 只有 A 是好版本 -> 烧 B */
    if(a_ok && !b_ok) {
        return SLOT_B;
    }
    /* 只有 B 是好版本 -> 烧 A */
    if(b_ok && !a_ok) {
        return SLOT_A;
    }
    /* 两槽都无好版本 (首次 / 两槽都坏) -> 从 Bank A 起步 */
    if(!a_ok && !b_ok) {
        return SLOT_A;
    }
    /* 两槽都是好版本 -> 烧非活动槽, A/B 交替升级 */
    return (rec->active_slot == SLOT_A) ? SLOT_B : SLOT_A;
}

/* 进入 OTA 接收; 成功返回目标槽, 失败返回 -1。总是保留可回滚的好版本 */
static int do_ota_upgrade(const boot_record_t *rec)
{
    uint32_t target = ota_target_slot(rec);
    uint32_t taddr = bootmgr_slot_addr(target);
    uint32_t size = 0;
    uint32_t crc;
    boot_record_t r = *rec;

    ota_log_print("Erase bank ");
    ota_log_send_byte((target == SLOT_A) ? 'A' : 'B');
    ota_log_print(" ...\r\n");
    if(ota_flash_erase(taddr, APP_MAX_SIZE) != 0) {
        ota_log_print("Erase FAILED!\r\n");
        return -1;
    }

    g_target_addr = taddr;
    ota_log_print("Waiting YMODEM (CRC, 1K) transfer...\r\n");
    if(ymodem_receive(flash_write_cb, 0, &size) != 0) {
        ota_log_print("YMODEM receive FAILED!\r\n");
        return -1;
    }
    if((size == 0u) || (size > APP_MAX_SIZE)) {
        ota_log_print("Bad image size!\r\n");
        return -1;
    }

    crc = ota_crc32((const uint8_t *)(uintptr_t)taddr, size);
    ota_log_print("Received ");
    ota_log_print_hex(size);
    ota_log_print(" bytes, crc=");
    ota_log_print_hex(crc);
    ota_log_print("\r\n");

    ota_log_print("Update boot record ...\r\n");
    r.state[target] = ST_TRYING;
    r.app_size[target] = size;
    r.app_crc[target] = crc;
    r.active_slot = target;
    r.try_count = 0;
    r.request = REQ_NONE;
    if(bootmgr_record_write(&r) != 0) {
        ota_log_print("Boot record write FAILED!\r\n");
        return -1;
    }

    ota_log_print("Success. Rebooting ...\r\n");
    return (int)target;
}

int main(void)
{
    boot_record_t rec;
    uint32_t active;
    uint32_t st;

    SystemInit();
    delay_init(168);
    ota_log_init();
    ota_dl_init();
    led_init();
    led_blink(3, 150);   /* 上电心跳: 3 次快闪, 表示 Bootloader 已运行 */

#if UART_ECHO_TEST
    /* ===== PA2/PA3 串口回显测试 =====
       此时跳过正常引导流程:
       - 下载口 USART1: PA3(RX) 收到字节 -> 原样从 PA2(TX) 发回
       - 日志口 USART0: 同步再发一份, 方便你确认 RX 确实收到了
       测试方法: 在"下载口"串口助手里随便敲字
         -> 下载口应原样回显 (证明 PA2 TX + PA3 RX 都通)
         -> 日志口也应同时出现同样的字 (单独证明 PA3 RX) */
    ota_log_print("\r\n[ECHO TEST] download port (PA2/PA3) echo...\r\n");
    for(;;) {
        int c = ota_dl_recv_byte(100);
        if(c >= 0) {
            ota_dl_send_byte((uint8_t)c);
            ota_log_send_byte((uint8_t)c);
        }
    }
#endif

    ota_log_print("\r\n=== GD32F407 OTA Bootloader ===\r\n");

    if(bootmgr_record_read(&rec) != 0) {
        ota_log_print("No boot record, init default.\r\n");
        bootmgr_record_init(&rec);
        bootmgr_record_write(&rec);
    }
    print_slot_state(SLOT_A, &rec);
    print_slot_state(SLOT_B, &rec);
    ota_log_print("active=");
    ota_log_send_byte((rec.active_slot == SLOT_A) ? 'A' : 'B');
    ota_log_print("\r\n");

    /* 升级请求 (App 调用 bootmgr_request_update 后复位) */
    if(rec.request == REQ_UPDATE) {
        ota_log_print("Update requested.\r\n");
        while(do_ota_upgrade(&rec) < 0) {
            ota_log_print("YMODEM retry ...\r\n");
        }
        NVIC_SystemReset();
    }

    active = (rec.active_slot == SLOT_B) ? SLOT_B : SLOT_A;
    st = rec.state[active];

    if(st == ST_CONFIRMED) {
        if(bootmgr_image_check(active, &rec) == 0) {
            ota_log_print("Jump to ");
            ota_log_send_byte((active == SLOT_A) ? 'A' : 'B');
            ota_log_print("\r\n");
            bootmgr_jump_to_app(bootmgr_slot_addr(active));
            /* 返回说明跳转目标无效, 落入 OTA */
        } else {
            ota_log_print("Active image CRC mismatch!\r\n");
        }
    } else if(st == ST_TRYING) {
        if(bootmgr_image_check(active, &rec) == 0) {
            rec.try_count++;
            if(rec.try_count >= TRY_LIMIT) {
                uint32_t other = (active == SLOT_A) ? SLOT_B : SLOT_A;
                ota_log_print("Try limit reached, rollback!\r\n");
                rec.state[active] = ST_INVALID;
                rec.try_count = 0;
                if((rec.state[other] == ST_CONFIRMED) && (bootmgr_image_check(other, &rec) == 0)) {
                    rec.active_slot = other;
                    bootmgr_record_write(&rec);
                    bootmgr_jump_to_app(bootmgr_slot_addr(other));
                }
                /* 另一槽也不可用 -> 落入 OTA */
            } else {
                /* 给 trying 镜像一次机会 */
                bootmgr_record_write(&rec);
                bootmgr_jump_to_app(bootmgr_slot_addr(active));
            }
        } else {
            ota_log_print("Trying image CRC mismatch!\r\n");
        }
    }

    /* 无有效 App: 进入 OTA, 失败则反复重试直到成功 */
    ota_log_print("No valid app, enter OTA.\r\n");
    while(do_ota_upgrade(&rec) < 0) {
        ota_log_print("YMODEM retry ...\r\n");
    }
    NVIC_SystemReset();
    for(;;) {
        LED_TOGGLE();
        delay_ms(200);
    }
}
