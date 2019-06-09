/*------------------------------------------------------------------------------------------------
 * @Brief:						SRF05 - ultra sonic sensor. Used to measure distance (Source file).
 * @Team:						Dinh Vi So R&D
 * @Author(s):					Duy Vinh To
 -------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------------*/
#include "SRF05.h"
/*-----------------------------------------------------------------------------------------------
 * VARIABLES
 ------------------------------------------------------------------------------------------------*/
static SRF05_delay10us_cb_t SRF05_delay10us;
static SRF05_wrtTRIG_cb_t SRF05_wrtTRIG;
static SRF05_rdECHO_cb_t SRF05_rdECHO;
/*-----------------------------------------------------------------------------------------------
 * FUNCTIONS
 ------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------
 * Brief:
 -----------------------------------------------------------------------------------------------*/
void SRF05_setup(SRF05_delay10us_cb_t delay10usCB, SRF05_wrtTRIG_cb_t wrtTRIGCB, SRF05_rdECHO_cb_t rdECHOCB)
{
	SRF05_delay10us = delay10usCB;
	SRF05_wrtTRIG = wrtTRIGCB;
	SRF05_rdECHO = rdECHOCB;
}

/*----------------------------------------------------------------------------------------------
 * Brief: read SRF05
 * Param:	dist	|	OUT	|	distance. Unit: cm
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -----------------------------------------------------------------------------------------------*/
U8 SRF05_rd(U16 *dist)
{
	U16 echoTime = 0;

	SRF05_wrtTRIG(1);
	SRF05_delay10us(100);
	SRF05_wrtTRIG(0);
	echoTime = 0;
	while (!SRF05_rdECHO())
	{
		if (echoTime++ >= 210)
		{
			return 1;
		}
		SRF05_delay10us(1);
	}
	echoTime = 0;
	while (SRF05_rdECHO())
	{
		if (echoTime++ >= 3501)
		{
			return 2;
		}
		SRF05_delay10us(1);
	}
	if (echoTime <= 7)
	{
		// echoTime <= 7 -> Distance <= 12mm: inaccurate distance, must be greater than 12mm.
		return 3;
	}
	*dist = echoTime * 100 / 58;
//	*dist = *dist * 172 / 100;
	return 0;
}
