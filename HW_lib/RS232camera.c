/*
 * RS232camera.c
 *
 *  Created on: Mar 24, 2017
 *      Author: dv198
 */

/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/
#include "RS232camera.h"
#include "string.h"
#include "dbgPrint.h"
/*-----------------------------------------------------------------------------------------
 * DEFINE
 ------------------------------------------------------------------------------------------*/
#define RS232CAM_ID	0
/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
static void RS232CAM_dbgPrt_pseudo(const U8 *s, ...);
static void RS232CAM_sdBytes(U16 byteNum, const U8 *bytes);
static U8 RS232CAM_buf_rdByte(U8 *byte);
static U16 RS232CAM_buf_rdBytes(U16 timeout, U16 byteNum, U8 *bytes);
static void RS232CAM_buf_clr(void);
static U8 RS232CAM_waitACK(U16 timeout, U8 *cmdByte);

/*-----------------------------------------------------------------------------------------
 * VARIABLES
 ------------------------------------------------------------------------------------------*/
static U16 RS232CAM_bufSize = 0;
static U8 *RS232CAM_buf = NULL;
static U16 RS232CAM_bufwrtIndex = 0; // write index in buffer.
static U16 RS232CAM_bufrdIndex = 0; // read index in buffer.
static U16 RS232CAM_bufFill = 0;

static RS232CAM_dbgPrt_CB_t RS232CAM_dbgPrt = RS232CAM_dbgPrt_pseudo;
static RS232CAM_sendByte_CB_t RS232CAM_sendByte = NULL;
static RS232CAM_delay10ms_CB_t RS232CAM_delay10ms = NULL;

/*-----------------------------------------------------------------------------------------
 * FUNCTIONS
 ------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------
 * Brief: Just a pseudo debug printing function.
 *
 ---------------------------------------------------------------------------*/
void RS232CAM_dbgPrt_pseudo(const U8 *s, ...)
{
	return;
}
/*--------------------------------------------------------------------------
 * Brief: Setup RS232 camera driver 'debug printing' callback function.
 * Param:	cb		|	IN	|	'debug printing' callback function.
 *
 ---------------------------------------------------------------------------*/
void RS232CAM_setup_dbgPrt(RS232CAM_dbgPrt_CB_t cb)
{
	if (cb == NULL)
	{
		RS232CAM_dbgPrt = RS232CAM_dbgPrt_pseudo;
		return;
	}
	RS232CAM_dbgPrt = cb;
}
/*--------------------------------------------------------------------------
 * Brief: Setup RS232 camera driver with below parameters:
 * Param:	bufSize		|	IN	|	buffer size.
 * 			bufPtr		|	IN	|	pointer to buffer.
 * 			sendByteCB	|	IN	|	'send a byte to camera' callback function.
 * 			delay10msCB	|	IN	|	'delay (n)x10ms' callback function.
 *
 ---------------------------------------------------------------------------*/
void RS232CAM_setup(U16 bufSize, U8 *bufPtr, RS232CAM_sendByte_CB_t sendByteCB, RS232CAM_delay10ms_CB_t delay10msCB)
{
	RS232CAM_bufSize = bufSize;
	RS232CAM_buf = bufPtr;
	RS232CAM_sendByte = sendByteCB;
	RS232CAM_delay10ms = delay10msCB;
	RS232CAM_buf_clr();
}
/*--------------------------------------------------------------------------
 * Brief: Write a byte to buffer
 * Param:	c	|	I	|	character
 * Note: User should call this function to handle Rx data of (UART) port
 ---------------------------------------------------------------------------*/
void RS232CAM_buf_wrtByte(U8 byte)
{
	if (RS232CAM_buf == NULL)
	{
		return;
	}
	if (RS232CAM_bufwrtIndex < RS232CAM_bufSize)
	{
		RS232CAM_buf[RS232CAM_bufwrtIndex++] = byte;
	}
}
#if 0
/*--------------------------------------------------------------------------
 * Brief: Read 1 byte from buffer.
 * Ret:	0xFF		|	no byte to read.
 * 		other value	|	byte value.
 ---------------------------------------------------------------------------*/
