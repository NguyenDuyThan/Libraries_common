/*
 * NORflash_branchInfo.h
 *
 *  Created on: Jan 20, 2017
 *      Author: dv198
 */

#ifndef HW_LIB_NORFLASH_BRANCHINFO_H_
#define HW_LIB_NORFLASH_BRANCHINFO_H_

#define CYPRESS_S25FL256S_256Mb_PAGE_SIZE				256
#define CYPRESS_S25FL256S_256Mb_SECTOR_SIZE				(64 * 1024)
#define CYPRESS_S25FL256S_256Mb_UNIFORM_SECTOR_SIZE		(256 * 1024)
#define CYPRESS_S25FL256S_256Mb_TOTAL_SIZE				(32 * 1024 * 1024)

/* Below is sample values of some flash memory ICs */
// ST NOR flash M25P32: 32-Mb = 4-MB
#define ST_MANID				0x20 // ST manufacturer ID
#define ST_M25P32_32Mb_MEMTYP	0x20
// Cypress/Spansion NOR flash: S25FL256S
#define CYPRESS_MANID					0x01 // Cypress manufacturer ID
#define CYPRESS_S25FL256S_128Mb_MEMTYPE	0x20
#define CYPRESS_S25FL256S_256Mb_MEMTYPE	0x02
#define CYPRESS_S25FL256S_128Mb_MEMCAP	0x18
#define CYPRESS_S25FL256S_256Mb_MEMCAP	0x19

#endif /* HW_LIB_NORFLASH_BRANCHINFO_H_ */
