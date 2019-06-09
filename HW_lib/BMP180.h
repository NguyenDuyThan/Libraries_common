/*************************************************************************************************
 * @Brief:						BMP180: pressure sensor.
 * @Team:						Dinh Vi So R&D
 * @Author(s):					Duy Vinh To
 *
 * @details:
 *************************************************************************************************/
#ifndef BMP180_H_
#define BMP180_H_
/*************************************************************************************************
 * INCLUDE
 *************************************************************************************************/
#include "platform.h"
/*************************************************************************************************
 * TYPEDEFS
 *************************************************************************************************/
typedef u8 (*BMP180_wrtI2C_CB)(u8 slaveAddr, u16 wLen, const u8 *wDat);
typedef u8 (*BMP180_rdI2C_CB)(u8 slaveAddr, u16 rSize, u8 *rDat);
typedef u8 (*BMP180_delayus_CB)(u32 t);
/*************************************************************************************************
 * FUNC.PROTOTYPES
 *************************************************************************************************/
extern void BMP180_ini(BMP180_wrtI2C_CB wrtI2CCB, BMP180_rdI2C_CB rdI2CCB, BMP180_delayus_CB delayusCB);
extern uint8_t BMP180_getDat(int32_t *temp, int32_t *pressure);

#endif // BMP180_H_
