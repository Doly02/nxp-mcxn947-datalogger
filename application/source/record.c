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

static REC_config_t g_config;

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

REC_config_t RECORD_GetConfig(void)
{
	return g_config;
}

REC_version_t RECORD_GetVersion(void)
{
	return g_config.version;
}

FRESULT RECORD_CheckFileSystem(void)
{
	FILINFO fno;
	FRESULT res = f_stat("/", &fno);
	return res;
}

uint8_t RECORD_Init(void)
{
	FRESULT error;
	g_config.version 	= WCT_UNKOWN;
	g_config.baudrate 	= 0;

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

    if (FR_OK != RECORD_CheckFileSystem())
    {
        PRINTF("INFO: File system not found. Formatting...\r\n");
        /* Make File System */
        if (f_mkfs(driverNumberBuffer, 0, work, sizeof work))
        {
            PRINTF("ERR: Init File System Failed.\r\n");
            return -1;
        }
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

	error = f_open(&g_fileObject, _T("/test_1/log_2.log"), (FA_WRITE | FA_READ | FA_CREATE_ALWAYS));
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

uint8_t RECORD_Deinit(void)
{
	FRESULT error;

	/* Close All Opened Files */
	error = f_close(&g_fileObject);
	if (error != FR_OK)
	{
		PRINTF("ERR: Failed to Close File. Error=%d\r\n", error);
		if (error != FR_INVALID_OBJECT)
		{
			return -1;
		}
	}

	/* Unmount File System From Logic Drive */
	error = f_mount(NULL, "", 0);
	if (error != FR_OK)
	{
		PRINTF("ERR: Failed to unmount filesystem. Error=%d\r\n", error);
		return -1;
	}

#if (true == DEBUG_ENABLED)

	PRINTF("DEBUG: Filesystem Deinicialized.\r\n");

#endif /* (true == DEBUG_ENABLED) */

	return 0;
}


uint8_t RECORD_ReadConfig(void)
{
    FRESULT error;
    DIR dir;           					//<! Opened Directory
    FILINFO fno;       					//<! Information About File
    FIL configFile;    					//<! Opened File
    UINT bytesRead;    					//<! Number of Read Bytes

    error = f_opendir(&dir, "/");
    if (FR_OK != error)
    {
        PRINTF("ERR: Failed to Open Root Dir. ERR=%d\r\n", error);
        return -1;
    }

    while (1)
	{
    	error = f_readdir(&dir, &fno);
    	if (error != FR_OK || fno.fname[0] == 0)
		{
			break; 	// End of Directory or Error
		}

    	PRINTF("DEBUG: File Name: %s\r\n", fno.fname);

    	if (!(fno.fattrib & AM_DIR))	// If Not Directory
    	{
    		if (0 == strcmp(fno.fname, CONFIG_FILE))	// If .config File
			{
    			PRINTF("DEBUG: Found .config File: %s\r\n", fno.fname);

    			error = f_open(&configFile, CONFIG_FILE, FA_READ);
    			if (FR_OK != error)
				{
    				PRINTF("ERR: Failed to open .config file. ERR=%d\r\n", error);
					f_closedir(&dir); 	// Close Root Directory
					return -1;
				}
    			memset(g_bufferRead, 0, sizeof(g_bufferRead));
    			error = f_read(&configFile, g_bufferRead, sizeof(g_bufferRead) - 1, &bytesRead);
				if (FR_OK != error)
				{
					PRINTF("ERR: Failed To Read .config File. ERR=%d\r\n", error);
					f_close(&configFile);
					f_closedir(&dir);
					return -1;
				}

				if (SUCCESS != RECORD_ProccessConfigFile((const char *)g_bufferRead))
				{
					f_close(&configFile);
					f_closedir(&dir);
					return E_FAULT;
				}

				// Processed Configuration File
				f_close(&configFile);
				f_closedir(&dir);
				return SUCCESS;
			}
    	}
	}
    PRINTF("ERR: .config file not found in root directory.\r\n");
	f_closedir(&dir);
	return E_FAULT;

}

uint8_t RECORD_ProccessConfigFile(const char *content)
{
    const char *key = "baudrate=";
    char *found;
    int32_t baudrate;

    // Find Key "baudrate="
    found = strstr(content, key);
    if (NULL == found)
    {
        PRINTF("ERR: Key 'baudrate=' Not Found.\r\n");
        return E_FAULT;
    }

    // Move Pointer Behind "baudrate="
    found += strlen(key);
    // Convert Value To INT
    baudrate = atoi(found);
    if (0 >= baudrate)
	{
		PRINTF("ERR: Invalid Baudrate Value.\r\n");
		return E_FAULT;
	}

    switch (baudrate)
	{
		case 320400U:
			g_config.version = WCT_AUTOS2;
			break;
		case 115200U:
			g_config.version = WCT_AUTOS1;
			break;
		default:
			PRINTF("ERR: Unsupported Baudrate Value: %d\r\n", baudrate);
			return E_FAULT;
	}

    g_config.baudrate = baudrate;
    return SUCCESS;
}
