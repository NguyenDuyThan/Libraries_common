/************************************************************************************
 * author:Duy Vinh
 *
 * description:
 * 	provide function to work with CAN module of STM32 MCU (Header file).
 *
 ************************************************************************************/

#ifndef HW_LIB_STM32F1_CAN_H_
#define HW_LIB_STM32F1_CAN_H_

/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/
#include "stm32F10x_can.h"
/*-----------------------------------------------------------------------------------------
 * DEFINES
 ------------------------------------------------------------------------------------------*/
#define CAN1_RX_PIN		GPIO_Pin_11
#define CAN1_RX_PORT	GPIOA

#define CAN1_TX_PIN		GPIO_Pin_12
#define CAN1_TX_PORT	GPIOA

#define CAN1_enableIOclk	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE)

#define MASK_11b		0x7FF
/*-----------------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef void (*CAN_rx_CB)(CanRxMsg msg);
/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
extern void CAN1_setup(CAN_rx_CB CANrxCB);
extern U8 CAN1_sendMseg(U32 id, U8 len, const U8 *dat);
extern void CAN1_updateFilter(U32 filterId, U32 filterMask);
#if 0
extern U8 CAN1_msegWait(U8 FIFOx);
extern void CAN1_rdMseg(U8 FIFOx, U8 *size, U32 *id, U8 *dat);
#endif
#endif /* HW_LIB_STM32F1_CAN_H_ */
