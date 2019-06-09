/************************************************************************************
 * author:Duy Vinh
 *
 * description:
 * 	provide function to work with Quectel M95 (Source file).
 *
 ************************************************************************************/

/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/
#include "SIM900.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stddef.h"
//#include "dbgPrint.h"
/*-----------------------------------------------------------------------------------------
 * MACROS
 ------------------------------------------------------------------------------------------*/
#define NOT(x)			(~x)
/* Advanced bitwise operations */
#define BIT(p)			(0x1 << p)
#define GETBIT(x,p)		((x & BIT(p)) ? 1 : 0)
#define SETBIT(x,p)		x |= BIT(p)
#define CLRBIT(x,p)		x &= NOT(BIT(p))
/*-----------------------------------------------------------------------------------------
 * DEFINE
 ------------------------------------------------------------------------------------------*/
#define CIPMODE_SEL		0
#define CIPMUX_SEL		1
#define CIPRXGET_SEL	1
#define CTRLZ_KEY		0x1A
#define ECS_KEY			0x1B
#define CONNSTAT_LIST	{"IP INITIAL"\
						, "IP START"\
						, "IP CONFIG"\
						, "IP IND"\
						, "IP GPRSACT"\
						, "IP STATUS"\
						, "TCP CONNECTING"\
						, "UDP CONNECTING"\
						, "IP CLOSE"\
						, "CONNECT OK"\
						, "PDP DEACT"\
						}
#define CONNSTAT_MAXINDEX			11
#define CONNSTAT_INDEX_IPINITIAL	0
#define CONNSTAT_INDEX_IPSTART		1
#define CONNSTAT_INDEX_IPCONFIG		2
#define CONNSTAT_INDEX_IPIND		3
#define CONNSTAT_INDEX_IPGPRSACT	4
#define CONNSTAT_INDEX_IPSTATUS		5
#define CONNSTAT_INDEX_TCPCONNECTING	6
#define CONNSTAT_INDEX_UDPCONNECTING	7
#define CONNSTAT_INDEX_IPCLOSE		8
#define CONNSTAT_INDEX_CONNECTOK	9
#define CONNSTAT_INDEX_PDPDEACT		10

#define SIM900_SPECRESP_INDEX_3E			1 // 3E is character '>'
#define SIM900_SPECRESP_INDEX_SENDOK		2
#define SIM900_SPECRESP_INDEX_CMGS		3

#define SOCKINDEX_MAX		8 //From 0 to 5
/* Below is socket index, used when enable mux mode on GSM module */
#define NATS_SOCKINDEX		1
#define NTP_SOCKINDEX		2

#define SOCKRXINDEXSTACK_MAX	SOCKINDEX_MAX
#if (SOCKRXINDEXSTACK_MAX < SOCKRXINDEXSTACK_MAX)
#error "INVALID !!!"
#endif
/*-----------------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
static void SIM900_dbgPrt_nothing(const U8 *s, ...);
static int SIM900_strsearch(const U8 *sRoot, const U8 *sTarget);
static void SIM900_strsplit(const U8* str, U8 chr, U8* out_head, U16 head_length, U8* out_tail, U16 tail_length);
static U8 SIM900_strrmv(U8 *s, const U8 *sCut);
static void SIM900_bufRst(void);
static void SIM900_bufAdd(U8 c);
static U8 SIM900_bufGet(U8 *c);
static U16 SIM900_bufGets(U16 timeout, U16 size, U8 *s);
static U16 SIM900_bufGetResp(U16 timeout, U16 size, const U8 *header, U8 *s);
static void SIM900_bufClr(void);
static U16 SIM900_bufGetLen(void);
static U8 SIM900_bufWaitResp(U16 timeout, const U8 *s);
static void SIM900_prints(U16 len, const U8 *s);
static U8 SIM900_offEcho(void);
static U8 SIM900_readFile(const U8 *fname, U32 ofs, U32 *rSize, U8 *rDat);
//static U8 NETSRVC_genChksum(const U8 *s, U16 len);
static U8 SIM900_getCIPMUX(void);
static U8 SIM900_setCIPMODE(U8 mode);
static U8 SIM900_setCIPMUX(U8 mode);
static U8 SIM900_setCIPRXGET(U8 mode);
/*-----------------------------------------------------------------------------------------
 * VARIABLES
 ------------------------------------------------------------------------------------------*/
