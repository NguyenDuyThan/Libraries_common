/*************************************************************************************************
 * @Brief:						MPL3115A2: measure pressure and temperature
 * @Team:						Dinh Vi So R&D
 * @Author(s):					Duy Vinh To
 *
 * @details:
 *************************************************************************************************/

#if ENABLE_MPL3115A2
/*************************************************************************************************
 * INCLUDE
 *************************************************************************************************/
#include "string.h"
#include "sI2C.h"
#include "MPL3115A2.h"

/*************************************************************************************************
 * DEFINE & MACRO
 *************************************************************************************************/
#define MPL3115A2_ADDR						0xC0

#define MPL3115A2REG_OUT_P_MSB				0x01
#define MPL3115A2REG_OUT_P_CSB				0x02
#define MPL3115A2REG_OUT_P_LSB				0x03
#define MPL3115A2REG_OUT_T_MSB				0x04
#define MPL3115A2REG_OUT_T_LSB				0x05
#define MPL3115A2REG_WHO_AM_I				0x0C
/*************************************************************************************************
 * TYPEDEF
 *************************************************************************************************/

/*************************************************************************************************
 * FUNCTIONS
 *************************************************************************************************/
static void MPL3115A2_wrt(u8 REG);
static void MPL3115A2_rd(u16 size, u8 *dat);
static void MPL3115A2_initHW(void);

/*-----------------------------------------------------------------------------
 * Brief:
 ------------------------------------------------------------------------------*/
void MPL3115A2_initHW(void)
{
	sI2C_startup();
}
/*-----------------------------------------------------------------------------
 * Brief:
 ------------------------------------------------------------------------------*/
void MPL3115A2_ini(void)
{
	MPL3115A2_initHW();
	//MPL3115A2_wrt(MPL3115A2_CONTINOUSLY_HRES_MODE, NULL);
}
/*-----------------------------------------------------------------------------
 * Brief:
 ------------------------------------------------------------------------------*/
void MPL3115A2_wrt(u8 REG)
{
	int res;

	sI2C_genStart();
	sI2C_writeSlaveAddr(MPL3115A2_ADDR, 0);
	res = sI2C_readACK();
	if(res)
	{
		return;
	}
	sI2C_write(REG);
	res = sI2C_readACK();
	if(res)
	{
		return;
	}
	sI2C_genStop();
}
/*-----------------------------------------------------------------------------
 * Brief:
 ------------------------------------------------------------------------------*/
void MPL3115A2_rd(u16 size, u8 *dat)
{
	int res, i = 0;

	sI2C_genStart();
	sI2C_writeSlaveAddr(MPL3115A2_ADDR, 1);
	res = sI2C_readACK();
	if(res)
	{
		for (i = 0; i < size; i++)
		{
			dat[i] = 0;
		}
		return;
	}
	for (i = 0; i < size; i++)
	{
		dat[i] = sI2C_read();
		sI2C_writeACK((i  < (size - 1)) ? 0 : 1);
	}
	sI2C_genStop();
}
/*-----------------------------------------------------------------------------
 * Brief:
 ------------------------------------------------------------------------------*/
u8 MPL3115A2_verify(void)
{
	u8 tmp[1];

	MPL3115A2_wrt(MPL3115A2REG_WHO_AM_I);
	MPL3115A2_rd(1, tmp);
	return tmp[0];
}
/*-----------------------------------------------------------------------------
 * Brief:
 ------------------------------------------------------------------------------*/
u32 MPL3115A2_rdPressure(void)
{
	u8 tmp[3];
	
	MPL3115A2_wrt(MPL3115A2REG_OUT_P_MSB);
	MPL3115A2_rd(3, tmp);
	return (tmp[0] << 16 | tmp[1] << 8 | tmp[2]);
}
#endif // ENABLE_MPL3115A2
