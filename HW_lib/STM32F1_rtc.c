/************************************************************************************
 * author:Duy Vinh
 *
 * description:
 * 	provide function to work with RTC module of STM32 MCU (Source file).
 *
 ************************************************************************************/

/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/
#include "stm32f10x_rcc.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_iwdg.h"
#include "misc.h"
#include "stm32f10x.h"
//#include "dbgPrint.h"
//#include "macro.h"
#include "STM32F1_rtc.h"
#include "stdlib.h"
#include "string.h"
#include "stddef.h"
#include "STM32F1_nvic.h"
#include "additionalIRQhdler.h"
/*-----------------------------------------------------------------------------------------
 * DEFINES
 ------------------------------------------------------------------------------------------*/
#define AUTO_DETECT_CLOCKSOURCE	1

/*-----------------------------------------------------------------------------------------
 * VARIABLES
 ------------------------------------------------------------------------------------------*/
extern RCC_ClocksTypeDef RCC_Clocks;
#if 0
static uint32_t prescaleRTC_val = 0;
#endif
static uint16_t month[14] = { 0, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
static uint16_t monthleap[14] = { 0, 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };
static uint16_t prescaler_32KHz = 32768;//40832;
static uint32_t sysRunTime = 0;
static uint32_t autoCalib_RT = 0;
static uint32_t autoCalib_lastSetUTC = 0;
static RTC_interruptSec_CB RTC_interruptSecCB = NULL;

/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
//extern uint32_t CountSec( DATETIME_t date );
//extern unsigned int CountDay( DATETIME_t date );
static uint8_t isLeapYear(uint16_t year);
/*-----------------------------------------------------------------------------------------
 * FUNCTION
 ------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
 * brief:		check if input year is a leap year
 * Return:		1	|	leap year
 * 				0	|	not leap year
 -------------------------------------------------------------------------------*/
uint8_t isLeapYear(uint16_t year)
{
	if(year % 4 == 0)
	{
		if(year % 100 == 0)
		{
			if(year % 400 == 0)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 1;
		}
	}
	else
	{
		return 0;
	}
}
/****************************************************************************
 * brief:	convert full date string to dateTime structure
 *
 * Param:	opt	|	I	|	convert option:
 * 							0: full conversion: input format:"ddMMyyhhmmss"
 * 							1: full conversion: input format:"ddMMyy" (Note: result will have hour = 0, min = 0, sec = 0)
 * 			s	|	I	|	string to convert
 *
 ****************************************************************************/
DATETIME_t IRTC_s2dt(uint8_t opt, uint8_t* s)
{
	DATETIME_t dt;
	char sTmp[4];

	switch (opt)
	{
		case 0://format: ddMMyyhhmmss
			strlcpy(sTmp, &s[0], 2 + 1);
			dt.day = atoi(sTmp);
			strlcpy(sTmp, &s[2], 2 + 1);
			dt.month = atoi(sTmp);
			strlcpy(sTmp, &s[4], 2 + 1);
			dt.year = atoi(sTmp);
			strlcpy(sTmp, &s[6], 2 + 1);
			dt.hour = atoi(sTmp);
			strlcpy(sTmp, &s[8], 2 + 1);
			dt.minute = atoi(sTmp);
			strlcpy(sTmp, &s[10], 2 + 1);
			dt.second = atoi(sTmp);
			break;
		case 1://format: <dd><mm><yy>
			dt.second = 0;
			dt.minute = 0;
			dt.hour = 0;
			strlcpy(sTmp, &s[0], 2 + 1);
			dt.day = atoi(sTmp);
			strlcpy(sTmp, &s[2], 2 + 1);
			dt.month = atoi(sTmp);
			strlcpy(sTmp, &s[4], 2 + 1);
			dt.year = atoi(sTmp);
			break;
#if 0
		case 17://format: <hh>:<mm>:<ss> <dd>-<mm>-<yy>
			LIB_strSub(6, 2, s, temp);
			dt->second = atoi(temp);
			LIB_strSub(3, 2, s, temp);
			dt->minute = atoi(temp);
			LIB_strSub(0, 2, s, temp);
			dt->hour = atoi(temp);
			LIB_strSub(15, 2, s, temp);
			dt->year = atoi(temp);
			LIB_strSub(12, 2, s, temp);
			dt->month = atoi(temp);
			LIB_strSub(9, 2, s, temp);
			dt->day = atoi(temp);
			break;
#endif
		default:
			dt.second = 0;
			dt.minute = 0;
			dt.hour = 0;
			dt.year = 0;
			dt.month = 0;
			dt.day = 0;
			break;
	}
	dt.year += 2000;
#if 0
	if (dt->hour > 23)
	{
		dt->hour = dt->hour - 24;
		dt->day++;
		switch (dt->month)
		{
			case 1:
			case 3:
			case 5:
			case 7:
			case 8:
			case 10:
			case 12:
				if (dt->day > 31)
				{
					dt->day = 1;
					dt->month++;
					if (dt->month > 12)
					{
						dt->year++;
						dt->month = 1;
					}
				}
				break;
			case 4:
			case 6:
			case 9:
			case 11:
				if (dt->day > 30)
				{
					dt->day = 1;
					dt->month++;
				}
				break;
			case 2:
				if (dt->day > 28)
				{
					if ((dt->year % 100 == 0) || (dt->year % 4 != 0))
					{
						dt->day = 1;
						dt->month++;
					}
				}
				if (dt->day > 29)
				{
					dt->day = 1;
					dt->month++;
				}
				break;
		}
	}
#endif
	return dt;
}
#if 0
/****************************************************************************
 * brief:	convert dateTime struct to date string
 *
 * Param:	date	|	I	|	input dateTime structure.
 * 			*str	|	O	|	output date string.
 * 			ssize	|	I	|	string size.
 *
 * Details:	date string format is only "ddMMyy"
 *
 ****************************************************************************/
void date_2_dateS(DateTime* date, char* str, uint16_t ssize)
{
	snprintf(str, ssize, "%02d%02d%02d", date->day, date->month, date->year);
//	if(strlen(str) != 6)
//	{
//		strlcpy(str, "000000", ssize);
//	}
}

/****************************************************************************
 * brief:	convert dateTime struct to full date string
 *
 * Param:	date	|	I	|	input dateTime structure
 * 			*str	|	O	|	output full date string
 *
 * Details:	full date string format is "ddMMyyhhmmss"
 *
 ****************************************************************************/
void date_2_FullDateS(DateTime date, char* str)
{
	snprintf(str, 13, "%02d%02d%02d%02d%02d%02d", date.day, date.month,
			date.year, date.hour, date.minute, date.second);
	if(strlen(str) != 12)
	{
		strlcpy(str, "000000000000", 13);
	}
}
#endif
/****************************************************************************
 * Brief:	convert UTC to dateTime structure
 *
 * Param:	utc	|	I	|	Unix timestamp
 * 			GMT	|	I	|	*google it yourself*
 * Ret: 'DATETIME_t' type
 ****************************************************************************/
DATETIME_t IRTC_utc2dt(uint32_t utc, int8_t GMT)
{
	DATETIME_t dt;
	uint32_t day;
	//uint16_t year;

	if(utc == 0)
	{
		dt.day = 0;
		dt.month = 0;
		dt.year = 0;
		dt.hour = 0;
		dt.minute = 0;
		dt.second = 0;
		return dt;
	}
	utc += HOUR2SEC(GMT);
	day = utc / HOUR2SEC(24);
	dt.year = REFYEAR;
	while(day > 366)
	{
		if (isLeapYear(dt.year))
		{
			day = day - 366;
		}
		else
		{
			day = day - 365;
		}
		dt.year++;
	}
	if ((isLeapYear(dt.year) == 1) && (day == 366))
	{
		day = 0;
		dt.year++;
	}
	else if ((isLeapYear(dt.year) != 1) && (day >= 365))
	{
		day = day - 365;
		dt.year++;
	}
	if (isLeapYear(dt.year))
	{
		for (uint8_t i = 0; i < 14; i++)
		{
			if (monthleap[i] > day)
			{
				dt.month = i - 1;
				dt.day = day - monthleap[i - 1] + 1;
				break;
			}
		}
	}
	else
	{
		for (uint8_t i = 0; i < 14; i++)
		{
			if (month[i] > day)
			{
				dt.month = i - 1;
				dt.day = day - month[i - 1] + 1;
				break;
			}
		}
	}
	dt.second = utc % 60;
	dt.minute = (utc / 60) % 60;
	dt.hour = (utc / HOUR2SEC(1)) % 24;
	return dt;
}
#if 0
/*******************************************************************************
 * brief:		convert UTC to full form date string
 *
 * param:		timestamp	|	I	|	input UTC
 * 				*date_str	|	O	|	output full form date string
 * 				GMT			|	I	|	*google it yourself*
 *
 * @details:	full form date string format is "dd-MM-yy hh:mm:ss"
 *******************************************************************************/
void UTC_2_dateS(uint32_t timestamp, char *date_str, unsigned int GMT)
{
	DateTime date = IRTC_utc2dt(timestamp);
	snprintf(date_str, 18, "%02d-%02d-%02d %02d:%02d:%02d", date.day,
			date.month, date.year, date.hour, date.minute, date.second);
}

/*******************************************************************************
 * brief:		convert UTC to time string
 *
 * param:		timestamp	|	I	|	input UTC
 * 				*date_str	|	O	|	output time string
 *
 * @details:	time string format is "hh:mm:ss"
 *******************************************************************************/
void UTC_2_timeS(uint32_t timestamp, char *time_str)
{
	DateTime date = IRTC_utc2dt(conv2Pos(timestamp));
	snprintf(time_str, 9, "%02d:%02d:%02d", date.hour, date.minute, date.second);
}
#endif
/*------------------------------------------------------------------------------
 * brief:		convert dateTime structure to UTC
 * param:		date	|	I	|	input dateTime structure
 * 				GMT		|	I	|	"google it yourself"
 * Return:		UTC
 -------------------------------------------------------------------------------*/
uint32_t IRTC_dt2utc(DATETIME_t dt)
{
	uint16_t year;
	uint32_t timestamp = dt.second;

	timestamp += MIN2SEC(dt.minute);
	timestamp += HOUR2SEC(dt.hour);
	timestamp += (dt.day - 1) * HOUR2SEC(24);
	if (!isLeapYear(dt.year))
	{
		timestamp += month[dt.month] * HOUR2SEC(24);
	}
	else
	{
		timestamp += monthleap[dt.month] * HOUR2SEC(24);
	}
	year = REFYEAR;
	while (year < dt.year)
	{
		if (!isLeapYear(year))
		{
			timestamp += 365 * HOUR2SEC(24);
		}
		else
		{
			timestamp += 366 * HOUR2SEC(24);
		}
		year++;
	}
	return timestamp;
}
#if 0
/*******************************************************************************
 * brief:		convert UTC to half form date structure
 *
 * param:		value	|	I	|	input UTC
 * 				*date	|	O	|	output half form date structure
 *
 * @details:	half form date structure means there are only hour, minute, second
 *******************************************************************************/
void RTC_utc2time(uint32_t value, DATETIME_t *date)
{
	date->second = value % 60;
	date->minute = (value / 60) % 60;
	date->hour = (value / HOUR2SEC(1));
}
#endif
/*------------------------------------------------------------------------------
 * Brief:
 -------------------------------------------------------------------------------*/
uint32_t IRTC_getPrescaler(void)
{
	return (RTC->PRLH << 16 | RTC->PRLL);
//	return RTCprescale;
}
/*------------------------------------------------------------------------------
 * Brief: setup RTC
 * Ret:	1	|	use LSI
 * 		0	|	use LSE
 -------------------------------------------------------------------------------*/
void IRTC_enableBackupAccess(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP | RCC_APB1Periph_PWR, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
}
/*------------------------------------------------------------------------------
 * Brief: setup RTC
 * Ret:	1	|	use LSI
 * 		0	|	use LSE
 -------------------------------------------------------------------------------*/
void IRTC_disableBackupAccess(void)
{
	//PWR_BackupAccessCmd(DISABLE);
}
/*------------------------------------------------------------------------------
 * Brief: setup RTC
 * Ret:
 -------------------------------------------------------------------------------*/
IRTC_CLKSEL_t IRTC_setup(uint16_t PS32KHz, uint8_t outputTamper512KHz, RTC_interruptSec_CB cb)
{
	IRTC_CLKSEL_t clkSel;
	uint32_t timepass = 100000;//, i = 0;

	prescaler_32KHz = PS32KHz;
#if AUTO_DETECT_CLOCKSOURCE
	DBG_print("\r\n Start");
	IRTC_enableBackupAccess();
	RCC_LSEConfig(RCC_LSE_ON);/* Enable LSE */
	DBG_print("\r\n Get PS");
	if (IRTC_getPrescaler() == 40000)
	{
		goto IRTC_SETUP_WITH_LSI;
	}
	DBG_print("\r\n Wait LSE");
	/* Check if it can be clocked by LSE */
	while (1)
	{
		if (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)
		{
			DBG_print("\r\n Sel LSE");
			clkSel = IRTCCS_LSE;
			break;
		}
		if (!timepass--)
		{
			clkSel = IRTCCS_LSI;
			break;
		}
	}
	IRTC_SETUP_WITH_LSI://
	/* It should be clocked by LSI */
	if (clkSel == IRTCCS_LSI)
	{
		DBG_print("\r\n Wait LSI");
		/* Wait till LSI is ready */
		RCC_LSICmd(ENABLE);/* Enable LSI */
		while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);

	}
	DBG_print("\r\n Sel CLKSRC");
	RCC_RTCCLKConfig((clkSel == IRTCCS_LSI) ? RCC_RTCCLKSource_LSI : RCC_RTCCLKSource_LSE);/* Select RTC Clock Source */
	DBG_print("\r\n En RTC");
	RCC_RTCCLKCmd(ENABLE);/* Enable RTC Clock */
	RTC_WaitForSynchro();/* Wait for RTC registers synchronization */
	RTC_WaitForLastTask();
	RTC_ITConfig(RTC_IT_ALR, DISABLE);
	RTC_WaitForLastTask(); /* Wait until last write operation on RTC registers is terminated */
	RTC_ITConfig(RTC_IT_SEC, ENABLE); /* Enable the RTC Second */
	RTC_WaitForLastTask(); /* Wait until last write operation on RTC registers is terminated */
	DBG_print("\r\n Set PS");
	RTC_SetPrescaler((clkSel == IRTCCS_LSI) ? 40000 : prescaler_32KHz);/* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
	RTC_WaitForLastTask();
	RTC_interruptSecCB = cb;
	if (outputTamper512KHz)
	{
		/* Disable the Tamper Pin */
		BKP_TamperPinCmd(DISABLE); /* To output RTCCLK/64 on Tamper pin, the tamper
									 functionality must be disabled */
		/* Enable RTC Clock Output on Tamper Pin */
		BKP_RTCOutputConfig(BKP_RTCOutputSource_CalibClock);
		//BKP_RTCOutputConfig(BKP_RTCOutputSource_Second);
	}
	NVIC_add(RTC_IRQn);
	return clkSel;
