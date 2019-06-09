/*------------------------------------------------------------------------------------------
 * INTRODUCE
 * Brief: This is header file contain function to setup IO pin.
 * Author: Duy Vinh To - Team DinhViSo
 * Detail:
 -------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------
 * INCLUDES
 -------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------
 * DEFINES
 -------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------
 * TYPEDEFS
 -------------------------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;
typedef enum
{
	IOPORT_A,
	IOPORT_B,
	IOPORT_C,
	IOPORT_D,
	IOPORT_E,
	IOPORT_F,
}IO_PORT_t;

typedef enum
{
	IODIR_INF,// In floating.
	IODIR_IPU,// In pulled up.
	IODIR_IPD,// In pulled down.
	IODIR_OPP,// Out push-pull.
	IODIR_OOD,// Out open drain.
}IO_DIR_t;
/*------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 -------------------------------------------------------------------------------------------*/
extern void IO_test(void);
extern void IO_setup(IO_PORT_t port, U8 pin, IO_DIR_t dir);
extern void IO_wrt(IO_PORT_t port, U8 pin, U8 val);
extern void IO_toggle(IO_PORT_t port, U8 pin);
extern U8 IO_rd(IO_PORT_t port, U8 pin);
