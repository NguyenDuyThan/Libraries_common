#ifndef LCD_driver_H_
#define LCD_driver_H_

/*----------------------------------------------------------------------------
 * INCLUDE
 -----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * DEFINES
 -----------------------------------------------------------------------------*/
#define CLCD_TYPE_1604	0
#define CLCD_TYPE_2004	1

#define CLCD_TYPE		CLCD_TYPE_2004

#if (CLCD_TYPE == CLCD_TYPE_1604)
#define CLCD_SIZE_X		16
#define CLCD_SIZE_Y		2
#endif
#if (CLCD_TYPE == CLCD_TYPE_2004)
#define CLCD_SIZE_X		20
#define CLCD_SIZE_Y		4
#endif

#define CGRAMADDR_0		0
#define CGRAMADDR_1		1
#define CGRAMADDR_2		2
#define CGRAMADDR_3		3
#define CGRAMADDR_4		4
#define CGRAMADDR_5		5
#define CGRAMADDR_6		6
#define CGRAMADDR_7		7

#define LCHAR_CELCIUS	0xA501
#define LCHAR_HUMIDITY	0xA502

#define EMode1				1//B hien thi ben phai A
#define EMode2				2//B hien thi ben trai A
#define EMode3				3//dich A ve ben trai, B hien thi ben phai A
#define EMode4				4//dich A ve ben phai, B hien thi ben trai A
#define Font5x10			1//define 0 to use Font 5x8
#define DISPLAY_CURSOR		0
#define BLINKING_CURSOR		0

#define SHIFTLINECNT_MODE_RIGHT	0
#define SHIFTLINECNT_MODE_LEFT	1

#define LCD_LINE1				0
#define LCD_LINE2				1
/*
 * Address of 2 line
 *               -DISPLAY ADDR- | -HIDDEN ADDR-
 *            X :  0  1 ... 15  | 16 ... 39
 * LINE0 (Y = 0): 00 01 ... 0F  |
 * LINE1 (Y = 1): 40 41 ... 4F  |
 */
#if (CLCD_TYPE == CLCD_TYPE_1602)
#define LINE_SIZE				40
#endif
#if (CLCD_TYPE == CLCD_TYPE_2004)
#define LINE_SIZE				20
#endif
#if (CLCD_TYPE == CLCD_TYPE_1602)
#define ORIGNADDR_LINE(Y)		(LINE_SIZE * Y)
#endif
#if (CLCD_TYPE == CLCD_TYPE_2004)
#define ORIGNADDR_LINE(Y)		(\
		(Y == 0) ? 0x0 : ((Y == 1) ? 0x40 : ((Y == 2) ? 0x14 : ((Y == 3) ? 0x54 : 0)))\
								)
#endif
//#define END_DISPLAY_ADDR_LINE0	(ORIGIN_ADDR_LINE0 + 15)
//#define END_ADDR_LINE0			(ORIGIN_ADDR_LINE0 + 39)
//#define END_DISPLAY_ADDR_LINE1	(ORIGIN_ADDR_LINE1 + 15)
//#define END_ADDR_LINE1			(ORIGIN_ADDR_LINE1 + 39)
/*
#define CGRAM_DAT_0		(CGRAM_DAT_t)\
						{	0b00111,\
							0b00111,\
							0b00111,\
							0b00111,\
							0b00111,\
							0b00111,\
							0b00111,\
							0b00111	}
#define CGRAM_DAT_1		(CGRAM_DAT_t)\
						{	0b11100,\
							0b11100,\
							0b11100,\
							0b11100,\
							0b11100,\
							0b11100,\
							0b11100,\
							0b11100 \
						}
#define CGRAM_DAT_2		(CGRAM_DAT_t)\
						{	0b11111,\
							0b11111,\
							0b11111,\
							0b00000,\
							0b00000,\
							0b11111,\
							0b11111,\
							0b11111 \
						}
#define CGRAM_DAT_3		(CGRAM_DAT_t)\
						{	0b00000,\
							0b00000,\
							0b00000,\
							0b00000,\
							0b00000,\
							0b11111,\
							0b11111,\
							0b11111 \
						}
#define CGRAM_DAT_4		(CGRAM_DAT_t)\
						{	0b11111,\
							0b11111,\
							0b11111,\
							0b00000,\
							0b00000,\
							0b00000,\
							0b00000,\
							0b00000 \
						}
#define CGRAM_DAT_5		(CGRAM_DAT_t)\
						{	0b00011,\
							0b00111,\
							0b00111,\
							0b01110,\
							0b01110,\
							0b11100,\
							0b11100,\
							0b11000 \
						}
#define CGRAM_DAT_6		(CGRAM_DAT_t)\
						{	0b00000,\
							0b11111,\
							0b11111,\
							0b11011,\
							0b11111,\
							0b11111,\
							0b00000,\
							0b00000\
						}
#define CGRAM_DAT_7		(CGRAM_DAT_t)\
						{	0b01110,\
							0b11001,\
							0b11100,\
							0b11001,\
							0b10011,\
							0b00111,\
							0b10011,\
							0b01110\
						}
						*/
