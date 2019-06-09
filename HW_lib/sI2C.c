/*
 ******************************************************************************
 * @file    sI2C.c
 * @author  Duy Vinh To - DinhViSo HW team
 * @version V1.0
 * @date	20/9/2014
 * @brief	bit bang (software) I2C source file
 ******************************************************************************
 * @attention:
 ******************************************************************************
 */

/*** INCLUDE ********************************************************************/
#include "sI2C.h"

/*** DEFINE & MACROS ************************************************************/

/*** IMPORTED VARIABLES *********************************************************/

/*** VARIABLES ******************************************************************/
static sI2C_wrtSDA_CB sI2C_wrtSDA;
static sI2C_rdSDA_CB sI2C_rdSDA;
static sI2C_wrtSCL_CB sI2C_wrtSCL;
static sI2C_delayus_CB sI2C_delayus;
static U8 sI2C_isBusy = 0;
/*** IMPORTED FUNCTIONS *********************************************************/

/*** FUNCTION PROTOTYPES ********************************************************/
static void sI2C_startup( void );
static void sI2C_genStart( void );
static void sI2C_genStop( void );
static void sI2C_writeSlaveAddr(U8 slaveAddr, U8 isRecv);
static void sI2C_write( U8 dat );
static U8 sI2C_readACK( void );
static U8 sI2C_read( void );
static void sI2C_writeACK( U8 ACKstt );
/*** FUNCTIONS ******************************************************************/

/*------------------------------------------------------------------------------------
 *
 -------------------------------------------------------------------------------------*/
void sI2C_setup(sI2C_wrtSDA_CB wrtSDAcb, sI2C_rdSDA_CB rdSDAcb, sI2C_wrtSCL_CB wrtSCLcb, sI2C_delayus_CB delayuscb)
{
	sI2C_wrtSDA = wrtSDAcb;
	sI2C_rdSDA = rdSDAcb;
	sI2C_wrtSCL = wrtSCLcb;
	sI2C_delayus = delayuscb;
	sI2C_startup();
}

/*************************************************************************************
 * @Brief:
 * @param:
 * @ret:
 *************************************************************************************/
void sI2C_startup( void )
{
	sI2C_wrtSCL(1);
	sI2C_wrtSDA(1);
}

/*************************************************************************************
 * @Brief:
 * @param:
 * @ret:
 *************************************************************************************/
void sI2C_genStart( void )
{
	//presume that PB8 and PB9 are being high
	sI2C_wrtSDA(0);
	sI2C_delayus(2);
	sI2C_wrtSCL(0);
	sI2C_delayus(1);
}
/*************************************************************************************
 * @Brief:
 * @param:
 * @ret:
 *************************************************************************************/
void sI2C_genStop( void )
{
	//presume that PB8 and PB9 are being low
	sI2C_wrtSCL(1);
	sI2C_delayus(2);
	sI2C_wrtSDA(1);
	sI2C_delayus(1);
}
/*************************************************************************************
 * @Brief:
 * @param:
 * @ret:
 *************************************************************************************/
void sI2C_writeSlaveAddr(U8 slaveAddr, U8 isRecv)
{
	sI2C_write((slaveAddr << 1) | isRecv);
}
/*************************************************************************************
 * @Brief:
 * @param:
 * @ret:
 *************************************************************************************/
void sI2C_write( U8 dat )
{
	//presume that PB8 and PB9 are being low
	int i = 0, lvl = 0;

	for( i = 0; i < 8; i++ )
	{
		sI2C_delayus(1);
		lvl = ((dat & 0x80) != 0) ? 1 : 0;
		dat <<= 1;
		sI2C_wrtSDA(lvl);
		sI2C_wrtSCL(1);
		sI2C_delayus(1);
		sI2C_wrtSCL(0);
		sI2C_delayus(1);
	}
}

/*************************************************************************************
 * @Brief:
 * @param:
 * @ret:	0	|	ACK = OK
 * 			1	|	No ACK (NACK)
 *************************************************************************************/
U8 sI2C_readACK( void )
{
	U8 res = 0;

	sI2C_wrtSCL(1);
	sI2C_delayus(2);
	res = sI2C_rdSDA();
	sI2C_wrtSCL(0);
	sI2C_delayus(1);

	return res;
}

/*************************************************************************************
 * @Brief:
 * @param:
 * @ret:	8 bit read data
 *************************************************************************************/
U8 sI2C_read( void )
{
	//presume that PB8 and PB9 are being low
	int i = 0;
	U8 dat = 0;
	int lvl = 0;

	for( i = 7; i >= 0; i-- )
	{
		sI2C_delayus(2);
		sI2C_wrtSCL(1);
		sI2C_delayus(2);
		lvl = sI2C_rdSDA();
		//1st step: shift; 2nd step: or new bit 0
		//Purpose: avoid unnecessary shift action of last bit reading
		dat <<= 1;
		dat |= lvl;
		sI2C_wrtSCL(0);
		sI2C_delayus(2);
	}
	return dat;
}
/*************************************************************************************
 * @Brief:
 * @param:	ACKstt: ACK state: 1: NACK / 0: ACK
 * @ret:
 *************************************************************************************/
void sI2C_writeACK( U8 ACKstt )
{
	sI2C_wrtSDA(ACKstt);
	sI2C_wrtSCL(1);
	sI2C_delayus(1);
	sI2C_wrtSCL(0);
	sI2C_delayus(1);
}
/*------------------------------------------------------------------------------------
 * Brief:
 * Param:
 * Ret:
 -------------------------------------------------------------------------------------*/
U8 sI2C_wrtDat(U8 slaveAddr, U16 wLen, const U8 *wDat)
{
	U16 i = 0;
	U8 res;

	while (sI2C_isBusy)
	{
		sI2C_delayus(1000);
	}
	sI2C_isBusy = 1;
	sI2C_genStart();
	sI2C_writeSlaveAddr(slaveAddr, 0);
	res = sI2C_readACK();
	if (res)
	{
		sI2C_genStop();
		sI2C_isBusy = 0;
		return 1;
	}
	for(i = 0; i < wLen; i++)
	{
		sI2C_write(wDat[i]);
		res = sI2C_readACK();
		if (res)
		{
			sI2C_genStop();
			sI2C_isBusy = 0;
			return (2 + i);
		}
	}
	sI2C_genStop();
	sI2C_isBusy = 0;
	return 0;
}
/*------------------------------------------------------------------------------------
 * Brief:
 * Param:
 * Ret:
 -------------------------------------------------------------------------------------*/
U8 sI2C_rdDat(U8 slaveAddr, U16 rSize, U8 *rDat)
{
	U8 res;
	U16 i = 0;

	while (sI2C_isBusy)
	{
		sI2C_delayus(1000);
	}
	sI2C_isBusy = 1;
	sI2C_genStart();
	sI2C_writeSlaveAddr(slaveAddr, 1);
	res = sI2C_readACK();
	if (res)
	{
		sI2C_genStop();
		sI2C_isBusy = 0;
		return 1;
	}
	for (i = 0; i < rSize; i++)
	{
		rDat[i] = sI2C_read();
		sI2C_writeACK((i  < (rSize - 1)) ? 0 : 1);
	}
	sI2C_genStop();
	sI2C_isBusy = 0;
	return 0;
}
