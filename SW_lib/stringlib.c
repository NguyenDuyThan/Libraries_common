/*-----------------------------------------------------------------------------------------
 * Author: Duy Vinh To
 * Team: DinhViSo corp.
 *
 * Created date: 7th July 2016
 *
 * Brief: process string
 ------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/
#include "stringlib.h"
#include "stdlib.h"
#include "macro.h"
/*-----------------------------------------------------------------------------------------
 *
 ------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------------------
 * FUNCTIONS
 ------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------
 * brief: string split
 * param[in]:const char, char, unsigned int, unsigned int
 * param[out]:char*,char*
 * retval:none
 *
 * details: use a character to split a string
 * Note: out_head and out_tail are protected (from overflow)
 * 			by head_length and tail_length (including NULL character at end of the string)
 * Example:
 * SL_split("AB C D", ' ', heads, 32, tails, 32);
 * Result: heads = "AB", tails = "C D"
 ------------------------------------------------------------------------------------------*/
void SL_split(const U8* str, U8 chr, U8* out_head, U16 head_length, U8* out_tail, U16 tail_length)
{
#define EN_STRINGSPLIT_BYTEPROCESS	1
#if EN_STRINGSPLIT_BYTEPROCESS
	U16 pos, pos_head = 0, pos_tail = 0, len = strlen(str);
	U8 flag_found = 0;

	if (len != 0)
	{
		for (pos = 0; pos < len; pos++)
		{
			if (flag_found == 1)
			{
				if ((pos_tail < (tail_length - 1)) || (tail_length == 0))
				{
					out_tail[pos_tail++] = str[pos];
				}
			}
			if ((str[pos] == chr) && (flag_found == 0))
			{
				flag_found = 1;
			}
			if ((str[pos] != chr) && (flag_found == 0))
			{
				if ((pos_head < (head_length - 1)) || (head_length == 0))
				{
					out_head[pos_head++] = str[pos];
				}
			}
		}
	}
	out_tail[pos_tail] = 0;
	out_head[pos_head] = 0;
#else
	unsigned int pos, len = strlen(str);

	//try new way
	if (len != 0)
	{
		for (pos = 0; pos < len; pos++)
		{
			if (str[pos] == chr)
			break;
		}
		TermPrint("\r\n StringSplit_ IN=[%s]",str);
//		memset(&out_head[0], 0, head_length);
		if (head_length > pos)
		head_length = pos;
//		memcpy(&out_head[0], &str[0], head_length);
		LIB_strSub(0,head_length,str,out_head);
		TermPrint("\r\n StringSplit_ H=[%s]",out_head);
		TermPrint("\r\n StringSplit_ INAH=[%s]",str);
		//
//		memset(&out_tail[0], 0, tail_length);
		if (tail_length > (len - pos - 1))
		tail_length = (len - pos - 1);
//		memcpy(&out_tail[0], &str[pos + 1], tail_length);
		LIB_strSub(pos+1,tail_length,str,out_tail);
		TermPrint("\r\n StringSplit_ START=%u LEN=%u T=[%s]",pos + 1,tail_length,out_tail);
	}
#endif
}
/*************************************************************
 * Check if a string "in" start with head
 *
 * return 0 if true
 * 		  !=0 if false
 * Example: 
 * res = SL_startwith("ABCZ", "AB");
 * -> res = 0
 *************************************************************/
int SL_startwith(const U8* s, const U8 *sStart)
{
	return strncmp(s, sStart, strlen(sStart));
}
/*****************************************************************************
 * remove a part (string format) off a string (1st wanted part found in string)
 *
 * return 0: found
 * 		  1: not found
 * Example: 
 * inS = "ABCZ"
 * res = SL_cut(inS, "AB");
 * -> res = 1, inS = "CZ"
 *****************************************************************************/
U8 SL_cut(U8 *s, const U8 *sCut)
{
	int res = SL_search(s, sCut);
	U16 len_cut = strlen(sCut);
	U16 len = strlen(s);
	if (res != -1)
	{
		while ((res + len_cut) <= len)
		{
			s[res] = s[res + len_cut];
			res++;
		}
		return 0;
	}
	else
	{
		return 1;
	}
}
/***************************************************************************
 * Brief: get a string from input string
 *
 * Param:	start	|	in	|	start/begin position in input string
 * 			length	|	in	|	length (be counted from start position) to get.
 *			src		|	in	|	source/input string pointer.
 *			dst		|	out	|	destination/output string pointer.
 *
 * Note:	above dst must have size = legnth + 1 (NULL char).
 * Example:
 * LIB_strSub(0,5,"vagabond",dst);
 * ==> dst = "vagab"
 **************************************************************************/
