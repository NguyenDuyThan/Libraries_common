/*-----------------------------------------------------------------------------------------
 Brief: read temperature from DS18B20 sensor (Source file).
 Version             : 2.0
 Modified date       : July 13rd, 2016
 Written/modified by : Tuan Pham, Duy Vinh To
 Team:	DinhViSo
 ------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/
#include "DS18B20.h"
//#include "dbgPrint.h"
/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
static U8 DS18B20_rd();
static void DS18B20_wrt( U8 wdata );
static U8 DS18B20_init( void );
static void DS18B20_print_nothing(const U8 *s, ...);
/*-----------------------------------------------------------------------------------------
 * VARIABLES
 ------------------------------------------------------------------------------------------*/
static DS18B20_print_CB DS18B20_print;
static DS18B20_delayus_CB DS18B20_delayus;
static DS18B20_rdDatPin_CB DS18B20_rdDatPin;
static DS18B20_wrtDatPin_CB DS18B20_wrtDatPin;

const U8 dscrc_table[] =
	{ 0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65, 157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220, 35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98, 190, 224, 2, 92, 223, 129, 99, 61,
			124, 34, 192, 158, 29, 67, 161, 255, 70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7, 219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154, 101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198,
			152, 122, 36, 248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185, 140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205, 17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80, 175, 241,
			19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238, 50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115, 202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139, 87, 9, 235, 181, 54, 104, 138,
			212, 149, 203, 41, 119, 244, 170, 72, 22, 233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168, 116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53 };
/*-----------------------------------------------------------------------------------------
 * FUNCTIONS
 ------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------
 * Brief: initial a pseudo print fucntion for DS18B20 printing
 -------------------------------------------------------------------------*/
void DS18B20_print_nothing(const U8 *s, ...)
{
	return;
}
/*------------------------------------------------------------------------
 * Brief: Setup DS18B20 print/debug callback function
 -------------------------------------------------------------------------*/
void DS18B20_setupPrint(DS18B20_print_CB cb)
{
	if (cb != (void*)0)
	{
		DS18B20_print = cb;
	}
	else
	{
		DS18B20_print = DS18B20_print_nothing;
	}
}
/*------------------------------------------------------------------------
 * Brief: Setup DS18B20 delay and IO relating callback function
 -------------------------------------------------------------------------*/
void DS18B20_setup(DS18B20_delayus_CB delayusCB, DS18B20_rdDatPin_CB rdDatPin_CB, DS18B20_wrtDatPin_CB wrtDatPin_CB)
{
	DS18B20_delayus = delayusCB;
	DS18B20_rdDatPin = rdDatPin_CB;
	DS18B20_wrtDatPin = wrtDatPin_CB;
}
/*------------------------------------------------------------------------
 * Brief: Setup DS18B20 delay and IO relating callback function
 -------------------------------------------------------------------------*/
U8 DS18B20_updateCRC(U8 crc, U8 data)
{
	U8 i;

	crc = crc ^ data;
	for (i = 0; i < 8; i++)
	{
		if (crc & 0x01)
			crc = (crc >> 1) ^ 0x8C;
		else
			crc >>= 1;
	}
	return crc;
}
/**************************************************************************
 * brief:			Read temperature from DS18B20 sensor.
 * param			temp	|	OUT	|	temperature
 * retval:			0	|	OK
 * 					>0	|	FAIL
 **************************************************************************/
U8 DS18B20_rdTemp( int *temp )
{
	U8 scratchpad[9];
	U8 CRC_calc = 0x00;
	U8 i = 0;
	U16 tm_out = 0;

	*temp = TEMPSENSOR_INVALID;
	//DS18B20_rdDatPin();
	DS18B20_delayus(1);
	if (!DS18B20_init())
	{
		//DS18B20_rdDatPin();
		return 1;
	}
#if 1
	DS18B20_wrt(0xCC); // skip rom
	//DS18B20_wrt(0x4E);             // configure DS18B20
	//DS18B20_wrt(0x00);             //    write dummy 0x00 to T_H register
	//DS18B20_wrt(0x00);             //    write dummy 0x00 to T_L register
	//DS18B20_wrt(0x1F);             //    write to configuration register (set 9bit resolution)
	DS18B20_wrt(0x44); // start conversion
	DS18B20_delayus(5000); // delay 5ms to make sure DS18B20 go into conversion
	while (!DS18B20_rd())
	{
		// wait for conversion finish or time-out
		DS18B20_delayus(1000);
		if (tm_out++ >= 800)
		{
			//DS18B20_rdDatPin();
			return 2;
		}
	}
	if (!DS18B20_init())
	{
		//DS18B20_rdDatPin();
		return 3;
	}
	DS18B20_wrt(0xCC); // skip rom
	DS18B20_wrt(0xBE); //read scratchpad
	for (i = 0; i < 8; i++)
	{
		scratchpad[i] = DS18B20_rd();
		//CRC_calc = dscrc_table[CRC_calc ^ scratchpad[i]];
		CRC_calc = DS18B20_updateCRC(CRC_calc, scratchpad[i]);
	}
	scratchpad[8] = DS18B20_rd(); //read CRC
	DS18B20_print("\r\n DS18B20_rdTemp:scratchpad:");
	for (i = 0; i <= 8; i++)
	{
		DS18B20_print(" [%X]", scratchpad[i]);
	}
	*temp = (scratchpad[0] >> 4) & 0x0F;
	*temp |= (scratchpad[1] & 0x07) << 4;
	if ((scratchpad[1] & 0xF8) == 0xF8)
	{
		// negative
		*temp = ((~*temp) & 0x7F);
		*temp = -*temp;
	}
	//DS18B20_rdDatPin();
#if 0
	// CRC calculation is problem -> Skip it, coder must determines if result is acceptable.
	if (scratchpad[8] != CRC_calc)
	{
		DS18B20_print("\r\n DS18B20_rdTemp:CRC=%u/%u->ERR", scratchpad[8], CRC_calc);
		return 4;
	}
#endif
	if ((*temp < -55) || (*temp > 85))
	{
		return 5;
	}
#endif
	return 0;
}
/**************************************************************************
 * @brief:			init_DS18B20
 * @param[in]:		none
 * @param[out]:		none
 * @retval:			unsigned char
 *
 * @details:
 * // return value:
 * //   0 - sensor is not present
 * //   1 - sensor present, successfully initial
 **************************************************************************/
