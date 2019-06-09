/*------------------------------------------------------------------------------------------
 * INTRODUCE
 * Brief: This is source file contain function to setup IO pin.
 * Author: Duy Vinh To - Team DinhViSo
 * Detail:
 -------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------
 * INCLUDES
 -------------------------------------------------------------------------------------------*/
#include "STM32F1_io.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
//#include "dbgPrint.h"
/*------------------------------------------------------------------------------------------
 * DEFINES
 -------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
 * TYPEDEFS
 -------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
 * VARIABLES
 -------------------------------------------------------------------------------------------*/
 
/*------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 -------------------------------------------------------------------------------------------*/
 
/*------------------------------------------------------------------------------------------
 * FUNCTIONS
 -------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------
 * Brief: Setup IO
 * Param:	port	|	IN	|	port selection: A,B,C,D,E,F
 * 			pin		|	IN	|	pin selection: 1->15
 * 			dir		|	IN	|	direction selection
 *
 -----------------------------------------------------------------------------------------*/
void IO_test(void)
{
	GPIO_TypeDef *portx = GPIOB;
	U16 pinx = GPIO_Pin_14;
	GPIOMode_TypeDef mode = GPIO_Mode_Out_PP;
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB //
				| RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD //
		, ENABLE);
	GPIO_InitStruct.GPIO_Pin = pinx;
	GPIO_InitStruct.GPIO_Mode = mode;
	GPIO_Init(portx, &GPIO_InitStruct);
	GPIO_SetBits(portx, pinx);
}
/*---------------------------------------------------------------------------------------
 * Brief: Setup IO
 * Param:	port	|	IN	|	port selection: A,B,C,D,E,F
 * 			pin		|	IN	|	pin selection: 1->15
 * 			dir		|	IN	|	direction selection
 *
 -----------------------------------------------------------------------------------------*/
void IO_setup(IO_PORT_t port, U8 pin, IO_DIR_t dir)
{
	GPIO_TypeDef *portx;
	U16 pinx;
	GPIOMode_TypeDef mode;
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB //
				| RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD //
		, ENABLE);
#if 1
	if (//
			((port == IOPORT_B) && (pin == 3))//
			|| ((port == IOPORT_A) && (pin == 13))//
			|| ((port == IOPORT_A) && (pin == 14))//
			|| ((port == IOPORT_A) && (pin == 15))//
		)
	{
		/* PB3: default is JTDO pin */
		/* PA13: default is JTMS-SWDIO */
		/* PA14: default is JTCK-SWCLK */
		/* PA15: default is JTDI */
		GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	}
#endif //
	GPIO_StructInit(&GPIO_InitStruct);
	switch (port)
	{
		case IOPORT_A: portx = GPIOA;
			break;
		case IOPORT_B: portx = GPIOB;
			break;
		case IOPORT_C: portx = GPIOC;
			break;
		case IOPORT_D: portx = GPIOD;
			break;
		default:
			return;
	}
	pinx = 1 << pin;
	switch (dir)
	{
		case IODIR_INF: mode = GPIO_Mode_IN_FLOATING;
			break;
		case IODIR_IPU: mode = GPIO_Mode_IPU;
			break;
		case IODIR_IPD: mode = GPIO_Mode_IPD;
			break;
		case IODIR_OPP: mode = GPIO_Mode_Out_PP;
			break;
		case IODIR_OOD: mode = GPIO_Mode_Out_OD;
			break;
		default:
			return;
	}
	GPIO_InitStruct.GPIO_Pin = pinx;
	GPIO_InitStruct.GPIO_Mode = mode;
	GPIO_Init(portx, &GPIO_InitStruct);
}
/*---------------------------------------------------------------------------------------
 * Brief: write IO
 * Param:	port	|	IN	|	port selection: A,B,C,D,E,F
 * 			pin		|	IN	|	pin selection: 1->15
 * 			val		|	IN	|	1:high 0:low
 *
 -----------------------------------------------------------------------------------------*/
void IO_wrt(IO_PORT_t port, U8 pin, U8 val)
{
	GPIO_TypeDef *portx;
	U16 pinx;

	switch (port)
	{
		case IOPORT_A: portx = GPIOA;
			break;
		case IOPORT_B: portx = GPIOB;
			break;
		case IOPORT_C: portx = GPIOC;
			break;
		case IOPORT_D: portx = GPIOD;
			break;
		default:
			return;
	}
	pinx = 1 << pin;
	GPIO_WriteBit(portx, pinx, val);
}
/*---------------------------------------------------------------------------------------
 * Brief: write IO
 * Param:	port	|	IN	|	port selection: A,B,C,D,E,F
 * 			pin		|	IN	|	pin selection: 1->15
 * 			val		|	IN	|	1:high 0:low
 *
 -----------------------------------------------------------------------------------------*/
void IO_toggle(IO_PORT_t port, U8 pin)
{
	GPIO_TypeDef *portx;
	U16 pinx;

	switch (port)
	{
		case IOPORT_A: portx = GPIOA;
			break;
		case IOPORT_B: portx = GPIOB;
			break;
		case IOPORT_C: portx = GPIOC;
			break;
		case IOPORT_D: portx = GPIOD;
			break;
		default:
			return;
	}
	pinx = 1 << pin;
	GPIO_WriteBit(portx, pinx, !GPIO_ReadOutputDataBit(portx, pinx));
}
/*---------------------------------------------------------------------------------------
 * Brief: write IO
 * Param:	port	|	IN	|	port selection: A,B,C,D,E,F
 * 			pin		|	IN	|	pin selection: 1->15
 * 			val		|	IN	|	1:high 0:low
 *
 -----------------------------------------------------------------------------------------*/
U8 IO_rd(IO_PORT_t port, U8 pin)
{
	GPIO_TypeDef *portx;
	U16 pinx;

	switch (port)
	{
		case IOPORT_A: portx = GPIOA;
			break;
		case IOPORT_B: portx = GPIOB;
			break;
		case IOPORT_C: portx = GPIOC;
			break;
		case IOPORT_D: portx = GPIOD;
			break;
		default:
			return 0;
	}
	pinx = 1 << pin;
	return GPIO_ReadInputDataBit(portx, pinx);
}
