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
#include "dbgPrint.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
/*----------------------------------------------------------------------------
 * VARIABLES
 -----------------------------------------------------------------------------*/
static DBG_printc_CB DBG_printcCB = NULL;
static U8 DBGbusy = 0;

static void DBG_print_c(U8 c);
static void DBG_print_s( const U8 *buf, U16 len );
static void DBG_print_d(int32_t num, U8 len);
static void DBG_print_u(U32 num, U8 len);
static void DBG_print_h(U8 upcase, U32 hexnum, U8 len);
static void DBG_print_f(double dbnum);
/*----------------------------------------------------------------------------
 * FUNCTION
 -----------------------------------------------------------------------------*/
/*---------------------------------------------
 * Brief:
 *--------------------------------------------*/
void DBG_setup(DBG_printc_CB cb)
{
	while (DBGbusy);
	DBGbusy = 1;
	DBG_printcCB = cb;
	DBGbusy = 0;
}
/*---------------------------------------------
 * Brief: send string
 *--------------------------------------------*/
void DBG_print_c(U8 c)
{
	if (DBG_printcCB != NULL)
	{
		DBG_printcCB(c);
	}
}
/*---------------------------------------------
 * Brief: send string
 *--------------------------------------------*/
void DBG_print_s( const U8 *buf, U16 len )
{
	U16 i = 0;

	if (buf == NULL)
	{
		DBG_print_c('?');
		return;
	}
	while (*buf)
	{
		if (*buf == 0)
		{
			break;
		}
		DBG_print_c(*buf);
		if( len )
		{
			i++;
			if (i >= len)
			{
				break;
			}
		}
		*buf++;
	}
}
/*---------------------------------------------
 * Brief: send number
 *--------------------------------------------*/
void DBG_print_d(int32_t num, U8 len)
{
	/*
	U8 tmp[11];

	snprintf(tmp, 11, "%d", num);
	tmp[10] = 0;
	DBG_print_s(tmp, 0);
	*/
	if (num < 0)
	{
		DBG_print_c('-');
		num = -num;
	}
	DBG_print_u(num, len);
}
/*---------------------------------------------
 * Brief: send number
 *--------------------------------------------*/
void DBG_print_u(U32 num, U8 len)
{
	/*
	U8 tmp[11];

	snprintf(tmp, 11, "%u", num);
	tmp[10] = 0;
	DBG_print_s(tmp, 0);
	*/
	U8 s[11];
	U8 l = 0, n = 0, i = 0;

	for (i = 0; i <= 10; i++)
	{
		s[i] = 0;
	}
	for (i = 0; i <= 10; i++)
	{
		n = num % 10;
		num /= 10;
		s[l++] = n + '0';
		if (!num)
		{
			break;
		}
	}

	if (len > l)
	{
		for (i = 0; i < (len - l); i++)
		{
			DBG_print_c('0');
		}
	}
	i = 10;
	while (1)
	{
		if (s[i])
		{
			DBG_print_c(s[i]);
		}
		if (!i)
		{
			break;
		}
		i--;
	}
}
/*---------------------------------------------
 * Brief: send hex number
 *--------------------------------------------*/
void DBG_print_h(U8 upcase, U32 hexnum, U8 len)
{
	U8 s[9], i = 0, tmp = 0, l = 0;

	if (!hexnum)
	{
		DBG_print_u(0, len);
		return;
	}
	memset(s, 0, 9);
	for (i = 0; i < 8; i++)
	{
		if (!hexnum)
		{
			break;
		}
		tmp = hexnum % 16;
		if ((tmp >= 0) && (tmp <= 9))
		{
			s[i] = tmp + '0';
		}
		else
		{
			s[i] = tmp - 10 + ((upcase) ? 'A' : 'a');
		}
		hexnum /= 16;
	}
	l = i;
	if (len > l)
	{
		for (i = 0; i < (len - l); i++)
		{
			DBG_print_c('0');
		}
	}
	i = l;
	while (1)
	{
		if (s[i])
		{
			DBG_print_c(s[i]);
		}
		if (!i)
		{
			break;
		}
		i--;
	}
}
/*----------------------------------------------------------------------------

 *----------------------------------------------------------------------------*/
void DBG_print_f(double dbnum)
{
	U8 step = 0;
	U32 decimal = 0;
	int integer = 0;

#if 0
	if (dbnum)
	{
		integer = (int)dbnum;
		dbnum -= integer;
		decimal = (U32)(dbnum * 1000000);
		while (!(decimal % 10) && decimal)
		{
			decimal /= 10;
		}
	}
	DBG_print_d(integer, 0);
	DBG_print_c('.');
	DBG_print_d(decimal, 0);
#else
	integer = (int)dbnum;
	dbnum -= integer;
	DBG_print_d(integer, 0);
	DBG_print_c('.');
	for (step = 0; step < 6; step++)
	{
		dbnum *= 10;
		integer = (int)dbnum;
		dbnum -= integer;
		if ((dbnum == 0) && step)
		{
			break;
		}
		DBG_print_u(integer, 0);
	}
#endif
}
/*---------------------------------------------
 * Brief: print with multiple types
 * Example:
 * DBG_print("System is %d% %s", 100, "working")
 * -> Result: "System is 100% working"
 *
 * Support printing type:
 * + s: string (char array).
 * + c: char.
 * + d: signed integer.
 * + u: unsigned integer.
 * + x: hex number.
 *--------------------------------------------*/
void DBG_print(const U8 *s, ...)
{
	va_list vl;
	U32 timepass = 0;
	U8 len = 0;

	while (DBGbusy && (timepass++ < 1000000));
	DBGbusy = 1;
	va_start(vl, s);
	//va_arg(vl, u8*);
	while(*s)
	{
#if UNHIDE_CRLF
		if (*s == '\r')
		{
			DBG_print_s("<CR>", 0);
		}
		if (*s == '\n')
		{
			DBG_print_s("<LF>", 0);
		}
#endif
		if(*s == '%')
		{
			*s++;
			if ((*s >= '0') && (*s <= '9'))
			{
				len = (*s - '0') * 10;
				*s++;
				len += *s - '0';
				*s++;
			}
			switch(*s)
			{
				case 's':
					DBG_print_s(va_arg(vl, U8*), 0);
					break;
				case 'c':
					DBG_print_c(va_arg(vl, U8*));
					break;
				case 'd':
					DBG_print_d(va_arg(vl, int32_t), len);
					break;
				case 'u':
					DBG_print_u(va_arg(vl, U32), len);
					break;
				case 'x':
					DBG_print_h(0, va_arg(vl, U32), len);
					break;
				case 'X':
					DBG_print_h(1, va_arg(vl, U32), len);
					break;
				case 'f':
					DBG_print_f(va_arg(vl, double));
					break;
				default:
					*s--;
					goto DBG_PRINT_NORMAL;
			}
			*s++;
			continue;
		}
		DBG_PRINT_NORMAL://
		DBG_print_c(*s);
		*s++;
	}
	va_end(vl);
	DBGbusy = 0;
}
