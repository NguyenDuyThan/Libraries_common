/************************************************************************************
 * author:Duy Vinh
 *
 * description:
 * 	provide function to work file system (Source file).
 *
 ************************************************************************************/

/***********************************************************************************************
 * INCLUDE
 ***********************************************************************************************/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
//#include "integer.h"
#include "diskio.h"
#include "ff.h"
//#include "rtc.h"
#include "fileSystem.h"
#include "dbgPrint.h"
/* freeRTOS inc file */
#include "macro.h"
#include "stringlib.h"

/***********************************************************************************************
 * DEFINITIONS & MACROS
 ***********************************************************************************************/
//#define USING_RTOS_SEMAPHORE
#define DISABLE_UNRESOLVED_MALFUNCTIONS		1
/***********************************************************************************************
 * IMPORTED VARIABLES
 ***********************************************************************************************/
static FS_INFO_t FS_info = {0, 0, 0};
static U8 lockAccess = 0;

/***********************************************************************************************
 * FUNCTION PROTOTYPES
 ***********************************************************************************************/
static void FS_createSem(void);
static U8 FS_setSem(void);
static void FS_dbgPrt_pseudo(const U8 *s, ...);

/***********************************************************************************************
 * VARIABLES
 ***********************************************************************************************/
static FATFS Fatfs[1]; /* File system object: storing access to physical drive */
#ifdef USING_RTOS_SEMAPHORE
xSemaphoreHandle xSemaphore_SDcard = NULL;
#else
static U8 SDcard_busy = 0;
#endif
static FS_delay10ms_CB FS_delay10ms;
static FS_dbgPrt_CB FS_dbgPrt = FS_dbgPrt_pseudo;

/***********************************************************************************************
 * FUNCTIONS
 ***********************************************************************************************/
/*-------------------------------------------------------------------------------------------
 * Brief:
 --------------------------------------------------------------------------------------------*/
void FS_dbgPrt_pseudo(const U8 *s, ...)
{
	return;
}
/*-------------------------------------------------------------------------------------------
 * Brief:
 --------------------------------------------------------------------------------------------*/
void FS_setup_dbgPrt(FS_dbgPrt_CB cb)
{
	if (cb == NULL)
	{
		FS_dbgPrt = FS_dbgPrt_pseudo;
		return;
	}
	FS_dbgPrt = cb;
}
/*-------------------------------------------------------------------------------------------
 * Brief: Setup File system
 * Param:	delay10msCB	|	IN	|	delay x 10ms callback.
 * Note:
 * + Because FatFS implement is very complicated, everything is done in diskio.c and diskio.h,
 * not via this setup function.
 * + This setup function only setup delay callback function that is used in this source file.
 --------------------------------------------------------------------------------------------*/
void FS_setup(FS_delay10ms_CB delay10msCB)
{
	FS_delay10ms = delay10msCB;
}
/***************************************************************************************************
 * Brief: 		occupy_SDcard_resource
 * Param[in]:	none
 * Param[out]:	none
 * Retval:		none
 *
 * @details:   should using RTOS semaphore to solving accessing shared resource conflict
 ****************************************************************************************************/
void FS_createSem(void)
{
#ifdef USING_RTOS_SEMAPHORE
	xSemaphore_SDcard = xSemaphoreCreateMutex();
#endif
}
/***************************************************************************************************
 * Brief: 		occupy_SDcard_resource
 * Param[in]:	none
 * Param[out]:	none
 * Retval:		none
 *
 * @details:   should using RTOS semaphore to solving accessing shared resource conflict
 ****************************************************************************************************/
U8 FS_setSem(void)
{
#ifdef USING_RTOS_SEMAPHORE
	if (xSemaphore_SDcard != NULL)
	{
		if (xSemaphoreTake(xSemaphore_SDcard, 1000) == pdTRUE)
		{
			return 1;
		}
	}
	FS_dbgPrt("CANNOT OCCUPY SDCARD RESOURCE");
	return 0;
#else
	U8 loop = 0;
	do
	{
		FS_delay10ms(10);
		if (loop++ >= 100)
		{
			FS_dbgPrt("\r\n %s:busy", __func__);
			loop = 0;
		}
	}
	while (SDcard_busy == 1);
	SDcard_busy = 1;
	return 1;
#endif
}
/***************************************************************************************************
 * Brief: 		release_SDcard_resource
 * Param[in]:	none
 * Param[out]:	none
 * Retval:		none
 *
 * @details:
 * Return:		should using RTOS semaphore to solving accessing shared resource conflict
 ****************************************************************************************************/
void FS_rstSem(void)
{
#ifdef USING_RTOS_SEMAPHORE
	xSemaphoreGive(xSemaphore_SDcard);
#else
	SDcard_busy = 0;
#endif
}
/***************************************************************************************************
 * Brief: 		Interact with lock access flag of file system
 ****************************************************************************************************/
void FS_lockAccess(U8 set)
{
	lockAccess = set;
}
/***************************************************************************************************
 * Brief: 		CreateFile
 * Param[in]:	U8*
 * Param[out]:	none
 * Retval:		none
 *
 * @details:	and create new file (if not existed)
 *
 * Return:		>0	|	FAIL
 * 				0	|	OK
 ****************************************************************************************************/
