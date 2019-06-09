/* ******************************************************************
 * name CLCD_driver.c
 *
 * author: Duy Vinh
 *
 * description: provide function to work with LCD16x2
 * ******************************************************************/

/*----------------------------------------------------------------------------
 * INCLUDE
 -----------------------------------------------------------------------------*/
#include "CLCD.h"

/*----------------------------------------------------------------------------
 * DEFINES
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * VARIABLES
 -----------------------------------------------------------------------------*/
static U8 CLCD_rdyFlg = 0;
static U32 timeoutReplace4BFwait = 0;

static CLCD_delay10us_CB CLCD_delay10us;
static CLCD_wrtCTRLpins_CB CLCD_wrtCTRLpins;
static CLCD_wrt4DATpins_CB CLCD_wrt4DATpins;
static CLCD_rd4DATpins_CB CLCD_rd4DATpins;
/*----------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 -----------------------------------------------------------------------------*/
static void CLCD_EntryModeSet( U8 EMode );
static void CLCD_FunctionSet( U8 useFont5x10 );
static void CLCD_wrtCmd( U8 Cmd );
static void CLCD_SetDDRamAddress( U8 DDrAddr );
static void CLCD_SetCGRamAddress( U8 CGrAddr );
static U8 CLCD_BFwait( void );
static U8 CLCD_rdDat( void );
static void CLCD_CursorHome( void );
static void CLCD_DisplayControl( U8 Dc, U8 Cc, U8 Bc );

/*----------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 -----------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
 * Brief: initial software (Mount callback functions)
 * Param: timeoutWoBFwait	|	IN	|	timeout wait BF
 * 			delayusCB		|	IN	|
 * 			wrtCTRLpinsCB	|	IN	|
 * 			wrt4DATpinsCB	|	IN	|
 * 			rd4DATpinsCB	|	IN	|
 ----------------------------------------------------------------------------*/
void CLCD_setup(U32 timeoutWoBFwait//
		, CLCD_delay10us_CB delay10usCB//
		, CLCD_wrtCTRLpins_CB wrtCTRLpinsCB//
		, CLCD_wrt4DATpins_CB wrt4DATpinsCB//
		, CLCD_rd4DATpins_CB rd4DATpinsCB)
{
	timeoutReplace4BFwait = timeoutWoBFwait;
	CLCD_delay10us = delay10usCB;
	CLCD_wrtCTRLpins = wrtCTRLpinsCB;
	CLCD_wrt4DATpins = wrt4DATpinsCB;
	CLCD_rd4DATpins = rd4DATpinsCB;
}
///*--------------------------------------------------------------------------------------
// *
// ---------------------------------------------------------------------------------------*/
//void CLCD_wrt4DATpins(U8 val)
//{
//	CLCD_wrtDATpin(4, (val) & 0x1);
//	CLCD_wrtDATpin(5, (val >> 1) & 0x1);
//	CLCD_wrtDATpin(6, (val >> 2) & 0x1);
//	CLCD_wrtDATpin(7, (val >> 3) & 0x1);
//}
/*
 * *********************************************************
 * Brief: 	wait until LCD finish processing previous command.
 * 			And receive current address
 * Ret:		0xFF	|	Error
 * *********************************************************
 */
U8 CLCD_BFwait( void )
{
	if (!timeoutReplace4BFwait)
	{
		U32 timeout = 0;
		U8 status = 1;
		U8 addr = 0;

		//CLCD_rd4DATpins();
		CLCD_wrtCTRLpins(0, 1, 1);
		while (timeout++ < 10)
		{
			status = CLCD_rd4DATpins() >> 3;
			if (!status)
			{
				addr = CLCD_rd4DATpins();
				addr <<= 4;
				CLCD_wrtCTRLpins(0, 1, 0);
				CLCD_wrtCTRLpins(0, 1, 1);
				addr |= CLCD_rd4DATpins();
				CLCD_wrtCTRLpins(0, 1, 0);
				CLCD_rdyFlg = 1;
				break;
			}
			CLCD_delay10us(20);
		}
		CLCD_wrt4DATpins(0);
		if (status)
		{
			addr = 0xFF; //Address to nowhere
			CLCD_rdyFlg = 0;
		}
		return addr;
	}
	else
	{
		CLCD_delay10us(timeoutReplace4BFwait);
		return 0;
	}
}
/*
 * *********************************************
 * write a commande to LCD
 * *********************************************
 */
