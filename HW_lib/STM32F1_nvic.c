/*
 * STM32F1_int.c
 *
 *  Created on: Apr 12, 2017
 *      Author: dv198
 */


#include "STM32F1_nvic.h"
#include "dbgPrint.h"

#define NVIC_MAX 67

static uint8_t NVICprio = 0;
static uint8_t NVIC_avail[NVIC_MAX];

void NVIC_add(IRQn_Type irq)
{
//	if (NVICprio >= NVIC_MAX)
//	{
//		return;
//	}
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannel = irq;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = NVICprio++;
	NVIC_Init(&NVIC_InitStruct);
	//NVIC_avail[NVICprio] = irq;
	//NVICprio++;
}
void NVIC_rmv(IRQn_Type irq)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannelCmd = DISABLE;
	NVIC_InitStruct.NVIC_IRQChannel = irq;
	NVIC_Init(&NVIC_InitStruct);
}
void NVIC_disableAll(void)
{
	for (uint8_t i = 0; i < NVICprio; i++)
	{
		NVIC_DisableIRQ(NVIC_avail[i]);
	}
}
void NVIC_enableAll(void)
{
	for (uint8_t i = 0; i < NVICprio; i++)
	{
		NVIC_EnableIRQ(NVIC_avail[i]);
	}
}