U8 RS232CAM_buf_rdByte(U8 *byte)
{
	if (RS232CAM_buf == NULL)
	{
		return 0xFF;
	}
	if (RS232CAM_bufFill > 0)
	{
		*byte = RS232CAM_buf[RS232CAM_bufrdIndex++];
		RS232CAM_bufFill--;
		return 0;
	}
	return 1;
}
#endif
/*--------------------------------------------------------------------------
 * Brief: Read bytes from buffer.
 * Ret:	number of read bytes.
 ---------------------------------------------------------------------------*/
U16 RS232CAM_buf_rdBytes(U16 timeout, U16 byteNum, U8 *bytes)
{
	U16 trueByteNum = 0;

	if (RS232CAM_buf == NULL)
	{
		return 0;
	}
	for (U16 t = 0; t < timeout; t++)
	{
		if (RS232CAM_bufwrtIndex >= (RS232CAM_bufrdIndex + byteNum))
		{
			break;
		}
		RS232CAM_delay10ms(1);
	}
	trueByteNum = RS232CAM_bufwrtIndex - RS232CAM_bufrdIndex;
	memcpy(bytes, &RS232CAM_buf[RS232CAM_bufrdIndex], (trueByteNum < byteNum) ? trueByteNum : byteNum);
	RS232CAM_bufrdIndex += (trueByteNum < byteNum) ? trueByteNum : byteNum;
	//RS232CAM_dbgPrt("\r\n %s:ByteNum=%u/%u RI=%u WI=%u", __func__, trueByteNum, byteNum, RS232CAM_bufrdIndex, RS232CAM_bufwrtIndex);
	return (trueByteNum < byteNum) ? trueByteNum : byteNum;
}
/*--------------------------------------------------------------------------
 * Brief: send bytes
 * Param:	byteNum	|	IN	|	number of bytes.
 * 			bytes	|	IN	|	bytes to send.
 ---------------------------------------------------------------------------*/
void RS232CAM_sdBytes(U16 byteNum, const U8 *bytes)
{
	for (U16 i = 0; i < byteNum; i++)
	{
		RS232CAM_sendByte(bytes[i]);
	}
}
/*--------------------------------------------------------------------------
 * Brief: clear buffer
 ---------------------------------------------------------------------------*/
void RS232CAM_buf_clr(void)
{
	RS232CAM_bufwrtIndex = 0;
	RS232CAM_bufrdIndex = 0;
	RS232CAM_bufFill = 0;
	memset(RS232CAM_buf, 0, RS232CAM_bufSize);
}
/*--------------------------------------------------------------------------
 * Brief: clear buffer.
 * Param:	timeout		|	IN	| 	timeout. Unit: millisecond.
 * 			cmdByte		|	OUT	|	received command byte.
 * Ret:	0	|	OK.
 * 		1	|	timeout.
 * 		2	|	wrong ACK.
 ---------------------------------------------------------------------------*/
U8 RS232CAM_waitACK(U16 timeout, U8 *cmdByte)
{
	U8 fb[4];

	if (RS232CAM_buf_rdBytes(timeout, 4, fb) < 4)
	{
		return 1;
	}
	if ((fb[0] != 'U')//
			//|| (fb[2] != 0)//
			|| (fb[3] != '#')//
			)
	{
		return 2;
	}
	if (cmdByte != NULL)
	{
		*cmdByte = fb[1];
	}
	return 0;
}
/*--------------------------------------------------------------------------
 * Brief: Take a photo
 * Param:	res			|	IN	|	camera resolution.
 * 			pSize		|	IN	|	max packet size. packet size must be smaller than buffer size minus 8 (header length).
 * 			timeout		|	IN	|	timeout to wait until photo is taken. Unit: millisecond
 * 			photoSize	|	OUT	|	photo size.
 * 			pNum		|	OUT	|	Number of packets.
 * Ret:	0	|	OK
 * 		>0	|	FAIL
 ---------------------------------------------------------------------------*/
