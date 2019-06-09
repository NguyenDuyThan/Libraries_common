/*-----------------------------------------------------------------------------------------
 Brief: read temperature from DS18B20 sensor (Header file).
 Version             : 2.0
 Modified date       : July 13rd, 2016
 Written/modified by : Tuan Pham, Duy Vinh To
 Team:	DinhViSo
 ------------------------------------------------------------------------------------------*/

#ifndef DS18B20_H_
#define DS18B20_H_
/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/
//#include "platform.h"
/*-----------------------------------------------------------------------------------------
 * DEFINE
 ------------------------------------------------------------------------------------------*/
#define TEMPSENSOR_INVALID	200
/*-----------------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef void (*DS18B20_delayus_CB)(U32 t);
typedef U8 (*DS18B20_rdDatPin_CB)(void);
typedef void (*DS18B20_wrtDatPin_CB)(U8 val);
typedef void (*DS18B20_print_CB)(const U8 *s, ...);
/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
extern void DS18B20_setupPrint(DS18B20_print_CB cb);
extern void DS18B20_setup(DS18B20_delayus_CB delayusCB, DS18B20_rdDatPin_CB rdDatPin_CB, DS18B20_wrtDatPin_CB wrtDatPin_CB);
extern U8 DS18B20_rdTemp( int *temp );

#endif//DS18B20_H_
