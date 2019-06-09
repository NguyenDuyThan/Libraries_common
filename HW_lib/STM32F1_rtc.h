/************************************************************************************
 * author:Duy Vinh
 *
 * description:
 * 	provide function to work with RTC module of STM32 MCU (Header file).
 *
 ************************************************************************************/
#ifndef STM32F1_RTC_H_
#define STM32F1_RTC_H_

/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/
#include "stm32f10x.h"

/*-----------------------------------------------------------------------------------------
 * DEFINES
 ------------------------------------------------------------------------------------------*/
#define REFYEAR	1970
/*-----------------------------------------------------------------------------------------
 * MACROS
 ------------------------------------------------------------------------------------------*/
#ifndef MIN2SEC
#define MIN2SEC(x)	(x * 60)
#endif // MIN2SEC
#ifndef SEC2MIN
#define SEC2MIN(x)	(x / 60)
#endif // SEC2MIN
#ifndef HOUR2MIN
#define HOUR2MIN(x)	(x * 60)
#endif // HOUR2SEC
#ifndef HOUR2SEC
#define HOUR2SEC(x)	MIN2SEC(HOUR2MIN(x))
#endif // HOUR2SEC

/*-----------------------------------------------------------------------------------------
 * TYPEDEF
 ------------------------------------------------------------------------------------------*/
typedef struct
{
	uint8_t day;
	uint8_t month;
	u16 year;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} DATETIME_t;

typedef enum
{
	IRTCCS_NO = 0,
	IRTCCS_LSE = 1,
	IRTCCS_LSI = 2,
	IRTCCS_HSE = 3,
	IRTCCS_HSI = 4,
} IRTC_CLKSEL_t;

typedef void (*RTC_interruptSec_CB)(void);
/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
extern DATETIME_t IRTC_utc2dt(uint32_t utc, int8_t GMT);
extern uint32_t IRTC_dt2utc(DATETIME_t dt);
/* Convert string to DATETIME_t structure */
extern DATETIME_t IRTC_s2dt(uint8_t opt, uint8_t* s);
extern uint32_t IRTC_getPrescaler(void);
extern void IRTC_enableBackupAccess(void);
extern IRTC_CLKSEL_t IRTC_setup(uint16_t PS32KHz, uint8_t outputTamper512KHz, RTC_interruptSec_CB cb);
extern void IRTC_setByDT(DATETIME_t dt);
extern void IRTC_setByUTC(uint32_t utc);
extern void IRTC_setAlrByDT(DATETIME_t dt);
extern DATETIME_t IRTC_getDT(int8_t GMT);
extern uint32_t IRTC_getUTC(void);
extern uint32_t IRTC_getSRT(void);
#if 0
extern void RTC_calib( uint32_t tick100usPerSec );
#endif

#endif // STM32F1_RTC_H_
