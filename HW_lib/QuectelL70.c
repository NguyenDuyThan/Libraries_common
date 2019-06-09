/*-----------------------------------------------------------------------------------------
 * Author: Duy Vinh To
 * Team: DinhViSo corp.
 *
 * Created date: 7th July 2016
 *
 * Brief: GPS relating function
 ------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------------*/
#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "string.h"
#include "QuectelL70.h"
//#include "dbgPrint.h"
/*-----------------------------------------------------------------------------------------------
 * DEFINE
 ------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------
 * MACRO
 ------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------------*/
static void QL70_dbgPrt_nothing(const U8 *s, ...);
static int QL70_strsearch(const U8 *s, const U8 *sTarget);
static void QL70_strsplit(const U8* str, U8 chr, U8* out_head, U16 head_length, U8* out_tail, U16 tail_length);
static U8 QL70_strrmv(U8 *s, const U8 *sCut);
static double QL70_convDescartes(double input);
static void QL70_prints(const U8* s, U16 len);
static U8 QL70_verifyNMEA( const U8 *NMEA_line );
static U8 QL70_parseGGA(const U8 *GGA_line, GNSS_DAT_t *output );
static U8 QL70_parseRMC(const U8 *RMC_line, GNSS_DAT_t *output );
static U8 QL70_parseGSA(const U8 *GSA_line, GNSS_DAT_t *output );
static U8 QL70_sendPMTKcmd( const U8 *PMTK_cmd, U16 respLen, U8 *respPMTK );
static float fastsqrt( float val );
/*-----------------------------------------------------------------------------------------------
 * VARIABLES
 ------------------------------------------------------------------------------------------------*/
static U8 pos_NMEA_line = 0;
static U8 NMEA_line[NMEAMSG_SIZE] = "";
static U8 GSA_line[NMEAMSG_SIZE] = "";
static U8 RMC_line[NMEAMSG_SIZE] = "";
static U8 GGA_line[NMEAMSG_SIZE] = "";
static U8 PMTK_line[NMEAMSG_SIZE] = "";
static U8 stopCollectData = 0;

static QL70_printc_CB QL70_printc;
static QL70_delay10ms_CB QL70_delay10ms;
static QL70_dbgPrt_CB_t QL70_dbgPrt = QL70_dbgPrt_nothing;

/*-----------------------------------------------------------------------------------------------
 * FUNCTIONS
 ------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 Convert from GPS device Descartes to Google Map Descartes

 example:
 1223.45678
 1223.45678 / 100 = 12.2345678
 dst = 12 + 0.2345678 * 10 / 6
 *----------------------------------------------------------------------------*/
double QL70_convDescartes(double input)
{
	int intg;
	double dec;

	input /= 100;
	intg = (int)input;
	dec = input - intg;
	return (intg + (dec * 10 / 6));
}
/*-----------------------------------------------------------------------------------------
 * brief: string split
 * param[in]:const char, char, unsigned int, unsigned int
 * param[out]:char*,char*
 * retval:none
 *
 * details: use a character to split a string
 * Note: out_head and out_tail are protected (from overflow)
 * 			by head_length and tail_length (including NULL character at end of the string)
 * Example:
 * QL70_strsplit("AB C D", ' ', heads, 32, tails, 32);
 * Result: heads = "AB", tails = "C D"
 ------------------------------------------------------------------------------------------*/
void QL70_strsplit(const U8* str, U8 chr, U8* out_head, U16 head_length, U8* out_tail, U16 tail_length)
{
	U16 pos, pos_head = 0, pos_tail = 0, len = strlen(str);
	U8 flag_found = 0;

	if (len != 0)
	{
		for (pos = 0; pos < len; pos++)
		{
			if (flag_found == 1)
			{
				if ((pos_tail < (tail_length - 1)) || (tail_length == 0))
				{
					out_tail[pos_tail++] = str[pos];
				}
			}
			if ((str[pos] == chr) && (flag_found == 0))
			{
				flag_found = 1;
			}
			if ((str[pos] != chr) && (flag_found == 0))
			{
				if ((pos_head < (head_length - 1)) || (head_length == 0))
				{
					out_head[pos_head++] = str[pos];
				}
			}
		}
	}
	out_tail[pos_tail] = 0;
	out_head[pos_head] = 0;
}
/**************************************************************************
 * find position of a tring in another larger one
 *
 * return -1 if cannot find
 * 		  ><-1 if found, this is position of "str_fnd" in "str_in"
 **************************************************************************/