U8 FS_createFile(const U8* fname, U8 overwrite)
{
	const int MAX_DIRNAME_LEN = 64;
	FRESULT rc;
	FIL fp;
	int i = 0, dirNameBeginPos = 0, dirNameLen = 0, fnameLen = strlen(fname);
	U8 rtry = 0;
	U8 dirName[MAX_DIRNAME_LEN + 1];

	if (lockAccess)
	{
		return 0xFF;
	}
	for (rtry = 0; rtry < 3; rtry++)
	{
		FS_setSem();
		if (overwrite)
		{
			f_unlink(fname);
			rc = f_open(&fp, fname, FA_CREATE_ALWAYS);
		}
		else
		{
			rc = f_open(&fp, fname, FA_CREATE_NEW);
		}
		f_close(&fp);
		FS_rstSem();
		if ((rc == (FRESULT) FR_OK) || (rc == (FRESULT) FR_EXIST))
		{
			FS_info.EC = (FS_info.EC > 0) ? (FS_info.EC - 1) : FS_info.EC;
			return 0;
		}
		FS_dbgPrt("\r\n %s:\"%s\",%u RC=%u Rtry=%d", __func__, fname, overwrite, rc, rtry);
		if (rc == (FRESULT) FR_NO_PATH)
		{
			// Auto create directory, dir name is parsed from filename
			for (i = dirNameBeginPos; i < fnameLen; i++)
			{
				if (fname[i] == '/')
				{
					dirNameLen = i - dirNameBeginPos;
					if (dirNameLen >= MAX_DIRNAME_LEN)
					{
						dirNameLen = MAX_DIRNAME_LEN - 1;
					}
					SL_sub(dirNameBeginPos, dirNameLen, fname, dirName);
					dirNameBeginPos += dirNameLen;
					i = dirNameBeginPos;
					FS_createDir(dirName);
				}
			}
		}
		else
		{
			FS_info.EC++;
			FS_remount();
		}
	}
	return 1;
}
/***************************************************************************************************
 * brief: 		WriteFile
 * param		fname	|	IN	|	filename
 * 				ofs		|	IN	|	Offset to write from that.
 * 				len		|	IN	|	write data length.
 * 				dat		|	IN	|	data to write.
 * return:		0	|	OK
 *				>1	|	Other error.
 ****************************************************************************************************/
U8 FS_wrt(const U8* fname, U32 ofs, U32 len, const void* dat)
{
	FRESULT rc;
	FIL fp;
	UINT s2 = 1;
	U8 rtry = 0, step = 0, rd[1];
	const U8 *ptr = dat;
	U32 i = 0;

	if (lockAccess)
	{
		return 0xFF;
	}
	len = (!len) ? strlen(dat) : len;
	for (rtry = 0; rtry < 3; rtry++)
	{
		FS_setSem();
		rc = f_open(&fp, fname, FA_OPEN_EXISTING | FA_WRITE | FA_READ);
		f_sync(&fp);
		if ((rc != FR_OK) && (rc != FR_NO_FILE))
		{
			step = 1;
			goto FS_WRTFILE_CONTINUE;
		}
		if (ofs)
		{
			rc = f_lseek(&fp, (FSIZE_t)ofs);
			if ((rc != FR_OK) && (rc != FR_NO_FILE))
			{
				step = 2;
				goto FS_WRTFILE_CONTINUE;
			}
		}
		for (i = 0; i < len; i++)
		{
			s2 = 1;
			if (f_read(&fp, rd, s2, &s2) == FR_OK)
			{
				if (s2 == 1)
				{
					//if (rd[0] == (U8)&dat[i])
					if (rd[0] == ptr[i])
					{
						continue;
					}
				}
			}
			//FS_dbgPrt("\r\n --FS_wrtFile[\"%s\":newDat]", fname);
			goto FS_WRTFILE_WRT;
		}
		//FS_dbgPrt("\r\n --FS_wrtFile[\"%s\":NoNewDat]", fname);
		goto FS_WRTFILE_ENDOK;
		FS_WRTFILE_WRT://
		if (ofs)
		{
			rc = f_lseek(&fp, (FSIZE_t)ofs);
			if ((rc != FR_OK) && (rc != FR_NO_FILE))
			{
				step = 2;
				goto FS_WRTFILE_CONTINUE;
			}
		}
		rc = f_write(&fp, dat, len, &s2);
		FS_info.WC++;
		if ((rc != FR_OK) && (rc != FR_NO_FILE))
		{
			step = 3;
			goto FS_WRTFILE_CONTINUE;
		}
		FS_WRTFILE_ENDOK://
		f_close(&fp);
		FS_rstSem();
		FS_info.EC = (FS_info.EC > 0) ? (FS_info.EC - 1) : FS_info.EC;
		return 0;
		FS_WRTFILE_CONTINUE: //
		FS_rstSem();
		FS_dbgPrt("\r\n %s:\"%s\":Step=%d RC=%d Rtry=%d", __func__, fname, step, rc, rtry);
		FS_info.EC++;
		FS_remount();
	}
	return (step + 200);
}
/***************************************************************************************************
 * brief: 		read file from SDCard.
 *
 * param:		fname	|	I	|	file name.
 * 				ofs		|	I	|	offset to begin reading.
 * 				size	|	I	|	Size to read.
 * 				dat		|	O	|	pointer to output data.
 * 				rLen	|	O	|	read length.
 *
 * return:		0	|	OK
 *				1	|	File/path not existed.
 *				>1	|	Other error.
 ****************************************************************************************************/
