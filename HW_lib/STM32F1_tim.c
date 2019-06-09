/*------------------------------------------------------------------------------------------
 * INTRODUCE
 * Brief: This is source file contain function to setup Timer peripherals.
 * Author: Duy Vinh To - Team DinhViSo
 * Detail:
 -------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------
 * INCLUDES
 -------------------------------------------------------------------------------------------*/
#include "stddef.h"
#include "STM32F1_tim.h"
#include "misc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "STM32F1_nvic.h"
#include "additionalIRQhdler.h"
/*------------------------------------------------------------------------------------------
 * DEFINES
 -------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
 * VARIABLES
 -------------------------------------------------------------------------------------------*/
static RCC_ClocksTypeDef RCC_Clocks;
static TIM_tick_CB TIM1_tick = (void*)0//
					, TIM2_tick = (void*)0//
					, TIM3_tick = (void*)0//
					, TIM4_tick = (void*)0//
					, TIM5_tick = (void*)0//
					;
static uint8_t TIM3CH2_IC_rdStep = 0//
		, TIM3CH1_IC_rdStep = 0//
		//, TIM2CH4_IC_rdStep = 0//
		;
static uint16_t TIM2CH4_IC_rdVal1 = 0, TIM2CH4_IC_rdVal2 = 0//
		, TIM3CH2_IC_rdVal1 = 0, TIM3CH2_IC_rdVal2 = 0, TIM3_CH2_IC_startTime = 0, TIM3_CH2_IC_endTime = 0//
		, TIM3CH1_IC_rdVal1 = 0, TIM3CH1_IC_rdVal2 = 0//
		;
static uint32_t TIM2CH4_IC_freq = 0, TIM3CH1_IC_freq = 0, TIM3CH2_IC_freq = 0, TIM3CH2_IC_OFct = 0;
static uint32_t TIM3CH2_ = 0;
/*----------------------------------------------------------------------------------------
 * Brief:	Setup Timer
 * Param:	tim		|	IN	|	timer selection. Only support timer 2 now.
 * 			freq	|	IN	|	frequency selection.
 * 			tickCB	|	IN	|	tick handler callback.
 -----------------------------------------------------------------------------------------*/
