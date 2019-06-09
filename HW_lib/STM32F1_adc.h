/************************************************************************************
 * author:Duy Vinh
 *
 * description:
 * 	provide function to work with ADC module of STM32 MCU (Header file).
 *
 ************************************************************************************/
#ifndef STM32F1_ADC_H_
#define STM32F1_ADC_H_
/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/
#include "stm32f10x.h"

/*-----------------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
/* ADC1 IN 9:  PB1 */
extern U8 ADC1IN9_rd(U16 *val);
/* ADC1 IN 10:  PC0 */
extern U8 ADC1IN10_rd(U16 *val);
/* ADC1 IN 10:  PC3 */
extern U8 ADC1IN13_rd(U16 *val);
/* ADC1 IN 0:  PA0 */
extern U8 ADC1IN0_rd(U16 *val);
/* ADC1 IN 1:  PA1 */
extern U8 ADC1IN1_rd(U16 *val);
/* ADC1 IN 2:  PA2 */
extern U8 ADC1IN2_rd(U16 *val);
/* ADC1 IN 8:  PB0 */
extern U8 ADC1IN8_rd(U16 *val);

extern U8 ADC_rd(U8 ADCx, U8 ch, U16 *val);

#endif // STM32F1_ADC_H_