void CLCD_wrtCmd( U8 Cmd )
{
	if( CLCD_BFwait() == 0xFF )
	{
		return;
	}
	CLCD_wrtCTRLpins(0, 0, 1);
	CLCD_wrt4DATpins(Cmd >> 4);
	CLCD_wrtCTRLpins(0, 0, 0);
	CLCD_wrtCTRLpins(0, 0, 1);
	CLCD_wrt4DATpins(Cmd);
	CLCD_wrtCTRLpins(0, 0, 0);
	CLCD_BFwait();
}
/*
 * *********************************************
 * write a data (character) to LCD
 * *********************************************
 */
void CLCD_wrtDats(U8 len, const U8 *s)
{
	for (U8 i = 0; i < len; i++)
	{
		CLCD_wrtDat(s[i]);
	}
}
/*
 * *********************************************
 * write a data (character) to LCD
 * *********************************************
 */
void CLCD_wrtDat( U8 dat )
{
	if( CLCD_BFwait() == 0xFF )
	{
		return;
	}
	CLCD_wrtCTRLpins(1, 0, 1);
	CLCD_wrt4DATpins(dat >> 4);
	CLCD_wrtCTRLpins(1, 0, 0);
	CLCD_wrtCTRLpins(1, 0, 1);
	CLCD_wrt4DATpins(dat);
	CLCD_wrtCTRLpins(1, 0, 0);
	CLCD_BFwait();
}
/************************************************************
 * @Brief:		CLCD_ReadData
 * @Param[in]:
 * @Param[out]:
 * @ret:
 *
 * @Details:
 ************************************************************/
U8 CLCD_rdDat( void )
{
	U8 data = 0;

	if( CLCD_BFwait() == 0xFF )
	{
		return 0xFF;
	}
	//CLCD_initDATpins(1);
	CLCD_wrtCTRLpins(1, 1, 1);
	data = CLCD_rd4DATpins();
	data <<= 4;
	CLCD_wrtCTRLpins(1, 1, 0);
	CLCD_wrtCTRLpins(1, 1, 1);
	data |= CLCD_rd4DATpins();
	CLCD_wrtCTRLpins(1, 1, 0);
	//CLCD_initDATpins(0);
	CLCD_BFwait();
	return data;
}
/*
 * *************************************************
 * Clear all LCD display
 * put cursor on first pos of first line
 * *************************************************
 */
void CLCD_clrDisp( void )
{
	U8 cmd = 0x1;
	CLCD_wrtCmd(cmd);
}

/*
 * *************************************************
 * Clear one line of LCD
 * put cursor on first pos of line
 *
 * EntryMode auto change to Emode1
 * user should set EntryMode again as you like
 * *************************************************
 */
void CLCD_clrLine( U8 Y )
{
	U8 pos = 0;

	CLCD_EntryModeSet(EMode1);
	CLCD_SetDDRamAddress(ORIGNADDR_LINE(Y));
	for( pos = 0; pos < LINE_SIZE; pos++ )
	{
		CLCD_wrtDat(' ');
	}
	CLCD_SetDDRamAddress(ORIGNADDR_LINE(Y));
}

/*
 * *********************************************
 * move cursor to 1st pos of 1st line
 * *********************************************
 */
void CLCD_CursorHome( void )
{
	U8 cmd = 0x2;
	CLCD_wrtCmd(cmd);
}

/*
 * *******************************************************
 * set font size 5x8 or 5x10
 * *******************************************************
 */
void CLCD_FunctionSet( U8 useFont5x10 )
{
	U8 cmd = 0x28;
	if( useFont5x10 )
	{
		cmd = cmd | 0x4;
	}
	CLCD_wrtCmd(cmd);
}