void TIM_setup(uint8_t tim, TIM_FREQ_t freq, TIM_tick_CB tickCB)
{
	TIM_TypeDef *TIMx;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	uint8_t TIMx_IRQn;

	RCC_GetClocksFreq(&RCC_Clocks);
	switch (tim)
	{
		case 1:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
			TIMx = TIM1;
			TIM1_tick = tickCB;
			TIMx_IRQn = TIM1_UP_IRQn;
			break;
		case 2:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
			TIMx = TIM2;
			TIM2_tick = tickCB;
			TIMx_IRQn = TIM2_IRQn;
			break;
		case 3:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
			TIMx = TIM3;
			TIM3_tick = tickCB;
			TIMx_IRQn = TIM3_IRQn;
			break;
		case 4:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
			TIMx = TIM4;
			TIM4_tick = tickCB;
			TIMx_IRQn = TIM4_IRQn;
			break;
#ifndef STM32F10X_HD
#else
#ifndef STM32F10X_HD_VL
#else
#ifndef STM32F10X_XL
#else
#ifndef STM32F10X_CL
#else
		case 5:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
			TIMx = TIM5;
			TIM5_tick = tickCB;
			TIMx_IRQn = TIM5_IRQn;
			break;
#endif // STM32F10X_CL
#endif // STM32F10X_XL
#endif // STM32F10X_HD_VL
#endif // STM32F10X_HD
		default:
			return;
	}
	switch (freq)
	{
		case TIMFREQ_1Hz:
			TIM_TimeBaseStructure.TIM_Period = 1000 + 1;
			TIM_TimeBaseStructure.TIM_Prescaler = (RCC_Clocks.SYSCLK_Frequency / 1000000) * 1000;
			TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
			break;
		case TIMFREQ_100Hz:
			TIM_TimeBaseStructure.TIM_Period = 1000 + 1;
			TIM_TimeBaseStructure.TIM_Prescaler = (RCC_Clocks.SYSCLK_Frequency / 1000000) * 10;
			TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
			break;
		case TIMFREQ_1KHz:
			TIM_TimeBaseStructure.TIM_Period = 1000 + 1;
			TIM_TimeBaseStructure.TIM_Prescaler = (RCC_Clocks.SYSCLK_Frequency / 1000000) * 1;
			TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
			break;
		case TIMFREQ_5KHz:
			TIM_TimeBaseStructure.TIM_Period = 500 + 1;
			TIM_TimeBaseStructure.TIM_Prescaler = (RCC_Clocks.SYSCLK_Frequency / 1000000) * 1;
			TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
			break;
		case TIMFREQ_10KHz:
			TIM_TimeBaseStructure.TIM_Period = 100 + 1;
			TIM_TimeBaseStructure.TIM_Prescaler = (RCC_Clocks.SYSCLK_Frequency / 1000000) * 1;
			TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
			break;
		case TIMFREQ_50KHz:
			TIM_TimeBaseStructure.TIM_Period = 20 + 1;
			TIM_TimeBaseStructure.TIM_Prescaler = (RCC_Clocks.SYSCLK_Frequency / 1000000) * 1;
			TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
			break;
		case TIMFREQ_100KHz:
			TIM_TimeBaseStructure.TIM_Period = 10 + 1;
			TIM_TimeBaseStructure.TIM_Prescaler = (RCC_Clocks.SYSCLK_Frequency / 1000000) * 1;
			TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
			break;
		case TIMFREQ_1MHz:
			TIM_TimeBaseStructure.TIM_Period = 1 + 1;
			TIM_TimeBaseStructure.TIM_Prescaler = (RCC_Clocks.SYSCLK_Frequency / 1000000) * 1;
			TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
			break;
		default:
			return;
	}
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIMx, &TIM_TimeBaseStructure);
	TIM_Cmd(TIMx, ENABLE);
	TIM_ITConfig(TIMx, TIM_IT_Update, (tickCB != NULL) ? ENABLE : DISABLE);
	if (tickCB != NULL)
	{
		NVIC_add(TIMx_IRQn);
	}
	else
	{
		NVIC_rmv(TIMx_IRQn);
	}
}
/*----------------------------------------------------------------------------------------
 * Brief: Setup PWM on Timer 1 channel 1: pin mapping to PA8
 -----------------------------------------------------------------------------------------*/
