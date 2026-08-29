/*!
    \file    ota_flash.c
    \brief   GD32F407 FMC 闪存擦除/写入/校验封装

    仅针对 GD32F407VET6 (512KB) 的扇区布局:
      扇区 0~3 : 各 16KB
      扇区 4   : 64KB
      扇区 5~7 : 各 128KB
*/

#include "ota_flash.h"
#include "gd32f4xx.h"

typedef struct {
    uint32_t start;
    uint32_t size;
    uint32_t sn;              /* fmc_sector_erase() 的扇区号参数 */
} flash_sector_t;

static const flash_sector_t flash_sectors[] = {
    { 0x08000000u, 0x00004000u, CTL_SECTOR_NUMBER_0 },
    { 0x08004000u, 0x00004000u, CTL_SECTOR_NUMBER_1 },
    { 0x08008000u, 0x00004000u, CTL_SECTOR_NUMBER_2 },
    { 0x0800C000u, 0x00004000u, CTL_SECTOR_NUMBER_3 },
    { 0x08010000u, 0x00010000u, CTL_SECTOR_NUMBER_4 },
    { 0x08020000u, 0x00020000u, CTL_SECTOR_NUMBER_5 },
    { 0x08040000u, 0x00020000u, CTL_SECTOR_NUMBER_6 },
    { 0x08060000u, 0x00020000u, CTL_SECTOR_NUMBER_7 },
};
#define FLASH_SECTOR_COUNT  ((uint32_t)(sizeof(flash_sectors) / sizeof(flash_sectors[0])))

#define FLASH_START  0x08000000u
#define FLASH_END    0x08080000u

int ota_flash_erase(uint32_t addr, uint32_t len)
{
    uint32_t end;
    uint32_t i;

    if(len == 0u) {
        return 0;
    }
    end = addr + len;
    if((addr < FLASH_START) || (end > FLASH_END) || (end < addr)) {
        return -1;
    }

    fmc_unlock();
    for(i = 0; i < FLASH_SECTOR_COUNT; i++) {
        if(flash_sectors[i].start >= end) {
            break;
        }
        if((flash_sectors[i].start + flash_sectors[i].size) <= addr) {
            continue;
        }
        if(FMC_READY != fmc_sector_erase(flash_sectors[i].sn)) {
            fmc_lock();
            return -1;
        }
    }
    fmc_lock();
    return 0;
}

int ota_flash_write(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    uint32_t a = addr;
    uint32_t i = 0;
    uint32_t n;
    uint32_t word;
    uint8_t *p;

    if(len == 0u) {
        return 0;
    }
    if((addr < FLASH_START) || ((addr + len) > FLASH_END) || ((addr + len) < addr)) {
        return -1;
    }

    fmc_unlock();

    /* 首部非对齐: 读-改-写 (前提: 目标区已被擦除为 0xFF) */
    if(a & 3u) {
        uint32_t base = a & ~3u;
        word = *(volatile uint32_t *)base;
        p = (uint8_t *)&word;
        n = 4u - (a & 3u);
        if(n > len) {
            n = len;
        }
        for(uint32_t k = 0; k < n; k++) {
            p[(a & 3u) + k] = buf[k];
        }
        if(FMC_READY != fmc_word_program(base, word)) {
            fmc_lock();
            return -1;
        }
        a += n;
        i += n;
    }

    /* 对齐的整字写入 */
    while((len - i) >= 4u) {
        word = (uint32_t)buf[i]
             | ((uint32_t)buf[i + 1] << 8)
             | ((uint32_t)buf[i + 2] << 16)
             | ((uint32_t)buf[i + 3] << 24);
        if(FMC_READY != fmc_word_program(a, word)) {
            fmc_lock();
            return -1;
        }
        a += 4u;
        i += 4u;
    }

    /* 尾部不足一个字: 读-改-写 */
    if((len - i) != 0u) {
        word = *(volatile uint32_t *)a;
        p = (uint8_t *)&word;
        n = len - i;
        for(uint32_t k = 0; k < n; k++) {
            p[k] = buf[i + k];
        }
        if(FMC_READY != fmc_word_program(a, word)) {
            fmc_lock();
            return -1;
        }
    }

    fmc_lock();
    return 0;
}

int ota_flash_verify(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    const volatile uint8_t *p = (const volatile uint8_t *)(uintptr_t)addr;
    for(uint32_t i = 0; i < len; i++) {
        if(p[i] != buf[i]) {
            return -1;
        }
    }
    return 0;
}
