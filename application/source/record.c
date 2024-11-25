/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      fatfs.h
 *  Author:         Tomas Dolak
 *  Date:           18.11.2024
 *  Description:    --
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           fatfs.h
 *  @author         Tomas Dolak
 *  @date           18.11.2024
 *  @brief          --
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <record.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* buffer size (in byte) for read/write operations */
#define BUFFER_SIZE (513U)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static FATFS g_fileSystem; /* File system object */

static FIL g_fileObject;   /* File object */

/* @brief decription about the read/write buffer
 * The size of the read/write buffer should be a multiple of 512, since SDHC/SDXC card uses 512-byte fixed
 * block length and this driver example is enabled with a SDHC/SDXC card.If you are using a SDSC card, you
 * can define the block length by yourself if the card supports partial access.
 * The address of the read/write buffer should align to the specific DMA data buffer address align value if
 * DMA transfer is used, otherwise the buffer address is not important.
 * At the same time buffer address/size should be aligned to the cache line size if cache is supported.
 */
/*! @brief Data written to the card */
SDK_ALIGN(uint8_t g_bufferWrite[BUFFER_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);
/*! @brief Data read from the card */
SDK_ALIGN(uint8_t g_bufferRead[BUFFER_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);
/*******************************************************************************
 * Code
 ******************************************************************************/
FIL* RECORD_CreateFile(RTC_date_t date, RTC_time_t time)
{
	FIL  *createdFile = NULL;
	FRESULT err;

	err = f_open(&createdFile, _T("test.log"), (FA_WRITE | FA_READ | FA_CREATE_ALWAYS));
	if (err)
	{
		if (FR_EXIST == err)
		{
			PRINTF("ERR: File Exists\r\n");
		}
		else
		{
			PRINTF("ERR: File Creation Failed\r\n");
		}
		return NULL;
	}

	return createdFile;
}


uint8_t RECORD_Init(void)
{
	FRESULT error;

	/* Logic Disk */
	const TCHAR driverNumberBuffer[3U] = {SDDISK + '0', ':', '/'};
	BYTE work[FF_MAX_SS];

	/* Mount File System */
	if (f_mount(&g_fileSystem, driverNumberBuffer, 0U))
	{
		PRINTF("ERR: Mount Volume Failed.\r\n");
		return -1;
	}

#if (FF_FS_RPATH >= 2U)

	/* Setup of Actual Work Logic Disk */
    error = f_chdrive((char const *)&driverNumberBuffer[0U]);
    if (error)
    {
        PRINTF("ERR: Change drive failed.\r\n");
        return -1;
    }
#endif	/* (FF_FS_RPATH >= 2U) */

#if FF_USE_MKFS

    /* Make File System */
    if (f_mkfs(driverNumberBuffer, 0, work, sizeof work))
    {
        PRINTF("ERR: Init File System Failed.\r\n");
        return -1;
    }
#endif 	/* FF_USE_MKFS */

    return 0;
}

uint8_t RECORD_Start(void)
{
	FRESULT error;
	UINT bytesWritten					= 0;			/*<! Bytes Written Into File 	*/
	volatile bool failedFlag           	= false;		/*<! Write Failed 				*/
	UINT bytesRead;

#if (DEBUG_ENABLED == true)

	PRINTF("DEBUG: Initialize File System\r\n");

#endif /* (DEBUG_ENABLED == true) */

	error = (FRESULT)RECORD_Init();
	if (0 != error)
	{
		return (uint8_t)error;
	}

#if (DEBUG_ENABLED == true)

	PRINTF("DEBUG: Create Directory\r\n");

#endif /* (DEBUG_ENABLED == true) */

	error = f_mkdir(_T("/test_1"));
	if (error)
	{
        if (error == FR_EXIST)
        {
            PRINTF("ERR: Directory Exists.\r\n");
        }
        else
        {
            PRINTF("ERR: Make DIR Failed.\r\n");
            return -1;
        }
	}

#if (DEBUG_ENABLED == true)

	PRINTF("DEBUG: Create a File in DIR\r\n");

#endif /* (DEBUG_ENABLED == true) */

	error = f_open(&g_fileObject, _T("/test_1/log_1.log"), (FA_WRITE | FA_READ | FA_CREATE_ALWAYS));
    if (error)
    {
        if (FR_EXIST == error)
        {
            PRINTF("ERR: File Exists.\r\n");
        }
        else
        {
            PRINTF("ERR: Open File Failed.\r\n");
            return -1;
        }
    }

    /* Prepare Buffer That Will Be Written Into SD Card */
    strncpy((char *)g_bufferWrite, "That is The Test.", (size_t)sizeof(g_bufferWrite));
    g_bufferWrite[BUFFER_SIZE - 2U] = '\r';
    g_bufferWrite[BUFFER_SIZE - 1U] = '\n';

#if (DEBUG_ENABLED == true)

    PRINTF("\r\nWrite to above created file.\r\n");

#endif /* (DEBUG_ENABLED == true) */

    error = f_write(&g_fileObject, g_bufferWrite, sizeof(g_bufferWrite), &bytesWritten);
	if ((error) || (bytesWritten != sizeof(g_bufferWrite)))
	{
		PRINTF("ERR: Write Into File Failed. \r\n");
		failedFlag = true;
	}

    /* Move the file pointer */
    if (f_lseek(&g_fileObject, 0U))
    {
        PRINTF("ERR: Set File Pointer Position Failed. \r\n");
        failedFlag = true;
    }

    if (f_close(&g_fileObject))
    {
        PRINTF("\r\nClose file failed.\r\n");
        return -1;
    }

    return failedFlag ? -1 : 0;
}
