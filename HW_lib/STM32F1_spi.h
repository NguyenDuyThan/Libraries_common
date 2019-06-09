/*
 * STM32F1_spi.h
 *
 *  Created on: Jan 4, 2017
 *      Author: dv198
 */

#ifndef HW_LIB_STM32F1_SPI_H_
#define HW_LIB_STM32F1_SPI_H_

/*------------------------------------------------------------------------------------------
 * INCLUDES
 -------------------------------------------------------------------------------------------*/
#include "stm32f10x.h"
/*------------------------------------------------------------------------------------------
 * DEFINES
 -------------------------------------------------------------------------------------------*/
#define SPI1_CLK_PORT	GPIOA
#define SPI1_CLK_PIN	GPIO_Pin_5
#define SPI1_MISO_PORT	GPIOA
#define SPI1_MISO_PIN	GPIO_Pin_6
#define SPI1_MOSI_PORT	GPIOA
#define SPI1_MOSI_PIN	GPIO_Pin_7
#define SPI1_CS_PORT	GPIOA
#define SPI1_CS_PIN		GPIO_Pin_4
//#define	UART1_TX_PIN	GPIO_Pin_9
//#define UART1_TX_PORT	GPIOA
//#define	UART1_RX_PIN	GPIO_Pin_10
//#define UART1_RX_PORT	GPIOA
//
//#define	UART2_TX_PIN	GPIO_Pin_2
//#define UART2_TX_PORT	GPIOA
//#define	UART2_RX_PIN	GPIO_Pin_3
//#define UART2_RX_PORT	GPIOA
//
//#define	UART3_TX_PIN	GPIO_Pin_10
//#define UART3_TX_PORT	GPIOB
//#define	UART3_RX_PIN	GPIO_Pin_11
//#define UART3_RX_PORT	GPIOB
//
//#define	UART4_TX_PIN	GPIO_Pin_10
//#define UART4_TX_PORT	GPIOC
//#define	UART4_RX_PIN	GPIO_Pin_11
//#define UART4_RX_PORT	GPIOC
//
//#define	UART5_TX_PIN	GPIO_Pin_12
//#define UART5_TX_PORT	GPIOC
//#define	UART5_RX_PIN	GPIO_Pin_2
//#define UART5_RX_PORT	GPIOD

/*------------------------------------------------------------------------------------------
 * TYPEDEFS
 -------------------------------------------------------------------------------------------*/
typedef enum
{
	SPI_SPEED_LOW = 0,
	SPI_SPEED_HIGH,
	SPI_SPEED_MAX,
} SPI_SPEED_t;
//typedef void (*UART_readc_CB)(uint8_t c);
/*------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 -------------------------------------------------------------------------------------------*/
extern void SPI_setup(uint8_t port, uint8_t useSoftCS, SPI_SPEED_t speed);
extern uint8_t SPI_wr1Byte(uint8_t port, uint8_t wByte);


#endif /* HW_LIB_STM32F1_SPI_H_ */