/* Constant variables */
static const U8 *connStatList[] = CONNSTAT_LIST;
static const U8 singleErrDbg[] = "\r\n %s:err";
/**/
static SIM900_printc_CB SIM900_printc;
static SIM900_reset_CB SIM900_reset;
static SIM900_delay10ms_CB	SIM900_delay10ms;
static SIM900_dbgPrt_CB_t SIM900_dbgPrt = SIM900_dbgPrt_nothing;
static U8 *SIM900_buf = NULL;
static U16 SIM900_bufsize = 0;
static U16 SIM900_bufIn = 0;
static U16 SIM900_bufOut = 0;
static U16 SIM900_bufLen = 0;
static U8 SIM900_bufLineCount = 0;
static U8 SIM900_bufOutNoCRLFdecLine = 0;
static U8 SIM900_unsolMsg[SIM900_UNSOLMSG_SIZE];
static U8 SIM900_unsolMsgLen = 0;
static U8 SIM900_skipUnsolChk = 0;
static U8 SIM900_specResp = 0; //
static U8 preSockHostChksum = 0;
static U8 SIM900_RxSMSindex = 0;
static U8 avoid_sockRxDat_unsolEvt  = 0;
static U8 SIM900_sockRxIndexStack[SOCKRXINDEXSTACK_MAX];
static U8 SIM900_sockRxIndexStack_in = 0;
static U8 SIM900_sockRxIndexStack_out = 0;
static U8 SIM900_sockRxIndexStack_len = 0;
static U8 SIM900_rdSock_direct = 0;
/*-----------------------------------------------------------------------------------------
 * FUNCTIONS
 ------------------------------------------------------------------------------------------*/
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
void SIM900_strsplit(const U8* str, U8 chr, U8* out_head, U16 head_length, U8* out_tail, U16 tail_length)
{
	U16 pos, pos_head = 0, pos_tail = 0, len = strlen(str);
	U8 flag_found = 0;

	if (len != 0)
	{
		for (pos = 0; pos < len; pos++)
		{
			if (flag_found == 1)
			{
				if (tail_length)
				{
					if (pos_tail <= (tail_length - 1))
					{
						out_tail[pos_tail++] = str[pos];
					}
				}
			}
			if ((str[pos] == chr) && (flag_found == 0))
			{
				flag_found = 1;
			}
			if ((str[pos] != chr) && (flag_found == 0))
			{
				if (head_length)
				{
					if (pos_head <= (head_length - 1))
					{
						out_head[pos_head++] = str[pos];
					}
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
int SIM900_strsearch(const U8 *sRoot, const U8 *sTarget)
{
	U16 pos//
		, len = strlen(sRoot)//
		, len_target = strlen(sTarget);
	if (len_target > len)
	{
		return -1;
	}
	for (pos = 0; pos <= (len - len_target); pos++)
	{
		if (strncmp(&sRoot[pos], sTarget, len_target) == 0)
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
U8 SIM900_strrmv(U8 *s, const U8 *sCut)
{
	int res = SIM900_strsearch(s, sCut);
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
/*------------------------------------------------------------------------------
 * Brief: add new byte to buffer
 * Param:	c	|	IN	|	read byte
 -------------------------------------------------------------------------------*/
void SIM900_bufRst(void)
{
	memset(SIM900_buf, 0, SIM900_bufsize);
	SIM900_bufIn = 0;
	SIM900_bufOut = 0;
	SIM900_bufLen = 0;
	SIM900_bufLineCount = 0;
	SIM900_bufOutNoCRLFdecLine = 0;
	memset(SIM900_unsolMsg, 0, SIM900_UNSOLMSG_SIZE);
	SIM900_unsolMsgLen = 0;
	SIM900_skipUnsolChk = 0;
	SIM900_specResp = 0; //
	preSockHostChksum = 0;
	SIM900_RxSMSindex = 0;
	avoid_sockRxDat_unsolEvt  = 0;
	memset(SIM900_sockRxIndexStack, 0, SOCKRXINDEXSTACK_MAX);
	SIM900_sockRxIndexStack_in = 0;
	SIM900_sockRxIndexStack_out = 0;
	SIM900_sockRxIndexStack_len = 0;
}
/*------------------------------------------------------------------------------
 * Brief: add new byte to buffer
 * Param:	c	|	IN	|	read byte
 -------------------------------------------------------------------------------*/
void SIM900_bufAdd(U8 c)
{
	if (SIM900_bufLen < SIM900_bufsize)
	{
		SIM900_buf[SIM900_bufIn++] = c;
		if (SIM900_bufIn >= SIM900_bufsize)
		{
			SIM900_bufIn = 0;
		}
		SIM900_bufLen++;
	}
}
/*------------------------------------------------------------------------------
 * Brief: Get 1 byte from buffer
 * Param:	c	|	OUT	|	got byte
 * Ret:	0	|	OK
 * 		>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_bufGet(U8 *c)
{
	if (SIM900_bufLen > 0)
	{
		*c = SIM900_buf[SIM900_bufOut++];
		if (SIM900_bufOut >= SIM900_bufsize)
		{
			SIM900_bufOut = 0;
		}
		SIM900_bufLen--;
		if ((*c == '\r') || (*c == '\n'))
		{
			if (SIM900_bufOutNoCRLFdecLine)
			{
				//SIM900_bufLineCount = (!SIM900_bufLineCount) ? 0 : (SIM900_bufLineCount - 1);
				SIM900_bufLineCount--;
				SIM900_bufOutNoCRLFdecLine = 0;
			}
		}
		else
		{
			SIM900_bufOutNoCRLFdecLine++;
		}
		return 0;
	}
	memset(SIM900_buf, 0, SIM900_bufsize);
	SIM900_bufIn = 0;
	SIM900_bufOut = 0;
	SIM900_bufLen = 0;
	SIM900_bufLineCount = 0;
	SIM900_bufOutNoCRLFdecLine = 0;
//	SIM900_bufIn = 0;
//	SIM900_bufOut = 0;
	return 1;
}
/*------------------------------------------------------------------------------
 * Brief: Get string from buffer
 * Param:	timeout	|	IN	|	timeout, must be multiple of 10. Unit: millisecond.
 * 			size	|	IN	|	max string size.
 * 			s		|	OUT	|	got string.
 * Ret:		string length.
 -------------------------------------------------------------------------------*/
U16 SIM900_bufGets(U16 timeout, U16 size, U8 *s)
{
	U8 rByte;
	U16 l = 0, timepass = 0;

	memset(s, 0, size);
	while (1)
	{
		if (SIM900_bufGetLen() != l)
		{
			l = SIM900_bufGetLen();
			if (l >= size)
			{
				break;
			}
			timepass = 0;
		}
		else
		{
			timepass += 10;
			if (timepass >= timeout)
			{
				break;
			}
			SIM900_delay10ms(1);
		}
	}
	for (U16 i = 0; i < l; i++)
	{
		if (SIM900_bufGet(&rByte))
		{
			l = i;
			break;
		}
		if (i < size)
		{
			s[i] = rByte;
		}
	}
	return l;
//	while (1)
//	{
//		if (!SIM900_bufGet(&rByte))
//		{
//			if (l >= size)
//			{
//				break;
//			}
//			s[l++] = rByte;
//			timepass = 0;
//			continue;
//		}
//		timepass += 10;
//		if (timepass >= timeout)
//		{
//			break;
//		}
//		SIM900_delay10ms(1);
//	}
//	return l;
}
/*------------------------------------------------------------------------------
 * Brief: Get 1 response from buffer
 * Param:	timeout	|	IN	|	timeout, must be multiple of 10. Unit: millisecond.
 * 			size	|	IN	|	max response size.
 * 			header	|	IN	|	only accept response with this header.
 * 			s		|	OUT	|	got response.
 * Ret:		response length.
 -------------------------------------------------------------------------------*/
U16 SIM900_bufGetResp(U16 timeout, U16 size, const U8 *header, U8 *s)
{
	U8 rByte;//, timeoutNoLine = 1;
	U16 l = 0, timepass = 0;

	memset(s, 0, size);
	while (1)
	{
		if (SIM900_bufLineCount)
		{
//			timeoutNoLine = 0;
			if (!SIM900_bufGet(&rByte))
			{
				if ((rByte != '\r') && (rByte != '\n'))
				{
					if (l < size)
					{
						s[l++] = rByte;
						//s[l] = 0;
					}
				}
				else if (l)
				{
					if (!strncmp(s, header, strlen(header)))
					{
						if (rByte == '\r')
						{
							//Read redundancy <LF> character.
							SIM900_bufGet(&rByte);
						}
						//SIM900_dbgPrt("\r\n %s:Matched (%u,%u,%u)", __func__, SIM900_bufIn, SIM900_bufOut, SIM900_bufLineCount);
						break;
					}
					else
					{
						//SIM900_dbgPrt("\r\n %s:\"%s\"/\"%s\"->Unmatched (%u,%u,%u)", __func__, s, header, SIM900_bufIn, SIM900_bufOut, SIM900_bufLineCount);
						l = 0;
						memset(s, 0, size);
					}
				}
				timepass = 0;
				//continue;
			}
		}
		timepass += 10;
		if (timepass >= timeout)
		{
//			if (timeoutNoLine)
//			{
//				SIM900_dbgPrt("\r\n %s:TimeoutNoLine", __func__, timeoutNoLine);
//			}
			break;
		}
		SIM900_delay10ms(1);
	}
	return l;
}
/*------------------------------------------------------------------------------
 * Brief: wait response
 * Param:	timeout	|	IN	|	timeout, must be multiple of 10. Unit: millisecond.
 * 			s		|	OUT	|	expecting response.
 * Ret:		0	|	OK
 * 			1	|	Timeout
 -------------------------------------------------------------------------------*/
U8 SIM900_bufWaitResp(U16 timeout, const U8 *s)
{
	U8 rByte;
	U16 i = 0, timepass = 0;//, unmatched = 0;
	while (1)
	{
		if (SIM900_bufLineCount)
		{
			if (!SIM900_bufGet(&rByte))
			{
#if 0
				if ((rByte != '\r') && (rByte != '\n'))
				{
					if (!unmatched)
					{
						if (rByte != s[i])
						{
							unmatched = 1;
							i = 0;
						}
						else
						{
							i++;
							if (!s[i])
							{
								return 0;
							}
						}
					}
				}
				else
				{
					unmatched = 0;
					i = 0;
				}
#else
				if (rByte != s[i])
				{
					i = 0;
				}
				else
				{
					i++;
					if (!s[i])
					{
						return 0;
					}
				}
#endif
				timepass = 0;
				continue;
			}
		}
		timepass += 10;
		if (timepass >= timeout)
		{
			break;
		}
		SIM900_delay10ms(1);
	}
	return 1;
}
/*------------------------------------------------------------------------------
 * Brief: clear receive buffer
 -------------------------------------------------------------------------------*/
void SIM900_bufClr(void)
{
	U8 c;
	while (!SIM900_bufGet(&c));
//	memset(SIM900_buf, 0, SIM900_bufsize);
//	SIM900_bufIn = 0;
//	SIM900_bufOut = 0;
//	SIM900_bufLen = 0;
//	SIM900_bufLineCount = 0;
//	SIM900_bufOutNoCRLFdecLine = 0;
}
/*------------------------------------------------------------------------------
 * Brief: clear receive buffer
 -------------------------------------------------------------------------------*/
U16 SIM900_bufGetLen(void)
{
	return SIM900_bufLen;
}
/*------------------------------------------------------------------------------
 * Brief: Setup Quectel M95 module
 * Param:
 -------------------------------------------------------------------------------*/
void SIM900_setup( U8 *allocBuf, U16 allocBufsize//
				, SIM900_printc_CB prtCB//
				, SIM900_reset_CB rstCB//
				, SIM900_delay10ms_CB delay10msCB)
{
	SIM900_buf = allocBuf;
	SIM900_bufsize = allocBufsize;
	SIM900_printc = prtCB;
	SIM900_reset = rstCB;
	SIM900_delay10ms = delay10msCB;
	for (U8 i = 0; i < SOCKRXINDEXSTACK_MAX; i++)
	{
		SIM900_sockRxIndexStack[i] = 0xFF;
	}
}
/*------------------------------------------------------------------------------
 * Brief:
 * Param:
 -------------------------------------------------------------------------------*/
void SIM900_dbgPrt_nothing(const U8 *s, ...)
{
	return;
}
/*------------------------------------------------------------------------------
 * Brief: Setup Quectel M95 module debug print feature
 * Param:
 -------------------------------------------------------------------------------*/
void SIM900_setup_dbgPrt(SIM900_dbgPrt_CB_t dbgPrtCB)
{
	if (dbgPrtCB == NULL)
	{
		SIM900_dbgPrt = SIM900_dbgPrt_nothing;
	}
	else
	{
		SIM900_dbgPrt = dbgPrtCB;
	}
}
/*------------------------------------------------------------------------------
 * Brief:	This function used to read data from GSM module.
 * Param:	c	|	IN	|	read byte from GSM module.
 -------------------------------------------------------------------------------*/
void SIM900_readc(U8 c)
{
	SIM900_bufAdd(c);
	if ((c != '\r') && (c != '\n'))
	{
		if (SIM900_unsolMsgLen < SIM900_UNSOLMSG_SIZE)
		{
			SIM900_unsolMsg[SIM900_unsolMsgLen++] = c;
			SIM900_unsolMsg[SIM900_unsolMsgLen] = 0;
		}
		if (!strncmp(SIM900_unsolMsg, "> ", 2))
		{
			// Waiting for SMS/TCPIP data input
			SIM900_unsolMsgLen = 0;
			SIM900_bufLineCount++;
		}
		if (c == '>')
		{
			SETBIT(SIM900_specResp, SIM900_SPECRESP_INDEX_3E);
		}
	}
	else if (SIM900_unsolMsgLen)
	{
		if (!SIM900_skipUnsolChk)
		{
			if (!strncmp(SIM900_unsolMsg, "+CMTI:", 6))
			{
				U8 tmp[4];
				SIM900_strsplit(SIM900_unsolMsg, ',', SIM900_unsolMsg, SIM900_UNSOLMSG_SIZE, tmp, 4);
				SIM900_RxSMSindex = atoi(tmp);
				// Received SMS indicator.
			}
			else if (SIM900_strsearch(SIM900_unsolMsg, "SEND OK") != -1)
			{
				SETBIT(SIM900_specResp, SIM900_SPECRESP_INDEX_SENDOK);
			}
			else if (!strncmp(SIM900_unsolMsg, "+CMGS:", 6))
			{
				SETBIT(SIM900_specResp, SIM900_SPECRESP_INDEX_CMGS);
			}
			else if (!strncmp(SIM900_unsolMsg, "RING", 4))
			{

			}
			else if (!strncmp(SIM900_unsolMsg, "+CIPRXGET: 1,", 13) || !strncmp(SIM900_unsolMsg, "+CIPRXGET:1,", 12))
			{
				SIM900_rdSock_direct = 0;
				if (!avoid_sockRxDat_unsolEvt)
				{
					/* Because index is smaller than 10 -> length is only 1 */
					if (SIM900_sockRxIndexStack_len < SOCKRXINDEXSTACK_MAX)
					{
						SIM900_sockRxIndexStack[SIM900_sockRxIndexStack_in++] = atoi(&SIM900_unsolMsg[strlen(SIM900_unsolMsg) - 1]);
						if (SIM900_sockRxIndexStack_in >= SOCKRXINDEXSTACK_MAX)
						{
							SIM900_sockRxIndexStack_in = 0;
						}
						SIM900_sockRxIndexStack_len++;
					}
				}
			}
			else if (!strncmp(SIM900_unsolMsg, "+RECEIVE,", 9))
			{
				SIM900_rdSock_direct = 1;
				/* Because index is smaller than 10 -> length is only 1 */
				if (SIM900_sockRxIndexStack_len < SOCKRXINDEXSTACK_MAX)
				{
					SIM900_sockRxIndexStack[SIM900_sockRxIndexStack_in++] = SIM900_unsolMsg[9] - '0';
					if (SIM900_sockRxIndexStack_in >= SOCKRXINDEXSTACK_MAX)
					{
						SIM900_sockRxIndexStack_in = 0;
					}
					SIM900_sockRxIndexStack_len++;
				}
			}
		}
		SIM900_unsolMsgLen = 0;
		SIM900_bufLineCount++;
	}
}
/*------------------------------------------------------------------------------
 * Brief: Configure Quectel M95 module
 * Param:
 -------------------------------------------------------------------------------*/
void SIM900_prints(U16 len, const U8 *s)
{
	U16 i = 0;

	if (!len)
	{
		len = strlen(s);
	}
	for (i = 0; i < len; i++)
	{
		SIM900_printc(s[i]);
	}
}
/*------------------------------------------------------------------------------
 * Brief: re-powering Quectel M95
 * Param: timeKeepPwrDwn	|	IN	|	Unit: millisecond
 -------------------------------------------------------------------------------*/
void SIM900_rst(void)
{
	SIM900_reset();
	SIM900_bufRst();
}
/*------------------------------------------------------------------------------
 * Brief:
 *
 -------------------------------------------------------------------------------*/
void SIM900_setAvoidSockRxDatEvt(U8 val)
{
	avoid_sockRxDat_unsolEvt = val;
}
/*------------------------------------------------------------------------------
 * Brief: Get socket received data index.
 * Ret:	0 to 5		|	valid index
 * 		0xFF (255)	|	invalid
 -------------------------------------------------------------------------------*/
U8 SIM900_get_sockRxDindex(void)
{
	U8 ret = 0xFF;
	if (SIM900_sockRxIndexStack_len)
	{
		ret = SIM900_sockRxIndexStack[SIM900_sockRxIndexStack_out];
	}
	return ret;
}
/*------------------------------------------------------------------------------
 * Brief:
 * Ret:
 -------------------------------------------------------------------------------*/
void SIM900_rst_sockRxDindex(void)
{
	/* index is only from 0 to 5 */
	if (SIM900_sockRxIndexStack_len)
	{
		SIM900_dbgPrt("\r\n %s:%u", __func__, SIM900_sockRxIndexStack[SIM900_sockRxIndexStack_out]);
		SIM900_sockRxIndexStack[SIM900_sockRxIndexStack_out++] = 0xFF;
		if (SIM900_sockRxIndexStack_out >= SOCKRXINDEXSTACK_MAX)
		{
			SIM900_sockRxIndexStack_out = 0;
		}
		SIM900_sockRxIndexStack_len--;
	}
}
/*------------------------------------------------------------------------------
 * Brief: Get newest SMS index
 * Ret:	>0	|	available
 * 		0	|	Not available
 * Note: index will be reset after calling this function.
 -------------------------------------------------------------------------------*/
U8 SIM900_get_RxSMSindex(void)
{
	return SIM900_RxSMSindex;
}
/*------------------------------------------------------------------------------
 * Brief:
 * Ret:
 -------------------------------------------------------------------------------*/
void SIM900_rst_RxSMSindex(void)
{
	SIM900_RxSMSindex = 0;
}
/*------------------------------------------------------------------------------
 * Brief: turn off echo mode.
 * Ret:	0	|	OK
 * 		>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_offEcho(void)
{
	U8 res = 0;

	SIM900_bufClr();
	SIM900_printc(ECS_KEY);//Ensure escaped from transfer data mode.
	SIM900_prints(0, "ATE0\r");
	res = SIM900_bufWaitResp(100, "OK");
	SIM900_bufClr();
	return res;
}
/*------------------------------------------------------------------------------
 * Brief: get network registration status
 * Param:	stt	|	OUT	|	registration status.
 * Ret:	0	|	OK
 * 		>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_getNetRegStt(NETREGSTT_t *stt)
{
	U8 resp[64], sTmp[2];

	SIM900_offEcho();
	SIM900_prints(0, "AT+CGREG?\r");
	if (!SIM900_bufGetResp(500, 64, "+CGREG:", resp))
	{
		*stt = NRS_UNKNOWN;
		SIM900_dbgPrt(singleErrDbg, __func__);
		return 1;
	}
	//SIM900_dbgPrt("\r\n SIM900_getNetRegStt:\"%s\"", resp);
	SIM900_strsplit(resp, ',', resp, 64, sTmp, 2);
	//SIM900_dbgPrt("\r\n SIM900_getNetwork:\"%s\"", s);
	*stt = atoi(sTmp);
	return 0;
}
/*------------------------------------------------------------------------------
 * Brief: get network registration status
 * Param:	stt	|	OUT	|	registration status.
 -------------------------------------------------------------------------------*/
U8 *SIM900_explainNetRegStt(NETREGSTT_t stt)
{
	switch (stt)
	{
		case 0:
		{
			static U8 ret[] = "Not registered";
			return  ret;
		}
		case 1:
		{
			static U8 ret[] = "Registered";
			return  ret;
		}
		case 2:
		{
			static U8 ret[] = "Searching";
			return  ret;
		}
		case 3:
		{
			static U8 ret[] = "Denied";
			return  ret;
		}
		case 4:
		{
			static U8 ret[] = "Unknown";
			return  ret;
		}
		case 5:
		{
			static U8 ret[] = "Roaming";
			return  ret;
		}
		default:
			break;
	}
	{
		static U8 ret[] = "NA";
		return  ret;
	}
}
/*------------------------------------------------------------------------------
 * Brief: Get network (operator) name
 * Ret:	0	|	OK
 * 		>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_getNetwork(U16 size, U8 *s)
{
	U8 resp[64];

	SIM900_offEcho();
	SIM900_prints(0, "AT+COPS?\r");
	if (!SIM900_bufGetResp(500, 64, "+COPS:", resp))
	{
		memset(s, 0, size);
		SIM900_dbgPrt(singleErrDbg, __func__);
		return 1;
	}
	SIM900_strsplit(resp, '\"', resp, 64, s, size);
	SIM900_strrmv(s, "\"");
	//SIM900_dbgPrt("\r\n SIM900_getNetwork:\"%s\"", s);
	return 0;
}
/*------------------------------------------------------------------------------
 * Brief: Get module IMEI
 * Param:	size	|	IN	|	max IMEI size to read
 * 			imei	|	OUT	|	IMEI string
 * Ret:	0	|	OK
 * 		>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_getIMEI(U8 size, U8 *imei)
{
	U8 resp[64];

	SIM900_offEcho();
	SIM900_prints(0, "AT+GSN\r");
	SIM900_bufGetResp(500, size, "", imei);
	if (SIM900_bufWaitResp(100, "OK"))
	{
		return 1;
	}
	return 0;
}
/*------------------------------------------------------------------------------
 * Brief: Get signal quality
 * Ret:	0	|	OK
 * 		>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_getSQ(U8 *sq)
{
	U8 resp[64], s[3];

	SIM900_offEcho();
	SIM900_prints(0, "AT+CSQ\r");
	if (!SIM900_bufGetResp(500, 64, "+CSQ:", resp))
	{
		*sq = 0;
		SIM900_dbgPrt(singleErrDbg, __func__);
		return 1;
	}
	//SIM900_dbgPrt("\r\n SIM900_getSQ:\"%s\"", resp);
	SIM900_strrmv(resp, "+CSQ: ");
	SIM900_strsplit(resp, ',', s, 3, resp, 64);
	*sq = atoi(s);
	return 0;
}
/*------------------------------------------------------------------------------
 * Brief: send SMS
 * Param:	sms	|	IN	|	sending SMS structure.
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_sendSMS(SMS_t sms)
{
	U8 s[64];

	SIM900_offEcho();
	SIM900_prints(0, "AT+CMGF=1\r");
	if (SIM900_bufWaitResp(300, "OK"))
	{
		SIM900_dbgPrt("\r\n %s:CMGF:err", __func__);
		return 1;
	}
	SIM900_prints(0, "AT+CSCS=\"GSM\"\r");
	if (SIM900_bufWaitResp(300, "OK"))
	{
		SIM900_dbgPrt("\r\n %s:CSCS:err", __func__);
		return 2;
	}
	snprintf(s, 64, "AT+CMGS=\"%s\"\r", sms.num);
	CLRBIT(SIM900_specResp, SIM900_SPECRESP_INDEX_3E);
	avoid_sockRxDat_unsolEvt++;
	SIM900_prints(0, s);
	/*
	for (U16 i = 0; i < 100; i++)
	{
		if (GETBIT(SIM900_specResp, SIM900_SPECRESP_INDEX_3E))
		{
			if (avoid_sockRxDat_unsolEvt)
			{
				avoid_sockRxDat_unsolEvt--;
			}
			goto SIM900_SENDSMS_TRANS;
		}
		SIM900_delay10ms(1);
	}
	*/
	SIM900_delay10ms(10);
	goto SIM900_SENDSMS_TRANS;
	if (avoid_sockRxDat_unsolEvt)
	{
		avoid_sockRxDat_unsolEvt--;
	}
	SIM900_printc(ECS_KEY);
	SIM900_dbgPrt("\r\n %s:CMGS:err", __func__);
	return 3;
	SIM900_SENDSMS_TRANS:// Start transfer SMS content
	CLRBIT(SIM900_specResp, SIM900_SPECRESP_INDEX_CMGS);
	SIM900_prints(0, sms.content);
	SIM900_printc(CTRLZ_KEY);
	for (U16 i = 0; i < 1000; i++)
	{
		if (GETBIT(SIM900_specResp, SIM900_SPECRESP_INDEX_CMGS))
		{
			return 0;
		}
		SIM900_delay10ms(10);
	}
	SIM900_dbgPrt("\r\n %s:CMGS:No resp", __func__);
	return 4;
}
/*------------------------------------------------------------------------------
 * Brief: get unread SMS index list.
 * Param:	size	|	IN	|	max list's size.
 * 			ls		|	OUT	|	index list.
 * Ret:		number of unread SMS found.
 -------------------------------------------------------------------------------*/
U8 SIM900_getSMSindexList(U8 size, U8 *ls)
{
	U8 s[64], tmp[2], i = 0;

	SIM900_offEcho();
	SIM900_prints(0, "AT+CMGL=\"ALL\",1\r");
	while (1)
	{
		if (!SIM900_bufGetResp(300, 64, "+CMGL:", s))
		{
			break;
		}
		SIM900_dbgPrt("\r\n %s:\"%s\"", __func__, s);
		SIM900_strrmv(s, "+CMGL: ");
		SIM900_strsplit(s, ',', s, 64, tmp, 2);
		if (i < size)
		{
			ls[i++] = atoi(s);
		}
		else
		{
			break;
		}
	}
	return i;
}
/*------------------------------------------------------------------------------
 * Brief: read SMS.
 * Param:	index	|	IN	|	index of SMS in storage.
 * 			sms		|	OUT	|	Received SMS structure.
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_readSMS(U8 index, SMS_t *sms)
{
	U8 s[64], tmp[2];
	U16 contentLen = 0;

	memset(sms, 0, sizeof(SMS_t));
	SIM900_offEcho();
	SIM900_prints(0, "AT+CMGF=1\r");
	if (SIM900_bufWaitResp(300, "OK"))
	{
		//return 1;
	}
	SIM900_prints(0, "AT+CSCS=\"GSM\"\r");
	if (SIM900_bufWaitResp(300, "OK"))
	{
		//return 2;
	}
	snprintf(s, 64, "AT+CMGR=%u\r", index);
	SIM900_prints(0, s);
	if (!SIM900_bufGetResp(300, 64, "+CMGR:", s))
	{
		SIM900_dbgPrt("\r\n %s:CMGR:err", __func__);
		return 3;
	}
	/* Parsing response header to get sender number */
	SIM900_dbgPrt("\r\n %s:Find number:\"%s\"", __func__, s);
	SIM900_strsplit(s, ',', tmp, 2, s, 64);
	SIM900_strsplit(s, ',', s, 64, tmp, 2);
	SIM900_strrmv(s, "\"");
	SIM900_strrmv(s, "\"");
	SIM900_dbgPrt("\r\n %s:Parse number:\"%s\"", __func__, s);
	strlcpy(sms->num, s, PHONENUM_MAXSIZE);
	/* Get SMS content */
	SIM900_bufGets(500, SMS_CONTENT_MAXSIZE, sms->content);
	SIM900_dbgPrt("\r\n %s:Content:\r\n", __func__);
	contentLen = strlen(sms->content);
	for(U16 i = 0; i < contentLen; i++)
	{
		if (sms->content[i] == '\r')
		{
			SIM900_dbgPrt("<CR>");
		}
		else if (sms->content[i] == '\n')
		{
			SIM900_dbgPrt("<LF>");
		}
		else
		{
			SIM900_dbgPrt("%c", sms->content[i]);
		}
	}
	/* Remove last "OK" */
	if (contentLen >= 8)
	{
		if (!strncmp(&sms->content[contentLen - 8], "\r\n\r\nOK\r\n", 8))
		{
			memset(&sms->content[contentLen - 8], 0, 8);
		}
	}
	return 0;
}
/*------------------------------------------------------------------------------
 * Brief: Delete SMS.
 * Param:	index	|	I	|	SMS index
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 * Note: If index = 0 -> Mean delete all SMS.
 -------------------------------------------------------------------------------*/
U8 SIM900_delSMS(U8 index)
{
	U8 s[64]//
		 //, tmp[2]//
		;

	if (!index)
	{
		snprintf(s, 64, "AT+CMGDA=\"DEL ALL\"\r");
	}
	else
	{
		snprintf(s, 64, "AT+CMGD=%u\r", index);
	}
	SIM900_offEcho();
	SIM900_prints(0, s);
	if (SIM900_bufWaitResp(500, "OK"))
	{
		SIM900_dbgPrt("\r\n %s:index=%u:err", __func__, index);
		return 1;
	}
	return 0;
}

/*------------------------------------------------------------------------------
 * Brief: setup HTTP.
 * Param:
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
void SIM900_checkHTTP()
{

}

/*------------------------------------------------------------------------------
 * Brief: run HTTP GET
 * Param:	url		|	IN	|	URL to get.
 * 			timeout	|	IN	|	timeout to get data. Unit: second.
 * 			rSize	|	IN	|	max received size.
 * 			rDat	|	OUT	|	received data.
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_runHTTPGET(const U8 *url, U16 timeout, U16 rSize, U8 *rDat)
{
	const U8 SSIZE = 64;
	U8 s[SSIZE];

	SIM900_offEcho();
	snprintf(s, SSIZE, "AT+QHTTPURL=%u,%u\r", strlen(url), 1);
	SIM900_prints(0, s);
	if (SIM900_bufWaitResp(10000, "CONNECT"))
	{
		SIM900_printc(ECS_KEY);
		SIM900_dbgPrt("\r\n %s:QHTTPURL:No CONNECT", __func__);
		return 1;
	}
	SIM900_prints(0, url);
	if (SIM900_bufWaitResp(1000, "OK"))
	{
		SIM900_dbgPrt("\r\n %s:QHTTPURL:err", __func__);
		return 2;
	}
	snprintf(s, SSIZE, "AT+QHTTPGET=%u\r", timeout);
	SIM900_prints(0, s);
	if (SIM900_bufWaitResp(timeout * 1000, "OK"))
	{
		SIM900_dbgPrt("\r\n %s:QHTTPGET:err", __func__);
		return 3;
	}
	snprintf(s, SSIZE, "AT+QHTTPREAD=%u\r", timeout);
	SIM900_prints(0, s);
	if (SIM900_bufWaitResp(timeout * 1000, "CONNECT"))
	{
		SIM900_printc(ECS_KEY);
		SIM900_dbgPrt("\r\n %s:QHTTPREAD:No CONNECT", __func__);
		return 3;
	}
	SIM900_bufGets(100, 1, s);
	/* Get HTTP GET received data */
	SIM900_bufGets(300, rSize, rDat);
	/* Remove last "OK" */
	if (SIM900_strsearch(rDat, "\r\nOK\r\n") == (strlen(rDat) - 6))
	{
		SIM900_strrmv(rDat, "\r\nOK\r\n");
	}
	return 0;
}
/*------------------------------------------------------------------------------
 * Brief: run HTTP POST
 * Param:	url		|	IN	|	URL to get.
 * 			timeout	|	IN	|	timeout to get data. Unit: second.
 * 			sSize	|	IN	|	sending data length.
 * 			sDat	|	IN	|	sending data.
 * 			rSize	|	IN	|	max received size.
 * 			rDat	|	OUT	|	received data.
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_runHTTPPOST(const U8 *url, U16 timeout, U16 sLen, const U8 *sDat, U16 rSize, U8 *rDat)
{
	const U8 SSIZE = 64;
	U8 s[SSIZE];

	memset(rDat, 0, rSize);
	SIM900_offEcho();
	snprintf(s, SSIZE, "AT+QHTTPURL=%u,%u\r", strlen(url), 1);
	SIM900_prints(0, s);
	if (SIM900_bufWaitResp(10000, "CONNECT"))
	{
		SIM900_printc(ECS_KEY);
		SIM900_dbgPrt("\r\n %s:QHTTPURL:No CONNECT", __func__);
		return 1;
	}
	SIM900_prints(0, url);
	if (SIM900_bufWaitResp(1000, "OK"))
	{
		SIM900_dbgPrt("\r\n %s:QHTTPURL:err", __func__);
		return 2;
	}
	snprintf(s, SSIZE, "AT+QHTTPPOST=%u\r", sLen);
	SIM900_prints(0, s);
	if (SIM900_bufWaitResp(10000, "CONNECT"))
	{
		SIM900_printc(ECS_KEY);
		SIM900_dbgPrt("\r\n %s:QHTTPPOST:No CONNECT", __func__);
		return 3;
	}
	SIM900_prints(sLen, sDat);
	if (SIM900_bufWaitResp(timeout * 1000, "OK"))
	{
		SIM900_printc(ECS_KEY);
		SIM900_dbgPrt("\r\n %s:QHTTPPOST:err", __func__);
		return 4;
	}
	snprintf(s, SSIZE, "AT+QHTTPREAD=%u\r", timeout);
	SIM900_prints(0, s);
	if (SIM900_bufWaitResp(timeout * 1000, "CONNECT"))
	{
		SIM900_printc(ECS_KEY);
		SIM900_dbgPrt("\r\n %s:QHTTPPOST:No CONNECT", __func__);
		return 5;
	}
	SIM900_bufGets(100, 1, s);
	/* Get HTTP GET received data */
	SIM900_bufGets(300, rSize, rDat);
	/* Remove last "OK" */
	if (SIM900_strsearch(rDat, "\r\nOK\r\n") == (strlen(rDat) - 6))
	{
		SIM900_strrmv(rDat, "\r\nOK\r\n");
	}
	return 0;
}
/*--------------------------------------------------------------------------------------
 * Brief:
 ---------------------------------------------------------------------------------------*/
U8 SIM900_genChksum(const U8 *s, U16 len)
{
	U8 cs = 0;
	for (U16 i = 0; i < len; i++)
	{
		cs ^= s[i];
	}
	return cs;
}
/*------------------------------------------------------------------------------
 * Brief:	set socket received data handling method.
 * Param:	method	|	IN	|	<>
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_setCIPRXGET(U8 mode)
{
	U8 s[16];

	SIM900_offEcho();
	SIM900_prints(0, "AT+CIPRXGET?\r");
	if (SIM900_bufGetResp(300, 16, "+CIPRXGET: ", s))
	{
		SIM900_strrmv(s, "+CIPRXGET: ");
		if (mode == atoi(s))
		{
			return 0;
		}
		else
		{
			snprintf(s, 16, "AT+CIPRXGET=%u\r", mode);
			SIM900_prints(0, s);
			return SIM900_bufWaitResp(500, "OK");
		}
	}
	return 2;
}
/*------------------------------------------------------------------------------
 * Brief:
 -------------------------------------------------------------------------------*/
U8 SIM900_setCIPMODE(U8 mode)
{
	U8 s[16];

	SIM900_offEcho();
	SIM900_prints(0, "AT+CIPMODE?\r");
	if (SIM900_bufGetResp(300, 16, "+CIPMODE: ", s))
	{
		SIM900_strrmv(s, "+CIPMODE: ");
		if (mode == atoi(s))
		{
			return 0;
		}
		else
		{
			snprintf(s, 16, "AT+CIPMODE=%u\r", mode);
			SIM900_prints(0, s);
			return SIM900_bufWaitResp(500, "OK");
		}
	}
	return 2;
}
/*------------------------------------------------------------------------------
 * Brief:
 -------------------------------------------------------------------------------*/
U8 SIM900_setCSCLK(U8 mode)
{
	U8 s[16];

	SIM900_offEcho();
	SIM900_prints(0, "AT+CSCLK?\r");
	if (SIM900_bufGetResp(300, 16, "+CSCLK: ", s))
	{
		SIM900_strrmv(s, "+CSCLK: ");
		if (mode == atoi(s))
		{
			return 0;
		}
		else
		{
			snprintf(s, 16, "AT+CSCLK=%u\r", mode);
			SIM900_prints(0, s);
			return SIM900_bufWaitResp(500, "OK");
		}
	}
	return 2;
}
/*------------------------------------------------------------------------------
 * Brief:
 -------------------------------------------------------------------------------*/
U8 SIM900_setCIPMUX(U8 mode)
{
	U8 s[16];

	SIM900_offEcho();
	SIM900_prints(0, "AT+CIPMUX?\r");
	if (SIM900_bufGetResp(300, 16, "+CIPMUX: ", s))
	{
		SIM900_strrmv(s, "+CIPMUX: ");
		if (mode == atoi(s))
		{
			return 0;
		}
		else
		{
			snprintf(s, 16, "AT+CIPMUX=%u\r", mode);
			SIM900_prints(0, s);
			return SIM900_bufWaitResp(500, "OK");
		}
	}
	return 2;
}
/*------------------------------------------------------------------------------
 * Brief:	Get MUX mode
 * Ret:		1		|	YES, MUX mode is enabled
 * 			0		|	NO
 * 			2		|	Unknown
 -------------------------------------------------------------------------------*/
U8 SIM900_getCIPMUX(void)
{
	U8 s[16];

	SIM900_offEcho();
	SIM900_prints(0, "AT+CIPMUX?\r");
	if (SIM900_bufGetResp(300, 16, "+CIPMUX: ", s))
	{
		SIM900_strrmv(s, "+CIPMUX: ");
		return atoi(s);
	}
	return 2;
}
/*------------------------------------------------------------------------------
 * Brief:	Get saved SIM number
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_getNum(U16 size, U8 *num)
{
	U8 s[64];

	memset(num, 0, size);
	SIM900_offEcho();
	SIM900_prints(0, "AT+CNUM\r");
	if (SIM900_bufGetResp(300, 64, "+CNUM: ", s))
	{
		SIM900_strrmv(s, "+CNUM: ");
		SIM900_strsplit(s, ',', num, size, s, 64);
		return 0;
	}
	return 1;
}
/*------------------------------------------------------------------------------
 * Brief:	Set SIM number
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_setNum(const U8 *num)
{
	U8 s[64];

	SIM900_offEcho();
	SIM900_prints(0, "AT+CPBS=\"ON\"\r");
	if (SIM900_bufWaitResp(300, "OK"))
	{
		return 1;
	}
	snprintf(s, 64, "AT+CPBW=1,\"%s\"\r", num);
	if (SIM900_bufWaitResp(300, "OK"))
	{
		return 2;
	}
	return 0;
}
/*------------------------------------------------------------------------------
 * Brief: Initial socket/TCPIP stack.
 * Param:	APN		|	I	|	APN name
 * 			APNusr	|	I	|	APN username
 * 			APNpwd	|	I	|	APN password
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_iniSock(const U8 *APN, const U8 *APNusr, const U8 *APNpwd)
{
	const U16 SSIZE = 128, RSIZE = 8;
	U8 s[SSIZE] , resp[RSIZE], exitOnErr = 0, overrideResultAsOK = 0;
	U16 timeout = 0;
//	NETREGSTT_t stt;
//	SOCKSTT_t sockstt;

	SIM900_dbgPrt("\r\n %s:APNprofile:\"%s\",\"%s\",\"%s\"", __func__, APN, APNusr, APNpwd);
	SIM900_setCIPMODE(CIPMODE_SEL);
	SIM900_setCIPMUX(CIPMUX_SEL);
	SIM900_setCIPRXGET(CIPRXGET_SEL);
//	SIM900_prints(0, "AT+CIPSCONT?\r");
//	SIM900_bufWaitResp(500, "OK");
	for (U8 i = 0; i < 255; i++)
	{
		timeout = 500;
		exitOnErr = 1;
		overrideResultAsOK = 0;
		strlcpy(resp, "OK", RSIZE);
		switch (i)
		{
//			case 1:
//				snprintf(s, SSIZE, "AT+CGATT?\r");
//				exitOnErr = 0;
//				break;
			case 0:
				SIM900_prints(0, "AT+CIFSR\r");
				if (!SIM900_bufWaitResp(500, "."))
				{
					/* Already got IP -> It's ok, skip other step */
					return 0;
				}
				snprintf(s, SSIZE, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r", APN, APNusr, APNpwd);
				exitOnErr = 0;
				break;
			case 1:
				snprintf(s, SSIZE, "AT+CIICR\r");
				timeout = 2000;
				overrideResultAsOK = 1;
				break;
			case 2:
				snprintf(s, SSIZE, "AT+CIFSR\r");
				strlcpy(resp, ".", RSIZE);
				timeout = 10000;
				break;
			default:
				return 0;
		}
		SIM900_prints(0, s);
		if (SIM900_bufWaitResp(timeout, resp))
		{
			if (exitOnErr)
			{
				SIM900_dbgPrt("\r\n %s:Step=%u:err", __func__, i);
				if (overrideResultAsOK)
				{
					return 0;
				}
				return i + 1;
			}
		}
	}
	return 0xFF;
}
/*------------------------------------------------------------------------------
 * Brief:	Read data from socket connection.
 * Param:	index		|	IN	|	socket index (when using MUX mode)
 * 			TOwFB		|	IN	|	Timeout wait for feedback from server, unit: millisecond.
 * 			rsize		|	IN	|	size of read buffer, and received buffer's size.
 * 			rdat		|	OUT	|	read buffer.
 * Ret:		length of read data.
 -------------------------------------------------------------------------------*/
U16 SIM900_rdSock(U8 index, U16 TOwFB, U16 rsize, U8 *rdat)
{
	if (SIM900_rdSock_direct)
	{
		const U8 SSIZE = 64;
		U8 s[SSIZE], header[16];
		U16 bytes = 0, ofs = 0;
		//U32 ofs = 0, blklen = 0;

		if (!rsize || (rdat == NULL))
		{
			return 0;
		}
		SIM900_FIND_HEADER://
		snprintf(header, 16, "+RECEIVE,%u,", index);
		if (!SIM900_bufGetResp(TOwFB, SSIZE, header, s))
		{
			return ofs;
		}
		SIM900_dbgPrt("\r\n %s:s=\"%s\"", __func__, s);
		SIM900_strrmv(s, header);
		SIM900_strrmv(s, ":");
		bytes = atoi(s);
		if (bytes > rsize)
		{
			bytes = rsize;
		}
		SIM900_bufGets(TOwFB, bytes, &rdat[ofs]);
		ofs += bytes;
		SIM900_dbgPrt("\r\n %s:ofs=%u rsize=%u", __func__, ofs, rsize);
		rsize -= ofs;
		if (rsize == 0)
		{
			return ofs;
		}
		SIM900_dbgPrt("\r\n %s:SockRxIndex=%u/%u", __func__, SIM900_get_sockRxDindex(), index);
		if (SIM900_get_sockRxDindex() == index)
		{
			SIM900_rst_sockRxDindex();
			goto SIM900_FIND_HEADER;
		}
		return ofs;
	}
	else
	{
		const U8 SSIZE = 64;
		U8 s[SSIZE];
		U32 ofs = 0, blklen = 0;

		if (!rsize || (rdat == NULL))
		{
			return 0;
		}
		SIM900_offEcho();
		memset(rdat, 0, rsize);
		ofs = 0;
		while (1)
		{
			SIM900_RDSOCK_QIRD_GETREADLEN://
			SIM900_dbgPrt("\r\n %s:Read:ofs=%u/%u", __func__, ofs, rsize);
			if (ofs >= rsize)
			{
				break;
			}
			blklen = rsize - ofs;
			if (blklen > TCP_MAXSENDLENGTH)
			{
				blklen = TCP_MAXSENDLENGTH;
			}
			if (SIM900_getCIPMUX() == 1)
			{
				snprintf(s, SSIZE, "AT+CIPRXGET=2,%u,%lu\r", index, blklen);
			}
			else
			{
				snprintf(s, SSIZE, "AT+CIPRXGET=2,%lu\r", blklen);
			}
			SIM900_bufClr();
			SIM900_prints(0, s);
			for (U16 i = 0; i < ((TOwFB > 100) ? (TOwFB / 100) : 1); i++)
			{
				if (SIM900_bufGetResp(100, SSIZE, "", s))
				{
					if (!SIM900_strsearch(s, "+CIPRXGET:"))
					{
						const U16 STMPSIZE = 5;
						U8 sTmp[STMPSIZE];
						U16 trueblklen = 0;
						SIM900_strsplit(s, ',', sTmp, STMPSIZE, s, SSIZE);
						SIM900_strsplit(s, ',', sTmp, STMPSIZE, s, SSIZE);
						SIM900_strsplit(s, ',', sTmp, STMPSIZE, s, SSIZE);
						trueblklen = atoi(sTmp);
						SIM900_dbgPrt("\r\n %s:B4Read:trueblklen=%u/%u", __func__, trueblklen, blklen);
						if (!trueblklen)
						{
							/* No more data to read */
							goto SIM900_RDSOCK_QIRD_FINISH;
						}
						SIM900_bufGets(100, trueblklen, &rdat[ofs]);
						if (trueblklen < blklen)
						{
							goto SIM900_RDSOCK_QIRD_FINISH;
						}
						ofs += trueblklen;
						SIM900_dbgPrt("\r\n %s:Read:newOfs=%u/%u", __func__, ofs, rsize);
						goto SIM900_RDSOCK_QIRD_GETREADLEN;
					}
					else if (!strcmp(s, "OK"))
					{
						/* Do nothing */
					}
					else if (!strcmp(s, "ERROR"))
					{
						SIM900_dbgPrt("\r\n %s:Read:err", __func__);
						goto SIM900_RDSOCK_QIRD_FINISH;
					}
					else
					{
						/* Do nothing */
					}
				}
			}
			break;
		}
		SIM900_RDSOCK_QIRD_FINISH://
		if ((rdat[ofs - 2] == '\r') && (rdat[ofs - 1] == '\n'))
		{
			rdat[ofs - 2] = 0;
			rdat[ofs - 1] = 0;
			ofs -= 2;
		}
		//SIM900_dbgPrt("\r\n %s:END:rsize=%u", __func__, ofs);
		return ofs;
	}
}
/*------------------------------------------------------------------------------
 * Brief: query connection state
 * Param:	index	|	IN	|	socket index (when using MUX mode)
 * 			host	|	IN	|
 * 			port	|	IN	|
 * Ret: View 'SOCKSTT_t' for more details.
 -------------------------------------------------------------------------------*/
SOCKSTT_t SIM900_sockStt(U8 index, const U8 *host, U16 port)
{
	const U16 SSIZE = 96;
	U8 s[SSIZE], muxmode = 0;

	muxmode = SIM900_getCIPMUX();
	SIM900_prints(0, "AT+CIPSTATUS\r");
	if (!muxmode)
	{
		SIM900_bufGetResp(100, SSIZE, "STATE:", s);

		for (U8 i = 0; i < CONNSTAT_MAXINDEX; i++)
		{
			//SIM900_dbgPrt("\r\n %s:Cmp to [%u]:\"%s\"", __func__, i, connStatList[i]);
			if (SIM900_strsearch(s, connStatList[i]) != -1)
			{
				SOCKSTT_t stt;

				if (i == CONNSTAT_INDEX_CONNECTOK)
				{
					stt = SOCKSTT_CONNECTED;
				}
				else
				{
					SIM900_dbgPrt("\r\n %s:Eresp=\"%s\"", __func__, s);
					switch (i)
					{
						case CONNSTAT_INDEX_TCPCONNECTING:
						case CONNSTAT_INDEX_UDPCONNECTING:
							stt = SOCKSTT_CONNECTING;
							break;
						default:
							stt = SOCKSTT_DISCONNECTED;
							break;
					}
				}
				return stt;
			}
		}
		SIM900_dbgPrt("\r\n %s:UnknownResp=\"%s\"", __func__, s);
	}
	else
	{
		for (U8 i = 0; i < SOCKINDEX_MAX; i++)
		{
			if (SIM900_bufGetResp(300, SSIZE, "+CIPSTATUS:", s))
			{
				SIM900_dbgPrt("\r\n %s:[%u]State=\"%s\"", __func__, i, s);
				if (i == index)
				{
					if (SIM900_strsearch(s, "\"\", \"\"") == -1)
					{
						return SOCKSTT_DISCONNECTED;
					}
					return SOCKSTT_CONNECTED;
				}
			}
		}
	}
	return SOCKSTT_UNKNOWN;
}

/*------------------------------------------------------------------------------
 * Brief:	send data via TCP protocol.
 * Param:	index		|	IN	|	socket index (when using MUX mode)
 * 			isUDP		|	IN	|	is UDP socket or not?
 * 			host		|	IN	|	*IP or domain name of host.
 * 			port		|	IN	|	TCP port.
 * 			blkNum		|	IN	|	number of data block to send
 * 			sdat		|	IN	|	data block to send.
* 			curBlkNum	|	O	|	current data block on sending.
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 * Attention:
 * + If host length is 4, it means input host is IP address, with 4 bytes is IP octet:
 * 		[Byte0:octet1][[Byte0:octet1][Byte0:octet3][Byte0:octet4]
 * + If host length is ZERO -> skip open connection, immediately sending.
 * 	Example: 192.168.1.2
 * 	+ If host length is larger than 4, it means input host is domain name.
 -------------------------------------------------------------------------------*/
U8 SIM900_sendSock(U8 index, U8 isUDP, const U8 *host, U16 port, U16 blkNum, const DATABLOCK_t *sdat, U8 *curBlkNum)
{
	const U16 SSIZE = 64;
	U8 s[SSIZE], hostIsDomainName = 1, muxMode = 0, ec = 0xFF, unknownRespCt = 0;
	U16 sndblklen = 0, ofs = 0, blk_index = 0, blkdat_index = 0, maxTCPsendSize = 0;
	U32 totalSize = 0;

	muxMode = SIM900_getCIPMUX();
	for (U16 i = 0; i < blkNum; i++)
	{
		totalSize += sdat[i].size;
	}
	SIM900_dbgPrt("\r\n %s:I=%u UDP=%u H=\"%s:%u\" S=%u", __func__, index, isUDP, host, port, totalSize);
	SIM900_offEcho();
	if (!host[0])
	{
		goto SIM900_SENDSOCK_QISEND;
	}
	else if (host[0] && host[1] && host[2] && host[3] && !host[4])
	{
		/* host is only 4 bytes: this is IP octet */
		hostIsDomainName = 0;
	}
	if (muxMode == 1)
	{
		snprintf(s, SSIZE, "AT+CIPSTART=%u,", index);
	}
	else
	{
		snprintf(s, SSIZE, "AT+CIPSTART=");
	}
	if (isUDP)
	{
		strlcat(s, "\"UDP\",", SSIZE);
	}
	else
	{
		strlcat(s, "\"TCP\",", SSIZE);
	}
	if (hostIsDomainName)
	{
		snprintf(s, SSIZE, "%s\"%s\",%u\r", s, host, port);
	}
	else
	{
		snprintf(s, SSIZE, "%s\"%u.%u.%u.%u\",%u\r", s, host[0], host[1], host[2], host[3], port);
	}
	SIM900_prints(0, s);
	unknownRespCt = 0;
	for (U8 i = 0; i < 50; i++)
	{
		if (SIM900_bufGetResp(100, SSIZE, "", s))
		{
			//SIM900_dbgPrt("\r\n %s:Resp=\"%s\"", __func__, s);
			if (!strcmp(s, "OK"))
			{
				// Do nothing
			}
			else if ((SIM900_strsearch(s, "ALREADY CONNECT") != -1) || (SIM900_strsearch(s, "CONNECT OK") != -1))
			{
				goto SIM900_SENDSOCK_QISEND;
			}
			else //if (!strcmp(s, "ERROR") || !strcmp(s, "CONNECT FAIL"))
			{
				if ((SIM900_strsearch(s, "CONNECT FAIL") != -1) //
						&& muxMode//
						)
				{
					/*-------------------------------------------------------------------------------
					 * Explain: when in MUX mode,
					 * Quectel M95 return 'CONNECT FAIL', instead of 'ALREADY CONNECT' or 'CONNECT OK'
					 *-------------------------------------------------------------------------------*/
					goto SIM900_SENDSOCK_QISEND;
				}
				ec = 3;
				if (unknownRespCt++ >= 2)
				{
					SIM900_dbgPrt("\r\n %s:UnknownResp", __func__);
					goto SIM900_SENDSOCK_RETURN_EC;
				}
			}
		}
	}
	ec = 4;
	goto SIM900_SENDSOCK_RETURN_EC;
	//SIM900_dbgPrt("\r\n %s:Send", __func__);
	SIM900_SENDSOCK_QISEND://
	if (totalSize > TCP_MINSENDLENGTH_SKIPCHK)
	{
		/* Get max sock sending size */
		SIM900_prints(0, "AT+CIPSEND?\r");
		for (U8 i = 0; i < 10; i++)
		{
			if (SIM900_bufGetResp(100, SSIZE, "+CIPSEND: ", s))
			{
				SIM900_strrmv(s, "+CIPSEND: ");
				if (!muxMode)
				{
					maxTCPsendSize = atoi(s);
					break;
				}
				else
				{
					int pos = SIM900_strsearch(s, ",");
					if (pos == -1)
					{
						break;
					}
					if ((s[0] - '0') == index)
					{
						maxTCPsendSize = atoi(&s[pos + 1]);
						break;
					}
				}
			}
		}
	}
	if (!maxTCPsendSize || (maxTCPsendSize > TCP_MAXSENDLENGTH))
	{
		maxTCPsendSize = TCP_MINSENDLENGTH_SKIPCHK;
	}
	SIM900_dbgPrt("\r\n %s:maxTCPsendSize=%u", __func__, maxTCPsendSize);
	while (1)
	{
		SIM900_SENDSOCK_QISEND_GETSENDLEN://
		sndblklen = totalSize - ofs;
		if (!sndblklen)
		{
			break;
		}
		if (sndblklen > maxTCPsendSize)
		{
			sndblklen = maxTCPsendSize;
		}
		if (muxMode == 1)
		{
			snprintf(s, SSIZE, "AT+CIPSEND=%u,%u\r", index, sndblklen);
		}
		else
		{
			snprintf(s, SSIZE, "AT+CIPSEND=%u\r", sndblklen);
		}
//		SIM900_bufClr();
		CLRBIT(SIM900_specResp, SIM900_SPECRESP_INDEX_3E);
		avoid_sockRxDat_unsolEvt++;
		SIM900_prints(0, s);
		for (U16 i = 0; i < 500; i++)
		{
			if (GETBIT(SIM900_specResp, SIM900_SPECRESP_INDEX_3E))
			{
				if (avoid_sockRxDat_unsolEvt)
				{
					avoid_sockRxDat_unsolEvt--;
				}
				SIM900_dbgPrt("\r\n %s:transfer", __func__);
				goto SIM900_SENDSOCK_TRANS;
			}
			SIM900_delay10ms(1);
		}
//		SIM900_delay10ms(10);
//		goto SIM900_SENDSOCK_TRANS;
		/* Timeout without '> ' */
		if (avoid_sockRxDat_unsolEvt)
		{
			avoid_sockRxDat_unsolEvt--;
		}
		SIM900_printc(ECS_KEY);
		if (curBlkNum != NULL)
		{
			*curBlkNum = blk_index;
		}
		ec = 6;
		goto SIM900_SENDSOCK_RETURN_EC;
		SIM900_SENDSOCK_TRANS:// Start transfer data
		CLRBIT(SIM900_specResp, SIM900_SPECRESP_INDEX_SENDOK);
		{
			const U8 *dat;
			for(U16 i = 0; i < sndblklen; i++)
			{
				dat = sdat[blk_index].dat;
				SIM900_printc(dat[blkdat_index++]);
				if (blkdat_index >= sdat[blk_index].size)
				{
					blk_index++;
					blkdat_index = 0;
				}
			}
			//SIM900_prints(sndblklen, &sdat[ofs]);
		}
		SIM900_printc(CTRLZ_KEY);
		for (U16 i = 0; i < 500; i++)
		{
			if (GETBIT(SIM900_specResp, SIM900_SPECRESP_INDEX_SENDOK))
			{
				ofs += sndblklen;
				goto SIM900_SENDSOCK_QISEND_GETSENDLEN;
			}
			SIM900_delay10ms(10);
		}
		/* Error; No response or wrong response */
		if (curBlkNum != NULL)
		{
			*curBlkNum = blk_index;
		}
		ec = 8;
		goto SIM900_SENDSOCK_RETURN_EC;
	}
	if (curBlkNum != NULL)
	{
		*curBlkNum = blk_index;
	}
	return 0;
	SIM900_SENDSOCK_RETURN_EC:// return error code
	SIM900_dbgPrt("\r\n %s:EC=%u at blk=%u/%u", __func__, ec, blk_index + 1, blkNum);
	return ec;
}
/*------------------------------------------------------------------------------
 * Brief: close TCP connection
 * Param:	index		|	IN	|	socket index (when using MUX mode)
 * Ret:	0	|	OK
 * 		>0	|	FAIL
 -------------------------------------------------------------------------------*/
U8 SIM900_closeSock(U8 index)
{
	U8 s[16], muxMode = 0;

	muxMode = SIM900_getCIPMUX();
	preSockHostChksum = 0;
	if (muxMode == 1)
	{
		snprintf(s, 16, "AT+CIPCLOSE=%u\r", index);
	}
	else
	{
		snprintf(s, 16, "AT+CIPCLOSE\r");
	}
	SIM900_prints(0, s);
	for (U8 i = 0; i < 10; i++)
	{
		SIM900_bufGetResp(1000, 16, "", s);
		if (SIM900_strsearch(s, "CLOSE OK") != -1)
		{
			return 0;
		}
		else if (!strcmp(s, "ERROR"))
		{
			return 1;
		}
	}
	return 2;
}

/*------------------------------------------------------------------------------
 * Brief: delete file from Quectel M95.
 * Param:
 -------------------------------------------------------------------------------*/
U8 SIM900_delFile(const U8 *fname)
{
	const U16 STMP_SIZE = 128;
	U8 sTmp[STMP_SIZE]//
			;

	SIM900_offEcho();
	snprintf(sTmp, STMP_SIZE, "AT+QFDEL=\"%s\"\r", fname);
	SIM900_prints(0, sTmp);
	if (SIM900_bufWaitResp(500, "OK"))
	{
		SIM900_dbgPrt("\r\n %s:F=\"%s\":ERR", __func__, fname);
		return 1;
	}
	return 0;
}
/*------------------------------------------------------------------------------
 * Brief: Read file from Quectel M95.
 * Param:	fname	|	IN	|	file name.
 * 			ofs		|	IN	|	offset.
 * 			rSize	|	I/O	|	read buffer size.
 * 			rDat	|	OUT	|	read buffer.
 -------------------------------------------------------------------------------*/
U8 SIM900_readFile(const U8 *fname, U32 ofs, U32 *rSize, U8 *rDat)
{
	const U16 STMP_SIZE = 128, SRESP_SIZE = 64;
	U8 sTmp[STMP_SIZE]//
			, sResp[SRESP_SIZE]//
			//, timeout = 0//
			;
	U32 fhandlerId = 0, rLen = 0, expectedBufSize = 0;

	SIM900_offEcho();
	memset(rDat, 0, *rSize);
	//SIM900_dbgPrt("\r\n SIM900_readFile:f=\"%s\" ofs=%u rSize=%u", fname, ofs, *rSize);
	snprintf(sTmp, STMP_SIZE, "%lu", *rSize);
	expectedBufSize = *rSize + strlen("CONNECT \r\nOK\r\n") + strlen(sTmp);
	if (expectedBufSize > SIM900_bufsize)
	{
		/* Unreasonable reading size */
		SIM900_dbgPrt("\r\n %s:bufsize not enough", __func__);
		return 1;
	}
	SIM900_offEcho();
	snprintf(sTmp, STMP_SIZE, "AT+QFOPEN=\"%s\",2\r", fname);
	SIM900_prints(0, sTmp);
	if (!SIM900_bufGetResp(1000, SRESP_SIZE, "+QFOPEN: ", sResp))
	{
		SIM900_dbgPrt("\r\n %s:QFOPEN:err", __func__);
		return 2;
	}
	SIM900_strrmv(sResp, "+QFOPEN: ");
	fhandlerId = atoi(sResp);
	//SIM900_dbgPrt("\r\n SIM900_readFile:fhandlerId=%u", fhandlerId);
	snprintf(sTmp, STMP_SIZE, "AT+QFSEEK=%lu,%lu,0\r", fhandlerId, ofs);
	SIM900_prints(0, sTmp);
	if (SIM900_bufWaitResp(1000, "OK"))
	{
		SIM900_dbgPrt("\r\n %s:QFSEEK:err", __func__);
		return 3;
	}
	SIM900_skipUnsolChk = 1;
	snprintf(sTmp, STMP_SIZE, "AT+QFREAD=%lu,%lu\r", fhandlerId, *rSize);
	SIM900_bufClr();
	SIM900_prints(0, sTmp);
//	timeout = 0;
//	while ((SIM900_bufGetLen() < expectedBufSize) && (timeout++ < 20))
//	{
//		SIM900_delay10ms(10);
//	}
	if (!SIM900_bufGetResp(1000, SRESP_SIZE, "CONNECT ", sResp))
	{
		SIM900_skipUnsolChk = 0;
		SIM900_dbgPrt("\r\n %s:QFREAD:No CONNECT", __func__);
		return 4;
	}
	SIM900_strrmv(sResp, "CONNECT ");
	rLen = atoi(sResp);
	SIM900_bufGets(500, rLen, rDat);
	*rSize = rLen;
	SIM900_skipUnsolChk = 0;
	snprintf(sTmp, STMP_SIZE, "AT+QFCLOSE=%lu\r", fhandlerId);
	SIM900_bufClr();
	SIM900_prints(0, sTmp);
	if (SIM900_bufWaitResp(300, "OK"))
	{
		//SIM900_dbgPrt("\r\n SIM900_readFile:Closing file seems to be failed");
		//SIM900_dbgPrt("\r\n %s:QFCLOSE:err", __func__);
		//return 5;
	}
	return 0;
}
/*------------------------------------------------------------------------------
 * Brief: get file size on FTP
 * Param:
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 * 			(0xFF or 255: FTP task return error)
 -------------------------------------------------------------------------------*/
U8 SIM900_FTPgetsize(const U8 *host//
						, U16 port//
						, const U8 *usr//
						, const U8 *pwd//
						, const U8 *path//
						, const U8 *fname//
						, U32 *remotefsize//
						)
{
	const U16 STMP_SIZE = 128, SRESP_SIZE = 64;
	//U32 FTPofs = 0;
	U16 timeout = 0;
	U8 sTmp[STMP_SIZE]//
		, sResp[SRESP_SIZE]//
		, step = 0//
		, exitOnFail = 0//
		//, RAMfname[64]//
		//, funcRes = 0//
		;
	SIM900_offEcho();
	for (step = 0; step < 255; step++)
	{
		//SIM900_dbgPrt("\r\n %s:Step=%u", __func__, step);
		snprintf(sResp, SRESP_SIZE, "OK");// Default respond.
		timeout = 300;// Default timeout
		exitOnFail = 1;
		switch (step)
		{
			case 0:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPCLOSE\r");
				snprintf(sResp, SRESP_SIZE, "+QFTPCLOSE:");
				timeout = 10000;
				break;
			case 1:
				snprintf(sTmp, STMP_SIZE, "AT+QIFGCNT=0\r");
				break;
			case 2:
				snprintf(sTmp, STMP_SIZE, "AT+QICSGP=1,\"%s\",\"%s\",\"%s\"\r", "m-wap", "mms", "mms");
				break;
			case 3:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPUSER=\"%s\"\r", usr);
				break;
			case 4:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPPASS=\"%s\"\r", pwd);
				break;
			case 5:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPOPEN=\"%s\",%u\r", host, port);
				snprintf(sResp, SRESP_SIZE, "+QFTPOPEN:0");
				timeout = 10000;
				break;
			case 6:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPSIZE=\"%s%s\"\r", path, fname);
				timeout = 0;
				SIM900_prints(0, sTmp);
				while (1)
				{
					if (timeout >= 10000)
					{
						/* Timeout */
						break;
					}
					if (!SIM900_bufGetResp(300, SRESP_SIZE, "", sResp))
					{
						timeout += 300;
						continue;
					}
					SIM900_dbgPrt("\r\n %s:Resp=\"%s\"", __func__, sResp);
					SIM900_dbgPrt(".");
					if (!strcmp(sResp, "OK"))
					{
						// Do nothing
					}
					else if (SIM900_strsearch(sResp, "+QFTPSIZE:") == 0)
					{
						SIM900_strrmv(sResp, "+QFTPSIZE:");
						if (SIM900_strsearch(sResp, "-") != -1)
						{
							return 0xFF;
						}
						*remotefsize = atoi(sResp);
						SIM900_dbgPrt("\r\n %s:remotefsize=%u", __func__, *remotefsize);
						goto SIM900_FTPGETSIZE_ENDASTEP;
					}
					else //if (!strcmp(sResp, "ERROR"))
					{
						SIM900_dbgPrt("\r\n %s:Eresp=\"%s\"", __func__, sResp);
						break;
					}
				}
				goto SIM900_FTPGETSIZE_ENDINFAIL;
				break;
			case 7:
				SIM900_delay10ms(200);
				return 0;
		}
		SIM900_prints(0, sTmp);
		if (SIM900_bufWaitResp(timeout, sResp))
		{
			SIM900_dbgPrt("\r\n %s:Step=%u:err", __func__, step);
			if (exitOnFail)
			{
				SIM900_FTPGETSIZE_ENDINFAIL://
				return (step + 1);
			}
		}
		SIM900_FTPGETSIZE_ENDASTEP://
		continue;
	}
	return 0xFF;
}
/*------------------------------------------------------------------------------
 * Brief: run a FTP download task
 -------------------------------------------------------------------------------*/
