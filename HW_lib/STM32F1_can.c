/************************************************************************************
 * author:Duy Vinh
 *
 * description:
 * 	provide function to work with CAN module of STM32 MCU (Source file).
 *
 ************************************************************************************/
/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/
#include "STM32F1_can.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_can.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include "string.h"
//#include "dbgPrint.h"
#include "STM32F1_nvic.h"
#include "additionalIRQhdler.h"
/*-----------------------------------------------------------------------------------------
 * DEFINE
 ------------------------------------------------------------------------------------------*/
#define USE_CAN1_IT_RX		1

#ifndef CAN1_SWFIFO0_SIZE
#define CAN1_SWFIFO0_SIZE	10
#endif
#ifndef CAN1_SWFIFO1_SIZE
#define CAN1_SWFIFO1_SIZE	10
#endif
/*-----------------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------
 * VARIABLES
 ------------------------------------------------------------------------------------------*/
static CAN_rx_CB CAN1_rx;
#if 0
static CanRxMsg CAN1_SWFIFO0[CAN1_SWFIFO0_SIZE];
static U8 CAN1_SWFIFO0_in = 0;
static U8 CAN1_SWFIFO0_out = 0;
static U8 CAN1_SWFIFO0_fill = 0;

static CanRxMsg CAN1_SWFIFO1[CAN1_SWFIFO1_SIZE];
static U8 CAN1_SWFIFO1_in = 0;
static U8 CAN1_SWFIFO1_out = 0;
static U8 CAN1_SWFIFO1_fill = 0;
#endif
/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------
 * FUNCTIONS
 ------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------
 * Brief: pseudo function
 ----------------------------------------------------------------------------------------*/
void CAN1_rx_pseudo(CanRxMsg msg)
{
	return;
}
/*---------------------------------------------------------------------------------------
 * Brief: setup CAN interface
 * Param: CANrxCB	|	IN	|	Callback function to handle received message.
 * Note:
 * + This function will setup CAN1, with baud rate is 125KHz.
 * + Applying no filter on both FIFO0 and FIFO1.
 * + Callback function will be called when message came to FIFO0 or FIFO1.
 ----------------------------------------------------------------------------------------*/
