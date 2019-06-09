/*
 * STM32F1_int.h
 *
 *  Created on: Apr 12, 2017
 *      Author: dv198
 */

#ifndef HW_LIB_STM32F1_NVIC_H_
#define HW_LIB_STM32F1_NVIC_H_

#include "stm32f10x.h"
#include "misc.h"

extern void NVIC_add(IRQn_Type irq);
extern void NVIC_rmv(IRQn_Type irq);
extern void NVIC_disableAll(void);
extern void NVIC_enableAll(void);

#endif /* HW_LIB_STM32F1_NVIC_H_ */
