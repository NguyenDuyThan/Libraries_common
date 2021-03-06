/*
 * IOmapping.h
 *
 *  Created on: Dec 13, 2016
 *      Author: dv198
 */

#ifndef IOMAPPING_ITRACKINGV2_3_H_
#define IOMAPPING_ITRACKINGV2_3_H_

/* START */
#define APP_FLASHPAGE_ORIGIN	52
#define APP_FLASHPAGE_NUM		(179 - 52 + 1)

#define BKPDR_BOOTSEL			BKP_DR1
#define BKPDR_UPGFW_MAINVER		BKP_DR2
#define BKPDR_UPGFW_SUBVER		BKP_DR3
#define BKPDR_RESETEVENTS		BKP_DR4
#define BKPDR_EG_ADC			BKP_DR5
#define BKPDR_LAT_16BIT_H		BKP_DR6
#define BKPDR_LAT_16BIT_L		BKP_DR7
#define BKPDR_LNG_16BIT_H		BKP_DR8
#define BKPDR_LNG_16BIT_L		BKP_DR9

#define TERM_UARTPORT		1
#define GNSS_UARTPORT		3
#define GNSS_UART_BRATE		9600
#define GNSS_UART_BRATE2	4800
#define GSM_UARTPORT		2
#define CAM_UARTPORT		5

#define MCUSYSOFF_PORT		IOPORT_C//
#define MCUSYSOFF_PIN		0//5
#define GNSS_PWRCTRL_PORT	IOPORT_C//A
#define GNSS_PWRCTRL_PIN	5//8
#define GNSS_RST_PORT		IOPORT_B//
#define GNSS_RST_PIN		0//5
#define GSM_PWRDWN_PORT		IOPORT_C//
#define GSM_PWRDWN_PIN		13//3
#define QM95_DTR_PORT		IOPORT_C//B
#define QM95_DTR_PIN		1//7
#define EXTRS232_PWRCTRL_PORT 	IOPORT_C//A//GSM TTT
#define EXTRS232_PWRCTRL_PIN 	3//13
#define SDC_PWR_PIN			GPIO_Pin_4//13
#define SDC_PWR_PORT		GPIOB//
#define SDC_CS_PIN			GPIO_Pin_4//
#define SDC_CS_PORT			GPIOA//C
//#define LEDPWR_PORT			IOPORT_B//
//#define LEDPWR_PIN			8//
//#define LEDGSM_PORT			IOPORT_B//
//#define LEDGSM_PIN			3//14
//#define LEDLINK_PORT		IOPORT_B//
//#define LEDLINK_PIN			9//15
//#define LEDGNSS_PORT		IOPORT_C//
//#define LEDGNSS_PIN			8//6
//#define LEDLOGON_PORT		IOPORT_C//
//#define LEDLOGON_PIN		9//7
//#define LEDSDC_PORT			IOPORT_C//
//#define LEDSDC_PIN			10//8
#define EXTFLASH_CS_PORT	IOPORT_B//
#define EXTFLASH_CS_PIN		5//9
#define KEYSIGNAL_PORT		IOPORT_A//
#define KEYSIGNAL_PIN		8//14
#define DOORSIGNAL_PORT		IOPORT_A//
#define DOORSIGNAL_PIN		11//15
#define ACSIGNAL_PORT		IOPORT_B//
#define ACSIGNAL_PIN		6//3
#define BUZZER_PORT			IOPORT_B//A
#define BUZZER_PIN			6//1

#define VIN_ADCx			1
#define VIN_ADC_CH			10
#define ANALOGIN_ADCx		1
#define ANALOGIN_ADC_CH		9
#define VBAT3V7_ADCx		1
#define VBAT3V7_ADC_CH		8

#endif /* IOMAPPING_ITRACKINGV2_3_H_ */