int QL70_strsearch(const U8 *s, const U8 *sTarget)
{
	U16 pos//
		, len = strlen(s)//
		, len_target = strlen(sTarget);
	if (len_target > len)
	{
		return -1;
	}
	for (pos = 0; pos <= (len - len_target); pos++)
	{
		if (strncmp(&s[pos], sTarget, len_target) == 0)
		{
			return pos;
		}
	}
	return -1;
}
/*****************************************************************************
 * remove a part (string format) off a string (1st wanted part found in string)
 *
 * return 0: found
 * 		  1: not found
 * Example:
 * inS = "ABCZ"
 * res = SL_cut(inS, "AB");
 * -> res = 1, inS = "CZ"
 *****************************************************************************/
U8 QL70_strrmv(U8 *s, const U8 *sCut)
{
	int res = QL70_strsearch(s, sCut);
	U16 len_cut = strlen(sCut);
	U16 len = strlen(s);
	if (res != -1)
	{
		while ((res + len_cut) <= len)
		{
			s[res] = s[res + len_cut];
			res++;
		}
		return 0;
	}
	else
	{
		return 1;
	}
}
#if 0
/*----------------------------------------------------------------------------
 Convert from string to double
 *----------------------------------------------------------------------------*/
double atof(const U8* str)
{
	U8 i, minus = 0;
	U32 val = 0, div = 0;
	double f = 0;

	if (str[0] == '-')
	{
		minus = 1;
		i = 1;
	}
	else
	{
		minus = 0;
		i = 0;
	}
	while (str[i])
	{
		if (str[i] == '.')
		{
			div = 1;
		}
		else if ((str[i] >= '0') && (str[i] <= '9'))
		{
			val = (val * 10) + (str[i] - '0');
			if (div)
			{
				div *= 10;
			}
		}
		else
		{
			return 0;
		}
		i++;
	}
	f = (double)val;
	if (div)
	{
		f = f / (double)div;
	}
	return (minus) ? -f : f;
}
#endif
/*------------------------------------------------------------------------------
 * brief:
 -------------------------------------------------------------------------------*/
void QL70_prints(const U8* s, U16 len)
{
	U16 i = 0;

	while (1)
	{
		QL70_printc(s[i]);
		i++;
		if (len)
		{
			if (i >= len)
			{
				return;
			}
		}
		else if (!s[i])
		{
			return;
		}
	}
}

/*------------------------------------------------------------------------------
 * Brief: Setup Quectel L70 module
 * Param:
 -------------------------------------------------------------------------------*/
void QL70_setup(QL70_printc_CB prtCB, QL70_delay10ms_CB delay10msCB)
{
	QL70_printc = prtCB;
	QL70_delay10ms = delay10msCB;
}
/*------------------------------------------------------------------------------
 * Brief:
 * Param:
 -------------------------------------------------------------------------------*/
void QL70_dbgPrt_nothing(const U8 *s, ...)
{
	return;
}
/*------------------------------------------------------------------------------
 * Brief: Setup Quectel L70 module debug print feature
 * Param:
 -------------------------------------------------------------------------------*/
void QL70_setup_dbgPrt(QL70_dbgPrt_CB_t dbgPrtCB)
{
	if (dbgPrtCB == NULL)
	{
		QL70_dbgPrt = QL70_dbgPrt_nothing;
	}
	else
	{
		QL70_dbgPrt = dbgPrtCB;
	}
}
/*------------------------------------------------------------------------------
 * brief
 * Ret:	0	|	OK
 * 		>0	|	FAIL
 -------------------------------------------------------------------------------*/
