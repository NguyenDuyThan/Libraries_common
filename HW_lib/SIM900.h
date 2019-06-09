/************************************************************************************
 * author:Duy Vinh
 *
 * description:
 * 	provide function to work with Quectel M95 (Header file).
 *
 ************************************************************************************/
#ifndef SIM900_H_
#define SIM900_H_
/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/

 /*-----------------------------------------------------------------------------------------
 * DEFINE
 ------------------------------------------------------------------------------------------*/
//#define SIM900_BUFSIZE			4096
#ifndef SIM900_UNSOLMSG_SIZE
#define SIM900_UNSOLMSG_SIZE		96
#endif // SIM900_UNSOLMSG_SIZE

#define PHONENUM_MAXSIZE		16
#define URL_MAXSIZE				256
#define SMS_CONTENT_MAXSIZE		201
//#define APNPROFILE_PARAM_SIZE	32

#define TCP_MAXSENDLENGTH		1460
#define TCP_MINSENDLENGTH_SKIPCHK	512
#define GPRS_MIN_ULRATE			500// GPRS minimum upload rate, unit: Bytes per second
/*-----------------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef enum
{
	TRH_DIRECTLY2UART = 0,
	TRH_NOTIFICATION = 1,
}TCPIP_RXDAT_HDLMETHOD_t;

typedef enum
{
	NRS_UNREG = 0,
	NRS_REG = 1,
	NRS_SEARCH = 2,
	NRS_DENINED = 3,
	NRS_UNKNOWN = 4,
	NRS_ROAMING = 5,
}NETREGSTT_t;

typedef struct {
	U8 num[PHONENUM_MAXSIZE];
	U8 content[SMS_CONTENT_MAXSIZE];
} SMS_t;

typedef struct {
	U8 (*handler)(U32 rBufSize, void *rBuf);
	U32 rBufSize;
	void *rBuf;
} DLFTP_HANDLE_RECVDAT_CALLBACK_t;

typedef enum
{
	SOCKSTT_UNKNOWN = 0,
	SOCKSTT_DISCONNECTED,
	SOCKSTT_CONNECTING,
	SOCKSTT_CONNECTED,
} SOCKSTT_t;

typedef struct
{
	const void *dat;
	U16 size;
} DATABLOCK_t;

typedef void (*SIM900_dbgPrt_CB_t)(const U8 *s, ...);
typedef void (*SIM900_printc_CB)(U8 c);
typedef void (*SIM900_PWRKEYPinSet_CB)(U8 set);
typedef void (*SIM900_reset_CB)(void);
typedef void (*SIM900_PWRDWNPinSet_CB)(U8 set);
typedef void (*SIM900_delay10ms_CB)(U32 t);

/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
extern void SIM900_setup( U8 *allocBuf, U16 allocBufsize//
		, SIM900_printc_CB prtCB//
		, SIM900_reset_CB rstCB//
		, SIM900_delay10ms_CB delay10msCB);
extern void SIM900_setup_dbgPrt(SIM900_dbgPrt_CB_t dbgPrtCB);
extern void SIM900_readc(U8 c);
extern U8 SIM900_getNetRegStt(NETREGSTT_t *stt);
extern U8 *SIM900_explainNetRegStt(NETREGSTT_t stt);
extern U8 SIM900_getIMEI(U8 size, U8 *imei);
extern U8 SIM900_getSQ(U8 *sq);
extern U8 SIM900_getNetwork(U16 size, U8 *s);
extern U8 SIM900_sendSMS(SMS_t sms);
extern U8 SIM900_getSMSindexList(U8 size, U8 *ls);
extern U8 SIM900_readSMS(U8 index, SMS_t *sms);
extern U8 SIM900_delSMS(U8 index);
extern U8 SIM900_connGPRS();
extern U8 SIM900_setCSCLK(U8 mode);
extern U8 SIM900_getNum(U16 size, U8 *num);
extern U8 SIM900_setNum(const U8 *num);
extern void SIM900_rst(void);
extern void SIM900_setAvoidSockRxDatEvt(U8 val);
extern U8 SIM900_get_sockRxDindex(void);
extern void SIM900_rst_sockRxDindex(void);
extern U8 SIM900_get_RxSMSindex(void);
extern void SIM900_rst_RxSMSindex(void);
extern U8 SIM900_setupAPN(const U8 *APN, const U8 *APNusr, const U8 *APNpwd);
extern U8 SIM900_runHTTPGET(const U8 *url, U16 timeout, U16 rSize, U8 *rDat);
extern U8 SIM900_runHTTPPOST(const U8 *url//
							, U16 timeout//
							, U16 sLen//
							, const U8 *sDat//
							, U16 rSize//
							, U8 *rDat);
extern U8 SIM900_iniSock(const U8 *APN, const U8 *APNusr, const U8 *APNpwd);
extern U16 SIM900_rdSock(U8 index, U16 TOwFB, U16 rsize, U8 *rdat);
extern U8 SIM900_sendSock(U8 index, U8 isUDP, const U8 *host, U16 port, U16 blkNum, const DATABLOCK_t *sdat, U8 *curBlkNum);
/*------------------------------------------------------------------------------
 * Brief: query connection state
 * Ret: View 'SOCKSTT_t' for more details.
 --*/
extern SOCKSTT_t SIM900_sockStt(U8 index, const U8 *host, U16 port);
extern U8 SIM900_closeSock(U8 index);
extern U8 SIM900_FTPgetsize(const U8 *host//
							, U16 port//
							, const U8 *usr//
							, const U8 *pwd//
							, const U8 *path//
							, const U8 *fname//
							, U32 *remotefsize//
							);
extern U8 SIM900_runFTPdownload(const U8 *host//
								, U16 port//
								, const U8 *usr//
								, const U8 *pwd//
								, const U8 *path//
								, const U8 *fname//
								, U32 ofs//
								, DLFTP_HANDLE_RECVDAT_CALLBACK_t hdl//
								);


#endif // SIM900_H_
