/* Time conversions */
#ifndef HOUR2SEC
#define HOUR2SEC(x)	(x * 3600)
#endif
#define MIN2SEC(x)	(x * 60)
/* Bitwise operations */
#define AND(x,y)		(x & y)
#define OR(x,y)			(x | y)
#define XOR(x,y)		(x ^ y)
#define NOT(x)			(~x)
/* Advanced bitwise operations
 * p: bit position/order
 * x: object to process bit.
 * */
#define BIT(p)			(0x1 << p)
#define GETBIT(x,p)		((x & BIT(p)) ? 1 : 0)
#define SETBIT(x,p)		x |= BIT(p)
#define CLRBIT(x,p)		x &= NOT(BIT(p))