#else
	IRTC_enableBackupAccess();
	tmpUtc = RTC_GetCounter();
	DBG_print("\r\n %s:utc=%u", __func__, tmpUtc);
	if (clkSel == IRTCCS_NO)
	{
		goto IRTC_SETUP_END_CLKCFG;
	}
	if (tmpUtc > 60)
	{
		goto IRTC_SETUP_END_CLKCFG;
	}
	BKP_DRx = BKP_DR1;
	//DBG_print("\r\n IRTC_setup:step=%u", 1);
	for(i = 1; i <= BKP_DP_MAX; i++)
	{
		BKP_DR_dat[i - 1] = BKP_ReadBackupRegister(BKP_DRx);
		BKP_DRx += 4;
	}
	//DBG_print("\r\n IRTC_setup:step=%u", 2);
	BKP_DeInit();
	if(clkSel == IRTCCS_LSI)
	{
		goto RTC_CFG_USELSI;
	}
	RCC_LSEConfig(RCC_LSE_OFF);/* Enable LSE */
	RCC_LSEConfig(RCC_LSE_ON);/* Enable LSE */
	/* Wait till LSE is ready */
	timepass = 0;
	//DBG_print("\r\n %s:Wait LSE rdy", __func__);
	while (1)
	{
		if (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)
		{
			clkSel = IRTCCS_LSE;
		}
		if (maxWait != 0xFFFFFFFF)
		{
			if (timepass++ >= maxWait)
			{
				clkSel = IRTCCS_LSI;
				break;
			}
		}
	}
	//DBG_print("\r\n %s:use %s", __func__, (clkSel == IRTCCS_LSE) ? "LSE" : "LSI");
	if (clkSel == IRTCCS_LSE)
	{
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);/* Select LSE as RTC Clock Source */
		RTCprescale = LSE_32KHZ;
	}
	else if (clkSel == IRTCCS_LSI)
	{
		RTC_CFG_USELSI://
		RCC_LSEConfig(RCC_LSE_OFF);/* disable LSE */
		RCC_LSICmd(ENABLE);/* Enable LSI */
		/* Wait till LSI is ready */
		while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);/* Select LSI as RTC Clock Source */
		RTCprescale = 40000;
	}
	RCC_RTCCLKCmd(ENABLE);/* Enable RTC Clock */
	RTC_WaitForSynchro();/* Wait for RTC registers synchronization */
	RTC_WaitForLastTask();
	RTC_SetPrescaler(RTCprescale);/* RTC period = RTCCLK/RTC_PR */
	RTC_WaitForLastTask();
	IRTC_SETUP_END_CLKCFG://
	if (tmpUtc)
	{
		RTC_SetCounter(tmpUtc);
		RTC_WaitForLastTask();
	}
	RTC_ITConfig(RTC_IT_SEC, ENABLE); /* Enable the RTC Second + RTC Alarm interrupt */
	RTC_WaitForLastTask(); /* Wait until last write operation on RTC registers is terminated */
	RTC_interruptSecCB = cb;
	NVIC_add(RTC_IRQn);
	BKP_DRx = BKP_DR1;
	for(i = 1; i <= BKP_DP_MAX; i++)
	{
		BKP_WriteBackupRegister(BKP_DRx, BKP_DR_dat[i - 1]);
		BKP_DRx += 4;
	}
	return clkSel;
