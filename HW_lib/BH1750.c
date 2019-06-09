/*************************************************************************************************
 * @Brief:						BH1750: measure light. Source file.
 * @Team:						Dinh Vi So R&D
 * @Author(s):					Duy Vinh To
 *
 * @details:
 * Illuminance - Recommended Light Levels: http://www.engineeringtoolbox.com/light-level-rooms-d_708.html
 *************************************************************************************************/

/*************************************************************************************************
 * INCLUDE
 *************************************************************************************************/
#include "string.h"
#include "sI2C.h"
#include "BH1750.h"
//#include "MCUperiphs.h"
//#include "dbgPrint.h"

/*************************************************************************************************
 * DEFINE & MACRO
 *************************************************************************************************/
#define BH1750_ADDRH						0x5C
#define BH1750_ADDRL						0x23
#define BH1750_CONTINOUSLY_HRES_MODE		0x10
#define BH1750_ONETIME_HRES_MODE			0x20
#define BH1750_POWERON_MODE					0x01

#define BH1750_ADDR_SEL						BH1750_ADDRL
#define	TIMEOUT_UNTILREAD						120 // Unit: millisecond
/*************************************************************************************************
 * TYPEDEF
 *************************************************************************************************/

static BH1750_wrtI2C_CB BH1750_wrtI2C;
static BH1750_rdI2C_CB BH1750_rdI2C;
static BH1750_delayms_CB BH1750_delayms;
/*-----------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------------*/
static U8 BH1750_wrt(U8 cmd);
static U8 BH1750_rd(U16 size, U8 *dat);

/*-----------------------------------------------------------------------------------------------
 * FUNCTIONS
 ------------------------------------------------------------------------------------------------*/
void BH1750_setup(BH1750_wrtI2C_CB wrtI2CCB, BH1750_rdI2C_CB rdI2CCB, BH1750_delayms_CB delaymsCB)
{
	BH1750_wrtI2C = wrtI2CCB;
	BH1750_rdI2C = rdI2CCB;
	BH1750_delayms = delaymsCB;
}
/*-----------------------------------------------------------------------------
 * Brief:
 ------------------------------------------------------------------------------*/
U8 BH1750_wrt(U8 cmd)
{
	return BH1750_wrtI2C(BH1750_ADDR_SEL, 1, &cmd);
}
/*-----------------------------------------------------------------------------
 * Brief:
 ------------------------------------------------------------------------------*/
U8 BH1750_rd(U16 size, U8 *dat)
{
	return BH1750_rdI2C(BH1750_ADDR_SEL, size, dat);
}
/*-----------------------------------------------------------------------------
 * Brief: Read lux value from BH1750
 * Ret: 0	|	OK
 * 		>0	|	FAIL
 ------------------------------------------------------------------------------*/
U8 BH1750_rdLux(U16 *lx)
{
	U8 tmp[2] = {0,0};
	U8 rc;

	rc = BH1750_wrt(BH1750_ONETIME_HRES_MODE);
	if (rc)
	{
		return 1;
	}
	BH1750_delayms(TIMEOUT_UNTILREAD);
	rc = BH1750_rd(2, tmp);
	if (rc)
	{
		return 2;
	}
	*lx = (tmp[0] << 8 | tmp[1]);
	return 0;
}