/*-----------------------------------------------------------------------------------------
 * Author: Duy Vinh To
 * Team: DinhViSo corp.
 *
 * Created date: 7th July 2016
 *
 * Brief: process string
 ------------------------------------------------------------------------------------------*/
#ifndef STRINGLIB_H_
#define STRINGLIB_H_
/*-----------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;
/*-----------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------*/
extern void SL_split(const U8* str, U8 chr, U8* out_head, U16 head_length, U8* out_tail, U16 tail_length);
extern int SL_startwith(const U8* s, const U8 *sStart);
extern U8 SL_cut(U8 *s, const U8 *sCut);
extern void SL_sub(U16 start, U16 length, const U8 *src, U8 *dst);
extern double SL_s2double(const U8* str);
extern int SL_search(const U8 *s, const U8 *sTarget);
extern void SL_parseDouble(double in, int *integer, U32 *decimal);
extern U8 *SL_double2s(double input, U32 size, U8 *s);
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
 *  + 0x19 (BIT 4) : extended filter.
 ----------------------------------------------------------------------------*/
extern U8 SL_verify( const U8 *s, U16 len, U8 filterMask, const U8 *extFilter, U16 efLen);

#endif // STRINGLIB_H_