U8 FS_rd(const U8* fname, U32 ofs, U32 size, void* dat, U32 *rLen)
{
	FRESULT rc;
	FIL fp;
	U8 rtry = 0, step = 0;
	U32 readLen = 0;

	if (lockAccess)
	{
		return 0xFF;
	}
	for (rtry = 0; rtry < 3; rtry++)
	{
		FS_setSem();
		// Open file for working.
		rc = f_open(&fp, fname, FA_OPEN_EXISTING | FA_READ);
		if ((rc == (FRESULT) FR_NO_FILE) || (rc == (FRESULT) FR_NO_PATH))
		{
			f_close(&fp);
			FS_info.EC = (FS_info.EC > 0) ? (FS_info.EC - 1) : FS_info.EC;
			FS_rstSem();
			return 0;
		}
		else if (rc != FR_OK)
		{
			step = 1;
			goto END_CYCLE_READFILE;
		}
		// Seek to offset to start reading.
		rc = f_lseek(&fp, (FSIZE_t)ofs);
		if (rc != FR_OK)
		{
			step = 2;
			goto END_CYCLE_READFILE;
		}
		// Reading data.
		memset(dat, 0, size);
		rc = f_read(&fp, dat, (UINT)size, (UINT)&readLen);
		if ((rc != (FRESULT) FR_OK) || (readLen > size))
		{
			step = 3;
			goto END_CYCLE_READFILE;
		}
		else
		{
			f_close(&fp);
			FS_info.EC = (FS_info.EC > 0) ? (FS_info.EC - 1) : FS_info.EC;
			if (rLen != NULL)
			{
				*rLen = readLen;
			}
			FS_rstSem();
			return 0;
		}
		END_CYCLE_READFILE: //
		f_close(&fp);
		FS_rstSem();
		FS_dbgPrt("\r\n %s:\"%s\" O=%u L=%u Step=%u RC=%d Rtry=%d" //
				, __func__, fname, ofs, readLen, step, rc, rtry);
		FS_info.EC++;
		FS_remount();
	}
	return (step + 200);
}

/***************************************************************************************************
 * brief: 		Append data at the end of file
 * param
 *
 * return:		0	|	OK
 *				1	|	File/path not existed.
 *				>1	|	Other error.
 *
 * NOTE:
 * 	+ input size MUST NOT COUNT NULL U8acter and END of data.
 * 	+ if size is ZERO -> get size of data by strlen() function
 ****************************************************************************************************/
U8 FS_append(const U8* fname, U32 len, const void* dat)
{
	FIL fp;
	FRESULT rc;
	UINT s2 = 0;
	DWORD size_total = 0;
	U8 rtry = 0, step = 0;

	if (lockAccess)
	{
		return 0xFF;
	}
	len = (len == 0) ? strlen(dat) : len;
	for (rtry = 0; rtry < 3; rtry++)
	{
		FS_setSem();
		step = 1;
		rc = f_open(&fp, fname, FA_OPEN_EXISTING | FA_WRITE);
		f_sync(&fp);
		if ((rc != FR_OK) && (rc != FR_NO_FILE))
		{
			goto FS_APPEND_ERROR;
		}
		size_total = fp.obj.objsize;
		step = 2;
		rc = f_lseek(&fp, size_total);
		if ((rc != FR_OK) && (rc != FR_NO_FILE))
		{
			goto FS_APPEND_ERROR;
		}
		step = 3;
		rc = f_write(&fp, dat, len, &s2);
		FS_info.WC++;
		if ((rc != FR_OK) && (rc != FR_NO_FILE))
		{
			goto FS_APPEND_ERROR;
		}
		f_close(&fp);
		FS_info.EC = (FS_info.EC > 0) ? (FS_info.EC - 1) : FS_info.EC;
		FS_rstSem();
		return 0;
		FS_APPEND_ERROR://
		FS_rstSem();
		FS_dbgPrt("\r\n $s:\"%s\",%u STEP=%u RC=%u Rtry=%d", __func__, fname, len, step, rc, rtry);
		FS_info.EC++;
		FS_remount();
	}
	return (step + 200);
}
/***************************************************************************************************
 * Brief: 		MountingSDcard
 * Param[in]:	none
 * Param[out]:	none
 * Retval:		int
 *
 * @details:	mount and check SDcard by read and write sample data
 *
 * Return:		0	|	OK
 * 				>0	|	FAIL
 ****************************************************************************************************/
U8 FS_mount(void)
{
	U8 content[6] = "12345", fname[] = "temp/temp.txt", dname[] = "temp";
	U32 rlen;

	FS_dbgPrt("\r\n %s:REMOUNT", __func__);
	FS_remount();
	FS_dbgPrt("\r\n %s:CREATE DIR", __func__);
	if (FS_createDir(dname))
	{
		return 1;
	}
	FS_dbgPrt("\r\n %s:CREATE FILE", __func__);
	if (FS_createFile(fname, 1))
	{
		return 2;
	}
	FS_dbgPrt("\r\n %s:1ST READ", __func__);
	memset(content, 0, 6);
	FS_rd(fname, 0, 5, content, &rlen);
	FS_dbgPrt("\r\n %s:content=\"%s\"", __func__, content);
	if (strlen(content))
	{
		return 3;
	}
	FS_dbgPrt("\r\n %s:WRITE", __func__);
	if (FS_append(fname, 0, "12345"))
	{
		return 4;
	}
	FS_dbgPrt("\r\n %s:2ND READ", __func__);
	memset(content, 0, 6);
	FS_rd(fname, 0, 5, content, &rlen);
	FS_dbgPrt("\r\n %s:content=\"%s\"", __func__, content);
	if (strcmp(content, "12345"))
	{
		return 5;
	}
	return 0;
}
/*********************************************************************************
 * Brief:		FormatSDcard
 * Param[in]
 * param[out]
 * Retval:
 *
 * @detail		format SDcard with FAT file system
 *********************************************************************************/
