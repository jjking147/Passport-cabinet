/*!
    \file    bootmgr.h
    \brief   启动记录管理 (A/B 双区 + 回滚), Bootloader 与 App 共用
*/

#ifndef BOOTMGR_H
#define BOOTMGR_H

#include <stdint.h>
#include "ota_config.h"

#define BOOTMGR_MAGIC   0x4F544142u   /* "OTAB" (字节序 "B","A","T","O") */

#define SLOT_A          0u
#define SLOT_B          1u

/* 槽位状态 */
#define ST_INVALID      0u   /* 无有效镜像 */
#define ST_CONFIRMED    1u   /* 已确认可运行 */
#define ST_TRYING       2u   /* 刚烧录, 等待 App 确认 */

/* 启动记录请求 */
#define REQ_NONE        0u
#define REQ_UPDATE      0x55504441u   /* "ADPU" : 请求进入 OTA */

/* 连续多次启动仍未确认则回滚 */
#define TRY_LIMIT       5u

typedef struct {
    uint32_t magic;
    uint32_t active_slot;     /* SLOT_A / SLOT_B */
    uint32_t state[2];        /* 每个槽的状态 ST_* */
    uint32_t try_count;       /* 当前 trying 槽已尝试启动次数 */
    uint32_t request;         /* REQ_* */
    uint32_t app_size[2];     /* 每个槽镜像大小 */
    uint32_t app_crc[2];      /* 每个槽镜像 CRC32 */
    uint32_t reserved[2];
} boot_record_t;

uint32_t bootmgr_slot_addr(uint32_t slot);
uint32_t bootmgr_slot_max_size(void);

void bootmgr_record_init(boot_record_t *r);
int  bootmgr_record_read(boot_record_t *r);         /* 0 = 成功 */
int  bootmgr_record_write(const boot_record_t *r);  /* 0 = 成功 */

int  bootmgr_confirm(void);                         /* App: 标记当前槽为 CONFIRMED */
int  bootmgr_request_update(void);                  /* App: 请求进入 OTA */

int  bootmgr_image_check(uint32_t slot, const boot_record_t *r); /* 0 = 有效 */

/* 跳转到 App (不返回); 目标无效时返回 -1 */
int  bootmgr_jump_to_app(uint32_t app_addr);

#endif /* BOOTMGR_H */
