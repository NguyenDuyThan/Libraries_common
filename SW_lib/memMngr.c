/*-----------------------------------------------------------------------------------------
 * Author: Duy Vinh To
 * Team: DinhViSo corp.
 *
 * Modified date: 14th July 2016
 *
 * Brief: advanced memory managing layer over freeRTOS dynamic memory allocation.
 * 		Source file.
 ------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------------*/
#include "memMngr.h"
#include "string.h"

/*-----------------------------------------------------------------------------------------------
 * DEFINE & MACRO
 ------------------------------------------------------------------------------------------------*/
#define USE_ADVANCED_MM	1
#ifndef MEMMNGR_LIST_SIZE
#define MEMMNGR_LIST_SIZE 10
#endif
/*-----------------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------------*/
typedef struct
{
	U32 t_keep; // How long to keep in memory.
	U32 t_getMem; // when allocated memory.
	U32 size;
	void *addr;
} MM_LIST_E_t;
/*-----------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------------*/
static void MM_guarding(void);
static void MM_dbgPrt_pseudo(const U8 *s,...);
/*-----------------------------------------------------------------------------------------------
 * VARIABLES
 ------------------------------------------------------------------------------------------------*/
static MM_LIST_E_t *MM_list = NULL;
static U8 smpMemMngr = 0;

static MM_allocCB MM_alloc;
static MM_deallocCB MM_dealloc;
static MM_delay10msCB MM_delay10ms;
static MM_printCB MM_dbgPrt = MM_dbgPrt_pseudo;
static U32 MM_runtime = 0;
static U32 MM_AFct = 0;// Allocation fail counter
static U32 MM_Atotal = 0; // Allocation total

/*-----------------------------------------------------------------------------------------------
 * FUNCTIONS
 ------------------------------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------------------
 * Brief:
 ----------------------------------------------------------------------------------------------*/
void MM_dbgPrt_pseudo(const U8 *s,...)
{
	return;
}
/*---------------------------------------------------------------------------------------------
 * Brief: setup debug print callback function.
 ----------------------------------------------------------------------------------------------*/
void MM_setup_dbgPrt(MM_printCB cb)
{
	if (cb == NULL)
	{
		MM_dbgPrt = MM_dbgPrt_pseudo;
		return;
	}
	MM_dbgPrt = cb;
}
/*---------------------------------------------------------------------------------------------
 * Brief: setup callback functions.
 ----------------------------------------------------------------------------------------------*/
void MM_setup(MM_allocCB allocCB, MM_deallocCB deallocCB, MM_delay10msCB delay10msCB)
{
	MM_alloc = allocCB;
	MM_dealloc = deallocCB;
	MM_delay10ms = delay10msCB;
	MM_list = MM_alloc(sizeof(MM_LIST_E_t) * MEMMNGR_LIST_SIZE);
}
/*---------------------------------------------------------------------------------------------
 * Brief: this callback function must be call in a timer tick 1s.
 ----------------------------------------------------------------------------------------------*/
void MM_timertick1sCB(void)
{
	MM_runtime++;
	MM_guarding();
}
/*---------------------------------------------------------------------------------------------
 * Brief: show memory manager table
 * Param: 	printCB			|	IN	|	print callback function
 * 			printlayout		|	IN	|	print layout. See below example to understand.
 * 			totalUM			|	O	|	current total used memory.
 * 			allocFailRate	|	O	|	allocation fail rate. Maximum is 1 (100%).
 * Note: print layout?
 * print layout is how you want to show each memory management node.
 * Each memory management node will have 4 parameters: order, size, live time and max live time.
 * All parameters are unsigned long type
 * Example:
 * printlayout = "\r\n %u:S=%u LT=%u/%u"
 * And result is: "\r\n 1:4096 LT=10/100"
 ----------------------------------------------------------------------------------------------*/
void MM_show(MM_printCB printCB, const U8 *printlayout, U32 *totalUM, float *allocFailRate)
{
	U32 count = 0;

	if (totalUM != NULL)
	{
		*totalUM = 0;
	}
	while(smpMemMngr)
	{
		MM_delay10ms(10);
	}
	smpMemMngr = 1;
	if (MM_list != NULL)
	{
		for(U32 i = 0; i < MEMMNGR_LIST_SIZE; i++ )
		{
			if (MM_list[i].size)
			{
				printCB(printlayout, count++, MM_list[i].size, MM_runtime - MM_list[i].t_getMem, MM_list[i].t_keep);
				if (totalUM != NULL)
				{
					*totalUM += MM_list[i].size;
				}
			}
		}
		if (allocFailRate != NULL)
		{
			*allocFailRate = MM_AFct / MM_Atotal;
		}
		if (!count)
		{
			printCB("\r\n<EMPTY>");
		}
	}
	smpMemMngr = 0;
}