void FS_format()
{
	FRESULT res;

	FS_setSem();
	res = f_mount(&Fatfs[0], "0:", 1);
	FS_dbgPrt("\r\n %s:mnt:res=%u", __func__, res);
	res = f_mkfs("0:", (BYTE) 0, 512);
	if (res != FR_OK)
	{
		FS_dbgPrt("\r\n %s:mkfs:res=%u", __func__, res);
	}
	FS_rstSem();
}
/*-----------------------------------------------------------------------
 * Brief: unmount file system
 *-----------------------------------------------------------------------*/
void FS_unmount()
{
	FRESULT rc = 0;

	FS_setSem();
	rc = f_mount(NULL, "0:", 1);
	FS_dbgPrt("\r\n %s:unmnt:res=%u", __func__, rc);
	FS_rstSem();
}
/*********************************************************************************
 * Brief:		ReMount
 * Param[in]
 * param[out]
 * Retval:
 *
 * @detail		simply re-mount SDcard
 *********************************************************************************/
void FS_remount()
{
	FRESULT rc = 0;

	FS_setSem();
	rc = f_mount(NULL, "0:", 1);
	FS_dbgPrt("\r\n %s:unmnt:res=%u", __func__, rc);
	rc = f_mount(&Fatfs[0], "0:", 1);
	FS_dbgPrt("\r\n %s:mnt:res=%u", __func__, rc);
	FS_rstSem();
}
/*********************************************************************************
 * Brief:		get File system capacity.
 * Ret:		capacity (in bytes).
 *********************************************************************************/
U32 FS_cap(void)
{
	U32 ret;

	//FS_setSem();
	/* Get total sectors and free sectors */
	ret = (Fatfs[0].n_fatent - 2) * Fatfs[0].csize / 2 * 1024;
	//FS_rstSem();
	return ret;
}
/*****************************************************************
 * Brief:			CheckDirExist
 * Param[in]		U8*
 * Param[out]		none
 * Retval			unsigned int
 *
 * @detail			1: yes
 * 					0: no
 *					2: unknown?
 *****************************************************************/
U8 FS_chkDirExist(const U8 *dirname)
{
	DIR dir;
	FRESULT rc = 0;
	int loop = 0;

	if (lockAccess)
	{
		return 0xFF;
	}
	for (loop = 0; loop < 2; loop++)
	{
		FS_setSem();
		rc = f_opendir(&dir, dirname);
		f_closedir(&dir);
		FS_rstSem();
		if (rc == (FRESULT) FR_OK)
		{
			FS_info.EC = (FS_info.EC > 0) ? (FS_info.EC - 1) : FS_info.EC;
			return 1;
		}
		if (rc == (FRESULT) FR_NO_PATH)
		{
			return 0;
		}
		FS_dbgPrt("\r\n %s:\"%s\" RC=%u Rtry=%d", __func__, dirname, rc, loop);
		FS_info.EC++;
		FS_remount();
	}
	return 2;
}
/*****************************************************************
 * Brief:			CheckFileExist
 *
 * Ret:	0	|	Existed
 * 		1	|	Not existed
 *		>1	|	Error
 *****************************************************************/
U8 FS_chkFileExist(const U8* filename)
{
	FIL fp;
	FRESULT rc = 0;
	int loop = 0;

	if (lockAccess)
	{
		return 0xFF;
	}
	for (loop = 0; loop < 2; loop++)
	{
		FS_setSem();
		rc = f_open(&fp, filename, FA_OPEN_EXISTING);
		f_close(&fp);
		FS_rstSem();
		if (rc == FR_OK)
		{
			FS_info.EC = (FS_info.EC > 0) ? (FS_info.EC - 1) : FS_info.EC;
			return 0;
		}
		if (rc == FR_NO_FILE)
		{
			return 1;
		}
		FS_dbgPrt("\r\n %s:\"%s\" rc=%u rtry=%d", __func__, filename, rc, loop);
		if (rc == FR_NO_PATH)
		{
			return 1;
		}
		FS_info.EC++;
		FS_remount();
	}
	return 3;
}
/***************************************************************************
 * Brief		get file size
 * Param		fname	|	IN	|	filename.
 * 				fsize	|	OUT	|	file size.
 * Retval		0	|	OK
 * 				1	|	File isn't existed.
 * 				>0	|	FAIL
 ***************************************************************************/
U8 FS_fsize(const U8 *fname, U32 *fsize)
{
	FRESULT rc;
	FILINFO fno;
	int loop = 0;

	if (lockAccess)
	{
		return 0xFF;
	}
	*fsize = 0;
	for (loop = 0; loop < 2; loop++)
	{
		FS_setSem();
		rc = f_stat(fname, &fno);
		FS_rstSem();
		if (rc == FR_OK)
		{
			FS_info.EC = (FS_info.EC > 0) ? (FS_info.EC - 1) : FS_info.EC;
			*fsize = fno.fsize;
		}
		else if ((rc == FR_NO_FILE) || (rc == FR_NO_PATH))
		{
			return 1;
		}
		FS_dbgPrt("\r\n %s:\"%s\" RC=%u rtry=%d", __func__, fname, rc, loop);
		FS_info.EC++;
		FS_remount();
	}
	return 2;
}
#if !DISABLE_UNRESOLVED_MALFUNCTIONS
/***************************************************************************
 * Brief		ReadLastLineFile
 * Param[in]	const U8*, int
 * Param[out]	U8*
 * Retval
 *
 * @details		read last line in file, no CR, LF left in line.
 * @NOTE:		input expectedSize already include NULL U8acter at end of string
 *
 * Return:		0	open failed
 * 				1	OK
 * 				2	error
 *
 * @NOTE: IF LINE LONGER THAN EXPECTED LENGTH (NOW IS 512) NOT ALL LINE BE GOT
 ***************************************************************************/
