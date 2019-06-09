/*
 * SD_MMC_spi.c
 *
 *  Created on: Jan 23, 2017
 *      Author: dv198
 */

/*##################################################################################
 * INCLUDE
 *##################################################################################*/
#include "SDC.h"
/*##################################################################################
 * DEFINE
 *##################################################################################*/
/* SDC/MMC commands table */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD0_CRC	0x95
#define CMD1	(0x40+1)	/* SEND_OP_COND (MMC) */
#define ACMD41	(0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD8_CRC	0x87
#define CMD9	(0x40+9)	/* SEND_CSD */
#define CMD10	(0x40+10)	/* SEND_CID */
#define CMD12	(0x40+12)	/* STOP_TRANSMISSION */
#define ACMD13	(0xC0+13)	/* SD_STATUS (SDC) */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD18	(0x40+18)	/* READ_MULTIPLE_BLOCK */
#define CMD23	(0x40+23)	/* SET_BLOCK_COUNT (MMC) */
#define ACMD23	(0xC0+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD25	(0x40+25)	/* WRITE_MULTIPLE_BLOCK */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */
#define DUMMY_CRC	0x1

/*##################################################################################
 * TYPEDEFS
 *##################################################################################*/
typedef enum
{
	SDT_unknown = 0,
	SDT_SDver2_BA, // SD ver.2 block address
	SDT_SDver2, // SD ver.2 byte address
	SDT_SDver1,
	SDT_MMCver3,
} SDC_TYPE_t;

/*##################################################################################
 * FUNC.PROTOTYPES
 *##################################################################################*/
static void SDC_dbgPrt_pseudo(const U8 *s,...);
static U8 SDC_waitRdy(U32 timeout);
/*##################################################################################
 * VARIABLES
 *##################################################################################*/
static SDC_dbgPrt_CB SDC_dbgPrt = SDC_dbgPrt_pseudo;
static SDC_delayms_CB SDC_delayms;
static SDC_wrSPI_CB SDC_wrSPI;
static SDC_wrtCSpin_CB SDC_wrtCSpin;
/*##################################################################################
 * FUNCTIONS
 *##################################################################################*/
/*--------------------------------------------------------------------------
 * Brief:
 ---------------------------------------------------------------------------*/
void SDC_dbgPrt_pseudo(const U8 *s,...)
{
	return;
}
/*--------------------------------------------------------------------------
 * Brief: Setup print debug callback function.
 * Param:	cb	|	IN	|	callback function.
 ---------------------------------------------------------------------------*/
void SDC_setup_dbgPrt(SDC_dbgPrt_CB cb)
{
	if (cb == (void*)0)
	{
		SDC_dbgPrt = SDC_dbgPrt_pseudo;
		return;
	}
	SDC_dbgPrt = cb;
}
/*--------------------------------------------------------------------------
 * Brief: Setup used callback functions.
 * Param:	delayms_cb	|	I	|	"delay (x) millisecond" function.
 * 			wrSPI_cb	|	I	|	"write and read 1 byte via SPI" function.
 * 			wrtCSpin_cb	|	I	|	"write CS pin" function.
 ---------------------------------------------------------------------------*/
void SDC_setup(SDC_delayms_CB delayms_cb, SDC_wrSPI_CB wrSPI_cb, SDC_wrtCSpin_CB wrtCSpin_cb)
{
	SDC_delayms = delayms_cb;
	SDC_wrSPI = wrSPI_cb;
	SDC_wrtCSpin = wrtCSpin_cb;
}
/*--------------------------------------------------------------------------
 * Brief: Wait until MISO is high, or timeout.
 * Param:	timeout	|	I	|	timeout. Unit: millisecond
 * ret:	0	|	OK
 * 		1	|	Timeout
 ---------------------------------------------------------------------------*/
U8 SDC_waitRdy(U32 timeout)
{
	SDC_wrSPI(0xFF);
	while ((SDC_wrSPI(0xFF) != 0xFF) || (timeout-- > 0))
	{
		SDC_delayms(1);
	}
	return (timeout) ? 0 : 1;
}
/*--------------------------------------------------------------------------
 * Brief: send command to SDC/MMC
 * Param:	cmd	|	I	|	8-bits command
 * 			arg	|	I	|	32-bits argument
 * ret:	0	|	OK
 * 		>0	|	FAIL
 ---------------------------------------------------------------------------*/
U8 SDC_sendCMD(U8 cmd, U32 arg, U32 timeout, U8 *resp)
{
	if (cmd & 0x80)
	{
		/* ACMD<n> is the command sequence of CMD55-CMD<n> */
		cmd &= 0x7F;
		return SDC_sendCMD(CMD55, 0, timeout, resp);
	}
	/* Re-Select the card and wait until ready */
	SDC_wrtCSpin(1);
	SDC_wrtCSpin(0);
	if (SDC_waitRdy(500))
	{
		return 1;
	}
	/* Send command */
	SDC_wrSPI(cmd); /* Start + Command index */
	/* Send argument */
	SDC_wrSPI((U8)(arg >> 24)); /* Argument[31..24] */
	SDC_wrSPI((U8)(arg >> 16)); /* Argument[23..16] */
	SDC_wrSPI((U8)(arg >> 8)); /* Argument[15..8] */
	SDC_wrSPI((U8)arg); /* Argument[7..0] */
	/* Send CRC */
	SDC_wrSPI((cmd == CMD0) ? CMD0_CRC : ((cmd == CMD8) ? CMD8_CRC : DUMMY_CRC));
	/* Receive command response */
	if(cmd == CMD12)
	{
		/* Skip a stuff byte when stop reading */
		SDC_wrSPI(0xFF);
	}
	/* Wait for a valid response or timeout */
	while (timeout-- > 0)
	{
		*resp = SDC_wrSPI(0xFF);
		if (!(*resp & 0x80))
		{
			return 0;
		}
		SDC_delayms(1);
	}
	return 2;
}
/*--------------------------------------------------------------------------
 * Brief: Initial SDC/MMC
 * ret:	0	|	OK
 * 		>0	|	FAIL
 ---------------------------------------------------------------------------*/
U8 SDC_ini(void)
{
	/* Keep CS high */
	SDC_wrtCSpin(1);
	/* >= 74 dummy clocks: 8 x 10 = 80 clocks */
	for (U8 i = 0; i < 10; i++)
	{
		SDC_wrSPI(0xFF);
	}
}
