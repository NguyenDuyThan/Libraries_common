/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _STM32F1_FLASH_H_
#define _STM32F1_FLASH_H_

/*-----------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------*/
#include "stm32f10x.h"
/*-----------------------------------------------------------------------------------
 * DEFINE
 ------------------------------------------------------------------------------------*/
#define FLASH_PAGESIZE		0x800
#define FLASH_BASEADDR		0x8000000
#define FLASH_PAGEADDR(x)	((x * FLASH_PAGESIZE) + FLASH_BASEADDR)
/*-----------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef void (*IFLASH_delay10ms_CB)(U32 t); // debug printing.
/*-----------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------*/
extern void IFLASH_setup(IFLASH_delay10ms_CB cb);
extern void IFLASH_rd(U32 addr, U16 size, void *dat);
extern U8 IFLASH_wrt(U32 addr, U16 len, const void *dat);
extern U8 IFLASH_massErase(U16 startPg, U16 pgNum);
extern void IFLASH_rdUID(U8 size, U8 *uid);
#if STM32F1_FLASH_SAFEWRITE
/* Please call below function in the safe zone (cannot be interfered by interrupts during writing) */
extern void IFLASH_safeWrtProceed(void);
#endif // #if STM32F1_FLASH_SAFEWRITE
#endif // _STM32F1_FLASH_H_
