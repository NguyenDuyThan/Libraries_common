/*
 ******************************************************************************
 * @author  Duy Vinh To - DinhViSo HW team
 * @brief	communicate with RFID reader "AT-COM" ( ISO15693 )
 ******************************************************************************
 * @attention:
 ******************************************************************************
 */

/*** INCLUDE ********************************************************************/
#include "DVS_RFIDreader.h"
#include "stdio.h"
#include "string.h"
/*** DEFINITIONS ****************************************************************/
#define DATRDY_CODE_DRVINFO	0x0001
#define DATRDY_CODE_VER		0x0002
#define DATRDY_CODE_LED		0x0008

/*** TYPEDEFS *******************************************************************/

/*** FUNCTION PROTOTYPES ********************************************************/
static void DVSRFID_strsplit(const U8* str, U8 chr, U8* out_head, U16 head_length, U8* out_tail, U16 tail_length);
static void DVSRFID_clrBuf( void );
static void DVSRFID_printc_pseudo(U8 c);
/*** VARIABLES ******************************************************************/
static U8 DVSRFID_buf[DVSRFID_BUF_SIZE];
static U16 DVSRFID_bufIndex = 0;
static U8 dataRdy = 0;

static DVSRFID_printc_CB DVSRFID_printc = DVSRFID_printc_pseudo;
static DVSRFID_delay10ms_CB DVSRFID_delay10ms;

/*** FUNCTIONS ******************************************************************/
/*-----------------------------------------------------------------------------------------
 * brief: string split
 * param[in]:const char, char, unsigned int, unsigned int
 * param[out]:char*,char*
 * retval:none
 *
 * details: use a character to split a string
 * Note: out_head and out_tail are protected (from overflow)
 * 			by head_length and tail_length (including NULL character at end of the string)
 * Example:
 * QL70_strsplit("AB C D", ' ', heads, 32, tails, 32);
 * Result: heads = "AB", tails = "C D"
 ------------------------------------------------------------------------------------------*/
void DVSRFID_strsplit(const U8* str, U8 chr, U8* out_head, U16 head_length, U8* out_tail, U16 tail_length)
{
	U16 pos, pos_head = 0, pos_tail = 0, len = strlen(str);
	U8 flag_found = 0;

	if (len != 0)
	{
		for (pos = 0; pos < len; pos++)
		{
			if (flag_found == 1)
			{
				if (tail_length)
				{
					if (pos_tail <= (tail_length - 1))
					{
						out_tail[pos_tail++] = str[pos];
					}
				}
			}
			if ((str[pos] == chr) && (flag_found == 0))
			{
				flag_found = 1;
			}
			if ((str[pos] != chr) && (flag_found == 0))
			{
				if (head_length)
				{
					if (pos_head <= (head_length - 1))
					{
						out_head[pos_head++] = str[pos];
					}
				}
			}
		}
	}
	out_tail[pos_tail] = 0;
	out_head[pos_head] = 0;
}
/*--------------------------------------------------------------------------
 * Brief: pseudo function: do nothing
 ---------------------------------------------------------------------------*/
void DVSRFID_printc_pseudo(U8 c)
{
	return;
}
/*--------------------------------------------------------------------------
 * Brief: print a string to DVS RFID reader
 ---------------------------------------------------------------------------*/
void DVSRFID_prints(U16 len, const U8 *s)
{
	for (U16 i = 0; i < len; i++)
	{
		DVSRFID_printc(s[i]);
	}
}
/*--------------------------------------------------------------------------
 * Brief: setup callback functions
 * Param:	printc_cb		|	I	|	print character function
 * 			delay10ms_cb	|	I	|	delay 10 ms function
 ---------------------------------------------------------------------------*/
void DVSRFID_setup(DVSRFID_printc_CB printc_cb, DVSRFID_delay10ms_CB delay10ms_cb)
{
	if (printc_cb == NULL)
	{
		DVSRFID_printc = DVSRFID_printc_pseudo;
	}
	else
	{
		DVSRFID_printc = printc_cb;
	}
	DVSRFID_delay10ms = delay10ms_cb;
	DVSRFID_clrBuf();
}

/*--------------------------------------------------------------------------
 * Brief: Clear Rx buffer
 ---------------------------------------------------------------------------*/
void DVSRFID_clrBuf( void )
{
	memset(DVSRFID_buf, 0, DVSRFID_BUF_SIZE);
	DVSRFID_bufIndex = 0;
	dataRdy = 0;
}
/*--------------------------------------------------------------------------
 * Brief: Read a character from (UART) port
 * Param:	c	|	I	|	character
 * Note: User should call this function to handle Rx data of (UART) port
 ---------------------------------------------------------------------------*/