#define CGRAM_DAT_7_1	(CGRAM_DAT_t)\
						{	0b01110,\
							0b11001,\
							0b11100,\
							0b11000,\
							0b10000,\
							0b00000,\
							0b00000,\
							0b00000\
						}
#define CGRAM_DAT_7_2	(CGRAM_DAT_t)\
						{	0b00000,\
							0b00000,\
							0b00000,\
							0b00001,\
							0b00011,\
							0b00111,\
							0b10011,\
							0b01110\
						}

#define CGRAM_DAT_0		(CGRAM_DAT_t)\
						{	0b00001,\
							0b00011,\
							0b00011,\
							0b00011,\
							0b00011,\
							0b00011,\
							0b00011,\
							0b00001	}
#define CGRAM_DAT_1		(CGRAM_DAT_t)\
						{	0b10000,\
							0b11000,\
							0b11000,\
							0b11000,\
							0b11000,\
							0b11000,\
							0b11000,\
							0b10000 \
						}
#define CGRAM_DAT_2		(CGRAM_DAT_t)\
						{	0b11111,\
							0b11111,\
							0b00000,\
							0b00000,\
							0b00000,\
							0b00000,\
							0b11111,\
							0b11111 \
						}
#define CGRAM_DAT_3		(CGRAM_DAT_t)\
						{	0b00000,\
							0b00000,\
							0b00000,\
							0b00000,\
							0b00000,\
							0b00000,\
							0b11111,\
							0b11111 \
						}
#define CGRAM_DAT_4		(CGRAM_DAT_t)\
						{	0b11111,\
							0b11111,\
							0b00000,\
							0b00000,\
							0b00000,\
							0b00000,\
							0b00000,\
							0b00000 \
						}
#define CGRAM_DAT_5		(CGRAM_DAT_t)\
						{	0b00001,\
							0b00011,\
							0b00110,\
							0b00110,\
							0b01100,\
							0b01100,\
							0b11000,\
							0b10000 \
						}
#define CGRAM_DAT_6		(CGRAM_DAT_t)\
						{	0b00000,\
							0b00000,\
							0b01110,\
							0b01010,\
							0b01110,\
							0b00000,\
							0b00000,\
							0b00000\
						}
#define CGRAM_DAT_7		(CGRAM_DAT_t)\
						{	0b01110,\
							0b11001,\
							0b11100,\
							0b11001,\
							0b10011,\
							0b00111,\
							0b10011,\
							0b01110\
						}
/*----------------------------------------------------------------------------
 * TYPEDEFS
 -----------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;
typedef struct
{
	U8 line[8];
} CGRAM_DAT_t;

typedef enum
{
	CLCDCP_RS = 0,
	CLCDCP_RW,
	CLCDCP_E,
}CLCD_CTRLPIN_t;

typedef void (*CLCD_delay10us_CB)(U32 t);
typedef void (*CLCD_wrt4DATpins_CB)(U8 val);
typedef U8 (*CLCD_rd4DATpins_CB)(void);
typedef void (*CLCD_wrtCTRLpins_CB)(U8 setRS, U8 setRW, U8 setE);
/*----------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 -----------------------------------------------------------------------------*/
extern void CLCD_setup(U32 timeoutWoBFwait//
		, CLCD_delay10us_CB delay10usCB//
		, CLCD_wrtCTRLpins_CB wrtCTRLpinsCB//
		, CLCD_wrt4DATpins_CB wrt4DATpinsCB//
		, CLCD_rd4DATpins_CB rd4DATpinsCB);
extern void CLCD_clrDisp(void);
extern void CLCD_clrLine(U8 Y);
extern void CLCD_GotoXY(U8 X, U8 Y);
extern void CLCD_wrtDats(U8 len, const U8 *s);
extern void CLCD_wrtDat(U8 dat);
extern U8 CLCD_rdy(void);
extern U8 CLCD_isAvail(void);
extern void CLCD_startup(void);
extern void CLCD_wrtCGRAMdat( U8 CGRAM_Haddr, CGRAM_DAT_t dat );
extern void CLCD_ShiftLineContent(U8 line, U8 SHIFTLINECNT_MODE, U8 step);
#if 0
extern void CLCD_showLargeClock(U8 Xstart, U8 hour, U8 min, U8 sec);
extern void CLCD_showLargeTemp(U8 Xstart, U8 temp );
extern void CLCD_showLargeHumi( U8 Xstart, U8 humi );
extern void CLCD_demoAni( void );
#endif

#endif
