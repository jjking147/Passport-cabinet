/*!
    \file    ymodem.c
    \brief   YMODEM(1K) 接收器实现

    协议帧格式:
      [SOH/STX][seq][~seq][data(128/1024)][CRC16_hi][CRC16_lo]
    接收流程:
      反复发 'C' -> 收文件名块(block 0) -> ACK
      -> 循环收数据块 (发送端在 ACK 后直接发 block1, 无需二次握手)
      -> 数据坏块发 NAK 要求重传; EOT 走结束握手
*/

#include "ymodem.h"
#include "ota_crc.h"
#include "ota_dl.h"

#define SOH     0x01u
#define STX     0x02u
#define EOT     0x04u
#define ACK     0x06u
#define NAK     0x15u
#define CAN     0x18u
#define CRC16_R 'C'

#define PKT_128 128u
#define PKT_1K  1024u

static int read_exact(uint8_t *buf, uint32_t n, uint32_t ms)
{
    for(uint32_t i = 0; i < n; i++) {
        int c = ota_dl_recv_byte(ms);
        if(c < 0) {
            return -1;
        }
        buf[i] = (uint8_t)c;
    }
    return 0;
}

/* 收一个完整帧; 返回 0=数据帧, 1=EOT, -1=中止/重试耗尽
 * prompt: 超时或坏块时回给发送端的提示符 ('C' 或 NAK) */
static int recv_packet(uint8_t prompt, uint8_t *seq, uint8_t *data, uint32_t *block_size)
{
    for(int attempt = 0; attempt < 12; attempt++) {
        int c = ota_dl_recv_byte(3000);
        if(c < 0) {
            ota_dl_send_byte(prompt);
            continue;
        }
        uint8_t hdr = (uint8_t)c;
        if(hdr == CAN) {
            return -1;
        }
        if(hdr == EOT) {
            return 1;
        }
        if((hdr != SOH) && (hdr != STX)) {
            continue;                       /* 杂散字节 */
        }

        uint32_t size = (hdr == SOH) ? PKT_128 : PKT_1K;
        int c2 = ota_dl_recv_byte(1000);
        if(c2 < 0) { ota_dl_send_byte(prompt); continue; }
        uint8_t seqnum = (uint8_t)c2;
        int c3 = ota_dl_recv_byte(1000);
        if(c3 < 0) { ota_dl_send_byte(prompt); continue; }
        uint8_t seqcomp = (uint8_t)c3;

        if(read_exact(data, size, 2000) != 0) { ota_dl_send_byte(prompt); continue; }
        int c4 = ota_dl_recv_byte(1000);
        if(c4 < 0) { ota_dl_send_byte(prompt); continue; }
        uint8_t crch = (uint8_t)c4;
        int c5 = ota_dl_recv_byte(1000);
        if(c5 < 0) { ota_dl_send_byte(prompt); continue; }
        uint8_t crcl = (uint8_t)c5;

        if(seqnum != (uint8_t)~seqcomp) { ota_dl_send_byte(prompt); continue; }
        uint16_t want = ota_crc16(data, size);
        uint16_t got  = (uint16_t)(((uint16_t)crch << 8) | crcl);
        if(want != got) { ota_dl_send_byte(prompt); continue; }

        *seq = seqnum;
        *block_size = size;
        return 0;
    }
    return -1;
}

/* 解析 block 0 里的文件尺寸: "<filename>\0<size 十进制>" */
static uint32_t parse_size(const uint8_t *d)
{
    const uint8_t *p = d;
    uint32_t v = 0;
    while(*p != 0u) {
        p++;
    }
    p++;   /* 跳过 NUL */
    while((*p >= '0') && (*p <= '9')) {
        v = v * 10u + (uint32_t)(*p - '0');
        p++;
    }
    return v;
}

int ymodem_receive(ymodem_write_fn write_fn, void *ctx, uint32_t *out_size)
{
    uint8_t data[PKT_1K];
    uint32_t size = 0;
    uint32_t total = 0;
    uint32_t file_size = 0;
    uint8_t seq = 0;
    uint8_t expect = 1;
    int r;

    /* 阶段1: 发 'C' 请求文件名块; 超时会持续发 'C' (方便随时接入发送端) */
    ota_dl_send_byte(CRC16_R);
    r = recv_packet(CRC16_R, &seq, data, &size);
    if(r != 0) {
        return -1;
    }
    file_size = parse_size(data);
    ota_dl_send_byte(ACK);

    /* 阶段2: 接收数据块 (发送端在 ACK 后直接发 block1) */
    for(;;) {
        r = recv_packet(NAK, &seq, data, &size);
        if(r < 0) {
            return -1;
        }
        if(r == 1) {
            /* EOT: 第一次 NAK 请求确认, 收到第二次 EOT 后 ACK */
            ota_dl_send_byte(NAK);
            r = recv_packet(NAK, &seq, data, &size);
            if(r < 0) {
                return -1;
            }
            if(r != 1) {
                continue;
            }
            ota_dl_send_byte(ACK);
            /* 空结束块 */
            ota_dl_send_byte(CRC16_R);
            r = recv_packet(CRC16_R, &seq, data, &size);
            ota_dl_send_byte(ACK);
            break;
        }

        /* 序号校验 (发送端从 1 开始, 255 回绕) */
        if(seq == (uint8_t)(expect - 1u)) {
            /* 上一包重发, 丢弃 */
            ota_dl_send_byte(ACK);
            continue;
        }
        if(seq != expect) {
            ota_dl_send_byte(NAK);
            continue;
        }

        uint32_t w = size;
        if((file_size != 0u) && ((total + size) > file_size)) {
            w = file_size - total;
        }
        if(write_fn(total, data, w, ctx) != 0) {
            ota_dl_send_byte(CAN);
            return -1;
        }
        total += w;
        expect++;
        ota_dl_send_byte(ACK);          /* 确认本块 -> 发送端发下一块 */
    }

    if(out_size != 0) {
        *out_size = total;
    }
    return 0;
}