U8 SIM900_runFTPdownload(const U8 *host//
						, U16 port//
						, const U8 *usr//
						, const U8 *pwd//
						, const U8 *path//
						, const U8 *fname//
						, U32 ofs//
						, DLFTP_HANDLE_RECVDAT_CALLBACK_t hdl//
						)
{
	const U16 STMP_SIZE = 128, SRESP_SIZE = 64;
	U32 FTPofs = 0;
	U16 timeout = 0;
	U8 sTmp[STMP_SIZE]//
		, sResp[SRESP_SIZE]//
		, step = 0//
		, exitOnFail = 0//
		, RAMfname[64]//
		, funcRes = 0//
		;

	snprintf(RAMfname, 64, "RAM:%s", fname);
	//SIM900_dbgPrt("\r\n SIM900_runFTPdownload:Clear RAM storage");
	SIM900_delFile(RAMfname);
	//SIM900_CONNECTFTP2DOWNLOAD_BEGIN://
	SIM900_offEcho();
	for (step = 0; step < 255; step++)
	{
		snprintf(sResp, SRESP_SIZE, "OK");// Default respond.
		timeout = 300;// Default timeout
		exitOnFail = 1;
		switch (step)
		{
			case 0:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPCLOSE\r");
				snprintf(sResp, SRESP_SIZE, "+QFTPCLOSE:");
				timeout = 10000;
				exitOnFail = 0;
				break;
			case 1:
				snprintf(sTmp, STMP_SIZE, "AT+QIFGCNT=0\r");
				break;
			case 2:
				snprintf(sTmp, STMP_SIZE, "AT+QICSGP=1,\"%s\",\"%s\",\"%s\"\r", "m-wap", "mms", "mms");
				break;
			case 3:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPUSER=\"%s\"\r", usr);
				break;
			case 4:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPPASS=\"%s\"\r", pwd);
				break;
			case 5:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPOPEN=\"%s\",%u\r", host, port);
				snprintf(sResp, SRESP_SIZE, "+QFTPOPEN:0");
				timeout = 10000;
				break;
			case 6:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPCFG=4,\"/%s/\"\r"//
						, "RAM"//
						//, "COM"//
						);
				snprintf(sResp, SRESP_SIZE, "+QFTPCFG:0");
				break;
			case 7:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPCFG=3,%lu\r", ofs);
				snprintf(sResp, SRESP_SIZE, "+QFTPCFG:0");
				break;
			case 8:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPPATH=\"%s\"\r", path);
				snprintf(sResp, SRESP_SIZE, "+QFTPPATH:0");
				break;
			case 9:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPGET=\"%s\"\r", fname);
				SIM900_prints(0, sTmp);
				while (1)
				{
					if (!SIM900_bufGetResp(300, SRESP_SIZE, "", sResp))
					{
						snprintf(sTmp, STMP_SIZE, "AT+QFTPSTAT\r");
						SIM900_prints(0, sTmp);
						continue;
					}
					//SIM900_dbgPrt("\r\n SIM900_runFTPdownload:\"%s\"", sResp);
					SIM900_dbgPrt(".");
					if (!strcmp(sResp, "OK"))
					{
						// Do nothing
					}
					else if (!SIM900_strsearch(sResp, "+QFTPSTAT:"))
					{
						if (!SIM900_strsearch(sResp, "CLOSED"))
						{
							SIM900_dbgPrt("\r\n %s:FTP is closed", __func__);
							break;
						}
					}
					else if (SIM900_strsearch(sResp, "+QFTPGET:") == 0)
					{
						goto SIM900_CONNECTFTP2DOWNLOAD_ENDASTEP;
					}
					else //if (!strcmp(sResp, "ERROR"))
					{
						SIM900_dbgPrt("\r\n %s:Eresp=\"%s\"", __func__, sResp);
						break;
					}
				}
				goto SIM900_CONNECTFTP2DOWNLOAD_ENDINFAIL;
			case 10:
				snprintf(sTmp, STMP_SIZE, "AT+QFTPLEN\r");
				SIM900_prints(0, sTmp);
				if (SIM900_bufGetResp(500, SRESP_SIZE, "+QFTPLEN: ", sResp))
				{
					U32 RAMfsize = atoi(sResp);
					//SIM900_dbgPrt("\r\n SIM900_runFTPdownload:\"%s\"", sResp);
					SIM900_strrmv(sResp, "+QFTPLEN: ");
					RAMfsize = atoi(sResp);
					//SIM900_dbgPrt("\r\n SIM900_runFTPdownload:RAMfsize=%u", RAMfsize);
					snprintf(sTmp, STMP_SIZE, "AT+QFTPCLOSE\r");
					SIM900_prints(0, sTmp);
					SIM900_bufWaitResp(5000, "+QFTPCLOSE:");
					{
						U32 ofs = 0, rSize = 0, rLen = 0;

						FTPofs += RAMfsize;
						while (1)
						{
							SIM900_dbgPrt(".");
							if (ofs == RAMfsize)
							{
								break;
							}
							else if (ofs > RAMfsize)
							{
								/* Wrong offset */
								goto SIM900_CONNECTFTP2DOWNLOAD_ENDINFAIL;
							}
							rSize = RAMfsize - ofs;
							if (rSize >= hdl.rBufSize)
							{
								rSize = hdl.rBufSize;
							}
							rLen = rSize;
							funcRes = SIM900_readFile(RAMfname, ofs, &rLen, hdl.rBuf);
							if (funcRes)
							{
								SIM900_dbgPrt("\r\n %s:Readfile:res=%u", __func__, funcRes);
								goto SIM900_CONNECTFTP2DOWNLOAD_ENDINFAIL;
							}
							//SIM900_dbgPrt("\r\n<DAT>%s</DAT>", hdl.rBuf);
							funcRes = hdl.handler(rLen, hdl.rBuf);
							if (funcRes)
							{
								SIM900_dbgPrt("\r\n %s:Handler:res=%u", __func__, funcRes);
								goto SIM900_CONNECTFTP2DOWNLOAD_ENDINFAIL;
							}
							if (rLen < rSize)
							{
								break;
							}
							ofs += rLen;
						}
					}
					goto SIM900_CONNECTFTP2DOWNLOAD_ENDASTEP;
				}
				goto SIM900_CONNECTFTP2DOWNLOAD_ENDINFAIL;
			case 11:
				SIM900_delay10ms(200);
				return 0;
		}
		SIM900_prints(0, sTmp);
		if (SIM900_bufWaitResp(timeout, sResp))
		{
			SIM900_dbgPrt("\r\n %s:Step=%u:err", __func__, step);
			if (exitOnFail)
			{
				SIM900_CONNECTFTP2DOWNLOAD_ENDINFAIL://
				return (step + 1);
			}
		}
		SIM900_CONNECTFTP2DOWNLOAD_ENDASTEP://
		continue;
	}
	return 0xFF;
}