U8 RS232CAM_takePhoto(IMG_RES_t res, U16 pSize, U16 timeout, U32 *photoSize, U16 *pNum)
{
	U8 cmd[7];
	U8 fb[10];

	/* Create command to request camera takes a photo */
	cmd[0] = 'U';
	cmd[1] = 'H';
	cmd[2] = RS232CAM_ID;
	switch (res)
	{
		case IMGRES_MEDIUM:
			cmd[3] = '2';
			break;
		case IMGRES_LARGE:
			cmd[3] = '3';
			break;
		default:
			cmd[3] = '1'; // Default is small.
			break;
	}
	cmd[4] = pSize & 0xFF;
	cmd[5] = (pSize >> 8) & 0xFF;
	cmd[6] = '#';
	/* Clear buffer, ready to receive ACK and result */
	RS232CAM_buf_clr();
	/* Send command */
	RS232CAM_sdBytes(7, cmd);
	/* Wait for ACK */
	if (RS232CAM_waitACK(timeout, NULL))
	{
		return 1;
	}
	/* Wait for reading result */
	if (RS232CAM_buf_rdBytes(timeout, 10, fb) < 10)
	{
		return 2;
	}
	/* check the result */
	if( (fb[0] != 'U')//
			|| (fb[1] != 'R')//
			//|| (fb[2] != RS232CAM_ID)//
			|| (fb[9] != '#') )
	{
		return 3;
	}
	*photoSize = fb[3] | (fb[4] << 8) | (fb[5] << 16) | (fb[6] << 24);
	*pNum = fb[7] | (fb[8] << 8);
	return 0;
}
/*--------------------------------------------------------------------------
 * Brief: Read a packet (part) of image.
 * Param:	timeout		|	IN	| 	timeout. Unit: millisecond.
 * 			pID			|	IN	|	packet ID/ packet index.
 * 			pDatMaxSize	|	IN	|	packet data max size.
 * 			pDat		|	OUT	|	packet data pointer.
 * 			pSize		|	OUT	|	true packet size.
 * Ret:	0	|	OK.
 * 		>0	|	FAIL.
 ---------------------------------------------------------------------------*/
U8 RS232CAM_rdPhotoPkt(U16 timeout, U16 pID, U16 pDatMaxSize, U8 *pDat, U16 *pSize)
{
	U8 cmd[6], fb[7];
	U8 commandByte = 0;
	U16 chksum = 0, pTrueSize = 0;

	/* Generate command */
	cmd[0] = 'U';
	cmd[1] = 'E';
	cmd[2] = RS232CAM_ID;
	cmd[3] = pID & 0xFF;
	cmd[4] = (pID >> 8) & 0xFF;
	cmd[5] = '#';
	/* Clear buffer, ready to receive ACK */
	RS232CAM_buf_clr();
	/* Send command */
	RS232CAM_sdBytes(6, cmd);
	/* Wait for ACK */
	if (RS232CAM_waitACK(timeout, &commandByte))
	{
		return 1;
	}
	if (commandByte != 'E')
	{
		return 2;
	}
	/* Wait for reading packet header */
	if (RS232CAM_buf_rdBytes(timeout, 7, fb) < 7)
	{
		return 3;
	}
	/* Verify and parse header */
	/* 3 bytes fixed header */
	if ((fb[0] != 'U') //
			|| (fb[1] != 'F')//
			//|| (fb[2] != 0)//
			)
	{
		return 4;
	}
	/* Next is 2 bytes packet ID */
	if ((fb[3] | (fb[4] << 8)) != pID)
	{
		return 5;
	}
	/* Next is 1 or 2 bytes packet size */
	pTrueSize = fb[5] | (fb[6] << 8);
	if (pSize != NULL)
	{
		*pSize = pTrueSize;
	}
	//RS232CAM_dbgPrt("\r\n %s:pTrueSize=%u BufFill=%u", __func__, pTrueSize, RS232CAM_bufFill);
	/* Summarize header bytes value to checksum */
	for (U8 i = 0; i < 7; i++)
	{
		chksum += fb[i];
	}
	/* Wait for reading packet data */
	if (RS232CAM_buf_rdBytes(timeout, pTrueSize, pDat) < pTrueSize)
	{
		return 6;
	}
	//RS232CAM_dbgPrt("\r\n %s:AR:pTrueSize=%u", __func__, pTrueSize);
	/* Summarize packet data value to checksum */
	for (U16 i = 0; i < pTrueSize; i++)
	{
		chksum += pDat[i];
	}
	/* Wait for 2 bytes checksum */
	if (RS232CAM_buf_rdBytes(timeout, 2, fb) < 2)
	{
		//return 7;
	}
	/* Final is 2 bytes checksum */
	if ((fb[0] | (fb[1] << 8)) != chksum)
	{
		return 8;
	}
	return 0;
}
