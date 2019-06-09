/*************************************************************************************************
 * @Brief:						DHT11 - humidity and temperature sensor
 * @Team:						Dinh Vi So R&D
 * @Author(s):					Duy Vinh To
 *
 * @details:
 *************************************************************************************************/

/*************************************************************************************************
 * INCLUDE
 *************************************************************************************************/
//#include "stm32f10x_rcc.h"
//#include "stm32f10x_gpio.h"
#include "DHT11.h"
#include "string.h"
//#include "MCUperiphs.h"
//#include "IOmapping.h"

/*************************************************************************************************
 * DEFINITIONS - TYPEDEFS - MACROS
 *************************************************************************************************/
#define BIT(x)			(1 << x)
#define GETBIT(n,x)		((n >> x) & 0x1)

typedef enum
{
	DIR_IN = 0, DIR_OUT,
} IO_DIR_t;
/*************************************************************************************************
 * VARIABLES
 *************************************************************************************************/
static DHT11_delayus_CB DHT11_delayus;
static DHT11_wrtIO_CB DHT11_wrtIO;
static DHT11_rdIO_CB DHT11_rdIO;
/*************************************************************************************************
 * FUNCTION PROTOTYPES
 *************************************************************************************************/
static U8 DHT11_waitStable( void );
static U8 DHT11_read1Byte( void );

/*-----------------------------------------------------------------------------------------------
 *
 ------------------------------------------------------------------------------------------------*/
void DHT11_setup(DHT11_delayus_CB delayusCB, DHT11_wrtIO_CB wrtIOCB, DHT11_rdIO_CB rdIOCB)
{
	DHT11_delayus = delayusCB;
	DHT11_wrtIO = wrtIOCB;
	DHT11_rdIO = rdIOCB;
}
/************************************************************************************************
 * @Brief:		wait for stability on IO of DHT11
 * @Ret:		1	|	OK
 * 				0	|	FAIL
 ************************************************************************************************/
U8 DHT11_waitStable( void )
{
	U32 tdelay = 0, tUnstatble = 0;
	U8 preIOstt = 0, IOstt = 0, ret = 0;
	//Keep low >18ms
	//wait DHT11 IOpin is stable (idle state).
	tdelay = 0;
	tUnstatble = 0;
	while( 1 )
	{
		IOstt = DHT11_rdIO();
		if( IOstt != preIOstt )
		{
			//DBG_print("\r\n DHT11_waitStable:rst tdelay");
			tdelay = 0;
		}
		else
		{
			//DBG_print("\r\n DHT11_waitStable:rst tUnstatble");
			//tUnstatble = 0;
		}
		preIOstt = IOstt;
		DHT11_delayus(1);
		if( tdelay++ >= 1000 )
		{
			ret = 1;
			break;
		}
		if( tUnstatble++ >= 5000 )
		{
			//TermPrint("--DHT11_waitStable:[Timeout]\r\n");
			ret = 0;
			break;
		}
	}
	DHT11_delayus(5000);
	return ret;
}
/************************************************************************************************
 * @Brief:		read 1 byte from DHT11
 * @Ret:		byte value
 ************************************************************************************************/
U8 DHT11_read1Byte( void )
{
	U8 recvBitOrder = 0, recvByte = 0, i = 0;
	//
	for( recvBitOrder = 1; recvBitOrder <= 8; recvBitOrder++ )
	{
		while (!DHT11_rdIO());
		for(i = 0; i < 10; i++)
		{
			if (!DHT11_rdIO())
			{
				break;
			}
			DHT11_delayus(10);
		}
		if (i >= 7)
		{
			recvByte |= 0x1;
		}
		else if (recvBitOrder <= 7)
		{
			recvByte <<= 1;
		}
	}
	return recvByte;
}
/************************************************************************************************
 * @Brief:		DHT11 read humidity and temperature
 * @Param:		env	|	O	|	humidity and temperature
 * @Ret:		0	|	OK
 * 				>0	|	FAIL
 ************************************************************************************************/