/*
 * *******************************************************
 * set display on/off control:
 * 		Dc: display screen
 * 		Cc: cursor
 * 		Bc: blinking cursor
 * *******************************************************
 */
void CLCD_DisplayControl( U8 Dc, U8 Cc, U8 Bc )
{
	U8 cmd = 0x8;
	if( Dc == 1 )
	{
		cmd = cmd | 0x4;
	}
	if( Cc == 1 )
	{
		cmd = cmd | 0x2;
	}
	if( Bc == 1 )
	{
		cmd = cmd | 0x1;
	}
	CLCD_wrtCmd(cmd);
}

/*
 * *******************************************************
 * set EntryMode:
 * 		Emode1:
 * 		Emode2:
 * 		Emode3:
 * 		Emode4:
 * *******************************************************
 */
void CLCD_EntryModeSet( U8 EMode )
{
	U8 cmd = 0x4;
	switch( EMode )
	{
		case EMode1:
			cmd = cmd | 0x2;
			break;
		case EMode2:
			break;
		case EMode3:
			cmd = cmd | 0x3;
			break;
		case EMode4:
			cmd = cmd | 0x1;
			break;
		default:
			cmd = cmd | 0x2;
			break;
	}
	CLCD_wrtCmd(cmd);
}

/*
 * ***********************************************
 * set DDRam address - set position of cursor
 * ***********************************************
 */
void CLCD_SetDDRamAddress( U8 DDrAddr )
{
	CLCD_wrtCmd(DDrAddr | 0x80);
}
/*
 * ***********************************************
 * set CGRam address\
 * ***********************************************
 */
void CLCD_SetCGRamAddress( U8 CGrAddr )
{
	CLCD_wrtCmd(CGrAddr | 0x40);
}
/*
 * ***********************************************
 * set cursor position on screen
 *
 * 		X: pos in line
 * 		Y: line
 * ***********************************************
 */
void CLCD_GotoXY( U8 X, U8 Y )
{
	CLCD_SetDDRamAddress(ORIGNADDR_LINE(Y) + X);
}
/************************************************************
 * @Brief:		CLCD_ShiftLineContent
 * @Param[in]:
 * @Param[out]:
 * @ret:
 *
 * @Details:	shift line's content
 ************************************************************/
void CLCD_ShiftLineContent( U8 line, U8 SHIFTLINECNT_MODE, U8 step )
{
	U8 data_arr[LINE_SIZE];
	U8 tmp = 0, tmpDat = 0;

	CLCD_GotoXY(0, line);
	for( tmp = 0; tmp < LINE_SIZE; tmp++ )
	{
		data_arr[tmp] = CLCD_rdDat();
	}
	while( step-- > 0 )
	{
		switch( SHIFTLINECNT_MODE )
		{
			case SHIFTLINECNT_MODE_RIGHT:
				tmpDat = data_arr[LINE_SIZE - 1];
				for( tmp = (LINE_SIZE - 1); tmp >= 1; tmp-- )
				{
					data_arr[tmp] = data_arr[tmp - 1];
				}
				data_arr[0] = tmpDat;
				break;
			case SHIFTLINECNT_MODE_LEFT:
				tmpDat = data_arr[0];
				for( tmp = 0; tmp <= (LINE_SIZE - 2); tmp++ )
				{
					data_arr[tmp] = data_arr[tmp + 1];
				}
				data_arr[LINE_SIZE - 1] = tmpDat;
				break;
			default:
				break;
		}
	}
	CLCD_GotoXY(0, line);
	for( tmp = 0; tmp < LINE_SIZE; tmp++ )
	{
		CLCD_wrtDat(data_arr[tmp]);
	}
}
/*----------------------------------------------
 *
 -----------------------------------------------*/
U8 CLCD_rdy( void )
{
	return CLCD_rdyFlg;
}
/*----------------------------------------------
 *
 -----------------------------------------------*/
