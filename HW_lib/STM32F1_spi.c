/*
 * STM32F1_spi.c
 *
 *  Created on: Jan 4, 2017
 *      Author: dv198
 */

/*------------------------------------------------------------------------------------------
 * INCLUDES
 -------------------------------------------------------------------------------------------*/
#include "STM32F1_spi.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
/*------------------------------------------------------------------------------------------
 * DEFINES
 -------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------
 * TYPEDEFS
 -------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 -------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
 * FUNCTIONS
 -------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
 * brief:
 -------------------------------------------------------------------------------*/
void SPI_setup(uint8_t port, uint8_t useSoftCS, SPI_SPEED_t speed)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	uint8_t dumpRead;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	//SPI_I2S_DeInit(SPI1);
	/* Configure SPI pins: SCK and MOSI with default alternate function (not re-mapped) push-pull */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = SPI1_CLK_PIN;
	GPIO_Init(SPI1_CLK_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = SPI1_MOSI_PIN;
	GPIO_Init(SPI1_MOSI_PORT, &GPIO_InitStructure);
	/* Configure MISO as Input with internal pull-up */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = SPI1_MISO_PIN;
	GPIO_Init(SPI1_MISO_PORT, &GPIO_InitStructure);

	/* SPI configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	switch (speed)
	{
		case SPI_SPEED_MAX:
			SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
			break;
		case SPI_SPEED_HIGH:
			SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
			break;
		default: // Default is always lowest speed.
			SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
			break;
	}
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPI1, &SPI_InitStructure);
	SPI_CalculateCRC(SPI1, DISABLE);
	SPI_Cmd(SPI1, ENABLE);

	/* drain SPI */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	dumpRead = SPI_I2S_ReceiveData(SPI1);
}
/*------------------------------------------------------------------------------
 * brief:
 -------------------------------------------------------------------------------*/
uint8_t SPI_wr1Byte(uint8_t port, uint8_t wByte)
{
	SPI_I2S_ClearFlag(SPI1, SPI_I2S_FLAG_TXE);SPI_I2S_ClearFlag(SPI1, SPI_I2S_FLAG_TXE | SPI_I2S_FLAG_RXNE);
	SPI_I2S_ClearFlag(SPI1, SPI_I2S_FLAG_RXNE);
	SPI_I2S_SendData(SPI1, wByte);
	/* Wait until sent a byte */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	/* Wait until received a byte */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	/* Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(SPI1);
}
