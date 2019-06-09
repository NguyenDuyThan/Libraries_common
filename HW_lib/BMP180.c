/*************************************************************************************************
 * @Brief:						BMP180: pressure sensor.
 * @Team:						Dinh Vi So R&D
 * @Author(s):					Duy Vinh To
 *
 * @details:
 *************************************************************************************************/

#if ENABLE_BMP180
/*************************************************************************************************
 * INCLUDE
 *************************************************************************************************/
#include "string.h"
#include "sI2C.h"
#include "MCUperiphs.h"
//#include "dbgPrint.h"
#include "BMP180.h"

/*************************************************************************************************
 * DEFINE & MACRO
 *************************************************************************************************/
#define BMP180_ADDR			(0xEE >> 1)

#define BMP180_CTRL_REGADDR		0xF4
#define BMP180_OUT_MSB_REGADDR	0xF6
#define BMP180_AC1_REGADDR	0xAA
#define BMP180_AC2_REGADDR	0xAC
#define BMP180_AC3_REGADDR	0xAE
#define BMP180_AC4_REGADDR	0xB0
#define BMP180_AC5_REGADDR	0xB2
#define BMP180_AC6_REGADDR	0xB4
#define BMP180_B1_REGADDR	0xB6
#define BMP180_B2_REGADDR	0xB8
#define BMP180_MB_REGADDR	0xBA
#define BMP180_MC_REGADDR	0xBC
#define BMP180_MD_REGADDR	0xBE

#define BMP180_RST_REGADDR	0xE0
#define BMP180_VALUE2RST	0xB6
/*************************************************************************************************
 * TYPEDEF
 *************************************************************************************************/

/*************************************************************************************************
 * VARIABLES
 *************************************************************************************************/
static BMP180_wrtI2C_CB BMP180_wrtI2C;
static BMP180_rdI2C_CB BMP180_rdI2C;
static BMP180_delayus_CB BMP180_delayus;
/*************************************************************************************************
 * FUNC.PROTOTYPES
 *************************************************************************************************/
static uint8_t BMP180_wrt(uint8_t regAddr, const uint8_t *dat, uint8_t len);
static uint8_t BMP180_rd(uint8_t regAddr, uint16_t size, uint8_t *dat);
static uint8_t BMP180_rd16bit(uint8_t regAddr, uint16_t *dat);
/*************************************************************************************************
 * FUNCTIONS
 *************************************************************************************************/
/*-----------------------------------------------------------------------------
 * Brief:
 ------------------------------------------------------------------------------*/
void BMP180_ini(BMP180_wrtI2C_CB wrtI2CCB, BMP180_rdI2C_CB rdI2CCB, BMP180_delayus_CB delayusCB)
{
	BMP180_wrtI2C = wrtI2CCB;
	BMP180_rdI2C = rdI2CCB;
	BMP180_delayus = delayusCB;
}
/*-----------------------------------------------------------------------------
 * Brief:
 ------------------------------------------------------------------------------*/
uint8_t BMP180_wrt(uint8_t regAddr , const uint8_t *dat, uint8_t len)
{
	if (BMP180_wrtI2C(BMP180_ADDR, 1, &regAddr))
	{
		return 0xFF;
	}
	return BMP180_wrtI2C(BMP180_ADDR, len, dat);
}
/*-----------------------------------------------------------------------------
 * Brief:
 ------------------------------------------------------------------------------*/
uint8_t BMP180_rd(uint8_t regAddr, uint16_t size, uint8_t *dat)
{
	int res, i = 0;

	if (BMP180_wrtI2C(BMP180_ADDR, 1, &regAddr))
	{
		return 0xFF;
	}
	return BMP180_rdI2C(BMP180_ADDR, size, dat);
}
/*-----------------------------------------------------------------------------
 * Brief:
 ------------------------------------------------------------------------------*/
