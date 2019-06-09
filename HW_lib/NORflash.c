/*
 * NORFL_diskio.c
 *
 *  Created on: Jan 4, 2017
 *      Author: dv198
 */

/*##################################################################################
 * INCLUDE
 *##################################################################################*/
#include <NORflash.h>
/*##################################################################################
 * DEFINES
 *##################################################################################*/
#define REV_DATA		0


#define WREN_CMD		0x06 // Write enable
#define WRDI_CMD		0x04 // write disable
#define RIDI_CMD		0x9F // Read identification.
#define RDSR_CMD		0x05 // Read status register.
#define WRSR_CMD		0x01 // Write status register.
#define READ_CMD		0x03 // Read data bytes, with input address is 24-bit.
#define _4READ_CMD		0x13 // Read data with input address is 32-bit (instead of 24-bit).
#define FAST_READ_CMD	0x0B // Read data bytes at high speed.
#define PP_CMD			0x02 // Page programming, with input address is 24-bit.
#define _4PP_CMD		0x12 // Page programming with input address is 32-bit (instead of 24-bit).
#define SE_CMD			0xD8 // Sector erasing, with input address is 24-bit.
#define _4SE_CMD		0xDC // Sector erasing with input address is 32-bit (instead of 24-bit).
#define BE_CMD			0xC7 // Bulk erasing.
#define P4E_CMD			0x20 // Parameter 4-kB sector erase, with input address is 24-bit.
#define _4P4E_CMD		0x21 // Parameter 4-kB sector erase with input address is 32-bit (instead of 24-bit).
#define DP_CMD			0xB9 // Deep power-down.
#define RES_CMD			0xAB // Release deep power-down, and read electronic signature.

/*##################################################################################
 * TYPEDEFS
 *##################################################################################*/

/*##################################################################################
 * FUNC.PROTOTYPES
 *##################################################################################*/
static void NORFL_print_pseudo(const U8 *s, ...);
static U8 NORFL_readStt(U8 *stt);
static U8 NORFL_enWrt(void);
/*##################################################################################
 * VARIABLES
 *##################################################################################*/
static NORFL_print_CB NORFL_dbgPrint = NORFL_print_pseudo;
static NORFL_delayms_CB NORFL_delayms;
static NORFL_wrSPI_CB NORFL_wrSPI;
static NORFL_wrtCSpin_CB NORFL_wrtCSpin;
static U32 NORFL_PAGE_SIZE = 512; // Default value
static U32 NORFL_SECTOR_SIZE = (64 * 1024); // default value

/*##################################################################################
 * FUNCTIONS
 *##################################################################################*/
/*--------------------------------------------------------------------------
 * Brief: pseudo print function.
 ---------------------------------------------------------------------------*/
void NORFL_print_pseudo(const U8 *s, ...)
{
	return;
}
/*--------------------------------------------------------------------------
 * Brief: Setup print debug callback function.
 * Param:	cb	|	IN	|	print debug function.
 ---------------------------------------------------------------------------*/
void NORFL_setup_dbgPrt(NORFL_print_CB cb)
{
	if (cb != (void*)0)
	{
		NORFL_dbgPrint = cb;
	}
	else
	{
		NORFL_dbgPrint = NORFL_print_pseudo;
	}
}
/*--------------------------------------------------------------------------
 * Brief: Setup used callback functions.
 * Param:	delayms_cb	|	I	|	"delay (x) millisecond" function.
 * 			wrSPI_cb	|	I	|	"write and read 1 byte via SPI" function.
 * 			wrtCSpin_cb	|	I	|	"write CS pin" function.
 * 			pageSize	|	I	|	page size.
 * 			sectorSize	|	I	|	sector size.
 ---------------------------------------------------------------------------*/
void NORFL_setup(NORFL_delayms_CB delayms_cb, NORFL_wrSPI_CB wrSPI_cb, NORFL_wrtCSpin_CB wrtCSpin_cb, U32 pageSize, U32 sectorSize)
{
	NORFL_delayms = delayms_cb;
	NORFL_wrSPI = wrSPI_cb;
	NORFL_wrtCSpin = wrtCSpin_cb;
	NORFL_wrtCSpin(1);
	NORFL_PAGE_SIZE = pageSize;
	NORFL_SECTOR_SIZE = sectorSize;
}
/*--------------------------------------------------------------------------
 * Brief: Release NOR flash memory from deep power down mode
 ---------------------------------------------------------------------------*/
