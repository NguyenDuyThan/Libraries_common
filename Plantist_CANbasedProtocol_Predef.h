/******************************************************************
 * Author: Duy Vinh To
 * Brief: This file contains all pre-definitions for CAN bases protocol
 * That are used in Plantist project.
 ******************************************************************/
#ifdef PLANTIST_PROJECT
/*----------------------------------------------------
 * Customer define CAN message standard Id structure 11 bit:
 * Bit 0 -> 3 (4 bits): sender Id
 * Bit 4 -> 7 (4 bits): receiver Id
 *----------------------------------------------------*/
#define CANID_F1	0x1
#define CANID_F2	0x2
#define CANID_F3	0x3
#define CANID_F4	0x4
#define CAN_MSG_STDID(receiverId, senderId)	((senderId & 0xF) | ((receiverId << 4) & 0xF0))

#define CAN_CMDCODE_ACK			0x1
#define CAN_RPTCODE_WATERLVVAL 	0x2
#define CAN_RPTCODE_TEMPVAL		0x3
#define CAN_RPTCODE_HUMIVAL		0x4
#define CAN_RPTCODE_LIGHTVAL	0x5
#define CAN_CMDCODE_REQUESTFULLREPORT			0x6
#define CAN_CMDCODE_7			0x7
#define CAN_CMDCODE_8			0x8
#define CAN_RPTCODE_WATERFLOWVAL	0x9
#define CAN_RPTCODE_WATEROFWNG1		0xA//Water overflow warning
#define CAN_RPTCODE_WATEROFWNG2		0xB
#define CAN_CMDCODE_PWMPCTSET		0xC// PWM percentage setting
#define CAN_RPTCODE_PWMPCT		0xD// PWM percentage get setting
#endif // PLANTIST_PROJECT