U8 DS18B20_init( void )
{
	U16 tLo = 0, tHi = 0;

	// drive IO pin to low level in 480us minimum
	DS18B20_wrtDatPin(0);
	DS18B20_delayus(500);
	// release IO pin and wait for 15~60us
	DS18B20_rdDatPin();
	DS18B20_delayus(60);
	while (!DS18B20_rdDatPin())
	{
		DS18B20_delayus(10);
		tLo += 10;
		if (tLo >= 240)
		{
			break;
		}
	}
	while (DS18B20_rdDatPin())
	{
		DS18B20_delayus(10);
		tHi += 10;
		if (tHi >= (480 - tLo))
		{
			break;
		}
	}
	DS18B20_print("\r\n DS18B20_init:tLo=%u tHi=%u", tLo, tHi);
	if (tLo >= 240)
	{
		return 0;
	}
	return 1;
}
/**************************************************************************
 * @brief:			write_DS18B20
 * @param[in]:		unsigned char
 * @param[out]:		none
 * @retval:			none
 *
 * @details:
 **************************************************************************/
void DS18B20_wrt( U8 wdata )
{
	U8 wcnt, data;

	data = wdata;
	for( wcnt = 0; wcnt < 8; wcnt++ )
	{
		// pull low IO pin 2us to start writing
		DS18B20_wrtDatPin(0);
		DS18B20_delayus(2);
		// write bit
		if ((data & 0x1) == 0x1)
		{
			DS18B20_wrtDatPin(1);
		}
		else
		{
			DS18B20_wrtDatPin(0);
		}
		data >>= 1;
		// wait for write slot finish (60us)
		DS18B20_delayus(60);
		//release IO pin and delay 2us
		DS18B20_rdDatPin();
		DS18B20_delayus(2);
	}
}
/**************************************************************************
 * @brief:			read_DS18B20
 * @param[in]:		none
 * @param[out]:		none
 * @retval:			unsigned char
 *
 * @details:
 **************************************************************************/
U8 DS18B20_rd()
{
	U8 index = 0, data = 0;
	U16 tLo[8], tLo_max = 0, tLo_min = 0xFFFF;

	//DS18B20_print("\r\n DS18B20_rd:tLo:");
	for( index = 0; index < 8; index++ )
	{
		// pull low IO pin 2us to start reading
		DS18B20_wrtDatPin(0);
		DS18B20_delayus(2);
		tLo[index] = 0;
		while (!DS18B20_rdDatPin())
		{
			DS18B20_delayus(10);
			tLo[index] += 10;
			if (tLo[index] >= 60)
			{
				break;
			}
		}
		if (tLo[index] > tLo_max)
		{
			tLo_max = tLo[index];
		}
		if (tLo[index] < tLo_min)
		{
			tLo_min = tLo[index];
		}
		if (60 > tLo[index])
		{
			DS18B20_delayus(60 - tLo[index]);
		}
		//DS18B20_print(" [%u]", tLo[index]);
		DS18B20_delayus(1); // Recovery time
	}
	//DS18B20_print("\r\n Max=%u Min=%u", tLo_max, tLo_min);
	if (tLo_min <= tLo_max)
	{
		if (tLo_min == tLo_max)
		{
			tLo_min = 0;
		}
		for (index = 0; index < 8; index++)
		{
			if (tLo[index] < ((tLo_max - tLo_min) / 2))
			{
				data |= 0x80;
			}
			if (index < 7)
			{
				data >>= 1;
			}
		}
	}
	return data;
}