void CAN1_setup(CAN_rx_CB CANrxCB)
{
	CAN_InitTypeDef CAN_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
	CAN1_enableIOclk;

	GPIO_StructInit(&GPIO_InitStructure);
	/* Configure CAN pin: RX */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = CAN1_RX_PIN;
	GPIO_Init(CAN1_RX_PORT, &GPIO_InitStructure);

	/* Configure CAN pin: TX */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = CAN1_TX_PIN;
	GPIO_Init(CAN1_TX_PORT, &GPIO_InitStructure);

	/* CAN register init */
	CAN_DeInit(CAN1);
	CAN_StructInit(&CAN_InitStructure);

	/* CAN cell init */
	CAN_InitStructure.CAN_TTCM = DISABLE;
	CAN_InitStructure.CAN_ABOM = DISABLE;
	CAN_InitStructure.CAN_AWUM = DISABLE;
	CAN_InitStructure.CAN_NART = ENABLE;
	CAN_InitStructure.CAN_RFLM = DISABLE;
	CAN_InitStructure.CAN_TXFP = DISABLE;
	CAN_InitStructure.CAN_Mode = CAN_Mode_Normal;

	/* CAN Baudrate = 125KHz*/
	CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
	CAN_InitStructure.CAN_BS1 = CAN_BS1_3tq;
	CAN_InitStructure.CAN_BS2 = CAN_BS2_5tq;
	CAN_InitStructure.CAN_Prescaler = 48;
	CAN_Init(CAN1, &CAN_InitStructure);

#if USE_CAN1_IT_RX
	if (CANrxCB == NULL)
	{
		CAN1_rx = CAN1_rx_pseudo;
	}
	else
	{
		CAN1_rx = CANrxCB;
	}
	CAN_ITConfig(CAN1, CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0 | CAN_IT_FMP1 | CAN_IT_FF1 | CAN_IT_FOV1, ENABLE);
	NVIC_add(CAN1_RX1_IRQn);
	NVIC_add(USB_LP_CAN1_RX0_IRQn);
#endif // USE_CAN1_IT_RX
	/* Assign filters */
	CAN1_updateFilter(0, 0);
}
/*---------------------------------------------------------------------------------------
 * Brief:	send message to CAN network via CAN1.
 * Param:	id	|	IN	|	message identifier.
 * 			len	|	IN	|	message data length. Maximum length is 8 bytes.
 * 			dat	|	IN	|	message data.
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 ----------------------------------------------------------------------------------------*/
U8 CAN1_sendMseg(U32 id, U8 len, const U8 *dat)
{
	CanTxMsg TxMessage;
	U8 TransmitMailbox;

	/* Transmit */
	TxMessage.StdId = id;
	TxMessage.RTR = CAN_RTR_DATA;
	TxMessage.IDE = CAN_ID_STD;
	TxMessage.DLC = len;
	memcpy(TxMessage.Data, dat, len);
	TransmitMailbox = CAN_Transmit(CAN1, &TxMessage);
	for (U32 i = 0; i < 10000; i++)
	{
		if (CAN_TransmitStatus(CAN1, TransmitMailbox) == CAN_TxStatus_Ok)
		{
			goto CAN1_SENDOK;
		}
	}
	CAN_CancelTransmit(CAN1, TransmitMailbox);
	//DBG_print("\r\n CAN_sendMseg:ERR");
	return 1;
	CAN1_SENDOK:
	//DBG_print("\r\n CAN_sendMseg:OK");
	return 0;
}
#if 0
/*---------------------------------------------------------------------------------------
 * Brief:	check to know how many received messages are there in CAN1's FIFO0 or FIFO1
 * Param:	FIFOx	|	IN	|	Which FIFO to check: 0 or 1
 * Ret:		number of received messages
 ----------------------------------------------------------------------------------------*/
U8 CAN1_msegWait(U8 FIFOx)
{
#if USE_CAN1_IT_RX
	U8 ret = (!FIFOx) ? CAN1_SWFIFO0_fill : CAN1_SWFIFO1_fill;
	//DBG_print("\r\n CAN1_msegWait:FIFO%u:%u", FIFOx, ret);
	return ret;
#else
	return CAN_MessagePending(CAN1, (!FIFOx) ? CAN_FIFO0 : CAN_FIFO1);
#endif
}
/*---------------------------------------------------------------------------------------
 * Brief:	read a message from CAN1's FIFO0 or FIFO1.
 * Param:	FIFOx	|	IN	|	Which FIFO to read: 0 or 1
 * 			size	|	I/O	|	read message data max size. Maximum is 8 bytes.
 * 			id		|	OUT	|	read message identifier.
 * 			dat		|	OUT	|	read message data.
 ----------------------------------------------------------------------------------------*/