U32 FS_readFinalLine(const U8* filename, U16 expectedSize, U8* str)
{
	U32 LEN = 512;
	U32 REAL_LEN = LEN + 1;
	U32 pos_start = 0;
	U32 pos_stop = 0;
	U32 ofs = 0;
	int pos = 0;
	FIL *fp = NULL;
	FRESULT rc;
	U8 *str_tmp = NULL;

	fp = MM_get(MIN2SEC(2), sizeof(FIL), 0);
	if (fp != NULL)
	{
		FS_setSem();
		rc = f_open(fp, filename, FA_OPEN_EXISTING | FA_READ);
		if ((rc != (FRESULT) FR_OK) && (rc != (FRESULT) FR_NO_FILE) && (rc != (FRESULT) FR_NO_PATH)) //unsuccessfully open file
		{
			FS_dbgPrt("ReadLastLine file=[%s] err=[%u]", filename, rc);
			MM_free(fp);
			FS_info.EC++;
			FS_rstSem();
			FS_remount();
			return 0;
		}
		str_tmp = MM_get(MIN2SEC(2), sizeof(U8) * REAL_LEN, 500);
		if (str_tmp == NULL)
		{
			MM_free(fp);
			FS_rstSem();
			return 2;
		}
		ofs = (fp->obj.objsize > (unsigned long) LEN) ? (fp->obj.objsize - (unsigned long) LEN) : 0;
		//FS_dbgPrt("ReadLastLine  offset=%u, fsize=%u", ofs, File1.obj.objsize);
		f_lseek(fp, ofs);
		f_read(fp, str_tmp, LEN, &LEN);
		str_tmp[LEN] = 0;
		//FS_dbgPrt("ReadLastLine readData=[%s]", str_tmp);
		f_close(fp);
		MM_free(fp);
		FS_rstSem(); //below this line, no fatFS function be called
	}
	pos_stop = LEN;
	for (pos = LEN - 1; pos >= 0; pos--) //scan through string to find end of line
	{
		if ((str_tmp[pos] != '\r') && (str_tmp[pos] != '\n'))
		{
			pos_stop = (unsigned int) (pos + 1);
			break;
		}
	}
	for (pos = (pos_stop - 1); pos >= 0; pos--) //scan through string to find start of line
	{
		if ((str_tmp[pos] == '\r') || (str_tmp[pos] == '\n'))
		{
			break;
		}
		pos_start = (unsigned int) pos;
	}
	if (pos_stop >= (pos_start + expectedSize - 1))
	{
		pos_stop = pos_start + expectedSize - 1;
	}
	//FS_dbgPrt("ReadLastLine stop_pos=%u", pos_stop);
	//FS_dbgPrt("ReadLastLine start_pos=%u", pos_start);
	if (pos_stop >= pos_start)
	{
		LIB_strSub(pos_start, pos_stop - pos_start, str_tmp, str); //filter last line
	}
	//FS_dbgPrt("ReadLastLine parsedRes=[%s]", str);
	FS_info.EC = (FS_info.EC > 0) ? (FS_info.EC - 1) : FS_info.EC;
	MM_free(str_tmp);
	return 1;
}
#endif // DISABLE_UNRESOLVED_MALFUNCTIONS
#if !DISABLE_UNRESOLVED_MALFUNCTIONS
/*******************************************************************************
 * Brief     	forward_all_file_content
 * Param[in]	const U8*, void*, unsigned long, unsigned long
 * Param[out]	none
 * Retval
 *
 * @details
 * 			forward data (with expected size is "len") from offset "ofs"
 * 			if len = 0: read to end of file.
 * return:
 * 			1	OK, forward all file
 * 			0	open file failed
 * 			-1	error, middle block incompletely be read
 * 			-2 	memory error
 *******************************************************************************/
int FS_fwdFullFileContent(const U8* filename, U32 ofs, U32 len, void (*outTermBytes)(const U8* data, ...))
{
	const unsigned int SIZEREAD = 4096;
	unsigned int numBlock = 0;
	unsigned int pos_block = 0;
	unsigned int reading_length = 0;
	U8 *data = NULL;
	unsigned long f_size = 0;

	data = MM_get(MIN2SEC(2), sizeof(U8) * (SIZEREAD + 1), 500);
	if (data == NULL)
	{
		return -2;
	}
	if (FS_chkFileExist(filename) == 0)
	{
		outTermBytes("<FILE NOT FOUND>");
		MM_free(data);
		return 0;
	}
	f_size = FS_fsize(filename);
	len = (len == 0) ? (f_size - ofs) : len;
	len = ((f_size - ofs) < len) ? (f_size - ofs) : len;
	numBlock = len / SIZEREAD + 1;
	for (pos_block = 0; pos_block < numBlock; pos_block++) //read as block
	{
		if (FS_rd(filename, ofs,  (len > SIZEREAD) ? SIZEREAD : len, data, &reading_length))
		{
			MM_free(data);
			return -1;
		}
		else if (reading_length == 0)
			break;
		else
		{
			outTermBytes(data);
			ofs += reading_length;
			len -= reading_length;
		}
	}
	MM_free(data);
	return 1;
}
#endif // !DISABLE_UNRESOLVED_MALFUNCTIONS
/***** these functions will not close file on finish ********************************************
 ************************************************************************************************/