U8 CLCD_isAvail(void)
{
	U8 i = 0;
	CLCD_GotoXY(0,0);
	for(i = 0; i < 16; i++)
	{
		CLCD_wrtDat(i + '0');
	}
	for(i = 0; i < 16; i++)
	{
		if( CLCD_rdDat() != (i + '0'))
		{
			return 0;
		}
	}
	return 1;
}
/*
 * *********************************************
 * Startup LCD
 *
 * 		data length 4bit
 * 		font 5x8
 * 		EMode1
 * 		display screen on, cursor on
 * *********************************************
 */
void CLCD_startup( void )
{
	U8 Cmd = 0;

	//DBG_print("\r\n CLCDini:start");
	//Strictly follow these step to initial LCD1602, DO NOT FOLLOW DATASHEET (UNSTABLE)
	CLCD_delay10us(1600);
	CLCD_wrt4DATpins(0x0);
	CLCD_wrtCTRLpins(0, 0, 0);

	Cmd = 0x3;
	CLCD_wrtCTRLpins(0, 0, 1);
	CLCD_wrt4DATpins(Cmd);
	CLCD_wrtCTRLpins(0, 0, 0);
	CLCD_delay10us(500);

	Cmd = 0x3;
	CLCD_wrtCTRLpins(0, 0, 1);
	CLCD_wrt4DATpins(Cmd);
	CLCD_wrtCTRLpins(0, 0, 0);
	CLCD_delay10us(20);

	Cmd = 0x3;
	CLCD_wrtCTRLpins(0, 0, 1);
	CLCD_wrt4DATpins(Cmd);
	CLCD_wrtCTRLpins(0, 0, 0);
	CLCD_delay10us(500);

	Cmd = 0x2;
	CLCD_wrtCTRLpins(0, 0, 1);
	CLCD_wrt4DATpins(Cmd);
	CLCD_wrtCTRLpins(0, 0, 0);
	CLCD_delay10us(10);

	//DBG_print("\r\n CLCDini:setting");
	//Config LCD as you want
	CLCD_FunctionSet(1);
	CLCD_DisplayControl(1, DISPLAY_CURSOR, BLINKING_CURSOR);
	CLCD_clrDisp();
	CLCD_EntryModeSet(EMode1);
	CLCD_CursorHome();
}
/*
 * *********************************************
 *
 * *********************************************
 */
void CLCD_wrtCGRAMdat( U8 CGRAM_Haddr, CGRAM_DAT_t dat )
{
	U8 i = 0;

	CLCD_SetCGRamAddress(CGRAM_Haddr << 3);
	for( i = 0; i < 8; i++ )
	{
		CLCD_wrtDat(dat.line[i]);
	}
}
/*
 * *********************************************
 *
 * *********************************************
 */
void CLCD_loadCGRAM( void )
{
	CLCD_wrtCGRAMdat(CGRAMADDR_0, CGRAM_DAT_0);
	CLCD_wrtCGRAMdat(CGRAMADDR_1, CGRAM_DAT_1);
	CLCD_wrtCGRAMdat(CGRAMADDR_2, CGRAM_DAT_2);
	CLCD_wrtCGRAMdat(CGRAMADDR_3, CGRAM_DAT_3);
	CLCD_wrtCGRAMdat(CGRAMADDR_4, CGRAM_DAT_4);
	CLCD_wrtCGRAMdat(CGRAMADDR_5, CGRAM_DAT_5);
	CLCD_wrtCGRAMdat(CGRAMADDR_6, CGRAM_DAT_6);
	CLCD_wrtCGRAMdat(CGRAMADDR_7, CGRAM_DAT_7_1);
}
/*
 * *********************************************
 * Brief:	write large size character.
 * 			(occupied 2 lines).
 *
 * Param:	X	|	In	|	position in line
 * 			c	|	In	|	character
 * Note: supported following large characters:
 * '0','1','2','3','4','5','6','7','8','9'
 * ':'
 * *********************************************
 */
