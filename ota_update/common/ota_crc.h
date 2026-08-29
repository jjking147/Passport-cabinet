/*!
    \file    ota_crc.h
    \brief   OTA 用到的两种校验: CRC16(XMODEM) 与 CRC32(镜像校验)
*/

#ifndef OTA_CRC_H
#define OTA_CRC_H

#include <stdint.h>

/* CRC-CCITT (XMODEM), 多项式 0x1021, 初值 0x0000, MSB 先行 */
uint16_t ota_crc16(const uint8_t *data, uint32_t len);

/* 标准 CRC32 (反射, 多项式 0xEDB88320, 初值/异或 0xFFFFFFFF) */
uint32_t ota_crc32(const uint8_t *data, uint32_t len);

#endif /* OTA_CRC_H */
