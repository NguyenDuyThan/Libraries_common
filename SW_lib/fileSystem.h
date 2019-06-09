/************************************************************************************
 * author:Duy Vinh
 *
 * description:
 * 	provide function to work file system (Header file).
 *
 ************************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_
/*-----------------------------------------------------------------------------------------------
 * INCLUDE
 ------------------------------------------------------------------------------------------------*/
#include "ff.h"
/*-----------------------------------------------------------------------------------------------
 * DEFINES
 ------------------------------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned long U32;
/*-----------------------------------------------------------------------------------------------
 * TYPEDEFS
 ------------------------------------------------------------------------------------------------*/
typedef struct
{
	U32 EC;	// Error counter
	U32 WC; // Write counter
}FS_INFO_t;

typedef void(*FS_delay10ms_CB)(U32 t);
typedef void(*FS_dbgPrt_CB)(const U8 *s, ...);
/*-----------------------------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 ------------------------------------------------------------------------------------------------*/
extern void FS_setup_dbgPrt(FS_dbgPrt_CB cb);
extern void FS_setup(FS_delay10ms_CB delay10msCB);
extern void FS_lockAccess(U8 set);
extern void FS_unmount();
extern void FS_remount();
extern U8 FS_mount();
extern void FS_format();
//extern FS_INFO_t FS_accessInfo(void);
extern U8 FS_createDir( const U8* dirname );
extern U8 FS_createFile( const U8* fname, U8 overwrite );
extern U8 FS_wrt( const U8* fname, U32 ofs, U32 len, const void* dat );
extern U8 FS_rd(const U8* fname, U32 ofs, U32 size, void* dat, U32 *rLen);
extern U8 FS_append( const U8* fname, U32 len, const void* dat );
extern void FS_renameFile( const U8 *oldname, const U8 *newname );
extern void FS_delFile( const U8* Filename );
extern void FS_delDir( const U8* Dirname );
/**********************************************************************************
 * Brief		search for FIRST file in directory with input extension
 * Param		dirname		|	I	|	directory name
 * 				filterS		|	I	|	file must contain this string in name
 * 				filename	|	O	|	<>
 * 				flnameSize	|	I	|	<>
 * 				resCount	|	O	|	result counter (matched condition)
 * @reval		none
 *********************************************************************************/
extern void FS_searchFileInDir(const U8 *dname, U8 get1st, const U8 *s2filter, U16 fnameSize, U8 *fname, U16* matchedCt);
extern U8 FS_rdDir( const U8* dname, void (*outTerm)( const U8* data, ... ) );
extern U8 FS_chkDirExist( const U8 *dirname );
extern U8 FS_chkFileExist( const U8* filename );
extern U8 FS_fsize(const U8 *fname, U32 *fsize);
extern U32 FS_cap( void );
extern U8 FS_stream( const U8 *fname, U32 ofs, U32 bts, U32 *bs, U8 *dat, U16 size, U8 (func)( const U8*, U32 ), void (errCode)( U8 ) );

extern U32 FS_readFinalLine( const U8* filename, U16 expectedSize, U8* str );
extern int FS_fwdFullFileContent( const U8* filename, U32 ofs, U32 len, void (*outTermBytes)( const U8* data, ... ) );

#endif // FILESYSTEM_H_