void NORFL_releaseFromDeepPwrDwn(void)
{
	NORFL_wrtCSpin(0);
	NORFL_wrSPI(RES_CMD);
	NORFL_delayms(500);
	NORFL_wrtCSpin(1);
	NORFL_delayms(500);
}
/*--------------------------------------------------------------------------
 * Brief: Read identification.
 * Param:	manId		|	O	|	manufacturer Id.
 * 			memType		|	O	|	memory type.
 * 			memCap		|	O	|	memory capacity.
 * 			extraSize	|	I	|	extra bytes size.
 * 			extraBytes	|	O	|	extra bytes.
 * Ret: 0	|	OK
 * 		>0	|	FAIL
 * Note: Output parameter is only code number, not readable value.
 * 		This function is used for verifying memory identification.
 ---------------------------------------------------------------------------*/
void NORFL_readId(U8 *manId, U8 *memType, U8 *memCap, U8 extraSize, U8 *extraBytes)
{
	/* Select chip */
	NORFL_wrtCSpin(0);
	/* Write command */
	NORFL_wrSPI(RIDI_CMD);
	/* Read manufacturer identification */
	*manId = NORFL_wrSPI(0xFF);
	/* Read memory type */
	*memType = NORFL_wrSPI(0xFF);
	/* Read memory capacity */
	*memCap = NORFL_wrSPI(0xFF);
	/* Read extra bytes */
	for(U8 i = 0; i < extraSize; i++)
	{
		extraBytes[i] = NORFL_wrSPI(0xFF);
	}
	/* Release chip */
	NORFL_wrtCSpin(1);
}
/*--------------------------------------------------------------------------
 * Brief: Read status register
 ---------------------------------------------------------------------------*/
U8 NORFL_readStt(U8 *stt)
{
	/* Select chip */
	NORFL_wrtCSpin(0);
	/* Write command */
	NORFL_wrSPI(RDSR_CMD);
	/* Read status value */
	*stt = NORFL_wrSPI(0xFF);
	/* Release chip */
	NORFL_wrtCSpin(1);
	return 0;
}
/*--------------------------------------------------------------------------
 * Brief: Enable writing
 ---------------------------------------------------------------------------*/
U8 NORFL_enWrt(void)
{
	/* Select chip */
	NORFL_wrtCSpin(0);
	/* Write command */
	NORFL_wrSPI(WREN_CMD);
	/* Release chip */
	NORFL_wrtCSpin(1);
	return 0;
}
/*--------------------------------------------------------------------------
 * Brief: Compare data
 * Param:	addr	|	I	|	address
 * 			len		|	I	|	read length.
 * 			dat		|	O	|	Comparing data buffer pointer.
 * Ret:	0	|	OK.
 * 		1	|	Busy on writing.
 * 		2	|	Unmatched.
 ---------------------------------------------------------------------------*/
U8 NORFL_cmpDat(U32 addr, U32 len, const void *dat)
{
	U32 i = 0, diffCt = 0;
	U8 *cDat = (U8*)dat, stt = 0, byte = 0;

	/* Read status register */
	NORFL_readStt(&stt);
	if (stt & 0x1)
	{
		/* Busy on writing */
		return 1;
	}
	/* Select chip */
	NORFL_wrtCSpin(0);
	/* Write command */
	if (addr <= 0xFFFFFF)
	{
		NORFL_wrSPI(READ_CMD);
	}
	else
	{
		NORFL_wrSPI(_4READ_CMD);
		/* Write address: 1 byte more with 32 bit address */
		NORFL_wrSPI((addr >> 24) & 0xFF);
	}
	/* Write address */
	NORFL_wrSPI((addr >> 16) & 0xFF);
	NORFL_wrSPI((addr >> 8) & 0xFF);
	NORFL_wrSPI(addr & 0xFF);
	/* Read data bytes value */
	NORFL_dbgPrint("\r\n NORFL_cmpDat:A=%X L=%u:<START>\r\n", addr, len);
	for (i = 0; i < len; i++)
	{
		byte = NORFL_wrSPI(0xFF);
#if REV_DATA
		byte = ~byte;
#endif
		NORFL_dbgPrint("\r\n[%u]%02X %02X", i, byte, cDat[i]);
		if(byte != cDat[i])
		{
			NORFL_wrtCSpin(1);
			//NORFL_dbgPrint("\r\n NORFL_cmpDat:A=%X:%X/%X", addr + i, byte, cDat[i]);
			diffCt++;
		}
		//NORFL_print("\r\n NORFL_rdDat:[%u]=%X", i, dat[i]);
	}
	NORFL_dbgPrint("\r\n<END>");
	/* Release chip */
	NORFL_wrtCSpin(1);
	return (!diffCt) ? 0 : 2;
}
/*--------------------------------------------------------------------------
 * Brief: Read data
 * Param:	addr	|	I	|	address
 * 			len		|	I	|	read length.
 * 			dat		|	O	|	read data buffer pointer.
 * Ret:	0	|	OK
 * 		>0	|	FAIL
 ---------------------------------------------------------------------------*/
