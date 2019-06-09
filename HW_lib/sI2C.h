/*
 ******************************************************************************
 * @file    sI2C.h
 * @author  Duy Vinh To - DinhViSo HW team
 * @version V1.0
 * @date	20/9/2014
 * @brief	bit bang (software) I2C header file
 ******************************************************************************
 * @attention:
 ******************************************************************************
 */
#ifndef SI2C_H_
#define SI2C_H_
/*----------------------------------------------------------------------------
 * INCLUDE
 -----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * TYPEDEFS
 -----------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef void (*sI2C_wrtSDA_CB)(U8 set);
typedef U8 (*sI2C_rdSDA_CB)(void);
typedef void (*sI2C_wrtSCL_CB)(U8 set);
typedef void (*sI2C_delayus_CB)(U32 t);

/*----------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 -----------------------------------------------------------------------------*/
extern void sI2C_setup(sI2C_wrtSDA_CB wrtSDAcb, sI2C_rdSDA_CB rdSDAcb, sI2C_wrtSCL_CB wrtSCLcb, sI2C_delayus_CB delayuscb);
extern U8 sI2C_wrtDat(U8 slaveAddr, U16 wLen, const U8 *wDat);
extern U8 sI2C_rdDat(U8 slaveAddr, U16 rSize, U8 *rDat);

#endif// SI2C_H_