void TIM1CH1_PWM_setup(TIM_FREQ_t freq)
{
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	TIM_setup(1, freq, (void*)0);
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High; //TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}
/*----------------------------------------------------------------------------------------
 * Brief:
 -----------------------------------------------------------------------------------------*/
void TIM1CH1_PWM_set(uint16_t val)
{
	TIM_SetCompare1(TIM1, val);
}
/*----------------------------------------------------------------------------------------
 * Brief: Setup PWM on Timer 2 channel 2: pin mapping to PA0
 * Param:	freq	|	IN	|	setup frequency
 * Note:
 * - Partial remap 1:
 *   + TIM2CH1: PA0 -> PA15
 *   + TIM2CH2: PA1 -> PB3
 *   + TIM2CH3: PA2
 *   + TIM2CH4: PA3
 -----------------------------------------------------------------------------------------*/
void TIM2CH1_PWM_setup(TIM_FREQ_t freq)
{
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	TIM_setup(2, freq, (void*)0);
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High; //TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

	TIM_OC1Init(TIM2, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	TIM_CtrlPWMOutputs(TIM2, ENABLE);
}
/*----------------------------------------------------------------------------------------
 * Brief:
 -----------------------------------------------------------------------------------------*/
void TIM2CH1_PWM_set(uint16_t val)
{
	TIM_SetCompare1(TIM2, val);
}
/*----------------------------------------------------------------------------------------
 * Brief: Setup PWM on Timer 2 channel 2: pin mapping to PB3 (Setup partial remap 1)
 * Param:	freq	|	IN	|	setup frequency
 * Note:
 * - Partial remap 1:
 *   + TIM2CH1: PA0 -> PA15
 *   + TIM2CH2: PA1 -> PB3
 *   + TIM2CH3: PA2
 *   + TIM2CH4: PA3
 -----------------------------------------------------------------------------------------*/
void TIM2CH2_PWM_setup(TIM_FREQ_t freq)
{
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	TIM_setup(2, freq, (void*)0);
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High; //TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

	TIM_OC2Init(TIM2, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);

	GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	TIM_CtrlPWMOutputs(TIM2, ENABLE);
}
/*----------------------------------------------------------------------------------------
 * Brief:
 -----------------------------------------------------------------------------------------*/
void TIM2CH2_PWM_set(uint16_t val)
{
	TIM_SetCompare2(TIM2, val);
}
/*----------------------------------------------------------------------------------------
 * Brief: Setup input capture feature on Timer 2 channel 4: pin mapping PA3
 -----------------------------------------------------------------------------------------*/
void TIM2CH4_IC_setup(void)
{
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_GetClocksFreq(&RCC_Clocks);
	/* Enable clock source */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Register to NVIC */
	NVIC_add(TIM2_IRQn);
	/* Configure TIM input capture */
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_ICInit(TIM2, &TIM_ICInitStructure);
	/* TIM enable counter */
	TIM_Cmd(TIM2, ENABLE);
	/* Enable the CC2 Interrupt Request */
	TIM_ITConfig(TIM2, TIM_IT_CC4, ENABLE);
}
/*----------------------------------------------------------------------------------------
 * Brief:
 -----------------------------------------------------------------------------------------*/
uint32_t TIM2CH4_IC_get(void)
{
	uint32_t ret = TIM2CH4_IC_freq;
	TIM2CH4_IC_freq = 0;
	return ret;
}
/*----------------------------------------------------------------------------------------
 * Brief: Setup input capture feature on Timer 3 channel 1: pin mapping PA6
 -----------------------------------------------------------------------------------------*/
void TIM3CH1_IC_setup(void)
{
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_GetClocksFreq(&RCC_Clocks);
	/* Enable clock source */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	/* TIM3 channel 2 pin (PA.07) configuration */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Register to NVIC */
	NVIC_add(TIM3_IRQn);
	/* Configure TIM input capture */
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);
	/* TIM enable counter */
	TIM_Cmd(TIM3, ENABLE);
	/* Enable the CC2 Interrupt Request */
	TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
}
/*----------------------------------------------------------------------------------------
 * Brief:
 -----------------------------------------------------------------------------------------*/
uint32_t TIM3CH1_IC_get(void)
{
	uint32_t ret = TIM3CH1_IC_freq;
	TIM3CH1_IC_freq = 0;
	return TIM3CH1_IC_freq;
}
/*----------------------------------------------------------------------------------------
 * Brief: Setup input capture feature on Timer 3 channel 2: pin mapping PA7
 -----------------------------------------------------------------------------------------*/
void TIM3CH2_IC_setup(void)
{
	TIM_ICInitTypeDef TIM_ICInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_GetClocksFreq(&RCC_Clocks);
	/* Enable clock source */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = (RCC_Clocks.SYSCLK_Frequency / 1000000) - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
	/* TIM3 channel 2 pin (PA.07) configuration */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* Configure TIM input capture */
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);
	/* Register to NVIC */
	NVIC_add(TIM3_IRQn);
	/* Enable the CC2 Interrupt Request */
	TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);
	/* TIM enable counter */
	TIM_Cmd(TIM3, ENABLE);
}
/*----------------------------------------------------------------------------------------
 * Brief: Read input capture on Timer 3 channel 2: pin mapping PA7
 -----------------------------------------------------------------------------------------*/
uint32_t TIM3CH2_IC_get(void)
{
	uint32_t ret = TIM3CH2_IC_freq;
	TIM3CH2_IC_freq = 0;
	return ret;
}
/*----------------------------------------------------------------------------------------
 * Brief: Setup PWM on Timer 3 channel 1: pin mapping to PA6
 * Param:	freq	|	IN	|	setup frequency
 * Note:
 * - Partial remap:
 *   + TIM3CH1: PA6 -> PB4
 *   + TIM3CH2: PA7 -> PB5
 *   + TIM3CH3: PB0
 *   + TIM3CH4: PB1
 -----------------------------------------------------------------------------------------*/