U8 NORFL_rdDat(U32 addr, U32 len, void *dat)
{
	U32 i = 0;
	U8 *rDat = (U8*)dat, stt = 0;

	/* Read status register */
	NORFL_readStt(&stt);
	if (stt & 0x1)
	{
		/* Busy on writing */
		return 1;
	}
	/* Select chip */
	NORFL_wrtCSpin(0);
	/* Write command */
	if (addr <= 0xFFFFFF)
	{
		NORFL_wrSPI(READ_CMD);
	}
	else
	{
		NORFL_wrSPI(_4READ_CMD);
		/* Write address: 1 byte more with 32 bit address */
		NORFL_wrSPI((addr >> 24) & 0xFF);
	}
	/* Write address */
	NORFL_wrSPI((addr >> 16) & 0xFF);
	NORFL_wrSPI((addr >> 8) & 0xFF);
	NORFL_wrSPI(addr & 0xFF);
	/* Read data bytes value */
	for (i = 0; i < len; i++)
	{
		rDat[i] = NORFL_wrSPI(0xFF);
#if REV_DATA
		rDat[i] = ~rDat[i];
#endif
		//NORFL_print("\r\n NORFL_rdDat:[%u]=%X", i, dat[i]);
	}
	/* Release chip */
	NORFL_wrtCSpin(1);
	return 0;
}
/*--------------------------------------------------------------------------
 * Brief: Programming page.
 * Param:	addr	|	I	|	address
 * 			len		|	I	|	data length.
 * 			dat		|	I	|	data.
 * 			timeout	|	I	|	timeout when programming. Unit: millisecond.
 * Ret:	0		|	OK
 * 		0xFF	|	Address is invalid (not aligned to sector size).
 * 		>0		|	FAIL
 * Note: Address must be aligned to page size, if not, error = 0xFF will be returned.
 * 		Example: If page size is 256 bytes, program size is 256, 512,...
 ---------------------------------------------------------------------------*/
U8 NORFL_progPg(U32 addr, U32 len, const void *dat, U32 timeout)
{
	U8 stt, *wDat = (U8*)dat, byte = 0;

	NORFL_dbgPrint("\r\n NORFL_progPg:A=%X L=%u:<START>\r\n", addr, len);
	for (U32 i = 0; i < len; i++)
	{
		NORFL_dbgPrint("%02X", wDat[i]);
	}
	NORFL_dbgPrint("\r\n<END>");
	/* Read status register */
	NORFL_readStt(&stt);
	if (stt & 0x1)
	{
		/* Busy on writing */
		return 1;
	}
	NORFL_enWrt();
	/* Select chip */
	NORFL_wrtCSpin(0);
	/* Write command */
	if (addr <= 0xFFFFFF)
	{
		NORFL_wrSPI(PP_CMD);
	}
	else
	{
		NORFL_wrSPI(_4PP_CMD);
		/* Write address: 1 byte more with 32 bit address */
		NORFL_wrSPI((addr >> 24) & 0xFF);
	}
	/* Write address */
	NORFL_wrSPI((addr >> 16) & 0xFF);
	NORFL_wrSPI((addr >> 8) & 0xFF);
	NORFL_wrSPI(addr & 0xFF);
	/* Write data bytes value */
	for (U32 i = 0; i < len; i++)
	{
		//NORFL_print("\r\n NORFL_progPg:[%u]=%X", i, dat[i]);
		byte = wDat[i];
#if REV_DATA
		byte = ~byte;
#endif
		NORFL_wrSPI(byte);
	}
	/* Release chip */
	NORFL_wrtCSpin(1);
	/* Wait until timeout or writing is done */
	for (U32 t = 0; t < timeout; t++)
	{
		NORFL_readStt(&stt);
		if (!(stt & 0x1))
		{
			/* Writing is done */
			return 0;
		}
		NORFL_delayms(1);
	}
	/* Timeout */
	return 2;
}
/*--------------------------------------------------------------------------
 * Brief: Erase sector
 * Param:	addr	|	IN	|	address.
 * 			timeout	|	IN	|	timeout when erasing. Unit: millisecond.
 * Ret:	0		|	OK
 * 		0xFF	|	Address is invalid (not aligned to sector size).
 * 		>0		|	FAIL
 * Note: Address must be aligned to sector size, if not, error = 0xFF will be returned.
 * 		Example: Sector size is 64KB, valid address is 64K, 128K, 256K,...
 ---------------------------------------------------------------------------*/