uint8_t BMP180_rd16bit(uint8_t regAddr, uint16_t *dat)
{
	uint8_t rc, tmp[2];

	rc = BMP180_rd(regAddr, 2, tmp);
	*dat = tmp[0] << 8 | tmp[1];
	return rc;
}
/*-----------------------------------------------------------------------------
 * Brief:	get data from BMP180: temperature and pressure
 * Param:
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 ------------------------------------------------------------------------------*/
uint8_t BMP180_getDat(int32_t *temp, int32_t *pressure)
{
	uint8_t calibDat[22];
	uint8_t tmp[3];
	uint8_t rc;
	uint8_t oss = 3;
	int16_t AC1, AC2, AC3, B1, B2, MB, MC, MD;
	uint16_t AC4, AC5, AC6;
	uint32_t B4, B7;
	int32_t B3, B5, B6, X1, X2, X3, p, UT, UP;
	uint16_t timeWaitConversion = (oss) ? (oss * 10) : 10;

	// Read calib data
#if 0
	memset(calibDat, 0, 22);
	rc = BMP180_rd(BMP180_AC1_REGADDR, 22, calibDat);
	if (rc)
	{
		DBG_print("\r\n BMP180_getDat:rc=%u", rc);
		return 1;
	}
	AC1 = calibDat[0] << 8 | calibDat[1];
	AC2 = calibDat[2] << 8 | calibDat[3];
	AC3 = calibDat[4] << 8 | calibDat[5];
	AC4 = calibDat[6] << 8 | calibDat[7];
	AC5 = calibDat[8] << 8 | calibDat[9];
	AC6 = calibDat[10] << 8 | calibDat[11];
	B1 = calibDat[12] << 8 | calibDat[13];
	B2 = calibDat[14] << 8 | calibDat[15];
	MB = calibDat[16] << 8 | calibDat[17];
	MC = calibDat[18] << 8 | calibDat[19];
	MD = calibDat[20] << 8 | calibDat[21];
#else
	rc = BMP180_rd16bit(BMP180_AC1_REGADDR, &AC1);
	if (rc)
	{
		return 10 + 1;
	}
	rc = BMP180_rd16bit(BMP180_AC2_REGADDR, &AC2);
	if (rc)
	{
		return 10 + 2;
	}
	rc = BMP180_rd16bit(BMP180_AC3_REGADDR, &AC3);
	if (rc)
	{
		return 10 + 3;
	}
	rc = BMP180_rd16bit(BMP180_AC4_REGADDR, &AC4);
	if (rc)
	{
		return 10 + 4;
	}
	rc = BMP180_rd16bit(BMP180_AC5_REGADDR, &AC5);
	if (rc)
	{
		return 10 + 5;
	}
	rc = BMP180_rd16bit(BMP180_AC6_REGADDR, &AC6);
	if (rc)
	{
		return 10 + 6;
	}
	rc = BMP180_rd16bit(BMP180_B1_REGADDR, &B1);
	if (rc)
	{
		return 10 + 7;
	}
	rc = BMP180_rd16bit(BMP180_B2_REGADDR, &B2);
	if (rc)
	{
		return 10 + 8;
	}
	rc = BMP180_rd16bit(BMP180_MB_REGADDR, &MB);
	if (rc)
	{
		return 10 + 9;
	}
	rc = BMP180_rd16bit(BMP180_MC_REGADDR, &MC);
	if (rc)
	{
		return 100 + 10;
	}
	rc = BMP180_rd16bit(BMP180_MD_REGADDR, &MD);
	if (rc)
	{
		return 100 + 11;
	}
#endif
	{
		int32_t chksum = AC1 + AC2 + AC3 + AC4 + AC5 + AC6 + B1 + B2 + MB + MC + MD;
		if (!chksum || (chksum == (0xFFFF * 11)))
		{
			return 100 + 12;
		}
	}
	// Read temperature raw data
	tmp[0] = 0x2E;
	rc = BMP180_wrt(BMP180_CTRL_REGADDR, tmp, 1);
	if (rc)
	{
		return 2;
	}
	BMP180_delayus(5000);
	tmp[0] = 0;
	rc = BMP180_rd(BMP180_OUT_MSB_REGADDR, 2, tmp);
	if (rc)
	{
		return 3;
	}
	UT = tmp[0] << 8 | tmp[1];
	// Read pressure raw data
	tmp[0] = 0x34 + (oss << 6);
	rc = BMP180_wrt(BMP180_CTRL_REGADDR, tmp, 1);
	if (rc)
	{
		return 4;
	}
	for(uint8_t i = 0; i < timeWaitConversion; i++)
	{
		BMP180_delayus(1000);
		tmp[0] = 0;
		rc = BMP180_rd(BMP180_CTRL_REGADDR, 1, tmp);
		if (rc)
		{
			return 40 + 1;
		}
		if (!((tmp[0] >> 5) & 0x1))
		{
			break;
		}
	}
	tmp[0] = 0;
	rc = BMP180_rd(BMP180_OUT_MSB_REGADDR, 3, tmp);
	if (rc)
	{
		return 5;
	}
	UP = (tmp[0] << 16 | tmp[1] << 8 | tmp[2]) >> (8 - oss);
	//DBG_print("\r\n BMP180_getDat:UP=%d", UP);
	// Calculate temperature
	X1 = (UT - AC6) * AC5 / (1 << 15);
	X2 = MC * (1 << 11) / (X1 + MD);
	B5 = X1 + X2;
	*temp = (B5 +8) / (1 << 4);
	// Calculate pressure
	B6 = B5 - 4000;
	X1 = (B2 * (B6 * B6 / (1 << 12))) / (2 << 11);
	X2 = AC2 * B6 / (2 << 11);
	X3 = X1 + X2;
	B3 = ((((s32)AC1 * 4 + X3) << oss) + 2) / 4;
	X1 = AC3 * B6 / (1 << 13);
	X2 = (B1 * (B6 * B6 / (1 << 12))) / (1 << 16);
	X3 = ((X1 + X2) + 2) / (1 << 2);
	B4 = AC4 * (u32)(X3 + 32768) / (1 << 15);
	B7 = ((u32)UP - B3) * (50000 >> oss);
	if (B7 < 0x80000000)
	{
		p = (B7 * 2) / B4;
	}
	else
	{
		p = (B7 / B4) * 2;
	}
	X1 = (p / (1 << 8)) * (p / (1 << 8));
	X1 = (X1 * 3038) / (1 << 16);
	X2 = (-7357 * p) / (1 << 16);
	*pressure = p + (X1 + X2 + 3791) / (1 << 4);
	return 0;
}
#endif // ENABLE_BMP180
