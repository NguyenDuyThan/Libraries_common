/*
 * NVIC_prio.c
 *
 *  Created on: Apr 12, 2017
 *      Author: dv198
 */

#include "NVIC_priorities.h"

static uint8_t NVICprio = 1;

/*----------------------------------------------------------------------------------
 *
 -----------------------------------------------------------------------------------*/
uint8_t NVICPRIO_get(void)
{
	uint8_t ret = NVICprio++;
	return ret;
}
