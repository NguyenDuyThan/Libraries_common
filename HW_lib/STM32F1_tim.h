/*------------------------------------------------------------------------------------------
 * INTRODUCE
 * Brief: This is header file contain function to setup Timer peripherals.
 * Author: Duy Vinh To - Team DinhViSo
 * Detail:
 -------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------
 * INCLUDES
 -------------------------------------------------------------------------------------------*/
#include "stm32f10x.h"
/*------------------------------------------------------------------------------------------
 * DEFINES
 -------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------
 * TYPEDEFS
 -------------------------------------------------------------------------------------------*/
typedef enum
{
	TIMFREQ_1Hz,
	TIMFREQ_100Hz,
	TIMFREQ_1KHz,
	TIMFREQ_5KHz,
	TIMFREQ_10KHz,
	TIMFREQ_50KHz,
	TIMFREQ_100KHz,
	TIMFREQ_1MHz,
}TIM_FREQ_t;
typedef void (*TIM_tick_CB)(void);
/*------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 -------------------------------------------------------------------------------------------*/
extern void TIM_setup(uint8_t tim, TIM_FREQ_t freq, TIM_tick_CB tickCB);
extern void TIM1CH1_PWM_setup(TIM_FREQ_t freq);
extern void TIM1CH1_PWM_set(uint16_t val);
extern void TIM2CH1_PWM_setup(TIM_FREQ_t freq);
extern void TIM2CH1_PWM_set(uint16_t val);
extern void TIM2CH2_PWM_setup(TIM_FREQ_t freq);
extern void TIM2CH2_PWM_set(uint16_t val);
extern void TIM2CH4_IC_setup(void);
extern uint32_t TIM2CH4_IC_get(void);
extern void TIM3CH1_IC_setup(void);
extern uint32_t TIM3CH1_IC_get(void);
extern void TIM3CH2_IC_setup(void);
extern uint32_t TIM3CH2_IC_get(void);
extern void TIM3CH1_PWM_setup(TIM_FREQ_t freq);
extern void TIM3CH1_PWM_set(uint16_t val);
extern void TIM3CH2_PWM_setup(TIM_FREQ_t freq);
extern void TIM3CH2_PWM_set(uint16_t val);
extern void TIM4CH1_PWM_setup(TIM_FREQ_t freq);
extern void TIM4CH1_PWM_set(uint16_t val);
extern void TIM4CH3_PWM_setup(TIM_FREQ_t freq);
extern void TIM4CH3_PWM_set(uint16_t val);