void SL_sub(U16 start, U16 length, const U8 *src, U8 *dst)
{
#define EN_SUBSTRING_BYTEPROCESS	0
#if EN_SUBSTRING_BYTEPROCESS
	int pos = start;
	int pos_dst = 0;

	while((src[pos] != 0) && (pos_dst < length))
	{
		dst[pos_dst++] = src[pos];
		pos++;
	}
	dst[pos_dst] = 0;
#else
	strlcpy(&dst[0], &src[start], length + 1);
#endif
}
/*----------------------------------------------------------------------------
 Convert from string to double
 *----------------------------------------------------------------------------*/
double SL_s2double(const U8* str)
{
	U8 i, minus = 0;
	U32 val = 0, div = 0;
	double f = 0;

	if (str[0] == '-')
	{
		minus = 1;
		i = 1;
	}
	else
	{
		minus = 0;
		i = 0;
	}
	while (str[i])
	{
		if (str[i] == '.')
		{
			div = 1;
		}
		else if ((str[i] >= '0') && (str[i] <= '9'))
		{
			val = (val * 10) + (str[i] - '0');
			if (div)
			{
				div *= 10;
			}
		}
		else
		{
			return 0;
		}
		i++;
	}
	f = (double)val;
	if (div)
	{
		f = f / (double)div;
	}
	return (minus) ? -f : f;
}

/**************************************************************************
 * find position of a tring in another larger one
 *
 * return -1 if cannot find
 * 		  ><-1 if found, this is position of "sTarget" in "s"
 **************************************************************************/
int SL_search(const U8 *s, const U8 *sTarget)
{
	U16 pos//
		, pos_mark = 0//
		, pos_fnd = 0//
		, len = strlen(s)//
		, len_target = strlen(sTarget);
	if (len_target > len)
	{
		return -1;
	}
	for (pos = 0; pos <= (len - len_target); pos++)
	{
		if (strncmp(&s[pos], sTarget, len_target) == 0)
		{
			return pos;
		}
	}
	return -1;
}
/*----------------------------------------------------------------------------

 *----------------------------------------------------------------------------*/
void SL_parseDouble(double in, int *integer, U32 *decimal)
{
	U8 step = 0;
	*decimal = 0;
	*integer = (int)in;
	in -= *integer;
	if (in < 0)
	{
		in = -in;
	}
	*decimal = (U32)(in * 1000000);
}
/*----------------------------------------------------------------------------
 * Brief: convert double to string
 * Note: size should be larger than 11
 *----------------------------------------------------------------------------*/
U8 *SL_double2s(double input, U32 size, U8 *s)
{
	int intg = 0;
	U32 dec = 0;
	SL_parseDouble(input, &intg, &dec);
	s[size - 1] = 0;
	snprintf(s, size, "%d.%06u", intg, dec);
	return s;
}
/*---------------------------------------------------------------------------
 * Brief: verify is a string that satisfy filter.
 * Param: 	s			|	IN	|	input string.
 * 			len			|	IN	|	string length.
 * 			filterMark	|	IN	|	filter mask bits.
 * 			extFilter	|	IN	|	extended filter: list of characters to filter.
 * 			efLen		|	IN	|	extended filter lenght.
 * Ret:	0	|	OK
 * 		>0	|	FAIL
 * Note: Filter bit:
 *  + 0x01 (BIT 0) : alphabet characters (include capitals).
 *  + 0x02 (BIT 1) : numeric characters.
 *  + 0x04 (BIT 2) : <CR>,<LF> characters.
 *  + 0x08 (BIT 3): printable characters (from ' ' to '~').
 *  + 0x10 (BIT 4) : extended filter.
 ----------------------------------------------------------------------------*/
U8 SL_verify( const U8 *s, U16 len, U8 filterMask, const U8 *extFilter, U16 efLen)
{
	U16 i = 0, i2 = 0;

	for(i = 0; i < len; i++)
	{
		if ((filterMask & BIT(0)) && (((s[i] >= 'a') && (s[i] <= 'z')) || ((s[i] >= 'A') && (s[i] <= 'Z'))))
		{
			goto LIB_STRVRF_ENDACYCLE;
		}
		else if ((filterMask & BIT(1)) && ((s[i] >= '0') && (s[i] <= '9')))
		{
			goto LIB_STRVRF_ENDACYCLE;
		}
		else if ((filterMask & BIT(2)) && ((s[i] == '\r') || (s[i] == '\n')))
		{
			goto LIB_STRVRF_ENDACYCLE;
		}
		else if ((filterMask & BIT(3)) && ((s[i] >= ' ') && (s[i] <= '~')))
		{
			goto LIB_STRVRF_ENDACYCLE;
		}
		else if (filterMask & BIT(4))
		{
			for(i2 = 0; i2 < efLen; i2++)
			{
				if (s[i] == extFilter[i2])
				{
					goto LIB_STRVRF_ENDACYCLE;
				}
			}
		}
		return (i + 1);
		LIB_STRVRF_ENDACYCLE://
		continue;
	}
	return 0;
}
