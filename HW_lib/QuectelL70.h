/*-----------------------------------------------------------------------------------------
 * Author: Duy Vinh To
 * Team: DinhViSo corp.
 *
 * Created date: 7th July 2016
 *
 * Brief: GPS relating function
 ------------------------------------------------------------------------------------------*/
#ifndef QUECTELL70_H_
#define QUECTELL70_H_
/*-----------------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------------*/
#define NMEAMSG_SIZE	128
/*-----------------------------------------------------------------------------------------------
 * TYPEDEFS
 -------------------------------------------s-----------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;
/* Callback: print/send a character to UART module */
typedef void (*QL70_printc_CB)(U8 c);
typedef void (*QL70_delay10ms_CB)(U32 t);

typedef void (*QL70_dbgPrt_CB_t)(const U8 *s, ...);

typedef struct
{
	U8 year;
	U8 mon;
	U8 day;
	U8 hour;
	U8 min;
	U8 sec;
}GNSS_DATETIME_t;

typedef struct
{
	U8 GGAavail;
	U8 GSAavail;
	U8 RMCavail;
	U8 NMEAavail; // If GGA and RMC are available, this will be also available.
	U8 fix3D;
	U8 fixQuality;
	U8 NumOfSat;
	float HDOP;
	float lat;
	float lng;
	GNSS_DATETIME_t dt;
	U16 spd;
	U16 course;
} GNSS_DAT_t;
/*-----------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------------*/
extern void QL70_setup(QL70_printc_CB prtCB, QL70_delay10ms_CB delay10msCB);
extern void QL70_setup_dbgPrt(QL70_dbgPrt_CB_t dbgPrtCB);
extern void QL70_readc(U8 c);
extern void QL70_getDat(GNSS_DAT_t *output);
extern U8 QL70_sleep(void);
extern U8 QL70_wkup(void);
extern U8 QL70_getVersion( U16 sLen, U8 *sVer );
extern U32 calcDist(float lat1, float lng1, float lat2, float lng2);

#endif // QUECTELL70_H_