void QL70_getDat(GNSS_DAT_t *output)
{
	stopCollectData = 1; /* Avoid get new GNSS NMEA data when parsing */
//	QL70_dbgPrt("\r\n %s:GGAmsg=\%s\"", __func__, GGA_line);
//	QL70_dbgPrt("\r\n %s:GSAmsg=\%s\"", __func__, GSA_line);
//	QL70_dbgPrt("\r\n %s:RMCmsg=\%s\"", __func__, RMC_line);
	if (!QL70_parseGGA(GGA_line, output))
	{
		output->GGAavail = 1;
		QL70_dbgPrt("\r\n %s:GGA is Avail:F=%u Sat=%u HDOP=%f", __func__//
				, output->fixQuality, output->NumOfSat, output->HDOP);
	}
	else
	{
		output->GGAavail = 0;
		/* Only check GSA message when GGA is unavailable */
		if (!QL70_parseGSA(GSA_line, output))
		{
			output->GSAavail = 1;
			QL70_dbgPrt("\r\n %s:GSA is Avail:F3D=%u Sat=%u HDOP=%f", __func__//
					, output->fix3D, output->NumOfSat, output->HDOP);
			if (output->NumOfSat && (output->HDOP > 0) && (output->HDOP < 99))
			{
				output->fixQuality = 1;
			}
		}
		else
		{
			output->GSAavail = 0;
		}
	}
	if (!QL70_parseRMC(RMC_line, output))
	{
		output->RMCavail = 1;
		QL70_dbgPrt("\r\n %s:RMC is Avail:lat=%f lng=%f C=%u DT=%02u%02u%02u-%02u%02u%02u", __func__//
				, output->lat, output->lng, output->course//
				, output->dt.year, output->dt.mon, output->dt.day//
				, output->dt.hour, output->dt.min, output->dt.sec);
	}
	else
	{
		output->RMCavail = 0;
	}
	strcpy(GGA_line, "");
	strcpy(GSA_line, "");
	strcpy(RMC_line, "");
	output->NMEAavail = (output->GGAavail | output->GGAavail)  & output->RMCavail;
	stopCollectData = 0;
}
/*------------------------------------------------------------------------------
 * Brief:	This function used to add data from GNSS module.
 * Param:	c	|	IN	|	read byte from GNSS module.
 -------------------------------------------------------------------------------*/
void QL70_readc(U8 c)
{
	//detect end line
	if ((c == '\r') || (c == '\n'))
	{
		if (pos_NMEA_line)
		{
			if (!strncmp(NMEA_line, "$GPRMC", 6))
			{
				if (!stopCollectData)
				{
					memcpy(RMC_line, NMEA_line, NMEAMSG_SIZE);
				}
			}
			else if (!strncmp(NMEA_line, "$GPGSA", 6))
			{
				if (!stopCollectData)
				{
					memcpy(GSA_line, NMEA_line, NMEAMSG_SIZE);
				}
			}
			else if (!strncmp(NMEA_line, "$GPGGA", 6))
			{
				if (!stopCollectData)
				{
					strlcpy(GGA_line, NMEA_line, NMEAMSG_SIZE);
				}
			}
			else if (!strncmp(&NMEA_line[1], "PMT", 3))
			{
				if (!strlen(PMTK_line))
				{
					strlcpy(PMTK_line, NMEA_line, NMEAMSG_SIZE);
				}
			}
			goto L70_READC_RST_NMEA_LINE;
		}
	}
	else
	{
		NMEA_line[pos_NMEA_line++] = c;
		//line length is too long -> skip it
		if (pos_NMEA_line >= NMEAMSG_SIZE)
		{
			goto L70_READC_RST_NMEA_LINE;
		}
	}
	return;
	L70_READC_RST_NMEA_LINE://
	memset(NMEA_line, 0, NMEAMSG_SIZE);
	pos_NMEA_line = 0;
}
/********************************************************************************
 * @brief:		Verify NMEA data by checksum.
 * @param[in]:	const char*
 * @param[out]:	none
 * @retval:		int
 *
 * @details:
 ********************************************************************************/
