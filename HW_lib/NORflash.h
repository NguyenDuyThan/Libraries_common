/*
 * NORFL_diskio.h
 *
 *  Created on: Jan 4, 2017
 *      Author: dv198
 */

#ifndef NORFLASH_H_
#define NORFLASH_H_

/*##################################################################################
 * INCLUDE
 *##################################################################################*/

/*##################################################################################
 * DEFINES
 *##################################################################################*/
#define _4K_				4096

/*##################################################################################
 * TYPEDEFS
 *##################################################################################*/
typedef unsigned char U8;
//typedef unsigned short U16;
typedef unsigned long U32;

typedef void (*NORFL_delayms_CB)(U32 t); // delay millisecond.
typedef U8 (*NORFL_wrSPI_CB)(U8 dat); // Write and read one byte via SPI.
typedef void (*NORFL_wrtCSpin_CB)(U8 val); // write CS pin.
typedef void (*NORFL_print_CB)(const U8 *s, ...); // Print debug.
/*##################################################################################
 * FUNC.PROTOTYPES
 *##################################################################################*/
/*--------------------------------------------------------------------------
 * Brief: Setup print debug callback function.
 * Param:	cb	|	IN	|	callback function.
 --*/
extern void NORFL_setup_dbgPrt(NORFL_print_CB cb);
/*--------------------------------------------------------------------------
 * Brief: Setup used callback functions.
 * Param:	delayms_cb	|	I	|	"delay (x) millisecond" function.
 * 			wrSPI_cb	|	I	|	"write and read 1 byte via SPI" function.
 * 			wrtCSpin_cb	|	I	|	"write CS pin" function.
 --*/
extern void NORFL_setup(NORFL_delayms_CB delayms_cb, NORFL_wrSPI_CB wrSPI_cb, NORFL_wrtCSpin_CB wrtCSpin_cb, U32 pageSize, U32 sectorSize);
/*--------------------------------------------------------------------------
 * Brief: Release NOR flash memory from deep power down mode
 --*/
extern void NORFL_releaseFromDeepPwrDwn(void);
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
 --*/
extern void NORFL_readId(U8 *manId, U8 *memType, U8 *memCap, U8 extraSize, U8 *extraBytes);
/*--------------------------------------------------------------------------
 * Brief: Compare data
 * Param:	addr	|	I	|	address
 * 			len		|	I	|	read length.
 * 			dat		|	O	|	Comparing data buffer pointer.
 * Ret:	0	|	OK.
 * 		1	|	Busy on writing.
 * 		2	|	Unmatched.
 --*/
extern U8 NORFL_cmpDat(U32 addr, U32 len, const void *dat);
/*--------------------------------------------------------------------------
 * Brief: Read data
 * Param:	addr	|	I	|	address
 * 			len		|	I	|	read length.
 * 			dat		|	O	|	read data buffer pointer.
 * Ret:	0	|	OK
 * 		>0	|	FAIL
 --*/
extern U8 NORFL_rdDat(U32 addr, U32 len, void *dat);
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
 --*/
extern U8 NORFL_progPg(U32 addr, U32 len, const void *dat, U32 timeout);
/*--------------------------------------------------------------------------
 * Brief: Erase sector
 * Param:	addr	|	IN	|	address.
 * 			timeout	|	IN	|	timeout when erasing. Unit: millisecond.
 * Ret:	0		|	OK
 * 		0xFF	|	Address is invalid (not aligned to sector size).
 * 		>0		|	FAIL
 * Note: Address must be aligned to sector size, if not, error = 0xFF will be returned.
 * 		Example: Sector size is 64KB, valid address is 64K, 128K, 256K,...
 --*/
extern U8 NORFL_eraseSect(U32 addr, U32 timeout);
/*--------------------------------------------------------------------------
 * Brief: Erase 4-kB only
 * Param:	addr	|	IN	|	address
 * 			timeout	|	IN	|	timeout when erasing. Unit: millisecond.
 * Ret:	0		|	OK
 * 		0xFF	|	Address is invalid (not aligned to sector size).
 * 		>0		|	FAIL
 * Note: Address must be aligned to 4K, if not, error = 0xFF will be returned.
 * 		Example: Valid address is 4K, 8K, 24K,...
 --*/
extern U8 NORFL_erase4kB(U32 addr, U32 timeout);
//extern U8 NORFL_wrtDat(U32 addr, U32 len, void *dat);
//extern U8 NORFL_wrtDat4kB(U32 addr, U32 len, void *dat);

#endif /* NORFLASH_H_ */
