/*!
    \file    ota_config.h
    \brief   OTA/IAP 唯一分区定义 —— Bootloader 与 App 两侧必须保持一致
             (GD32F407VET6, 512KB Flash)
*/

#ifndef OTA_CONFIG_H
#define OTA_CONFIG_H

#include <stdint.h>

/* ============================================================
 * GD32F407VET6 (512KB) Flash 物理布局
 *   扇区 0  : 16KB   0x08000000
 *   扇区 1  : 16KB   0x08004000
 *   扇区 2  : 16KB   0x08008000
 *   扇区 3  : 16KB   0x0800C000
 *   扇区 4  : 64KB   0x08010000
 *   扇区 5  : 128KB  0x08020000
 *   扇区 6  : 128KB  0x08040000
 *   扇区 7  : 128KB  0x08060000
 * ============================================================
 * 本项目分区 (A/B 双区 + 回滚):
 *   Bootloader : 扇区 0~2, 48KB    0x08000000 .. 0x0800BFFF
 *   BootMgr    : 扇区 3,   16KB    0x0800C000 .. 0x0800FFFF   (启动记录)
 *   Bank A     : 扇区 4~5, 192KB   0x08010000 .. 0x0803FFFF
 *   Bank B     : 扇区 6~7, 256KB   0x08040000 .. 0x0807FFFF   (本示例只用前 192KB)
 * ============================================================ */

#define BOOTLOADER_ADDR    0x08000000u   /* 扇区 0~2, 共 48KB */
#define BOOTLOADER_SIZE    0x0000C000u

#define BOOTMGR_ADDR       0x0800C000u   /* 扇区 3, 16KB, 启动记录 */
#define BOOTMGR_SIZE       0x00004000u

#define BANK_A_ADDR        0x08010000u   /* 扇区 4~5, 192KB */
#define BANK_A_SIZE        0x00030000u

#define BANK_B_ADDR        0x08040000u   /* 扇区 6~7, 256KB (示例只用前 192KB) */
#define BANK_B_SIZE        0x00040000u

/* App 镜像最大尺寸: Bank A 与 Bank B 取小者 = 192KB */
#define APP_MAX_SIZE       0x00030000u

/* App 编译链接地址 (Keil 工程通过 -DAPP_ADDR=0x... 注入);
   未定义时默认 Bank A。App 端用它配置 SCB->VTOR。 */
#ifndef APP_ADDR
#define APP_ADDR           BANK_A_ADDR
#endif

#endif /* OTA_CONFIG_H */