inline U8 QL70_verifyNMEA( const U8* NMEA_line )
{
	U16 i = 0, i2 = 0, checksum = 0;
	U8 start_count_checksum = 0, end_count_checksum = 0;
	U8 checksum_inNMEA_str[5], checksum_str[5];

	i = 0;
	i2 = 0;
	////TermPrint_logGPS("--L70_verifyNMEAdat:[NMEA_line=%s]", NMEA_line);
	while( NMEA_line[i] != 0 )
	{
		switch( NMEA_line[i] )
		{
			case '$':
				start_count_checksum = 1;
				break;
			case '*':
				end_count_checksum = start_count_checksum;
				break;
			default:
				if (start_count_checksum)
				{
					if(!end_count_checksum)
					{
						checksum ^= NMEA_line[i];
					}
					else
					{
						checksum_inNMEA_str[i2] = NMEA_line[i];
						checksum_inNMEA_str[i2 + 1] = 0;
						i2++;
					}
				}
				break;
		}
		i++;
		if (i >= NMEAMSG_SIZE)
		{
			//UNKNOWN ERROR
			////TermPrint_logGPS("--L70_verifyNMEAdat:[E1]");
			return 0;
		}
	}
	if (!end_count_checksum)
	{
		////TermPrint_logGPS("--L70_verifyNMEAdat:[E2]");
		return 0;
	}
	if ((checksum == 0) || (checksum_inNMEA_str[0] == 0))
	{
		////TermPrint_logGPS("--L70_verifyNMEAdat:[E3]");
		return 0;
	}
	snprintf(checksum_str, 5, "%02X", checksum);
	checksum_str[4] = 0;
	i = 0;
	do
	{
		if (checksum_str[i] != checksum_inNMEA_str[i])
		{
			////TermPrint_logGPS("--L70_verifyNMEAdat:[E3:calc=%s emb=%s]", checksum_str, checksum_inNMEA_str);
			return 0;
		}
		i++;
	}
	while( checksum_str[i] != 0 );
	return 1;
}
/********************************************************************************
 * @brief:		getGPGGA
 * @param[in]:	none
 * @param[out]:	CAR*
 * @retval:		int
 *
 * @details:	get following information:
 * 	//+ location (latitude, longitude)
 * 	+ fix quality
 * 	+ HDOP
 * 	+ numb of Sat
 ********************************************************************************/
U8 QL70_parseGGA(const U8 *GGA_line, GNSS_DAT_t *output)
{
	const U8 LENGTH = 20;
	U8 subString[LENGTH];
	U8 i;
	U16 len = strlen(GGA_line);

	if (!strlen(GGA_line))
	{
		return 1;
	}
	if (!QL70_verifyNMEA(GGA_line))
	{
		return 1;
	}
	/* Parse GGA message */
	for (i = 0; i <= 8; i++)
	{
		QL70_strsplit((const)GGA_line, ',', subString, LENGTH, GGA_line, len + 1);
		switch (i)
		{
			/*
			case 2:
				 output->lat = QL70_convDescartes(atof(subString));
				break;
			case 3:
				if (subString[0] == 'S')
				{
					output->lat *= (double)-1;
				}
				break;
			case 4:
				output->lng = QL70_convDescartes(atof(subString));
				break;
			case 5:
				if (subString[0] == 'W')
				{
					output->lng *= (double)-1;
				}
				break;
				*/
			case 6:
				output->fixQuality = atoi(subString);
				break;
			case 7:
				output->NumOfSat = (U8) atoi(subString);
				break;
			case 8:
				output->HDOP = atof(subString);
				break;
			default:
				break;
		}
	}
	return 0;
}
/********************************************************************************
 * @brief:		getGPRMC
 * @param[in]:	none
 * @param[out]:	CAR*
 * @retval:		int
 *
 * @details:	get following information:
 *  + velocity
 * 	+ course
 * 	+ UNIX timestamp
 ********************************************************************************/
