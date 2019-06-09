/*-----------------------------------------------------------------------------------------
 * Author: Duy Vinh To
 * Team: DinhViSo corp.
 *
 * Modified date: 14th July 2016
 *
 * Brief: advanced memory managing layer over freeRTOS dynamic memory allocation.
 * 		Header file.
 ------------------------------------------------------------------------------------------*/

#ifndef MEMMNGR_H_
#define MEMMNGR_H_

/*-----------------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------
 * DEFINES
 ------------------------------------------------------------------------------------------------*/
#ifndef NULL
#define NULL	((void*)0)
#endif
/*-----------------------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef void (*MM_delay10msCB)(U32 t);
typedef void* (*MM_allocCB)(U32 size);
typedef void (*MM_deallocCB)(void *addr);
typedef void (*MM_printCB)(const U8 *s,...);
/*-----------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------------*/
extern void MM_timertick1sCB(void);
extern void MM_setup_dbgPrt(MM_printCB cb);
extern void MM_setup(MM_allocCB allocCB, MM_deallocCB deallocCB, MM_delay10msCB delay10msCB);
extern void MM_show(MM_printCB printCB, const U8 *printlayout, U32 *totalUM, float *allocFailRate);

extern void* MM_get( U32 tKeep, U32 size );
extern void MM_free( void *addr );

#endif // MEMMNGR_H_
