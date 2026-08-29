/*!
    \file    bootmgr.c
    \brief   启动记录管理实现 (存放于 BOOTMGR_ADDR 扇区)
*/

#include "bootmgr.h"
#include "ota_flash.h"
#include "ota_crc.h"
#include "gd32f4xx.h"

uint32_t bootmgr_slot_addr(uint32_t slot)
{
    return (slot == SLOT_B) ? BANK_B_ADDR : BANK_A_ADDR;
}

uint32_t bootmgr_slot_max_size(void)
{
    return APP_MAX_SIZE;
}

void bootmgr_record_init(boot_record_t *r)
{
    r->magic = BOOTMGR_MAGIC;
    r->active_slot = SLOT_A;
    r->state[SLOT_A] = ST_INVALID;
    r->state[SLOT_B] = ST_INVALID;
    r->try_count = 0;
    r->request = REQ_NONE;
    r->app_size[SLOT_A] = 0;
    r->app_size[SLOT_B] = 0;
    r->app_crc[SLOT_A] = 0;
    r->app_crc[SLOT_B] = 0;
    r->reserved[0] = 0;
    r->reserved[1] = 0;
}

int bootmgr_record_read(boot_record_t *r)
{
    const uint32_t *src = (const uint32_t *)(uintptr_t)BOOTMGR_ADDR;
    uint32_t *dst = (uint32_t *)r;

    if(src[0] != BOOTMGR_MAGIC) {
        return -1;
    }
    for(uint32_t i = 0; i < sizeof(boot_record_t) / 4u; i++) {
        dst[i] = src[i];
    }
    return 0;
}

int bootmgr_record_write(const boot_record_t *r)
{
    boot_record_t w = *r;
    w.magic = BOOTMGR_MAGIC;

    if(ota_flash_erase(BOOTMGR_ADDR, BOOTMGR_SIZE) != 0) {
        return -1;
    }
    if(ota_flash_write(BOOTMGR_ADDR, (const uint8_t *)&w, sizeof(w)) != 0) {
        return -1;
    }
    return 0;
}

int bootmgr_confirm(void)
{
    boot_record_t r;
    uint32_t s;

    if(bootmgr_record_read(&r) != 0) {
        return -1;
    }
    s = (r.active_slot == SLOT_B) ? SLOT_B : SLOT_A;
    r.state[s] = ST_CONFIRMED;
    r.try_count = 0;
    r.request = REQ_NONE;
    return bootmgr_record_write(&r);
}

int bootmgr_request_update(void)
{
    boot_record_t r;

    if(bootmgr_record_read(&r) != 0) {
        bootmgr_record_init(&r);
    }
    r.request = REQ_UPDATE;
    return bootmgr_record_write(&r);
}

int bootmgr_image_check(uint32_t slot, const boot_record_t *r)
{
    uint32_t size, crc;

    if(slot > SLOT_B) {
        return -1;
    }
    if(r->state[slot] == ST_INVALID) {
        return -1;
    }
    size = r->app_size[slot];
    crc  = r->app_crc[slot];
    if((size == 0u) || (size > APP_MAX_SIZE)) {
        return -1;
    }
    if(ota_crc32((const uint8_t *)(uintptr_t)bootmgr_slot_addr(slot), size) != crc) {
        return -1;
    }
    return 0;
}

int bootmgr_jump_to_app(uint32_t app_addr)
{
    uint32_t stack;
    void (*app)(void);

    stack = *(volatile uint32_t *)app_addr;
    /* 栈顶必须落在主 SRAM (0x20000000 ~ 0x20030000) 内 */
    if((stack < 0x20000000u) || (stack >= 0x20030000u)) {
        return -1;
    }

    __disable_irq();
    app = (void (*)(void))*(volatile uint32_t *)(app_addr + 4);
    __set_MSP(stack);
    app();
    for(;;) {
    }
}