/*******************************************************************************
 * brief
 * param
 * return
 *******************************************************************************/
void MM_guarding(void)
{
	while(smpMemMngr)
	{
		MM_delay10ms(10);
	}
	smpMemMngr = 1;
	if (MM_list != NULL)
	{
		// scanning and freeing unused memory, and sorting by living time
		for (U16 i = 0; i < MEMMNGR_LIST_SIZE; i++)
		{
			if (MM_list[i].size)
			{
				MM_dbgPrt("\r\n %s:A=%X S=%u T=%u/%u", __func__, MM_list[i].addr, MM_list[i].size, MM_runtime - MM_list[i].t_getMem, MM_list[i].t_keep);
				if ((MM_runtime >= (MM_list[i].t_getMem + MM_list[i].t_keep)) && (MM_list[i].t_keep))
				{
					/* Keep time is over */
					MM_dbgPrt("\r\n %s:AutoFree:A=%X", __func__, MM_list[i].addr);
					MM_dealloc(MM_list[i].addr);
					memset(&MM_list[i], 0, sizeof(MM_LIST_E_t));
				}
				else
				{
					/* Sorting */
#if 0
					for (U16 i2 = i; i2 < MEMMNGR_LIST_SIZE; i2++)
					{
						if (MM_list[i2].addr != NULL)
						{
							if ((MM_list[i2].t_getMem < MM_list[i].t_getMem) || //
									((MM_list[i2].t_getMem == MM_list[i].t_getMem) && //
											(MM_list[i2].size > MM_list[i].size)))
							{
								MM_LIST_E_t temp = MM_list[i2];
								MM_list[i2] = MM_list[i];
								MM_list[i] = temp;
							}
						}
					}
#endif
				}
			}
		}
	}
	smpMemMngr = 0;
}
/*******************************************************************************
 * brief:	   	get memory
 *
 * param:		tKeep	|	I	|	time keep allocated variable in memory before being automatically freed
 * 									, unit:sec.
 * 									Attention: tKeep = 0 -> Keep it forever in memory.
 * 				size	|	I	|	size of expected memory.
 *
 * return:		pointer of new memory.
 * Note:		new memory is already reset (set all memory to zero values).
 *******************************************************************************/
void* MM_get( U32 tKeep, U32 size )
{
#if USE_ADVANCED_MM
	void* pointer = NULL;

	while (smpMemMngr)
	{
		MM_delay10ms(10);
	}
	smpMemMngr = 1;
	pointer = MM_alloc(size);
	if (pointer != NULL)
	{
		memset(pointer, 0, size);
		if (MM_list != NULL)
		{
			for (U32 i = 0; i < MEMMNGR_LIST_SIZE; i++)
			{
				if (MM_list[i].size == 0)
				{
					MM_list[i].t_getMem = MM_runtime;
					MM_list[i].addr = pointer;
					MM_list[i].size = size;
					MM_list[i].t_keep = tKeep;
					MM_dbgPrt("\r\n %s:A=%X", __func__, MM_list[i].addr);
					break;
				}
			}
		}
	}
	else
	{
		MM_AFct++;
	}
	MM_Atotal++;
	smpMemMngr = 0;
	return pointer;
#else
	return MM_alloc(size);
#endif
}
/*******************************************************************************
 * brief	   	getMem
 * param
 * return
 *************************************************s******************************/
void MM_free( void *addr )
{
#if USE_ADVANCED_MM
	if (addr != NULL)
	{
		while (smpMemMngr)
		{
			MM_delay10ms(10);
		}
		smpMemMngr = 1;
		if (MM_list != NULL)
		{
			for (U32 i = 0; i < MEMMNGR_LIST_SIZE; i++)
			{
				if (MM_list[i].addr == addr)
				{
					MM_dbgPrt("\r\n %s:A=%X", __func__, MM_list[i].addr);
					memset(&MM_list[i], 0, sizeof(MM_LIST_E_t));
					break;
				}
			}
		}
		MM_dealloc(addr);
		addr = NULL;
		smpMemMngr = 0;
	}
#else
	MM_dealloc(addr);
#endif
}