/**********************************************************************************
 * Brief		DeleteFile
 * Param[in]	U8*
 * Param[out]	none
 * @reval		none
 *
 *
 *********************************************************************************/
void FS_renameFile(const U8 *oldname, const U8 *newname)
{
	FRESULT res;

	if (lockAccess)
	{
		return;
	}
	FS_setSem();
	res = f_rename(oldname, newname);
	if (res != FR_OK)
	{
		FS_dbgPrt("\r\n %s:\"%s\"->\"%s\":Res=%u", __func__, res);
	}
	FS_rstSem();
}
/**********************************************************************************
 * Brief		DeleteFile
 * Param[in]	U8*
 * Param[out]	none
 * @reval		none
 *
 *
 *********************************************************************************/
void FS_delFile(const U8* Filename)
{
	FRESULT res;

	if (lockAccess)
	{
		return;
	}
	FS_setSem();
	res = f_unlink(Filename);
	if ((res != FR_OK) && (res != FR_NO_FILE) && (res != FR_NO_PATH))
	{
		FS_dbgPrt("\r\n %s:\"%s\" rc=%u", __func__, Filename, res);
	}
	FS_rstSem();
}
/**********************************************************************************
 * Brief		DeleteDir
 * Param[in]	U8*
 * Param[out]	none
 * @reval		none
 *
 * @details:	delete a directory with all it's files.
 * Note:		ONLY WORK WITH DIR CONTAINS NO DIR.
 *********************************************************************************/
void FS_delDir(const U8* Dirname)
{
	FRESULT rc;
	FILINFO fno;
	DIR dir;
	U8 *fn; /* This function is assuming non-Unicode cfg. */
	U8 FilePath[64];
#if _USE_LFN
	//static U8 lfn[_MAX_LFN + 1];
	//fno.fname = lfn;
	//fno.lfsize = sizeof lfn;
#endif

	if (lockAccess)
	{
		return;
	}
	FS_setSem();
	rc = f_opendir(&dir, Dirname); /* Open the directory */
	if (rc == (FRESULT) FR_OK)
	{
		FS_info.EC = (FS_info.EC > 0) ? (FS_info.EC - 1) : FS_info.EC;
		//loop to scan all file in directory, and delete each file
		for (;;)
		{
			rc = f_readdir(&dir, &fno); /* Read a directory item */
			if ((rc != (FRESULT) FR_OK) || (fno.fname[0] == 0))
			{
				break; /* Break on error or end of dir */
			}
			if (fno.fname[0] == '.')
			{
				continue; /* Ignore dot entry */
			}
			fn = fno.fname;
			snprintf(FilePath, 64, "%s/%s", Dirname, fn);
			FilePath[63] = 0;
			f_unlink(FilePath);
		}
		//after delete all file under dir, delete that dir too
		f_unlink(Dirname);
		f_closedir(&dir);
	}
	else if (rc == FR_NO_PATH)
	{
		//FS_dbgPrt("Delete dir=[%s] err=[NOT FOUND]", Dirname);
	}
	else
	{
		FS_dbgPrt("\r\n %s:\"%s\" R=%d", __func__, Dirname, rc);
		FS_remount();
	}
	FS_rstSem();
}
#if 1//!DISABLE_UNRESOLVED_MALFUNCTIONS
/**********************************************************************************
 * Brief		search for file in directory
 * Param		dname		|	I	|	directory name
 * 				get1st		|	I	|	get first found file (1), OR last found one (0). View ATTENTION please
 * 				s2filter	|	I	|	string to filter result (filename must contain this string).
 * 				fnameSize	|	I	|	file name max size.
 * 				fname		|	O	|	file name
 * 				matchedCt	|	O	|	result matched counter (matched condition)
 * @reval		none
 * ATTENTION: If get1st is (1), matchedCt will always 1 (if found) or 0 (not found).
 *********************************************************************************/
