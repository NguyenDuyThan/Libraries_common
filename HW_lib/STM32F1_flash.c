/************************************************************************************
 *	INCLUDE
 ************************************************************************************/
#include "STM32F1_flash.h"
#include "STM32F1_nvic.h"
#include "stdlib.h"
#include "stm32f10x_flash.h"
#include "misc.h"
#include "stdio.h"
#include "string.h"
//#include "dbgPrint.h"
/************************************************************************************
 *	TYPEDEF and DEFINE
 ************************************************************************************/
typedef enum
{
	FAILED = 0, PASSED = !FAILED
} TestStatus;


/************************************************************************************
 *	VARIABLES
 ************************************************************************************/
static IFLASH_delay10ms_CB IFLASH_delay10ms = NULL;
static U8 flashIsBusy = 0;
#if STM32F1_FLASH_SAFEWRITE
static U8 safeWrtBuf[FLASH_PAGESIZE];
static U16 safeWrtLen = 0;
static U32 safeWrtAddr = 0;
static U8 safeWrtDone = 1;
static U8 safeWrtRes = 0;
#endif // #if STM32F1_FLASH_SAFEWRITE

/************************************************************************************
 *	FUNCTION PROTOTYPES
 ************************************************************************************/

/************************************************************************************
 *	FUNCTIONS
 ************************************************************************************/

/**********************************************************************
 * @brief:		read_flash
 * @param[in]:	U32
 * @param[out]: none
 * @retval;		unsigned int
 *
 * @details:	read data at flash address
 **********************************************************************/
U32 IFLASH_rdWord( U32 Address )
{
	/* Check the correction of written data */
	U32 Data;
	Data = (*(U32*) Address);
	return Data;
}
/*---------------------------------------------------------------------
 * @brief:	initial flash memory.
 ----------------------------------------------------------------------*/
void IFLASH_setup(IFLASH_delay10ms_CB cb)
{
	IFLASH_delay10ms = cb;
	RCC_HSICmd(ENABLE);
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	FLASH_SetLatency(FLASH_Latency_2);
	while( FLASH_GetStatus() != FLASH_COMPLETE )
	{
		IFLASH_delay10ms(1);
	}

}
/*--------------------------------------------------------------------
 *
 ---------------------------------------------------------------------*/
void IFLASH_rd(U32 addr, U16 size, void *dat)
{
	memcpy(dat, (void*)addr, size);
}

/*--------------------------------------------------------------------
 * Brief: write data to internal flash
 * Param:	addr	|	IN	|	flash address.
 * 			len		|	IN	|	data length.
 * 			dat		|	IN	|	data buffer.
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 ---------------------------------------------------------------------*/
U8 IFLASH_wrt(U32 addr, U16 len, const void *dat)
{
	FLASH_Status FLASHStatus;
	U32 wTmp = 0, prim = 0;
	const U8 *pDat = dat;

	if (!memcmp(dat, (void*)addr, len))
	{
		/* Already matched with data in flash */
		return 0;
	}
	while (flashIsBusy)
	{
		IFLASH_delay10ms(1);
	}
	flashIsBusy = 1;
#if STM32F1_FLASH_SAFEWRITE
	{
		U8 pageCt = (len / FLASH_PAGESIZE) + ((len % FLASH_PAGESIZE) ? 1 : 0);
		for (U8 i = 0; i < pageCt; i++)
		{
			safeWrtDone = 1;
			safeWrtLen = len - (i * FLASH_PAGESIZE);
			if (safeWrtLen > FLASH_PAGESIZE)
			{
				safeWrtLen = FLASH_PAGESIZE;
			}
			safeWrtAddr = addr + (i * FLASH_PAGESIZE);
			memcpy(safeWrtBuf, &pDat[i * FLASH_PAGESIZE], safeWrtLen);
			DBG_print("\r\n%s:%X %u", __func__, safeWrtAddr, safeWrtLen);
			safeWrtDone = 0;
			while (!safeWrtDone)
			{
				IFLASH_delay10ms(1);
			}
			if (safeWrtRes)
			{
				break;
			}
		}
		flashIsBusy = 0;
		return safeWrtRes;
	}
#else
	/* Unlock the Flash Program Erase controller */
	FLASH_Unlock();
	/* Clear All pending flags */
	//FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR | FLASH_FLAG_OPTERR);
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	for (U16 i = 0; i < len; i += 4)
	{
		if (!(addr % FLASH_PAGESIZE))
		{
			FLASHStatus = FLASH_ErasePage(addr);
			if( FLASHStatus != FLASH_COMPLETE)
			{
				FLASH_Lock();
				flashIsBusy = 0;
				return 2;
			}
		}
		memcpy(&wTmp, &pDat[i], 4);// 1 word = 4 bytes
		FLASHStatus = FLASH_ProgramWord(addr, wTmp);
		if (FLASHStatus != FLASH_COMPLETE)
		{
			FLASH_Lock();
			flashIsBusy = 0;
			return 3;
		}
		addr += 4;
	}
	FLASH_Lock();
	flashIsBusy = 0;
	return 0;
#endif // #if STM32F1_FLASH_SAFEWRITE
}