U8 DHT11_read( ENVIRONMENT_t *env )
{
	const U8 SAMPLEVAULT_SIZE = 40;
	U8 samplevault[SAMPLEVAULT_SIZE], tmpEnv[5];
	U32 chksum = 0, DHT11chksum = 0;
	U32 i = 0;
	U8 timeKeepHigh = 0, EC = 1, maxTimeKeepHigh = 0, minTimeKeepHigh = 0xFF, avgTimeKeepHigh = 0;
	U16 value = 0;

	//wait DHT11 IOpin is stable (idle state).
	if( DHT11_waitStable() == 1 )
	{
		//DBG_print("\r\n DHT11:stable");
		//Keep low 0.8 -> 20ms
		DHT11_wrtIO(0);
		DHT11_delayus(1000);
		//Keep high 20-200us
		DHT11_wrtIO(1);
		DHT11_delayus(30);
		// Reading verification
		timeKeepHigh = 0;
		while (!DHT11_rdIO())
		{
			if (timeKeepHigh++ >= 0xFF)
			{
				EC = 2;
				goto DHT11_READ_END;
			}
		}
		timeKeepHigh = 0;
		while (DHT11_rdIO())
		{
			DHT11_delayus(10);
			if (timeKeepHigh++ >= 0xFF)
			{
				 EC = 3;
				goto DHT11_READ_END;
			}
		}
		// reading 40 bytes data.
		for (i = 0; i < SAMPLEVAULT_SIZE; i++)
		{
			timeKeepHigh = 0;
			while (!DHT11_rdIO())
			{
				if (timeKeepHigh++ >= 0xFF)
				{
					EC = 4;
					goto DHT11_READ_END;
				}
			}
			timeKeepHigh = 0;
			while (DHT11_rdIO())
			{
				DHT11_delayus(10);
				if (timeKeepHigh++ >= 0xFF)
				{
					 EC = 5;
					goto DHT11_READ_END;
				}
			}
			samplevault[i] = timeKeepHigh;
			if (timeKeepHigh > maxTimeKeepHigh)
			{
				maxTimeKeepHigh = timeKeepHigh;
			}
			if (timeKeepHigh < minTimeKeepHigh)
			{
				minTimeKeepHigh = timeKeepHigh;
			}
		}
		memset(tmpEnv, 0, 5);
		avgTimeKeepHigh = (maxTimeKeepHigh + minTimeKeepHigh) / 2;
		for (i = 0; i < SAMPLEVAULT_SIZE; i++)
		{
			if (samplevault[i] > avgTimeKeepHigh)
			{
				tmpEnv[i / 8] |= BIT(8 - (i % 8) - 1);
			}
			//DBG_print("\r\n DHT11:B=%u b=%u T=%u/%u", i / 8, 8 - (i % 8) - 1, samplevault[i], avgTimeKeepHigh);
			if ((i % 8) == 7)
			{
				//DBG_print("\r\n ByteValue=%u", tmpEnv[i / 8]);
			}
		}
		chksum = (tmpEnv[0] + tmpEnv[1] + tmpEnv[2] + tmpEnv[3]) & 0xFF;
		if( chksum != tmpEnv[4] )
		{
			//DBG_print("\r\nDHT11:CS=%u/%u", chksum, tmpEnv[4]);
			EC = 6;
			goto DHT11_READ_END;
		}
		EC = 0;
		value = (tmpEnv[0] << 8) | tmpEnv[1];
		env->humi_int = (value / 10);
		env->humi_dec = (value % 10);
		value = (tmpEnv[2] << 8) | tmpEnv[3];
		env->temp_int = (value / 10);
		env->temp_dec = (value % 10);
	}
	DHT11_READ_END://
	DHT11_wrtIO(1);
//	if (EC)
//	{
//		memset(env, 0, sizeof(ENVIRONMENT_t));
//	}
	return EC;
}

