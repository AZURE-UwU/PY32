/**
  ******************************************************************************
  * @file    bsp_flash.h
  * @author  Bowen (wbw20)
  * @date    2026-09-02
  * @version V1.0
  * @hardware PY32F003F18P6TU 开发板（TSSOP20），主 Flash 64KB
  * @brief   内部 Flash 配置页读写（掉电存储底层）
  *
  * 设计说明：
  *   - 配置页取主 Flash 倒数第二页 0x0800FF00（128B）：
  *     本工程代码+RO+RW 仅约 25KB（到 ~0x08006C18），距配置页约 37KB 裕量；
  *     不用最后一页 0x0800FF80，是避开 HAL 边界宏的"末页越界"判断。
  *   - 本芯片编程按整页进行（FLASH_TYPEPROGRAM_PAGE，一次 128B），
  *     因此上层必须凑满一页再写，未用字节填 0xFF。
  *
  * CHANGELOG:
  *   V1.0 (2026-09-02) 首次创建。
  ******************************************************************************
  */

#ifndef __BSP_FLASH_H__
#define __BSP_FLASH_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 配置页地址与大小（页大小由器件定义：128B） */
#define CFG_PAGE_ADDR   0x0800FF00UL
#define CFG_PAGE_SIZE   128U

/* 配置页缓冲：字节/字联合，保证按 32bit 对齐传给 HAL（Cortex-M0 不能非对齐取指/取数） */
typedef union {
    uint8_t  b[CFG_PAGE_SIZE];
    uint32_t w[CFG_PAGE_SIZE / 4U];
} CfgPage_t;

/* 整页写入（内部先擦除后编程），成功返回 1，失败返回 0 */
uint8_t BSP_Flash_CfgPageWrite(const CfgPage_t *page);

/* 整页读入（Flash 读取无需解锁） */
void    BSP_Flash_CfgPageRead(CfgPage_t *page);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_FLASH_H__ */

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
