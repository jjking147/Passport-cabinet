/*!
    \file    ota_flash.h
    \brief   GD32F407 FMC 闪存擦除/写入/校验封装
*/

#ifndef OTA_FLASH_H
#define OTA_FLASH_H

#include <stdint.h>

/* 擦除 [addr, addr+len) 覆盖的所有扇区; 成功返回 0 */
int ota_flash_erase(uint32_t addr, uint32_t len);

/* 写 len 字节到 addr (内部按字对齐, 处理首尾非对齐); 成功返回 0 */
int ota_flash_write(uint32_t addr, const uint8_t *buf, uint32_t len);

/* 校验 flash 内容与 buf 是否一致; 一致返回 0 */
int ota_flash_verify(uint32_t addr, const uint8_t *buf, uint32_t len);

#endif /* OTA_FLASH_H */