/*---------------------------------------------------------------------
 * @brief:	massive erase flash
 ----------------------------------------------------------------------*/
U8 IFLASH_massErase(U16 startPg, U16 pgNum)
{
	FLASH_Status FLASHStatus;

	while (flashIsBusy)
	{
		IFLASH_delay10ms(1);
	}
	flashIsBusy = 1;
#if STM32F1_FLASH_SAFEWRITE
	for (U8 i = 0; i < pgNum; i++)
	{
		safeWrtDone = 1;
		safeWrtLen = FLASH_PAGESIZE;
		safeWrtAddr = FLASH_PAGEADDR(startPg) + (i * FLASH_PAGESIZE);
		for (U16 i2 = 0; i2 < FLASH_PAGESIZE; i2++)
		{
			safeWrtBuf[i2] = 0xFF;
		}
		DBG_print("\r\n%s:%X %u", __func__, safeWrtAddr, safeWrtLen);
		safeWrtDone = 0;
		while (!safeWrtDone)
		{
			IFLASH_delay10ms(1);
		}
		if (safeWrtRes)
		{
			break;
		}
	}
	flashIsBusy = 0;
	return safeWrtRes;
#else
	/* Unlock the Flash Program Erase controller */
	FLASH_Unlock();
	/* Clear All pending flags */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	for (U16 pg = startPg; pg < (startPg + pgNum); pg++)
	{
		FLASHStatus = FLASH_ErasePage(FLASH_PAGEADDR(pg));
		if( FLASHStatus != FLASH_COMPLETE)
		{
			//DBG_print("\r\n %s:erase PG=%X:Res=%u", __func__, pg, FLASHStatus);
			FLASH_Lock();
			flashIsBusy = 0;
			return 1;
		}
	}
	FLASH_Lock();
	flashIsBusy = 0;
	return 0;
#endif // #if STM32F1_FLASH_SAFEWRITE
}
/*---------------------------------------------------------------------
 * @brief:	Read unique ID that embedded in each MCU
 ----------------------------------------------------------------------*/
void IFLASH_rdUID(U8 size, U8 *uid)
{
	if (size > 12)
	{
		/* Unique ID's length is 96 bits = 12 bytes */
		size = 12;
	}
	IFLASH_rd(0x1FFFF7E8, size, uid);
}
#if STM32F1_FLASH_SAFEWRITE
/*---------------------------------------------------------------------
 * Brief: Safe writing flash
 * Note: This function must be called in the safe zone
 * 		(cannot be interfered by interrupts during writing)
 ----------------------------------------------------------------------*/
void IFLASH_safeWrtProceed(void)
{
	if (!safeWrtDone)
	{
		FLASH_Status FLASHStatus;
		U32 wTmp = 0, addr = safeWrtAddr;
		const U8 *pDat = safeWrtBuf;

//		if (!memcmp(safeWrtBuf, (void*)safeWrtAddr, safeWrtLen))
//		{
//			/* Already matched with data in flash */
//			safeWrtRes = 0;
//			goto SAFEWRTPROCEED_DONE;
//		}
		/* Unlock the Flash Program Erase controller */
		FLASH_Unlock();
		/* Clear All pending flags */
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
		for (U16 i = 0; i < safeWrtLen; i += 4)
		{
			if (!(addr % FLASH_PAGESIZE))
			{
				FLASHStatus = FLASH_ErasePage(addr);
				if( FLASHStatus != FLASH_COMPLETE)
				{
					FLASH_Lock();
					safeWrtRes = 2;
					goto SAFEWRTPROCEED_DONE;
				}
			}
			memcpy(&wTmp, &pDat[i], 4);// 1 word = 4 bytes
			FLASHStatus = FLASH_ProgramWord(addr, wTmp);
			if (FLASHStatus != FLASH_COMPLETE)
			{
				FLASH_Lock();
				safeWrtRes = 3;
				goto SAFEWRTPROCEED_DONE;
			}
			addr += 4;
		}
		FLASH_Lock();
		safeWrtRes = 0;
		SAFEWRTPROCEED_DONE://
		safeWrtDone = 1;
	}
}
#endif // #if STM32F1_FLASH_SAFEWRITE