void FS_searchFileInDir(const U8 *dname, U8 get1st, const U8 *s2filter, U16 fnameSize, U8 *fname, U16* matchedCt)
{
	FRESULT rc;
	FILINFO fno;
	DIR dir;
	U8 rtry = 0;
	U16 tmp_matchedCnt = 0;
	U8 *fn; /* This function is assuming non-Unicode cfg. */
	//U8 FilePath[64];
#if _USE_LFN
	//static U8 lfn[_MAX_LFN + 1];
	//fno.fname = lfn;
	//fno.lfsize = sizeof lfn;
#endif

	if (lockAccess)
	{
		return;
	}
	memset(fname, 0, fnameSize);
	for (rtry = 0; rtry < 3; rtry++)
	{
		FS_setSem();
		rc = f_opendir(&dir, dname); /* Open the directory */
		FS_rstSem();
		if (rc == (FRESULT) FR_OK)
		{
			FS_info.EC = (FS_info.EC > 0) ? (FS_info.EC - 1) : FS_info.EC;
			tmp_matchedCnt = 0;
			for (;;)
			{
				FS_setSem();
				rc = f_readdir(&dir, &fno); /* Read a directory item */
				FS_rstSem();
				if (rc != (FRESULT) FR_OK)
				{
					goto FS_SEARCHFILEINDIR_ERR;
				}
				if (fno.fname[0] == 0)
				{
					f_closedir(&dir);
					goto FS_SEARCHFILEINDIR_END; /* Break on end of dir */
				}
				if (fno.fname[0] == '.')
				{
					continue; /* Ignore dot entry */
				}
				fn = fno.fname;
				if (SL_search(fn, s2filter) != -1)
				{
					if (get1st == 1)
					{
						if (tmp_matchedCnt == 0)
						{
							//only record 1st result.
							snprintf(fname, fnameSize, "%s/%s", dname, fn);
							fname[fnameSize - 1] = 0;
							tmp_matchedCnt = 1;
							f_closedir(&dir);
							goto FS_SEARCHFILEINDIR_END;
						}
					}
					else
					{
						snprintf(fname, fnameSize, "%s/%s", dname, fn);
						fname[fnameSize - 1] = 0;
					}
				}
			}
			f_closedir(&dir);
		}
		else if (rc == (FRESULT) FR_NO_PATH)
		{
			FS_dbgPrt("\r\n %s:\"%s\" E=NOT_FOUND", __func__, dname);
			return;
		}
		else
		{
			FS_SEARCHFILEINDIR_ERR: //
			FS_dbgPrt("\r\n %s:\"%s\" R=%d", __func__, dname, rc);
			FS_remount();
		}
	}
	FS_SEARCHFILEINDIR_END: //
	tmp_matchedCnt++;
	FS_dbgPrt("\r\n %s:\"%s\" MatchedCt=%u", __func__, fn, tmp_matchedCnt);
	if (matchedCt != NULL)
	{
		*matchedCt = tmp_matchedCnt;
	}
}
#endif // #if !DISABLE_UNRESOLVED_MALFUNCTIONS
#if 1// !DISABLE_UNRESOLVED_MALFUNCTIONS
/**********************************************************************************
 * Brief		read and show list of files/directories in a directory.
 * Param		Dirname	|	IN	|
 * 				outTerm	|	IN	|
 * Ret			0	|	No problem
 * 				1	|	File system is inaccessible.
 * 				2	|	Reading error.
 * 				3	|	File broken is detected.
 *********************************************************************************/
U8 FS_rdDir(const U8* dname, void (*outTerm)(const U8* data, ...))
{
	FRESULT rc;
	FILINFO fno;
	DIR dir;
	U8 *fn; /* This function is assuming non-Unicode cfg. */
	U8 FilePath[64];
#if _USE_LFN
	//static U8 lfn[_MAX_LFN + 1];
	//fno.fname = lfn;
	//fno.lfsize = sizeof lfn;
#endif

	if (lockAccess)
	{
		return;
	}
	//FS_dbgPrt("Read dir=[%s]", Dirname);
	FS_setSem();
	rc = f_opendir(&dir, dname); /* Open the directory */
	FS_rstSem();
	if (rc == (FRESULT) FR_OK)
	{
		FS_info.EC = (FS_info.EC > 0) ? (FS_info.EC - 1) : FS_info.EC;
		for (;;)
		{
			FS_setSem();
			rc = f_readdir(&dir, &fno); /* Read a directory item */
			FS_rstSem();
			if ((rc != (FRESULT) FR_OK) || (fno.fname[0] == 0))
			{
				break; /* Break on error or end of dir */
			}
			if (fno.fname[0] == '.')
			{
				continue; /* Ignore dot entry */
			}
			fn = fno.fname;
			snprintf(FilePath, 64, "%s/%s", dname, fn);
			FilePath[63] = 0;
			if (fno.fattrib & AM_DIR)
			{
				if (outTerm != NULL)
				{
					//This is directory.
					outTerm("\r\n+ %s/", FilePath);
				}
			}
			else
			{
				if (outTerm != NULL)
				{
					outTerm("\r\n+ %s - %d(Bytes)", FilePath, fno.fsize);
				}
			}
			{
				U8 extFilter[] = {' ', '/', '_', '-', '.'};
				if (!SL_verify(FilePath, strlen(FilePath), BIT(0) | BIT(1), extFilter, 5))
				{
					FS_dbgPrt("\r\n %s:\"%s\" -> Invalid path", __func__, FilePath);
					//return 3;
				}
			}
		}
		f_closedir(&dir);
	}
	else if (rc == (FRESULT) FR_NO_PATH)
	{
		FS_dbgPrt("\r\n %s:\"%s\" E=NOT_FOUND", __func__, dname);
		return 0;
	}
	FS_dbgPrt("\r\n %s:\"%s\" R=%d", __func__, dname, rc);
	FS_remount();
	return 2;
}
#endif // #if !DISABLE_UNRESOLVED_MALFUNCTIONS
/**********************************************************************************
 * brief	Create a directory.
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 *********************************************************************************/
U8 FS_createDir(const U8* dirname)
{
	U8 loop = 0;
	FRESULT rc;

	if (lockAccess)
	{
		return 0xFF;
	}
	for (loop = 0; loop < 2; loop++)
	{
		if (FS_chkDirExist(dirname) == 1)
		{
			return 0;
		}
		FS_setSem();
		rc = f_mkdir(dirname);
		FS_rstSem();
		if ((rc == FR_OK) || (rc == FR_EXIST))
		{
			FS_info.EC = (FS_info.EC > 0) ? (FS_info.EC - 1) : FS_info.EC;
			return 0;
		}
		FS_dbgPrt("\r\n %s:\"%s\" R=%u rtry=%d", __func__, dirname, rc, loop);
		FS_info.EC++;
		FS_remount();
	}
	return 0;
}
#if !DISABLE_UNRESOLVED_MALFUNCTIONS
/************************************************************************************************
 * @Brief:	streaming data.
 * Param:	fname	|	IN	|	file name.
 * 			ofs		|	IN	|	begin offset.
 * 			bts		|	IN	|	number of bytes to stream.
 * 			bs		|	OUT	|	number of bytes be streamed.
 * 			dat		|	I/O	|	streaming buffer.
 * 			size	|	IN	|	streaming buffer size.
 * 			func	|	IN	|	function to process streaming buffer.
 * 			errCode	|	IN	|	function to process error code.
 * Ret:		0	|	OK
 * 			>0	|	Error
 ************************************************************************************************/
