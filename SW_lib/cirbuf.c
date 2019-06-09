/*----------------------------------------------------------------------------
 * Author: Duy Vinh To
 * Created date: 5th July 2016
 * Modified date: 5th July 2016
 * Version: 1.0
 *
 * Describe:
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * INCLUDE
 -----------------------------------------------------------------------------*/
#include "cirbuf.h"
#include "string.h"
/*----------------------------------------------------------------------------
 * FUNCTION
 -----------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------
 *
 ------------------------------------------------------------------------------*/
void CIRBUF_setup(CIRBUF_t *cbuf, U16 unitsize, U16 size, void *buf)
{
	cbuf->in = 0;
	cbuf->out = 0;
	cbuf->count = 0;
	cbuf->unitsize = unitsize;
	cbuf->size = size;
	cbuf->buf = buf;
}
/*-----------------------------------------------------------------------------
 * Brief: add new data to circular buffer.
 * Param:	cbuf	|	I/O		|	circular buffer.
 * 			e		|	I		|	data to add.
 * Ret:		0	|	OK
 * 			1	|	FAIL (Buffer is full)
 ------------------------------------------------------------------------------*/
U8 CIRBUF_add(CIRBUF_t *cbuf, const void *e)
{
	if (cbuf->count < cbuf->size)
	{
		memcpy(&cbuf->buf[cbuf->in * cbuf->unitsize], e, cbuf->unitsize);
		cbuf->in++;
		if (cbuf->in >= cbuf->size)
		{
			cbuf->in = 0;
		}
		cbuf->count++;
		return 0;
	}
	return 1;
}
/*-----------------------------------------------------------------------------
 * Brief: Get data from circular buffer.
 * Param:	cbuf	|	I/O		|	circular buffer.
 * 			e		|	O		|	pointer to store gotten data.
 * Ret:		0	|	OK
 * 			1	|	FAIL (Buffer is empty)
 ------------------------------------------------------------------------------*/
U8 CIRBUF_get(CIRBUF_t *cbuf, void *e)
{
	if (cbuf->count > 0)
	{
		memcpy(e, &cbuf->buf[cbuf->out * cbuf->unitsize], cbuf->unitsize);
		cbuf->out++;
		if (cbuf->out >= cbuf->size)
		{
			cbuf->out = 0;
		}
		cbuf->count--;
		return 0;
	}
	return 1;
}
/*-----------------------------------------------------------------------------
 * Brief: Get length (number of available data) of circular buffer.
 * Param:	cbuf	|	I	|	circular buffer.
 * Ret:		length.
 ------------------------------------------------------------------------------*/
U16 CIRBUF_count(CIRBUF_t cbuf)
{
	return cbuf.count;
}