void TIM3CH1_PWM_setup(TIM_FREQ_t freq)
{
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	TIM_setup(3, freq, (void*)0);
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High; //TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

	TIM_OC1Init(TIM3, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	TIM_CtrlPWMOutputs(TIM3, ENABLE);
}
/*----------------------------------------------------------------------------------------
 * Brief:
 -----------------------------------------------------------------------------------------*/
void TIM3CH1_PWM_set(uint16_t val)
{
	TIM_SetCompare1(TIM3, val);
}
/*----------------------------------------------------------------------------------------
 * Brief: Setup PWM on Timer 3 channel 2: pin mapping to PB5 (Setup partial remap)
 * Param:	freq	|	IN	|	setup frequency
 * Note:
 * - Partial remap:
 *   + TIM3CH1: PA6 -> PB4
 *   + TIM3CH2: PA7 -> PB5
 *   + TIM3CH3: PB0
 *   + TIM3CH4: PB1
 -----------------------------------------------------------------------------------------*/
void TIM3CH2_PWM_setup(TIM_FREQ_t freq)
{
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	TIM_setup(3, freq, (void*)0);
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High; //TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

	TIM_OC2Init(TIM3, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	TIM_CtrlPWMOutputs(TIM3, ENABLE);
}
/*----------------------------------------------------------------------------------------
 * Brief:
 -----------------------------------------------------------------------------------------*/
void TIM3CH2_PWM_set(uint16_t val)
{
	TIM_SetCompare2(TIM3, val);
}
/*----------------------------------------------------------------------------------------
 * Brief: Setup PWM on Timer 4 channel 1: pin mapping to PB6
 * Param:	freq	|	IN	|	setup frequency
 -----------------------------------------------------------------------------------------*/
void TIM4CH1_PWM_setup(TIM_FREQ_t freq)
{
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	TIM_setup(4, freq, (void*)0);
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High; //TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	TIM_CtrlPWMOutputs(TIM4, ENABLE);
}
/*----------------------------------------------------------------------------------------
 * Brief:
 -----------------------------------------------------------------------------------------*/
void TIM4CH1_PWM_set(uint16_t val)
{
	TIM_SetCompare1(TIM4, val);
}
/*----------------------------------------------------------------------------------------
 * Brief: Setup PWM on Timer 4 channel 3: pin mapping to PB9
 * Param:	freq	|	IN	|	setup frequency
 -----------------------------------------------------------------------------------------*/
void TIM4CH3_PWM_setup(TIM_FREQ_t freq)
{
	TIM_OCInitTypeDef TIM_OCInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	TIM_setup(4, freq, (void*)0);
	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //TIM_OCPolarity_Low;
	TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High; //TIM_OCNPolarity_Low;
	TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

	TIM_OC3Init(TIM4, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	TIM_CtrlPWMOutputs(TIM4, ENABLE);
}
/*----------------------------------------------------------------------------------------
 * Brief:
 -----------------------------------------------------------------------------------------*/
void TIM4CH3_PWM_set(uint16_t val)
{
	TIM_SetCompare3(TIM4, val);
}
/*----------------------------------------------------------------------------------------
 * Brief:	Timer 4 interrupt handler.
 -----------------------------------------------------------------------------------------*/
void TIM1_IRQHandler(void)
{
	RTOS_enterIRQ();
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
		if (TIM1_tick != (void*)0)
		{
			TIM1_tick();
		}
	}
	RTOS_exitIRQ();
}
/*----------------------------------------------------------------------------------------
 * Brief:	Timer 2 interrupt handler.
 -----------------------------------------------------------------------------------------*/
void TIM2_IRQHandler(void)
{
	RTOS_enterIRQ();
	if (TIM_GetITStatus(TIM2, TIM_IT_CC4) == SET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC4);
		if(TIM2CH4_IC_rdVal2 == 0)
		{
			/* Get the Input Capture value */
			TIM2CH4_IC_rdVal1 = TIM_GetCapture4(TIM2);
			TIM2CH4_IC_rdVal2 = 1;
		}
		else if(TIM2CH4_IC_rdVal2 == 1)
		{
			/* Get the Input Capture value */
			TIM2CH4_IC_rdVal2 = TIM_GetCapture4(TIM2);
			/* Capture computation */
			if (TIM2CH4_IC_rdVal2 > TIM2CH4_IC_rdVal1)
			{
				TIM2CH4_IC_freq = (uint32_t) RCC_Clocks.SYSCLK_Frequency / (TIM2CH4_IC_rdVal2 - TIM2CH4_IC_rdVal1);
				//Capture = TIM3CH2_IC_rdVal2 - TIM3CH2_IC_rdVal1;
			}
			else
			{
				//TIM2CH4_IC_freq = (uint32_t) RCC_Clocks.SYSCLK_Frequency / ((0xFFFF - TIM2CH4_IC_rdVal1) + TIM2CH4_IC_rdVal2);
			}
			TIM2CH4_IC_rdVal2 = 0;
		}
	}
	else if(TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		if (TIM2_tick != (void*)0)
		{
			TIM2_tick();
		}
	}
	RTOS_exitIRQ();
}
/*----------------------------------------------------------------------------------------
 * Brief:	Timer 3 interrupt handler.
 -----------------------------------------------------------------------------------------*/
void TIM3_IRQHandler(void)
{
	RTOS_enterIRQ();
	if(TIM_GetITStatus(TIM3, TIM_IT_CC1) == SET)
	{
		 TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
	    if(TIM3CH1_IC_rdStep == 0)
	    {
			/* Get the Input Capture value */
	    	TIM3CH1_IC_rdVal1 = TIM_GetCapture1(TIM3);
			TIM3CH1_IC_rdStep = 1;
	    }
	    else if(TIM3CH1_IC_rdStep == 1)
	    {
	    	uint32_t tmp = 0;
			/* Get the Input Capture value */
	    	TIM3CH1_IC_rdVal2 = TIM_GetCapture1(TIM3);
			/* Capture computation */
			if (TIM3CH1_IC_rdVal2 > TIM3CH1_IC_rdVal1)
			{
				tmp = TIM3CH1_IC_rdVal2 - TIM3CH1_IC_rdVal1;
			}
			else
			{
				tmp = 0xFFFF - TIM3CH1_IC_rdVal1;
				tmp += TIM3CH1_IC_rdVal2;
			}
			TIM3CH1_IC_freq = RCC_Clocks.SYSCLK_Frequency / tmp;
			TIM3CH1_IC_rdStep = 0;
	    }
	}
	else if(TIM_GetITStatus(TIM3, TIM_IT_CC2) == SET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
//		if(TIM3CH2_IC_rdStep == 0)
//		{
//			/* Get the Input Capture value */
//			TIM3CH2_IC_rdVal1 = TIM_GetCapture2(TIM3);
//			TIM3CH2_IC_rdStep = 1;
//		}
//		else if(TIM3CH2_IC_rdStep == 1)
//		{
//			/* Get the Input Capture value */
//			TIM3CH2_IC_rdVal2 = TIM_GetCapture2(TIM3);
//			/* Capture computation */
//			if (TIM3CH2_IC_rdVal2 > TIM3CH2_IC_rdVal1)
//			{
//				TIM3CH2_IC_freq += (uint32_t)(TIM3CH2_IC_rdVal2 - TIM3CH2_IC_rdVal1);
//			}
//			else
//			{
//				TIM3CH2_IC_freq += ((uint32_t)(0xFFFF - TIM3CH2_IC_rdVal1) + TIM3CH2_IC_rdVal2);
//			}
//			TIM3CH2_IC_rdStep = 0;
//		}
		TIM3_CH2_IC_endTime = TIM_GetCapture2(TIM3);
		TIM3CH2_IC_freq = abs((TIM3CH2_IC_OFct << 16) - TIM3_CH2_IC_startTime + TIM3_CH2_IC_endTime);
		TIM3_CH2_IC_startTime = TIM3_CH2_IC_endTime;
		TIM3CH2_IC_OFct = 0;
	}
	else if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
		TIM3CH2_IC_OFct++;
		if (TIM3_tick != (void*)0)
		{
			TIM3_tick();
		}
	}
	RTOS_exitIRQ();
}
/*----------------------------------------------------------------------------------------
 * Brief:	Timer 4 interrupt handler.
 -----------------------------------------------------------------------------------------*/
void TIM4_IRQHandler(void)
{
	RTOS_enterIRQ();
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	if (TIM4_tick != (void*)0)
	{
		TIM4_tick();
	}
	RTOS_exitIRQ();
}
/*----------------------------------------------------------------------------------------
 * Brief:	Timer 4 interrupt handler.
 -----------------------------------------------------------------------------------------*/
void TIM5_IRQHandler(void)
{
	RTOS_enterIRQ();
	TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
	if (TIM5_tick != (void*)0)
	{
		TIM5_tick();
	}
	RTOS_exitIRQ();
}