void DVSRFID_readc(U8 c)
{
	if (DVSRFID_bufIndex < DVSRFID_BUF_SIZE)
	{
		if (dataRdy == 0)
		{
			if ((c == '\r') || (c == '\n'))
			{
				if ((strncmp(DVSRFID_buf, "$DVSRFID,RDY,", strlen("$DVSRFID,RDY,")) == 0) //
						|| (strncmp(DVSRFID_buf, "$DVSRFID,READ,", strlen("$DVSRFID,READ,")) == 0))
				{
					dataRdy |= DATRDY_CODE_DRVINFO;
				}
				else if (strncmp(DVSRFID_buf, "$DVSRFID,VER,", strlen("$DVSRFID,VER,")) == 0)
				{
					dataRdy |= DATRDY_CODE_VER;
				}
				else if (strncmp(DVSRFID_buf, "$DVSRFID,LED", strlen("$DVSRFID,LED")) == 0)
				{
					dataRdy |= DATRDY_CODE_LED;
				}
			}
			else
			{
				DVSRFID_buf[DVSRFID_bufIndex++] = c;
			}
		}
	}
}
/*--------------------------------------------------------------------------
 * Brief: Read full driver's information that was received from RFID reader
 * Param:	licenseSize	|	I	|	license string max size.
 * 			license		|	O	|	license string pointer.
 * 			nameSize	|	I	|	name string max size.
 * 			name		|	O	|	name string pointer.
 * 			uidSize		|	I	|	UID string max size.
 * 			uid			|	O	|	UID string pointer.
 * Ret:	0	|	OK
 * 		>0	|	FAIL / No information
 ---------------------------------------------------------------------------*/
U8 DVSRFID_readDrvInfo(U16 licenseSize, U8 *license, U16 nameSize, U8 *name, U16 uidSize, U8 *uid)
{
	const U8 TIMEOUT = 20;
	U8 tmpS[96];
	U8 i = 0;

	DVSRFID_clrBuf();
	DVSRFID_prints(strlen("$DVSRFID,READ\r"), "$DVSRFID,READ\r");
	for (i = 0; i < TIMEOUT; i++)
	{
		if (dataRdy & DATRDY_CODE_DRVINFO)
		{
			break;
		}
		DVSRFID_delay10ms(10);
	}
	if (i >= TIMEOUT)
	{
		return 2;
	}
	for (i = 0; i <= 4; i++)
	{
		memset(tmpS, 0, 96);
		DVSRFID_strsplit(DVSRFID_buf, ',', tmpS, 96, DVSRFID_buf, DVSRFID_BUF_SIZE);
		switch( i )
		{
			case 0:
				break;
			case 1:
				break;
			case 2:
				strlcpy(license, tmpS, licenseSize);
				break;
			case 3:
				strlcpy(name, tmpS, nameSize);
				break;
			case 4:
				strlcpy(uid, tmpS, uidSize);
				if (strlen(uid))
				{
					return 0;
				}
				break;
			default:
				break;
		}
	}
//	DVSRFID_clrBuf();
	return 4;
}
/*--------------------------------------------------------------------------
 * Brief: Read RFID reader version
 * Param:	vLen	|	I	|	version string max size.
 * 			ver		|	O	|	version string pointer.
 * Ret:	0	|	OK
 * 		>0	|	FAIL / No information
 ---------------------------------------------------------------------------*/
U8 DVSRFID_getVer(  U16 vLen, U8 *ver )
{
	U8 tmpS[96];
	U8 i = 0;


	DVSRFID_clrBuf();
	DVSRFID_prints(1, "\r");
	DVSRFID_prints(strlen("$DVSRFID,VER\r"), "$DVSRFID,VER\r");
	for (i = 0; i < 10; i++)
	{
		if(dataRdy & DATRDY_CODE_VER)
		{
			break;
		}
		DVSRFID_delay10ms(10);
	}
	if(i >= 10)
	{
		return 1;
	}
	for (i = 0; i <= 255; i++)
	{
		DVSRFID_strsplit(DVSRFID_buf, ',', tmpS, 96, DVSRFID_buf, DVSRFID_BUF_SIZE);
		switch (i)
		{
			case 0:
				break;
			case 1:
				break;
			case 2:
				if (vLen)
				{
					strlcpy(ver, tmpS, vLen);
				}
				return 0;
		}
	}
	return 1;
}
/*--------------------------------------------------------------------------
 * Brief: Set/write LED on RFID reader.
 * Param:	LEDindex	|	I	|	LED index. View 'DVSRFID_LEDINDEX_t' typedef for more details.
 * 			LEDval		|	I	|	LED value: 1->On; 0->Off
 * Ret:	0	|	OK
 * 		>0	|	FAIL
 ---------------------------------------------------------------------------*/
U8 DVSRFID_setLED( DVSRFID_LEDINDEX_t LEDindex, U8 LEDval )
{
	const U8 TIMEOUT = 20;
	U8 tmpS[96], i = 0;


	DVSRFID_clrBuf();
	snprintf(tmpS, 96, "$DVSRFID,LED,%u%u\r", LEDindex, LEDval);
	DVSRFID_prints(1, "\r");
	DVSRFID_prints(strlen(tmpS), tmpS);
	for (i = 0; i < TIMEOUT; i++)
	{
		if(dataRdy & DATRDY_CODE_LED)
		{
			break;
		}
		DVSRFID_delay10ms(10);
	}
	if (i >= TIMEOUT)
	{
		return 2;
	}
	return 0;
}
