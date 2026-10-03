/**
  ******************************************************************************
  * @file    bsp_flash.c
  * @author  Bowen (wbw20)
  * @date    2026-09-02
  * @version V1.0
  * @hardware PY32F003F18P6TU 开发板（TSSOP20），主 Flash 64KB
  * @brief   内部 Flash 配置页读写（掉电存储底层）
  *
  * 设计说明：
  *   - 底层只负责"擦页 + 整页编程 + 读页"，帧格式（MAGIC/CRC）由上层 app_ui 处理；
  *   - 擦写期间关闭中断保证流程原子；Flash 忙时取指会停等，阻塞数 ms 属正常；
  *   - 编程必须整页 128B 且地址 128 对齐，数据缓冲 32bit 对齐。
  *
  * CHANGELOG:
  *   V1.0 (2026-09-02) 首次创建。
  ******************************************************************************
  */

/* 头文件包含 --------------------------------------------------------*/
#include "bsp_flash.h"
#include "main.h"

#include <string.h>

/**
  * @brief  整页写入：先擦除配置页，再按整页编程
  * @param  page: 128 字节页缓冲（32bit 对齐）
  * @retval 1=成功，0=失败
  */
uint8_t BSP_Flash_CfgPageWrite(const CfgPage_t *page)
{
    if (page == NULL)
    {
        return 0;
    }

    FLASH_EraseInitTypeDef erase = {0};
    uint32_t page_err = 0xFFFFFFFFU;
    uint8_t  ok = 0;

    if (HAL_FLASH_Unlock() != HAL_OK)
    {
        return 0;
    }

    __disable_irq();   /* 擦写期间关闭中断：保证流程原子，避免打断写入序列 */

    erase.TypeErase   = FLASH_TYPEERASE_PAGEERASE;
    erase.PageAddress = CFG_PAGE_ADDR;
    erase.NbPages     = 1;

    if (HAL_FLASHEx_Erase(&erase, &page_err) == HAL_OK)
    {
        /* 整页 128B = 32 字一次编程 */
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_PAGE, CFG_PAGE_ADDR,
                              (uint32_t *)page->w) == HAL_OK)
        {
            ok = 1;
        }
    }

    __enable_irq();
    HAL_FLASH_Lock();
    return ok;
}

/**
  * @brief  读配置页（Flash 只读访问无需解锁）
  * @param  page: 输出 128 字节页缓冲
  */
void BSP_Flash_CfgPageRead(CfgPage_t *page)
{
    if (page == NULL)
    {
        return;
    }
    memcpy(page, (const void *)CFG_PAGE_ADDR, CFG_PAGE_SIZE);
}

/************************ (C) COPYRIGHT Bowen *****END OF FILE***************/
