/*----------------------------------------------------------------------------
 * Author: Duy Vinh To
 * Created date: 5th July 2016
 * Modified date: 5th July 2016
 * Version: 1.0
 *
 * Describe:
 -----------------------------------------------------------------------------*/
#ifndef CIRBUF_H_
#define CIRBUF_H_
/*----------------------------------------------------------------------------
 * INCLUDE
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * DEFINE & MACRO
 -----------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;
/*----------------------------------------------------------------------------
 * TYPEDEF
 -----------------------------------------------------------------------------*/
typedef struct
{
	U16 in;
	U16 out;
	U16 count;
	U16 unitsize;//Size of a unit in buffer.
	U16 size;//Size of buffer: number of unit in buffer.
	void *buf;//Pointer to buffer on RAM.
} CIRBUF_t;

/*----------------------------------------------------------------------------
 * FUNC. PROTOTYPES
 -----------------------------------------------------------------------------*/
extern void CIRBUF_setup(CIRBUF_t *cbuf, U16 unitsize, U16 size, void *buf);
extern U8 CIRBUF_add(CIRBUF_t *cbuf, const void *e);
extern U8 CIRBUF_get(CIRBUF_t *cbuf, void *e);
extern U16 CIRBUF_count(CIRBUF_t cbuf);

#endif // CIRBUF_H_