U8 QL70_parseRMC(const U8 *RMC_line, GNSS_DAT_t *output )
{
	const U8 LENGTH = 20;
	U8 course[LENGTH], subString[LENGTH], timeGPS[16];
	U8 i;
	U16 len = strlen(RMC_line);

	if (!strlen(RMC_line))
	{
		return 1;
	}
	if (!QL70_verifyNMEA(RMC_line))
	{
		return 1;
	}
	memset(timeGPS, 0, 16);
	for (i = 0; i <= 9; i++)
	{
		QL70_strsplit((const)RMC_line, ',', subString, LENGTH, RMC_line, len + 1);
		switch( i )
		{
			case 1:
				output->dt.hour = (subString[0] - '0') * 10 + (subString[1] - '0');
				output->dt.min = (subString[2] - '0') * 10 + (subString[3] - '0');
				output->dt.sec = (subString[4] - '0') * 10 + (subString[5] - '0');
				break;
			case 3: //GPS latitude
				output->lat = (float)QL70_convDescartes(atof(subString));
				break;
			case 4:
				if( subString[0] == 'S' )
				{
					output->lat *= (float)-1;
				}
				break;
			case 5: //GPS longitude
				output->lng = (float)QL70_convDescartes(atof(subString));
				break;
			case 6:
				if( subString[0] == 'W' )
				{
					output->lng *= (float)-1;
				}
				break;
			case 7:
				{
					float tSpd = 0;
					tSpd = (float)atof(subString);
					output->spd = (U16) (tSpd * 1.852 + 0.5);
				}
				break;
			case 8:
				QL70_strsplit((const)subString, '.', course, LENGTH, subString, LENGTH);
				output->course = atoi(course);
				break;
			case 9:
				output->dt.day = (subString[0] - '0') * 10 + (subString[1] - '0');
				output->dt.mon = (subString[2] - '0') * 10 + (subString[3] - '0');
				output->dt.year = (subString[4] - '0') * 10 + (subString[5] - '0');
				break;
			default:
				break;
		}
	}
	return 0;
}
/********************************************************************************
 * @brief:		getGPGSA
 * @param[in]:	none
 * @param[out]:	none
 * @retval:		int
 *
 * @details:	get following information:
 *  + fix 3D
 *  + HDOP
 *  + NumOfSat (manually calculating)
 ********************************************************************************/
U8 QL70_parseGSA(const U8 *GSA_line, GNSS_DAT_t *output )
{
	const U8 LENGTH = 20;
	U8 subString[LENGTH];
	U8 i = 0, satCt = 0;
	U16 len = strlen(GSA_line);

	if (!strlen(GSA_line))
	{
		return 1;
	}
	if (!QL70_verifyNMEA(GSA_line))
	{
		return 1;
	}
	/* Parse GSA message */
	for (i = 0; i <= 16; i++)
	{
		QL70_strsplit((const)GSA_line, ',', subString, LENGTH, GSA_line, len + 1);
		if (i == 2)
		{
			output->fix3D = atoi(subString);
		}
		else if ((i >= 3) && (i < 15))
		{
			if (strlen(subString))
			{
				satCt++;
			}
		}
		else if (i == 16)
		{
			output->HDOP = (float)atof(subString);
		}
	}
	output->NumOfSat = satCt;
	return 0;
}
/******************************************************************************
 * Brief:		Send PMTK command.
 *
 * Param:		PMTK_cmd	|	in	|	PMTK command to send (view below details).
 * 				respPMTK	|	out	|	responding of PMTK command.
 * 				respLen		|	in	|	length of respPMTK buffer (MUST include NULL character).
 *
 * Retval:		Is there any responding or not ?
 * 				0	|	Yes, module has responded.
 * 				>0	|	No, no responding.
 *
 * Details:		Input PMTK command:
 * 				+ Must begin with character '$'.
 * 				+ Must not include last character '*', checksum and <CR><LF>.
 * 				(auto generating checksum)
 ******************************************************************************/
