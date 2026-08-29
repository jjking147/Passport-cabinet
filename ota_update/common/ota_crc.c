/*!
    \file    ota_crc.c
    \brief   CRC16(XMODEM) 与 CRC32 实现
*/

#include "ota_crc.h"

uint16_t ota_crc16(const uint8_t *data, uint32_t len)
{
    uint16_t crc = 0x0000u;
    while(len--) {
        crc ^= (uint16_t)(*data++) << 8;
        for(int i = 0; i < 8; i++) {
            if(crc & 0x8000u) {
                crc = (uint16_t)((crc << 1) ^ 0x1021u);
            } else {
                crc = (uint16_t)(crc << 1);
            }
        }
    }
    return crc;
}

uint32_t ota_crc32(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFu;
    while(len--) {
        crc ^= *data++;
        for(int i = 0; i < 8; i++) {
            if(crc & 1u) {
                crc = (crc >> 1) ^ 0xEDB88320u;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFFu;
}