void CAN1_rdMseg(U8 FIFOx, U8 *size, U32 *id, U8 *dat)
{
	CanRxMsg RxMessage;

	memset(&RxMessage, 0, sizeof(CanRxMsg));
	if (!FIFOx)
	{
#if USE_CAN1_IT_RX
		//DBG_print("\r\n CAN1_rdMseg:FIFO0:%u", CAN1_SWFIFO0_fill);
		if (CAN1_SWFIFO0_fill > 0)
		{
			RxMessage = CAN1_SWFIFO0[CAN1_SWFIFO0_out++];
			if (CAN1_SWFIFO0_out >= CAN1_SWFIFO0_SIZE)
			{
				CAN1_SWFIFO0_out = 0;
			}
			CAN1_SWFIFO0_fill--;
		}
#else
		CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
#endif
	}
	else
	{
#if USE_CAN1_IT_RX
		//DBG_print("\r\n CAN1_rdMseg:FIFO1:%u", CAN1_SWFIFO1_fill);
		if (CAN1_SWFIFO1_fill > 0)
		{
			RxMessage = CAN1_SWFIFO1[CAN1_SWFIFO1_out++];
			if (CAN1_SWFIFO1_out >= CAN1_SWFIFO1_SIZE)
			{
				CAN1_SWFIFO1_out = 0;
			}
			CAN1_SWFIFO1_fill--;
		}
#else
		CAN_Receive(CAN1, CAN_FIFO1, &RxMessage);
#endif
	}
	*id = RxMessage.StdId;
	*size = (RxMessage.DLC < *size) ? RxMessage.DLC : *size;
	memcpy(dat, RxMessage.Data, *size);
	*size = RxMessage.DLC;
}
#endif
/*---------------------------------------------------------------------------------------
 * Brief:	Update CAN1 filters on both FIFO0 and FIFO1.
 * Param:
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 ----------------------------------------------------------------------------------------*/
void CAN1_updateFilter(U32 filterId, U32 filterMask)
{
	CAN_FilterInitTypeDef CAN_FilterInitStructure;

	CAN_FilterInitStructure.CAN_FilterNumber = 0;
	CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
	CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
	CAN_FilterInitStructure.CAN_FilterIdHigh = filterId >> 16;
	CAN_FilterInitStructure.CAN_FilterIdLow = filterId & 0xFFFF;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = filterMask >> 16;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow = filterMask & 0xFFFF;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
	CAN_FilterInit(&CAN_FilterInitStructure);
	CAN_FilterInitStructure.CAN_FilterNumber = 1;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_Filter_FIFO1;
	CAN_FilterInit(&CAN_FilterInitStructure);
}
/*---------------------------------------------------------------------------------------
 * Brief:
 ----------------------------------------------------------------------------------------*/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	CanRxMsg RxMessage;

	while (CAN_MessagePending(CAN1, CAN_FIFO0))
	{
		CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);
		CAN1_rx(RxMessage);
	}
#if 0
	if (CAN1_SWFIFO0_fill < CAN1_SWFIFO0_SIZE)
	{
		CAN1_SWFIFO0[CAN1_SWFIFO0_in++] = RxMessage;
		if (CAN1_SWFIFO0_in >= CAN1_SWFIFO0_SIZE)
		{
			CAN1_SWFIFO0_in = 0;
		}
		CAN1_SWFIFO0_fill++;
	}
#endif
	CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);
	CAN_ClearITPendingBit(CAN1, CAN_IT_FF0);
	CAN_ClearITPendingBit(CAN1, CAN_IT_FOV0);
}
/*---------------------------------------------------------------------------------------
 * Brief:
 ----------------------------------------------------------------------------------------*/
void CAN1_RX1_IRQHandler(void)
{
	CanRxMsg RxMessage;

	RTOS_enterIRQ();
	while (CAN_MessagePending(CAN1, CAN_FIFO1))
	{
		CAN_Receive(CAN1, CAN_FIFO1, &RxMessage);
		CAN1_rx(RxMessage);
	}
#if 0
	if (CAN1_SWFIFO1_fill < CAN1_SWFIFO1_SIZE)
	{
		CAN1_SWFIFO1[CAN1_SWFIFO1_in++] = RxMessage;
		if (CAN1_SWFIFO1_in >= CAN1_SWFIFO1_SIZE)
		{
			CAN1_SWFIFO1_in = 0;
		}
		CAN1_SWFIFO1_fill++;
	}
#endif
	CAN_ClearITPendingBit(CAN1, CAN_IT_FMP1);
	CAN_ClearITPendingBit(CAN1, CAN_IT_FF1);
	CAN_ClearITPendingBit(CAN1, CAN_IT_FOV1);
	RTOS_exitIRQ();
}