void CLCD_wrtLargeChar( U8 X, U16 c )
{
	switch( c )
	{
		case '0':
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(CGRAMADDR_0);
			CLCD_GotoXY(X, 1);
			CLCD_wrtDat(CGRAMADDR_0);
			CLCD_GotoXY(X + 1, 0);
			CLCD_wrtDat(CGRAMADDR_4);
			CLCD_GotoXY(X + 1, 1);
			CLCD_wrtDat(CGRAMADDR_3);
			CLCD_GotoXY(X + 2, 0);
			CLCD_wrtDat(CGRAMADDR_1);
			CLCD_GotoXY(X + 2, 1);
			CLCD_wrtDat(CGRAMADDR_1);
			break;
		case '1':
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X, 1);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X + 1, 0);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X + 1, 1);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X + 2, 0);
			CLCD_wrtDat(CGRAMADDR_1);
			CLCD_GotoXY(X + 2, 1);
			CLCD_wrtDat(CGRAMADDR_1);
			break;
		case '2':
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X, 1);
			CLCD_wrtDat(CGRAMADDR_0);
			CLCD_GotoXY(X + 1, 0);
			CLCD_wrtDat(CGRAMADDR_2);
			CLCD_GotoXY(X + 1, 1);
			CLCD_wrtDat(CGRAMADDR_3);
			CLCD_GotoXY(X + 2, 0);
			CLCD_wrtDat(CGRAMADDR_1);
			CLCD_GotoXY(X + 2, 1);
			CLCD_wrtDat(' ');
			break;
		case '3':
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X, 1);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X + 1, 0);
			CLCD_wrtDat(CGRAMADDR_2);
			CLCD_GotoXY(X + 1, 1);
			CLCD_wrtDat(CGRAMADDR_3);
			CLCD_GotoXY(X + 2, 0);
			CLCD_wrtDat(CGRAMADDR_1);
			CLCD_GotoXY(X + 2, 1);
			CLCD_wrtDat(CGRAMADDR_1);
			break;
		case '4':
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(CGRAMADDR_0);
			CLCD_GotoXY(X, 1);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X + 1, 0);
			CLCD_wrtDat(CGRAMADDR_3);
			CLCD_GotoXY(X + 1, 1);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X + 2, 0);
			CLCD_wrtDat(CGRAMADDR_1);
			CLCD_GotoXY(X + 2, 1);
			CLCD_wrtDat(CGRAMADDR_1);
			break;
		case '5':
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(CGRAMADDR_0);
			CLCD_GotoXY(X, 1);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X + 1, 0);
			CLCD_wrtDat(CGRAMADDR_2);
			CLCD_GotoXY(X + 1, 1);
			CLCD_wrtDat(CGRAMADDR_3);
			CLCD_GotoXY(X + 2, 0);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X + 2, 1);
			CLCD_wrtDat(CGRAMADDR_1);
			break;
		case '6':
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(CGRAMADDR_0);
			CLCD_GotoXY(X, 1);
			CLCD_wrtDat(CGRAMADDR_0);
			CLCD_GotoXY(X + 1, 0);
			CLCD_wrtDat(CGRAMADDR_2);
			CLCD_GotoXY(X + 1, 1);
			CLCD_wrtDat(CGRAMADDR_3);
			CLCD_GotoXY(X + 2, 0);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X + 2, 1);
			CLCD_wrtDat(CGRAMADDR_1);
			break;
		case '7':
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X, 1);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X + 1, 0);
			CLCD_wrtDat(CGRAMADDR_4);
			CLCD_GotoXY(X + 1, 1);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X + 2, 0);
			CLCD_wrtDat(CGRAMADDR_1);
			CLCD_GotoXY(X + 2, 1);
			CLCD_wrtDat(CGRAMADDR_1);
			break;
		case '8':
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(CGRAMADDR_0);
			CLCD_GotoXY(X, 1);
			CLCD_wrtDat(CGRAMADDR_0);
			CLCD_GotoXY(X + 1, 0);
			CLCD_wrtDat(CGRAMADDR_2);
			CLCD_GotoXY(X + 1, 1);
			CLCD_wrtDat(CGRAMADDR_3);
			CLCD_GotoXY(X + 2, 0);
			CLCD_wrtDat(CGRAMADDR_1);
			CLCD_GotoXY(X + 2, 1);
			CLCD_wrtDat(CGRAMADDR_1);
			break;
		case '9':
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(CGRAMADDR_0);
			CLCD_GotoXY(X, 1);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X + 1, 0);
			CLCD_wrtDat(CGRAMADDR_2);
			CLCD_GotoXY(X + 1, 1);
			CLCD_wrtDat(CGRAMADDR_3);
			CLCD_GotoXY(X + 2, 0);
			CLCD_wrtDat(CGRAMADDR_1);
			CLCD_GotoXY(X + 2, 1);
			CLCD_wrtDat(CGRAMADDR_1);
			break;
		case ':':
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(CGRAMADDR_6);
			CLCD_GotoXY(X, 1);
			CLCD_wrtDat(CGRAMADDR_6);
			break;
		case LCHAR_CELCIUS:
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(CGRAMADDR_6);
			CLCD_GotoXY(X, 1);
			CLCD_wrtDat(' ');
			//
			CLCD_GotoXY(X + 1, 0);
			CLCD_wrtDat(CGRAMADDR_0);
			CLCD_GotoXY(X + 1, 1);
			CLCD_wrtDat(CGRAMADDR_0);
			//
			CLCD_GotoXY(X + 2, 0);
			CLCD_wrtDat(CGRAMADDR_4);
			CLCD_GotoXY(X + 2, 1);
			CLCD_wrtDat(CGRAMADDR_3);
			//
			CLCD_GotoXY(X + 3, 0);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X + 3, 1);
			CLCD_wrtDat(' ');
			break;
		case LCHAR_HUMIDITY:
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(CGRAMADDR_0);
			CLCD_wrtDat(CGRAMADDR_1);
			CLCD_wrtDat(CGRAMADDR_5);
			//
			CLCD_GotoXY(X + 1, 1);
			CLCD_wrtDat(CGRAMADDR_5);
			CLCD_wrtDat(CGRAMADDR_0);
			CLCD_wrtDat(CGRAMADDR_1);
			break;
		default:
			CLCD_GotoXY(X, 0);
			CLCD_wrtDat(' ');
			CLCD_GotoXY(X, 1);
			CLCD_wrtDat(' ');
			break;
	}
}
#if 0
/*
 * *********************************************
 * Show large clock (hh:mm:ss)
 * *********************************************
 */
