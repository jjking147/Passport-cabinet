/*!
    \file    ota_dl.h
    \brief   下载串口 (USART1, PA2 TX / PA3 RX, 115200-8-N-1)
             用于 YMODEM 固件下发 (ACK/NAK/CAN/'C' 应答 + 数据)
*/

#ifndef OTA_DL_H
#define OTA_DL_H

#include <stdint.h>

void ota_dl_init(void);
void ota_dl_send_byte(uint8_t c);
/* 读取一个字节; 超时返回 -1 */
int  ota_dl_recv_byte(uint32_t timeout_ms);

#endif /* OTA_DL_H */
