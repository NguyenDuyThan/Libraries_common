/*
 * SDC_MMC_spi.h
 *
 *  Created on: Jan 23, 2017
 *      Author: dv198
 */

#ifndef HW_LIB_SDC_H_
#define HW_LIB_SDC_H_

/*##################################################################################
 * INCLUDE
 *##################################################################################*/

/*##################################################################################
 * TYPEDEFS
 *##################################################################################*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef void (*SDC_delayms_CB)(U32 t); // delay millisecond.
typedef U8 (*SDC_wrSPI_CB)(U8 dat); // Write and read one byte via SPI.
typedef void (*SDC_wrtCSpin_CB)(U8 val); // write CS pin.
typedef void (*SDC_dbgPrt_CB)(const U8 *s, ...); // Debug printing.
/*##################################################################################
 * FUNC.PROTOTYPES
 *##################################################################################*/
/*--------------------------------------------------------------------------
 * Brief: Setup print debug callback function.
 * Param:	cb	|	IN	|	callback function.
 --*/
extern void SDC_setup_dbgPrt(SDC_dbgPrt_CB cb);
/*--------------------------------------------------------------------------
 * Brief: Setup used callback functions.
 * Param:	delayms_cb	|	I	|	"delay (x) millisecond" function.
 * 			wrSPI_cb	|	I	|	"write and read 1 byte via SPI" function.
 * 			wrtCSpin_cb	|	I	|	"write CS pin" function.
 --*/
extern void SDC_setup(SDC_delayms_CB delayms_cb, SDC_wrSPI_CB wrSPI_cb, SDC_wrtCSpin_CB wrtCSpin_cb);

extern U8 SDC_init(void);

extern U8 SDC_rdDat(U32 addr, U32 len, U8 *dat);

extern U8 SDC_wrtDat(U32 addr, U32 len, const U8 *dat);

#endif /* HW_LIB_SDC_H_ */