void CLCD_showLargeClock( U8 Xstart, U8 hour, U8 min, U8 sec )
{
	CLCD_loadCGRAM();
	CLCD_wrtLargeChar(Xstart + 0, lib_numb2char(hour / 10));
	CLCD_wrtLargeChar(Xstart + 3, lib_numb2char(hour % 10));
	CLCD_wrtLargeChar(Xstart + 6, ':');
	CLCD_wrtLargeChar(Xstart + 7, lib_numb2char(min / 10));
	CLCD_wrtLargeChar(Xstart + 10, lib_numb2char(min % 10));
	//CLCD_wrtLargeChar(9, ':');
	//CLCD_wrtLargeChar(10, lib_numb2char(sec / 10));
	//CLCD_wrtLargeChar(13, lib_numb2char(sec % 10));
}
/*
 * *********************************************
 * Show large temperature
 * *********************************************
 */
void CLCD_showLargeTemp( U8 Xstart, U8 temp )
{
	CLCD_loadCGRAM();
	CLCD_wrtLargeChar(Xstart + 0, lib_numb2char(temp / 10));
	CLCD_wrtLargeChar(Xstart + 3, lib_numb2char(temp % 10));
	CLCD_wrtLargeChar(Xstart + 6, LCHAR_CELCIUS);
}
/*
 * *********************************************
 * Show large humidity
 * *********************************************
 */
void CLCD_showLargeHumi( U8 Xstart, U8 humi )
{
	CLCD_loadCGRAM();
	CLCD_wrtLargeChar(Xstart + 0, lib_numb2char(humi / 10));
	CLCD_wrtLargeChar(Xstart + 3, lib_numb2char(humi % 10));
	CLCD_wrtLargeChar(Xstart + 6, LCHAR_HUMIDITY);
}
#endif
