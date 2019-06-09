/* Martin Thomas 4/2009 */

#if _FS_NORTC
#include "RTC_STM32.h"
#include "ParseTime.h"
#include "platform_config.h"

unsigned long get_fattime(void)
{
	unsigned long res;
	DateTime date;

	UTC_2_date(BKPREG_getRTCcounter(), &date, 7);
	res = (((date.year + 2000) - 1980) << 25)//
			| (date.month << 21)//
			| (date.day << 16)//
			| (date.hour << 11)//
			| (date.minute << 5)//
			| (date.second >> 1);

	return res;
}
#endif // _FS_NORTC
