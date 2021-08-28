#ifndef __FLASH_H__
#define __FLASH_H__

#include "main.h"

/* FLASH大小：16K */
#define STM32FLASH_SIZE  0x00004000UL

/* FLASH起始地址 */
#define STM32FLASH_BASE  FLASH_BASE

/* FLASH结束地址 */
#define STM32FLASH_END   (STM32FLASH_BASE | STM32FLASH_SIZE)

/* FLASH页大小：1K */
#define STM32FLASH_PAGE_SIZE FLASH_PAGE_SIZE

/* FLASH总页数 */
#define STM32FLASH_PAGE_NUM  (STM32FLASH_SIZE / STM32FLASH_PAGE_SIZE)

/// 导出函数声明 
void FLASH_Init(void);
uint32_t STMFLASH_Read(uint32_t Address, void *Buffer, uint32_t Size);
uint32_t STMFLASH_Write(uint32_t Address, const uint16_t *Buffer, uint32_t NumToWrite);
int FLASH_ErasePage(uint32_t PageAddress, uint32_t NbPages);
uint32_t FLASH_WriteNotCheck(uint32_t Address, const uint16_t *Buffer, uint32_t NumToWrite);

#endif // !__FLASH_H
