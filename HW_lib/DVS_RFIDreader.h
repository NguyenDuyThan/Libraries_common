/*
 ******************************************************************************
 * @author  Duy Vinh To - DinhViSo HW team
 * @brief	communicate with RFID reader "DVS" ( ISO15693 )
 ******************************************************************************
 * @attention:
 ******************************************************************************
 */
#ifndef DVS_RFIDREADER_H_
#define DVS_RFIDREADER_H_

/*** INCLUDE ********************************************************************/

#define DVSRFID_BUF_SIZE	256

/*** TYPEDEFS *******************************************************************/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef void (*DVSRFID_printc_CB)(U8 c);
typedef void (*DVSRFID_delay10ms_CB)(U32 t);

typedef enum
{
	LEDINDEX_GPS = 1,
	LEDINDEX_GSM = 2,
	LEDINDEX_LOGON = 3,
}DVSRFID_LEDINDEX_t;
/*** FUNCTION PROTOTYPES ********************************************************/
extern void DVSRFID_setup(DVSRFID_printc_CB printc_cb, DVSRFID_delay10ms_CB delay10ms_cb);
extern void DVSRFID_readc(U8 c);
extern U8 DVSRFID_readDrvInfo(U16 licenseSize, U8 *license, U16 nameSize, U8 *name, U16 uidSize, U8 *uid);
extern U8 DVSRFID_getVer(  U16 vLen, U8 *ver );
extern U8 DVSRFID_setLED( DVSRFID_LEDINDEX_t LEDindex, U8 LEDval );

#endif // DVS_RFIDREADER_H_