#endif
}

/*------------------------------------------------------------------------------
 * Brief:	update RTC by dateTime
 -------------------------------------------------------------------------------*/
void IRTC_setByDT( DATETIME_t dt )
{
	IRTC_setByUTC(IRTC_dt2utc(dt));
}
/*------------------------------------------------------------------------------
 * Brief:	update RTC by UTC
 -------------------------------------------------------------------------------*/
void IRTC_setByUTC( uint32_t utc )
{
	if( utc < 1000000000 )
	{
		return;/* unacceptable UTC */
	}
	RTC_SetCounter(utc);
	RTC_WaitForLastTask();
#if EN_AUTOCALIBRTC
	if (!autoCalib_lastSetUTC)
	{
		autoCalib_lastSetUTC = utc;
		autoCalib_RT = 0;
	}
	else
	{
		if ((utc >= (autoCalib_lastSetUTC + 10)) && (autoCalib_RT >= 10))
		{
			RTCprescale = RTCprescale * autoCalib_RT / (utc - autoCalib_lastSetUTC);
			DBG_print("\r\n --AutoCalibUTC[RT=%u UTC=%u/%u -> PC=%u]", autoCalib_RT, utc, autoCalib_lastSetUTC, RTCprescale);
			if (RTCprescale)
			{
				RTC_SetPrescaler(RTCprescale);/* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
				RTC_WaitForLastTask();
			}
			autoCalib_lastSetUTC = utc;
			autoCalib_RT = 0;
		}
	}
#endif // EN_AUTOCALIBRTC
}
/*******************************************************************************
 * @brief     	setAlarmRealtime
 * @param[in]  None
 * @param[out] None
 * @retval     None
 *
 * @details
 *******************************************************************************/
void IRTC_setAlrByDT( DATETIME_t dt )
{
	RTC_WaitForLastTask();
	RTC_SetAlarm(IRTC_dt2utc(dt));
	RTC_WaitForLastTask();
}
/*******************************************************************************
 * brief     	Get current time in 'DATETIME_t' type
 * param		GMT	|	IN	|	<Google yourself>
 * Ret			'DATETIME_t' type
 *******************************************************************************/
DATETIME_t IRTC_getDT(int8_t GMT)
{
	DATETIME_t dt = IRTC_utc2dt(RTC_GetCounter(), GMT);
	return dt;
}
/*------------------------------------------------------------------------------
 *
 -------------------------------------------------------------------------------*/
uint32_t IRTC_getUTC(void)
{
	return RTC_GetCounter();
}
/*------------------------------------------------------------------------------
 * Brief: get system runtime.
 -------------------------------------------------------------------------------*/
uint32_t IRTC_getSRT(void)
{
	return sysRunTime;
}

#if 0
/*******************************************************************************
 * @brief     	GetRealtime
 * @param[in]  None
 * @param[out] None
 * @retval     None
 *
 * @details
 *******************************************************************************/
void RTC_calib( uint32_t tick100usPerSec )
{
	TermPrint("--RTC_calib[Last RTC_prescaler=%u]\r\n", RTC_GetPrescaler());
	if( abs(tick100usPerSec - 9999) <= 2 )
	{
		return;
	}
	RTC_CALIB_BEGIN: //
	if( prescaleRTC_val == 0 )
	{
		if( RTCclockedByLSI == 0 )
		{
			prescaleRTC_val = 32767;
		}
		else
		{
			prescaleRTC_val = 40000;
		}
	}
	TermPrint("--RTC_calib[Last prescaleRTC_val=%u]\r\n", prescaleRTC_val);
	prescaleRTC_val = 9999 * prescaleRTC_val / tick100usPerSec;
	TermPrint("--RTC_calib[prescaleRTC_val=%u]\r\n", prescaleRTC_val);
	if( prescaleRTC_val > 41000 )
	{
		prescaleRTC_val = 0;
		goto RTC_CALIB_BEGIN;
	}
	BKPREG_setRTCprescale(prescaleRTC_val);
}
#endif
/*-------------------------------------------------------------------------------
 *
 --------------------------------------------------------------------------------*/
void RTC_IRQHandler(void)
{
	RTOS_enterIRQ();
	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
	{
#if AUTO_DETECT_CLOCKSOURCE
		if (IRTC_getPrescaler() != prescaler_32KHz)
		{
			/* RTC isn't being clocked by LSE */
			if (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)
			{
				RCC_RTCCLKCmd(DISABLE);/* Enable RTC Clock */
				RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);/* Select LSI as RTC Clock Source */
				RCC_RTCCLKCmd(ENABLE);/* Enable RTC Clock */
				RTC_WaitForSynchro();/* Wait for RTC registers synchronization */
				RTC_WaitForLastTask();
				RTC_SetPrescaler(prescaler_32KHz);/* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
				RTC_WaitForLastTask();
			}
		}
#endif
		autoCalib_RT++;
		sysRunTime++;
		if (RTC_interruptSecCB != NULL)
		{
			RTC_interruptSecCB();
		}
		RTC_ClearITPendingBit(RTC_IT_SEC);
		RTC_WaitForLastTask(); // Always put it here!
	}
	RTOS_exitIRQ();
}

