/*!
    \file    ymodem.h
    \brief   YMODEM(1K, CRC16) 接收器
*/

#ifndef YMODEM_H
#define YMODEM_H

#include <stdint.h>

/* 每收到一段数据就落盘; 返回非 0 表示中止 */
typedef int (*ymodem_write_fn)(uint32_t offset, const uint8_t *data, uint32_t len, void *ctx);

/* 阻塞式接收整份固件; 成功返回 0, *out_size 为实际字节数 */
int ymodem_receive(ymodem_write_fn write_fn, void *ctx, uint32_t *out_size);

#endif /* YMODEM_H */
