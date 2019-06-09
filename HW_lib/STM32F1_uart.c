/*------------------------------------------------------------------------------------------
 * INTRODUCE
 * Brief: This is source file contain function to setup USART peripherals.
 * Author: Duy Vinh To - Team DinhViSo
 * Detail:
 -------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------
 * INCLUDES
 -------------------------------------------------------------------------------------------*/
#include "stddef.h"
#include "STM32F1_uart.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "NVIC_priorities.h"
#include "additionalIRQhdler.h"
/*------------------------------------------------------------------------------------------
 * DEFINES
 -------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
 * TYPEDEFS
 -------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
 * VARIABLES
 -------------------------------------------------------------------------------------------*/
static UART_readc_CB UART1_readc;
static UART_readc_CB UART2_readc;
static UART_readc_CB UART3_readc;
static UART_readc_CB UART4_readc;
static UART_readc_CB UART5_readc;

/*##########################################################################################
 * FUNC.PROTOTYPES
 *##########################################################################################*/
static void UART_readc_pseudo(uint8_t c);
/*------------------------------------------------------------------------------------------
 * FUNCTIONS
 -------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------
 * Brief: Pseudo read character callback
 -----------------------------------------------------------------------------------------*/
void UART_readc_pseudo(uint8_t c)
{
	return;
}
/*----------------------------------------------------------------------------------------
 * Brief: setup UART
 * Param:	port	|	IN	|	which UART port to setup. Range from 1 to 5.
 * 			br		|	IN	|	baud rate.
 *			readcCB	|	IN	|	callback function to handle reading data from RX interrupt.
 -----------------------------------------------------------------------------------------*/
void UART_setup(uint8_t port, uint32_t br, UART_readc_CB readcCB)
{
	USART_TypeDef *UARTx;
	uint16_t TxPin, RxPin;
	GPIO_TypeDef *TxPort, *RxPort;
	uint8_t IRQchannel;
	USART_InitTypeDef USART_InitStruct;
	GPIO_InitTypeDef GPIO_InitStruct;

	switch (port)
	{
		case 1:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
			UARTx = USART1;
			TxPin = UART1_TX_PIN;
			TxPort = UART1_TX_PORT;
			RxPin = UART1_RX_PIN;
			RxPort = UART1_RX_PORT;
			IRQchannel = USART1_IRQn;
			if (readcCB != NULL)
			{
				UART1_readc = readcCB;
			}
			else
			{
				UART1_readc = UART_readc_pseudo;
			}
			break;
		case 2:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
			UARTx = USART2;
			TxPin = UART2_TX_PIN;
			TxPort = UART2_TX_PORT;
			RxPin = UART2_RX_PIN;
			RxPort = UART2_RX_PORT;
			IRQchannel = USART2_IRQn;
			if (readcCB != NULL)
			{
				UART2_readc = readcCB;
			}
			else
			{
				UART2_readc = UART_readc_pseudo;
			}
			break;
		case 3:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
			UARTx = USART3;
			TxPin = UART3_TX_PIN;
			TxPort = UART3_TX_PORT;
			RxPin = UART3_RX_PIN;
			RxPort = UART3_RX_PORT;
			IRQchannel = USART3_IRQn;
			if (readcCB != NULL)
			{
				UART3_readc = readcCB;
			}
			else
			{
				UART3_readc = UART_readc_pseudo;
			}
			break;
#if STM32F10X_HD || STM32F10X_HD_VL || STM32F10X_XL || STM32F10X_CL
		case 4:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
			UARTx = UART4;
			TxPin = UART4_TX_PIN;
			TxPort = UART4_TX_PORT;
			RxPin = UART4_RX_PIN;
			RxPort = UART4_RX_PORT;
			IRQchannel = UART4_IRQn;
			if (readcCB != NULL)
			{
				UART4_readc = readcCB;
			}
			else
			{
				UART4_readc = UART_readc_pseudo;
			}
			break;
		case 5:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
			UARTx = UART5;
			TxPin = UART5_TX_PIN;
			TxPort = UART5_TX_PORT;
			RxPin = UART5_RX_PIN;
			RxPort = UART5_RX_PORT;
			IRQchannel = UART5_IRQn;
			if (readcCB != NULL)
			{
				UART5_readc = readcCB;
			}
			else
			{
				UART5_readc = UART_readc_pseudo;
			}
			break;
#endif
		default:
			return;
	}
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD//
			| RCC_APB2Periph_AFIO//
			, ENABLE);
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStruct.GPIO_Pin = TxPin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(TxPort, &GPIO_InitStruct);
	if (readcCB != NULL)
	{
		GPIO_InitStruct.GPIO_Pin = RxPin;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(RxPort, &GPIO_InitStruct);
	}
	USART_InitStruct.USART_BaudRate = br;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | ((readcCB != NULL) ? USART_Mode_Rx : 0);
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_DeInit(UARTx);
	USART_Init(UARTx, &USART_InitStruct);
	USART_Cmd(UARTx, ENABLE);

	USART_ITConfig(UARTx, USART_IT_RXNE, readcCB != NULL ? ENABLE : DISABLE);
	if (readcCB != NULL)
	{
		NVIC_add(IRQchannel);
	}
	else
	{
		NVIC_rmv(IRQchannel);
	}
}
/*----------------------------------------------------------------------------------------
 * Brief:	send 1 byte via UART
 * Param:	port	|	IN	|	which UART port to setup. Range from 1 to 5.
 * 			c		|	IN	|	sending byte.
 -----------------------------------------------------------------------------------------*/
