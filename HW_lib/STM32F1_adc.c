/************************************************************************************
 * author:Duy Vinh
 *
 * description:
 * 	provide function to work with ADC module of STM32 MCU (Source file).
 *
 ************************************************************************************/

/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/
#include "STM32F1_adc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x.h"

/*-----------------------------------------------------------------------------------------
 * DEFINE
 ------------------------------------------------------------------------------------------*/
#define ADC123IN13_PIN	GPIO_Pin_3
#define ADC123IN13_PORT	GPIOC
#define ADC123IN10_PIN	GPIO_Pin_0
#define ADC123IN10_PORT	GPIOC
#define ADC12IN9_PIN	GPIO_Pin_1
#define ADC12IN9_PORT	GPIOB
#define ADC123IN0_PIN	GPIO_Pin_0
#define ADC123IN0_PORT	GPIOA
#define ADC123IN1_PIN	GPIO_Pin_1
#define ADC123IN1_PORT	GPIOA
#define ADC123IN2_PIN	GPIO_Pin_2
#define ADC123IN2_PORT	GPIOA
#define ADC12IN8_PIN	GPIO_Pin_0
#define ADC12IN8_PORT	GPIOB
/*-----------------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------
 * VARIABLES
 ------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------
 * FUNCTIONS
 ------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------
 * brief:	Read ADC1 input 9
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 ADC1IN9_rd(U16 *val)
{
	unsigned int loop = 0;

	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	//unsigned int loop = 0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	//Deinit ADC----------------------------------------------------------
	ADC_Cmd(ADC1, DISABLE);
	ADC_DeInit(ADC1);
	//config ADC1 C13-----------------------------------------------------
	GPIO_InitStructure.GPIO_Pin = ADC12IN9_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC12IN9_PORT, &GPIO_InitStructure);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 1, ADC_SampleTime_71Cycles5);
	//enable ADC----------------------------------------------------------
	ADC_Cmd(ADC1, ENABLE);
	//calibrate ADC-------------------------------------------------------
	ADC_ResetCalibration(ADC1);
	while( ADC_GetResetCalibrationStatus(ADC1) );
	ADC_StartCalibration(ADC1);
	while( ADC_GetCalibrationStatus(ADC1) );
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while( ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET )
	{
		loop++;
		if( loop > 1000000 )
		{
			return 1;
		}
	}
	ADC_SoftwareStartConvCmd(ADC1, DISABLE);
	*val = ADC_GetConversionValue(ADC1);
	ADC_Cmd(ADC1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
	return 0;
}
/*------------------------------------------------------------------------------
 * brief:	Read ADC1 input 10
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 ADC1IN10_rd(U16 *val)
{
	unsigned int loop = 0;

	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	//unsigned int loop = 0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	//Deinit ADC----------------------------------------------------------
	ADC_Cmd(ADC1, DISABLE);
	ADC_DeInit(ADC1);
	//config ADC1 C13-----------------------------------------------------
	GPIO_InitStructure.GPIO_Pin = ADC123IN10_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC123IN10_PORT, &GPIO_InitStructure);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_71Cycles5);
	//enable ADC----------------------------------------------------------
	ADC_Cmd(ADC1, ENABLE);
	//calibrate ADC-------------------------------------------------------
	ADC_ResetCalibration(ADC1);
	while( ADC_GetResetCalibrationStatus(ADC1) );
	ADC_StartCalibration(ADC1);
	while( ADC_GetCalibrationStatus(ADC1) );
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while( ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET )
	{
		loop++;
		if( loop > 1000000 )
		{
			return 1;
		}
	}
	ADC_SoftwareStartConvCmd(ADC1, DISABLE);
	*val = ADC_GetConversionValue(ADC1);
	ADC_Cmd(ADC1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
	return 0;
}
/*------------------------------------------------------------------------------
 * brief:	Read ADC1 channel 13
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 ADC1IN13_rd(U16 *val)
{
	unsigned int loop = 0;

	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	//unsigned int loop = 0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	//Deinit ADC----------------------------------------------------------
	ADC_Cmd(ADC1, DISABLE);
	ADC_DeInit(ADC1);
	//config ADC1 C13-----------------------------------------------------
	GPIO_InitStructure.GPIO_Pin = ADC123IN13_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC123IN13_PORT, &GPIO_InitStructure);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 1, ADC_SampleTime_71Cycles5);
	//enable ADC----------------------------------------------------------
	ADC_Cmd(ADC1, ENABLE);
	//calibrate ADC-------------------------------------------------------
	ADC_ResetCalibration(ADC1);
	while( ADC_GetResetCalibrationStatus(ADC1) );
	ADC_StartCalibration(ADC1);
	while( ADC_GetCalibrationStatus(ADC1) );
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while( ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET )
	{
		loop++;
		if( loop > 1000000 )
		{
			return 1;
		}
	}
	ADC_SoftwareStartConvCmd(ADC1, DISABLE);
	*val = ADC_GetConversionValue(ADC1);
	ADC_Cmd(ADC1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
	return 0;
}
/*------------------------------------------------------------------------------
 * brief:	Read ADC1 channel 0: PA0
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 ADC1IN0_rd(U16 *val)
{
	unsigned int loop = 0;

	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	//unsigned int loop = 0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	//Deinit ADC----------------------------------------------------------
	ADC_Cmd(ADC1, DISABLE);
	ADC_DeInit(ADC1);
	//config ADC1 C13-----------------------------------------------------
	GPIO_InitStructure.GPIO_Pin = ADC123IN0_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC123IN0_PORT, &GPIO_InitStructure);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_71Cycles5);
	//enable ADC----------------------------------------------------------
	ADC_Cmd(ADC1, ENABLE);
	//calibrate ADC-------------------------------------------------------
	ADC_ResetCalibration(ADC1);
	while( ADC_GetResetCalibrationStatus(ADC1) );
	ADC_StartCalibration(ADC1);
	while( ADC_GetCalibrationStatus(ADC1) );
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while( ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET )
	{
		loop++;
		if( loop > 1000000 )
		{
			return 1;
		}
	}
	ADC_SoftwareStartConvCmd(ADC1, DISABLE);
	*val = ADC_GetConversionValue(ADC1);
	ADC_Cmd(ADC1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
	return 0;
}
/*------------------------------------------------------------------------------
 * brief:	Read ADC1 channel 1: PA1
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 ADC1IN1_rd(U16 *val)
{
	unsigned int loop = 0;

	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	//unsigned int loop = 0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	//Deinit ADC----------------------------------------------------------
	ADC_Cmd(ADC1, DISABLE);
	ADC_DeInit(ADC1);
	//config ADC1 C13-----------------------------------------------------
	GPIO_InitStructure.GPIO_Pin = ADC123IN1_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC123IN1_PORT, &GPIO_InitStructure);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_71Cycles5);
	//enable ADC----------------------------------------------------------
	ADC_Cmd(ADC1, ENABLE);
	//calibrate ADC-------------------------------------------------------
	ADC_ResetCalibration(ADC1);
	while( ADC_GetResetCalibrationStatus(ADC1) );
	ADC_StartCalibration(ADC1);
	while( ADC_GetCalibrationStatus(ADC1) );
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while( ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET )
	{
		loop++;
		if( loop > 1000000 )
		{
			return 1;
		}
	}
	ADC_SoftwareStartConvCmd(ADC1, DISABLE);
	*val = ADC_GetConversionValue(ADC1);
	ADC_Cmd(ADC1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
	return 0;
}
/*------------------------------------------------------------------------------
 * brief:	Read ADC1 channel 0
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 ADC1IN2_rd(U16 *val)
{
	U32 loop = 0;

	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	//unsigned int loop = 0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	//Deinit ADC----------------------------------------------------------
	ADC_Cmd(ADC1, DISABLE);
	ADC_DeInit(ADC1);
	//config ADC1 C13-----------------------------------------------------
	GPIO_InitStructure.GPIO_Pin = ADC123IN2_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC123IN2_PORT, &GPIO_InitStructure);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_71Cycles5);
	//enable ADC----------------------------------------------------------
	ADC_Cmd(ADC1, ENABLE);
	//calibrate ADC-------------------------------------------------------
	ADC_ResetCalibration(ADC1);
	while( ADC_GetResetCalibrationStatus(ADC1) );
	ADC_StartCalibration(ADC1);
	while( ADC_GetCalibrationStatus(ADC1) );
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while( ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET )
	{
		loop++;
		if( loop > 1000000 )
		{
			return 1;
		}
	}
	ADC_SoftwareStartConvCmd(ADC1, DISABLE);
	*val = ADC_GetConversionValue(ADC1);
	ADC_Cmd(ADC1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
	return 0;
}
/*------------------------------------------------------------------------------
 * brief:	Read ADC1 channel 8
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 ADC1IN8_rd(U16 *val)
{
	U32 loop = 0;

	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	//unsigned int loop = 0;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	//Deinit ADC----------------------------------------------------------
	ADC_Cmd(ADC1, DISABLE);
	ADC_DeInit(ADC1);
	//config ADC1 C13-----------------------------------------------------
	GPIO_InitStructure.GPIO_Pin = ADC12IN8_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(ADC12IN8_PORT, &GPIO_InitStructure);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_71Cycles5);
	//enable ADC----------------------------------------------------------
	ADC_Cmd(ADC1, ENABLE);
	//calibrate ADC-------------------------------------------------------
	ADC_ResetCalibration(ADC1);
	while( ADC_GetResetCalibrationStatus(ADC1) );
	ADC_StartCalibration(ADC1);
	while( ADC_GetCalibrationStatus(ADC1) );
	ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while( ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET )
	{
		loop++;
		if( loop > 1000000 )
		{
			return 1;
		}
	}
	ADC_SoftwareStartConvCmd(ADC1, DISABLE);
	*val = ADC_GetConversionValue(ADC1);
	ADC_Cmd(ADC1, DISABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
	return 0;
}
/*------------------------------------------------------------------------------
 * brief:	Read ADCx channel (IN)
 * Param:	ADCx	|	IN	|	ADC1->3
 * 			ch		|	IN	|	channel (0-15)
 *
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 ADC_rd(U8 ADCx, U8 ch, U16 *val)
{
	U32 loop = 0;
	U32 periph = 0;
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	ADC_TypeDef *ADC_x;
	//unsigned int loop = 0;

	switch (ADCx)
	{
		case 1:
			periph = RCC_APB2Periph_ADC1;
			ADC_x = ADC1;
			break;
		case 2:
			periph = RCC_APB2Periph_ADC2;
			ADC_x = ADC2;
			break;
		case 3:
			periph = RCC_APB2Periph_ADC3;
			ADC_x = ADC3;
			break;
		default:
			return 1;
	}
	RCC_APB2PeriphClockCmd(periph, ENABLE);
	//Deinit ADC----------------------------------------------------------
	ADC_Cmd(ADC_x, DISABLE);
	ADC_DeInit(ADC_x);
	//config ADC1 C13-----------------------------------------------------
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	switch (ch)
	{
		case 0:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			break;
		case 1:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			break;
		case 2:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			break;
		case 3:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			break;
		case 4:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			break;
		case 5:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			break;
		case 6:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			break;
		case 7:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
			GPIO_Init(GPIOA, &GPIO_InitStructure);
			break;
		case 8:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
			GPIO_Init(GPIOB, &GPIO_InitStructure);
			break;
		case 9:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
			GPIO_Init(GPIOB, &GPIO_InitStructure);
			break;
		case 10:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
			GPIO_Init(GPIOC, &GPIO_InitStructure);
			break;
		case 11:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
			GPIO_Init(GPIOC, &GPIO_InitStructure);
			break;
		case 12:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
			GPIO_Init(GPIOC, &GPIO_InitStructure);
			break;
		case 13:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
			GPIO_Init(GPIOC, &GPIO_InitStructure);
			break;
		case 14:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
			GPIO_Init(GPIOC, &GPIO_InitStructure);
			break;
		case 15:
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
			GPIO_Init(GPIOC, &GPIO_InitStructure);
			break;
		case 16: /* OnChip temperature sensor */
			break;
		default:
			return 2;
	}
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC_x, &ADC_InitStructure);
	ADC_RegularChannelConfig(ADC_x, ch, 1, ADC_SampleTime_71Cycles5);
	//enable ADC----------------------------------------------------------
	ADC_Cmd(ADC_x, ENABLE);
	//calibrate ADC-------------------------------------------------------
	ADC_ResetCalibration(ADC_x);
	while( ADC_GetResetCalibrationStatus(ADC_x) );
	ADC_StartCalibration(ADC_x);
	while( ADC_GetCalibrationStatus(ADC_x) );
	ADC_ClearFlag(ADC_x, ADC_FLAG_EOC);
	ADC_SoftwareStartConvCmd(ADC_x, ENABLE);
	while( ADC_GetFlagStatus(ADC_x, ADC_FLAG_EOC) == RESET )
	{
		loop++;
		if( loop > 1000000 )
		{
			return 3;
		}
	}
	ADC_SoftwareStartConvCmd(ADC_x, DISABLE);
	*val = ADC_GetConversionValue(ADC_x);
	ADC_Cmd(ADC_x, DISABLE);
	RCC_APB2PeriphClockCmd(periph, DISABLE);
	return 0;
}
