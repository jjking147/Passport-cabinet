/*!
    \file    ota_log.h
    \brief   日志串口 (USART0, PA9 TX / PA10 RX, 115200-8-N-1)
             用于打印调试信息和接收 'O' 升级命令
*/

#ifndef OTA_LOG_H
#define OTA_LOG_H

#include <stdint.h>

void ota_log_init(void);
void ota_log_send_byte(uint8_t c);
/* 读取一个字节; 超时返回 -1 */
int  ota_log_recv_byte(uint32_t timeout_ms);
void ota_log_print(const char *s);
void ota_log_print_hex(uint32_t v);

#endif /* OTA_LOG_H */