U8 NORFL_eraseSect(U32 addr, U32 timeout)
{
	U8 stt;

	if (addr % NORFL_SECTOR_SIZE)
	{
		/* Unacceptable address */
		return 0xFF;
	}
	/* Read status register */
	NORFL_readStt(&stt);
	if (stt & 0x1)
	{
		/* Busy on writing */
		return 1;
	}
	NORFL_enWrt();
	/* Select chip */
	NORFL_wrtCSpin(0);
	/* Write command */
	if (addr <= 0xFFFFFF)
	{
		NORFL_wrSPI(SE_CMD);
	}
	else
	{
		NORFL_wrSPI(_4SE_CMD);
		/* Write address: 1 byte more with 32 bit address */
		NORFL_wrSPI((addr >> 24) & 0xFF);
	}
	/* Write address */
	NORFL_wrSPI((addr >> 2) & 0xFF);
	NORFL_wrSPI((addr >> 1) & 0xFF);
	NORFL_wrSPI(addr & 0xFF);
	/* Release chip */
	NORFL_wrtCSpin(1);
	/* Wait until timeout or writing is done */
	for (U32 t = 0; t < timeout; t++)
	{
		NORFL_readStt(&stt);
		if (!(stt & 0x1))
		{
			/* Writing is done */
			return 0;
		}
		NORFL_delayms(1);
	}
	/* Timeout */
	return 2;
}
/*--------------------------------------------------------------------------
 * Brief: Erase 4-kB only
 * Param:	addr	|	IN	|	address
 * 			timeout	|	IN	|	timeout when erasing. Unit: millisecond.
 * Ret:	0		|	OK
 * 		0xFF	|	Address is invalid (not aligned to sector size).
 * 		>0		|	FAIL
 * Note: Address must be aligned to 4K, if not, error = 0xFF will be returned.
 * 		Example: Valid address is 4K, 8K, 24K,...
 ---------------------------------------------------------------------------*/
U8 NORFL_erase4kB(U32 addr, U32 timeout)
{
	U8 stt;

	if (addr % _4K_)
	{
		/* Unacceptable address */
		return 0xFF;
	}
	/* Read status register */
	NORFL_readStt(&stt);
	if (stt & 0x1)
	{
		/* Busy on writing */
		return 1;
	}
	NORFL_enWrt();
	/* Select chip */
	NORFL_wrtCSpin(0);
	/* Write command */
	if (addr <= 0xFFFFFF)
	{
		NORFL_wrSPI(P4E_CMD);
	}
	else
	{
		NORFL_wrSPI(_4P4E_CMD);
		/* Write address: 1 byte more with 32 bit address */
		NORFL_wrSPI((addr >> 24) & 0xFF);
	}
	/* Write address */
	NORFL_wrSPI((addr >> 16) & 0xFF);
	NORFL_wrSPI((addr >> 8) & 0xFF);
	NORFL_wrSPI(addr & 0xFF);
	/* Release chip */
	NORFL_wrtCSpin(1);
	/* Wait until timeout or writing is done */
	for (U32 t = 0; t < timeout; t++)
	{
		NORFL_readStt(&stt);
		if (!(stt & 0x1))
		{
			/* Writing is done */
			return 0;
		}
		NORFL_delayms(1);
	}
	/* Timeout */
	return 2;
}