U8 QL70_sendPMTKcmd( const U8 *PMTK_cmd, U16 respLen, U8 *respPMTK )
{
	U16 i = 0, len = 0, checksum = 0;
	U8 checksum_s[5];

	memset(PMTK_line, 0, NMEAMSG_SIZE);
	len = strlen(PMTK_cmd);
	for (i = 1; i < len; i++)
	{
		checksum ^= PMTK_cmd[i];
	}
	snprintf(checksum_s, 5, "*%02X", checksum);
	checksum_s[4] = 0;
	QL70_prints(PMTK_cmd, 0);
	QL70_prints(checksum_s, 0);
	QL70_prints("\r\n", 0);
	if (!respLen)
	{
		return 1;
	}
	for (i = 0; i < 20; i++)
	{
		if (strlen(PMTK_line))
		{
			//TermPrint_logGPS("sendPMTKcmd Resp=[%s]", PMTK_line);
			if (respPMTK != NULL)
			{
				strlcpy(respPMTK, PMTK_line, respLen);
			}
			return 0;
		}
		else
		{
			QL70_delay10ms(10);
		}
	}
	return 2;
}
/******************************************************************************
 * Brief: Wake up L70 from standby mode by sending ACK command.
 ******************************************************************************/
U8 QL70_sleep(void)
{
	return QL70_sendPMTKcmd("$PMTK161,0", 10, NULL); //setup standby mode
}
/******************************************************************************
 * Brief: Wake up L70 from standby mode by sending ACK command.
 ******************************************************************************/
U8 QL70_wkup(void)
{
	return QL70_sendPMTKcmd("$PMTK001", 10, NULL);
}
/******************************************************************************
 * Brief:		get GPS version string
 *
 * Retval:		0	|	OK
 * 				>0	|	FAIL
 ******************************************************************************/
U8 QL70_getVersion( U16 sLen, U8 *sVer )
{
	int i = 0, len = 0;

	if (QL70_sendPMTKcmd("$PMTK605", sLen, sVer))
	{
		return 1;
	}
	if (!strncmp(sVer, "$PMTK705,", strlen("$PMTK705,")))
	{
		QL70_strrmv(sVer, "$PMTK705,");
		len = strlen(sVer);
		for (i = len - 1; i > 0; i--)
		{
			if (sVer[i] == '*')
			{
				sVer[i] = 0;
				break;
			}
			sVer[i] = 0;
		}
		return 0;
	}
	return 2;
}
/***********************************************************************************************
 * @brief:replace for original sqrt() function in C
 *
 * Refer to this document link: http://en.wikipedia.org/wiki/Methods_of_computing_square_roots
 ***********************************************************************************************/
float fastsqrt( float val )
{
	union
	{
		int tmp;
		float val;
	} u;
	u.val = val;
	u.tmp -= 1 << 23; /* Remove last bit so 1.0 gives 1.0 */
	/* tmp is now an approximation to logbase2(val) */
	u.tmp >>= 1; /* divide by 2 */
	u.tmp += 1 << 29; /* add 64 to exponent: (e+127)/2 =(e/2)+63, */
	/* that represents (e/2)-64 but we want e/2 */
	return u.val;
}
/*--------------------------------------------------------------------------------------
 * Brief: calculate distance (in meter) between 2 coordinates
 * Param:	lat1	|	IN	| latitude 1
 * 			lng1	|	IN	| longitude 1
 * 			lat2	|	IN	| latitude 2
 * 			lng2	|	IN	| longitude 2
 * ret:	distance. Unit: meter
 ---------------------------------------------------------------------------------------*/
U32 calcDist(float lat1, float lng1, float lat2, float lng2)
{
	return (U32) ((fastsqrt((((lat1 - lat2) * (lat1 - lat2)) + ((lng1 - lng2) * (lng1 - lng2))))) * 111200);
}
