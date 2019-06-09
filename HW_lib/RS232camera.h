/*
 * RS232camera.h
 *
 *  Created on: Mar 24, 2017
 *      Author: dv198
 */

#ifndef HW_LIB_RS232CAMERA_H_
#define HW_LIB_RS232CAMERA_H_

/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef enum
{
	IMGRES_SMALL = 1, //160x128
	IMGRES_MEDIUM = 2, // 320x240
	IMGRES_LARGE = 3, // 640x480
} IMG_RES_t;

typedef void (*RS232CAM_dbgPrt_CB_t)(const U8 *s, ...);
typedef void (*RS232CAM_sendByte_CB_t)(U8 c);
typedef void (*RS232CAM_delay10ms_CB_t)(U32 t);
/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
extern void RS232CAM_setup_dbgPrt(RS232CAM_dbgPrt_CB_t cb);
extern void RS232CAM_setup(U16 bufSize, U8 *bufPtr, RS232CAM_sendByte_CB_t sendByteCB, RS232CAM_delay10ms_CB_t delay10msCB);
extern void RS232CAM_buf_wrtByte(U8 byte);
extern U8 RS232CAM_takePhoto(IMG_RES_t res, U16 pSize, U16 timeout, U32 *photoSize, U16 *pNum);
extern U8 RS232CAM_rdPhotoPkt(U16 timeout, U16 pID, U16 pDatMaxSize, U8 *pDat, U16 *pSize);

#endif /* HW_LIB_RS232CAMERA_H_ */
