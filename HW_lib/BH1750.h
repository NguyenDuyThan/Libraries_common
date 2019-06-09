/*************************************************************************************************
 * @Brief:						BH1750: measure light. Header file.
 * @Team:						Dinh Vi So R&D
 * @Author(s):					Duy Vinh To
 *
 * @details:
 * Illuminance - Recommended Light Levels: http://www.engineeringtoolbox.com/light-level-rooms-d_708.html
 *************************************************************************************************/

#ifndef _BH1750_H_
#define _BH1750_H_

/*-----------------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------------------*/
typedef unsigned char U8;
//typedef signed char S8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef U8 (*BH1750_wrtI2C_CB)(U8 slaveAddr, U16 wLen, const U8 *wDat);
typedef U8 (*BH1750_rdI2C_CB)(U8 slaveAddr, U16 rSize, U8 *rDat);
typedef void (*BH1750_delayms_CB)(U32 t);

/*-----------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------------*/
extern void BH1750_setup(BH1750_wrtI2C_CB wrtI2CCB, BH1750_rdI2C_CB rdI2CCB, BH1750_delayms_CB delaymsCB);
extern U8 BH1750_rdLux(U16 *lx);

#endif // _BH1750_H_
