/*
 * netService.h
 *
 *  Created on: Oct 29, 2016
 *      Author: dv198
 */

#ifndef SW_LIB_NETSERVICE_H_
#define SW_LIB_NETSERVICE_H_

/*##########################################################################################
 * INCLUDE
 *##########################################################################################*/

/*##########################################################################################
 * DEFINE
 *##########################################################################################*/

/*##########################################################################################
 * TYPEDEFS
 *##########################################################################################*/
typedef unsigned char U8;
typedef char S8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef void (*NETSRVC_dbgPrt_CB)(const U8 *s, ...); // debug printing.

/*##########################################################################################
 * FUNC.PROTOTYPES
 *##########################################################################################*/
extern void NETSRVC_setup_dbgPrt(NETSRVC_dbgPrt_CB cb);
extern U8 NETSRVC_HTTP_post(const U8 *url, U16 timeout, U16 sLen, const U8 *sDat, U16 rSize, U8 *rDat);
extern U8 NETSRVC_HTTP_get(const U8 *url, U16 timeout, U16 rSize, U8 *rDat);
extern U8 NETSRVC_NATS_connect(U8 sockindex, const U8 *host, U16 port//
		, U8 verbose//
		, U8 pedantic//
		, U8 ssl_required//
		, const U8 *auth_token//
		, const U8 *user//
		, const U8 *pass//
		, const U8 *name//
		, const U8 *version);
extern U8 NETSRVC_NATS_sub(U8 sockindex, const U8 *host, U16 port//
		, const U8 *subject//
		, const U8 *queueGroup//
		, U32 sid//
		);
extern U8 NETSRVC_NATS_pub(U8 sockindex, const U8 *host, U16 port//
		, const U8 *subject//
		, const U8 *reply_to//
		, U32 bytes//
		, const U8 *payload//
		);
extern U8 NETSRVC_NATS_ping(U8 sockindex, const U8 *host, U16 port);
//extern U8 NETSRVC_NATS_isSubscribed(void);
extern U8 NETSRVC_NATS_pong(U8 byPing, U8 sockindex, const U8 *host, U16 port);
extern U8 NETSRVC_NATS_hdlUnsolMsg(U16 len, U8 *msg, U16 MSG_subjectSize, U8 *MSG_subject, U16 *MSG_bytes, U8 *MSG_payload);
//extern U8 NETSRVC_NATSS_connRqst(const U8 *host, U16 port//
//		, const U8 *clientID//
//		, const U8 *heartbeatInbox);
extern U8 NETSRVC_rqstNTP(U8 sockindex, const U8 *host, U32 *utc);

#endif /* SW_LIB_NETSERVICE_H_ */
