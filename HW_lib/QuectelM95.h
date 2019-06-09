/************************************************************************************
 * author:Duy Vinh
 *
 * description:
 * 	provide function to work with Quectel M95 (Header file).
 *
 ************************************************************************************/
#ifndef QUECTELM95_H_
#define QUECTELM95_H_
/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/

 /*-----------------------------------------------------------------------------------------
 * DEFINE
 ------------------------------------------------------------------------------------------*/
//#define QM95_BUFSIZE			4096
#ifndef QM95_UNSOLMSG_SIZE
#define QM95_UNSOLMSG_SIZE		96
#endif // QM95_UNSOLMSG_SIZE

#define PHONENUM_MAXSIZE		16
#define URL_MAXSIZE				256
#define SMS_CONTENT_MAXSIZE		201
//#define APNPROFILE_PARAM_SIZE	32

#define TCP_MAXSENDLENGTH		1460
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
	U8 (*handler)(U32 rBufSize, void *rBuf); //callback function to process data in buffer.
	U32 rBufSize; // Buffer size.
	void *rBuf; // Buffer pointer.
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

typedef void (*QM95_dbgPrt_CB_t)(const U8 *s, ...);
typedef void (*QM95_printc_CB)(U8 c);
typedef void (*QM95_reset_CB)(void);
typedef void (*QM95_PWRKEYPinSet_CB)(U8 set);
typedef void (*QM95_PWRDWNPinSet_CB)(U8 set);
typedef void (*QM95_delay10ms_CB)(U32 t);

/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
extern void QM95_setup( U8 *allocBuf, U16 allocBufsize//
		, QM95_printc_CB prtCB//
		, QM95_reset_CB rstCB//
		, QM95_delay10ms_CB delay10msCB);
extern void QM95_setup_dbgPrt(QM95_dbgPrt_CB_t dbgPrtCB);
extern void QM95_readc(U8 c);
extern U8 QM95_getNetRegStt(NETREGSTT_t *stt);
extern U8 *QM95_explainNetRegStt(NETREGSTT_t stt);
extern U8 QM95_getSQ(U8 *sq);
extern U8 QM95_getNetwork(U16 size, U8 *s);
extern U8 QM95_sendSMS(SMS_t sms);
extern U8 QM95_getSMSindexList(U8 size, U8 *ls);
extern U8 QM95_readSMS(U8 index, SMS_t *sms);
extern U8 QM95_delSMS(U8 index);
extern U8 QM95_connGPRS();
extern U8 QM95_setQSCLK(U8 mode);
extern U8 QM95_getNum(U16 size, U8 *num);
extern void QM95_rst(void);
extern void QM95_setAvoidSockRxDatEvt(U8 val);
extern U8 QM95_get_sockRxDindex(void);
extern void QM95_rst_sockRxDindex(void);
extern U8 QM95_get_RxSMSindex(void);
extern void QM95_rst_RxSMSindex(void);
extern U8 QM95_setupAPN(const U8 *APN, const U8 *APNusr, const U8 *APNpwd);
extern U8 QM95_runHTTPGET(const U8 *url, U16 timeout, U16 rSize, U8 *rDat);
extern U8 QM95_runHTTPPOST(const U8 *url//
							, U16 timeout//
							, U16 sLen//
							, const U8 *sDat//
							, U16 rSize//
							, U8 *rDat);
extern U8 QM95_iniSock(const U8 *APN, const U8 *APNusr, const U8 *APNpwd);
extern U16 QM95_rdSock(U8 index, U16 TOwFB, U16 rsize, U8 *rdat);
extern U8 QM95_sendSock(U8 index, U8 isUDP, const U8 *host, U16 port, U16 blkNum, const DATABLOCK_t *sdat, U8 *curBlkNum);
/*------------------------------------------------------------------------------
 * Brief: query connection state
 * Ret: View 'SOCKSTT_t' for more details.
 --*/
extern SOCKSTT_t QM95_sockStt(U8 index, const U8 *host, U16 port);
extern U8 QM95_set_sockRxDatHdlMethod(TCPIP_RXDAT_HDLMETHOD_t method);
extern U8 QM95_closeSock(U8 index);
extern U8 QM95_FTPgetsize(const U8 *host//
							, U16 port//
							, const U8 *usr//
							, const U8 *pwd//
							, const U8 *path//
							, const U8 *fname//
							, U32 *remotefsize//
							);
extern U8 QM95_runFTPdownload(const U8 *host//
								, U16 port//
								, const U8 *usr//
								, const U8 *pwd//
								, const U8 *path//
								, const U8 *fname//
								, U32 ofs//
								, DLFTP_HANDLE_RECVDAT_CALLBACK_t hdl//
								);


#endif // QUECTELM95_H_