void UART_sendc(uint8_t port, uint8_t c)
{
	USART_TypeDef* UARTx;
	switch (port)
	{
		case 1:
			UARTx = USART1;
			break;
		case 2:
			UARTx = USART2;
			break;
		case 3:
			UARTx = USART3;
			break;
		case 4:
			UARTx = UART4;
			break;
		case 5:
			UARTx = UART5;
			break;
		default:
			return;
	}
	USART_SendData(UARTx, c);
	while (!USART_GetFlagStatus(UARTx, USART_FLAG_TXE));
}
/*----------------------------------------------------------------------------------------
 * Brief:	send data via UART
 * Param:	port	|	IN	|	which UART port to setup. Range from 1 to 5.
 * 			s		|	IN	|	sending data.
 * 			l		|	IN	|	length of sending data.
 -----------------------------------------------------------------------------------------*/
void UART_sends(uint8_t port, const uint8_t *s, uint16_t l)
{
	uint16_t i = 0;

	for(uint16_t i = 0; i < l; i++)
	{
		UART_sendc(port, s[i]);
	}
}
/*----------------------------------------------------------------------------------------
 * Brief:	USART1 interrupt handler
 -----------------------------------------------------------------------------------------*/
void USART1_IRQHandler(void)
{
	RTOS_enterIRQ();
	UART1_readc(USART_ReceiveData(USART1));
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	RTOS_exitIRQ();
}
/*----------------------------------------------------------------------------------------
 * Brief:	USART1 interrupt handler
 -----------------------------------------------------------------------------------------*/
void USART2_IRQHandler(void)
{
	RTOS_enterIRQ();
	UART2_readc(USART_ReceiveData(USART2));
	USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	RTOS_exitIRQ();
}
/*----------------------------------------------------------------------------------------
 * Brief:	USART1 interrupt handler
 -----------------------------------------------------------------------------------------*/
void USART3_IRQHandler(void)
{
	RTOS_enterIRQ();
	UART3_readc(USART_ReceiveData(USART3));
	USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	RTOS_exitIRQ();
}
/*----------------------------------------------------------------------------------------
 * Brief:	USART1 interrupt handler
 -----------------------------------------------------------------------------------------*/
void UART4_IRQHandler(void)
{
	RTOS_enterIRQ();
	UART4_readc(USART_ReceiveData(UART4));
	USART_ClearITPendingBit(UART4, USART_IT_RXNE);
	RTOS_exitIRQ();
}
/*----------------------------------------------------------------------------------------
 * Brief:	USART1 interrupt handler
 -----------------------------------------------------------------------------------------*/
void UART5_IRQHandler(void)
{
	RTOS_enterIRQ();
	UART5_readc(USART_ReceiveData(UART5));
	USART_ClearITPendingBit(UART5, USART_IT_RXNE);
	RTOS_exitIRQ();
}