U8 FS_stream(const U8 *fname, U32 ofs, U32 bts, U32 *bs, U8 *dat, U16 size, U8 (func)(const U8*, U32), void (errCode)(U8))
{
	U8 step = 0, rtry = 0, pct = 0, pre_pct = 0;
	FRESULT res;
	FIL *fp = NULL;
	U32 br = 0, btr = 0;

	FS_setSem();
	fp = MM_get(MIN2SEC(10), sizeof(FIL), 10);
	if (fp == NULL)
	{
		FS_rstSem();
		return 0xFF;
	}
	*bs = 0;
	FS_READ_BEGIN: //
	for (step = 0; step <= 3; step++)
	{
		switch(step)
		{
			case 0:
				res = f_open(fp, fname, FA_READ | FA_OPEN_EXISTING);
				break;
			case 1:
				res = f_lseek(fp, ofs);
				break;
			case 2:
				if (bts == 0)
				{
					bts = fp->obj.objsize;
				}
				while (1)
				{
					btr = bts > (*bs + size) ? size : (bts - *bs);
					res = f_read(fp, dat, btr, &br);
					if (res != FR_OK)
					{
						break;
					}
					ofs += br;
					res = func(dat, br);
					if (res != 0)
					{
						break;
					}
					*bs += br;
					pct = *bs * 100 / bts;
					if (pct > pre_pct)
					{
						errCode(pct);
						pre_pct = pct;
					}
					if (br < btr)
					{
						res = FR_OK;
						break;
					}
					if (*bs >= bts)
					{
						res = FR_OK;
						break;
					}
				}
				break;
			default:
				f_close(fp);
				MM_free(fp);
				FS_rstSem();
				return 0;
		}
		if (res != FR_OK)
		{
			FS_dbgPrt("\r\n %s:\"%s\"-%u-%u step=%u R=%u rtry=%u", __func__, fname, ofs, bts, step, res, rtry);
			f_close(fp);
			if ((rtry++ >= 2) || (res == FR_EXIST) || (res == FR_NO_FILE) || (res == FR_NO_PATH))
			{
				MM_free(fp);
				FS_rstSem();
				return (step + 1);
			}
			f_mount(NULL, "0:", 1);
			f_mount(&Fatfs[0], "0:", 1);
			goto FS_READ_BEGIN;
		}
	}
	MM_free(fp);
	FS_rstSem();
	return 0xFF;
}
#endif // #if !DISABLE_UNRESOLVED_MALFUNCTIONS
/*----------------------------------------------------------------------------------------------------
 *
 ----------------------------------------------------------------------------------------------------*/
U8 FS_demo(void)
{
	U8 dat[24];
	U8 i = 0;
	U32 rlen = 0;

	memset(dat, 0, 24);
	for(i = 0; i < 24; i++)
	{
		dat[i] = 'A' + i;
	}
	if (FS_createFile("demo.txt", 1))
	{
		return 1;
	}
	DBG_print("\r\n FS_demo:[w=%s]", dat);
	if (FS_wrt("demo.txt", 0, 24, dat))
	{
		return 2;
	}
	memset(dat, 0, 24);
	if (FS_rd("demo.txt", 0, 24, dat, &rlen))
	{
		return 3;
	}
	DBG_print("\r\n FS_demo:[r=%s]", dat);
	for(i = 0; i < 24; i++)
	{
		if (dat[i] != ('A' + i))
		{
			return 4;
		}
	}
	memset(dat, 0, 24);
	for(i = 0; i < 10; i++)
	{
		dat[i] = '0' + i;
	}
	DBG_print("\r\n FS_demo:[w=%s]", dat);
	if (FS_wrt("demo.txt", 24 - 10, 10, dat))
	{
		return 5;
	}
	memset(dat, 0, 24);
	if (FS_rd("demo.txt", 0, 24 - 10, dat, &rlen))
	{
		return 6;
	}
	DBG_print("\r\n FS_demo:[r=%s]", dat);
	for(i = 0; i < (24 - 10); i++)
	{
		if (dat[i] != ('A' + i))
		{
			return 7;
		}
	}
	memset(dat, 0, 24);
	rlen = 10;
	if (FS_rd("demo.txt", 24 - 10, 10, dat, &rlen))
	{
		return 8;
	}
	DBG_print("\r\n FS_demo:[r=%s]", dat);
	for(i = 0; i < 10; i++)
	{
		if (dat[i] != ('0' + i))
		{
			return 9;
		}
	}
	return 0;
}
