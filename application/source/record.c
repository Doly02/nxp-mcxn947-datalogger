/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      fatfs.h
 *  Author:         Tomas Dolak
 *  Date:           18.11.2024
 *  Description:    File Includes Operation For Recoding Mode.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           fatfs.h
 *  @author         Tomas Dolak
 *  @date           18.11.2024
 *  @brief          File Includes Operation For Recoding Mode.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <record.h>
#include "fsl_irtc.h"

#include <limits.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * @brief 	Software FIFO Size.
 */
#define FIFO_SIZE 					1024U

/**
 * @brief 	Block Size For Write To SDHC Card Operation (In Bytes).
 */
#define BLOCK_SIZE 					512U

/**
 * @brief Convert Time In Seconds To Number of Ticks.
 *
 * @param 	seconds Number of Seconds To Transfer.
 * @return 	Number of Ticks Corresponding To The Specified Number of Seconds.
 */
#define GET_WAIT_INTERVAL(seconds)  ((seconds) * 1000 / configTICK_RATE_HZ)

/*
 * @brief 	Time Interval Between LED Blinking.
 * @details	In Milliseconds.
 */
#define RECORD_LED_TIME_INTERVAL 	(uint32_t)(10U)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
/*
 * @brief 	File System Object,
 */
static FATFS g_fileSystem;

/*
 * @brief 	File Object.
 * @details Pointer to Current Opened File.
 */
static FIL g_fileObject;

/*
 * @brief	Name Of The Folder Where The Files (Logs)
 * 			From The Current Session Are Stored.
 */
static char g_currentDirectory[32];

/**
 * @brief 	Configuration of The Data Logger on The Basis of Data
 * 			Obtained From The Configuration File.
 */
static REC_config_t g_config;

/*
 *  @brief decription about the read/write buffer
 * 	The size of the read/write buffer should be a multiple of 512, since SDHC/SDXC card uses 512-byte fixed
 * 	block length and this driver example is enabled with a SDHC/SDXC card.If you are using a SDSC card, you
 * 	can define the block length by yourself if the card supports partial access.
 * 	The address of the read/write buffer should align to the specific DMA data buffer address align value if
 * 	DMA transfer is used, otherwise the buffer address is not important.
 * 	At the same time buffer address/size should be aligned to the cache line size if cache is supported.
 **************/

/**
 * @defgroup 	Recording Buffers and Recording Management
 * @brief 		Group Contains Variables For Management of Recording.
 * @details 	Includes DMA Buffers For Recording, Indexes, Pointers, ...
 * @{
 */

/**
 * @brief 	Data For Multi-Buffering - In Particular Dual-Buffering,
 * 			One Is Always Filled, The Other Is Processed.
 */
SDK_ALIGN(static uint8_t g_dmaBuffer1[BLOCK_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);

SDK_ALIGN(static uint8_t g_dmaBuffer2[BLOCK_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);

/**
 * @brief 	Active Buffer Which Receives Data From UART Periphery.
 */
static uint8_t* g_activeDmaBuffer 	= g_dmaBuffer1;

/**
 * @brief 	Pointer on DMA Buffer Into Which The Time Stamps Are Inserted.
 */
static uint8_t* g_processDmaBuffer 	= NULL;

/**
 * @brief 	Index Into Active DMA Buffer.
 */
static uint16_t g_dmaIndex 			= 0;

/**
 * @brief 	Indicates That Collection Buffer (Active Buffer) Is Full and Ready To Swap.
 */
static bool g_dmaBufferReady 		= false;

/**
 * @brief 	Value of Ticks When Last Character Was Received Thru LPUART.
 */
static TickType_t g_lastDataTick 	= 0;

/**
 * @brief	Tracks Current File Size.
 */
static uint32_t g_currentFileSize 	= 0;

/**
 * @brief	Counter For Unique File Names.
 */
static uint16_t g_fileCounter 		= 1;

/**
 * @brief 	Flush Completed Flag.
 * @details If No Data of The LPUART Periphery Are Received Within The `FLUSH_TIMEOUT_TICKS`
 * 			Interval, The Data Collected So Far In The Buffer Are Flushed To The File.
 */
static bool g_flushCompleted 		= false;

/**
 * @brief	Transferred Bytes Between Blinking LEDs.
 */
static uint32_t g_bytesTransfered	= 0U;

/** @} */ // End of Recording Buffers and Recording Management

/**
 * @defgroup 	UART Management
 * @brief 		Group Contains Variables For Recording From UART.
 * @details 	Data From UART Are Stored Into FIFO (Circular Buffer).
 * @{
 */

SDK_ALIGN(static volatile uint8_t g_fifo[FIFO_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);	// FIFO buffer

/**
 * @brief	Index For Writing Into FIFO.
 */
static volatile uint32_t g_writeIndex 		= 0;

/**
 * @brief 	Index For Reading From FIFO.
 */
static volatile uint32_t g_readIndex 		= 0;

/** @} */ // End of UART Management Group


/*******************************************************************************
 * Interrupt Service Routines (ISRs)
 ******************************************************************************/
/**
 * @brief LPUART3 IRQ Handler.
 *
 */

/*lint -e957 */
/* MISRA 2012 Rule 8.4:
 * Suppress: function 'LP_FLEXCOMM3_IRQHandler' defined without a prototype in scope.
 * LP_FLEXCOMM3_IRQHandler is declared WEAK in startup_mcxn947_cm33_core0.c and overridden here.
 */
void LP_FLEXCOMM3_IRQHandler(void)
{
    uint8_t data;
    uint32_t stat;

    /* Check For New Data */
    stat = LPUART_GetStatusFlags(LPUART3);
    if (0U != ((uint32_t)kLPUART_RxDataRegFullFlag & stat))	// TODO:
    {
        data = LPUART_ReadByte(LPUART3);
        /* Add Data To FIFO */
        uint32_t nextWriteIndex = ((g_writeIndex + 1UL) % FIFO_SIZE);
        if (nextWriteIndex != g_readIndex) // Check if FIFO is not full
        {
        	g_fifo[g_writeIndex] = data;
            g_writeIndex = nextWriteIndex;

            /* Update Time Of Last Receiving */
            g_lastDataTick = xTaskGetTickCount();
            g_flushCompleted = false;
        }
        g_bytesTransfered++;
    }

    /* Clear Interrupt Flag */
    (void)LPUART_ClearStatusFlags(LPUART3, (uint32_t)kLPUART_RxDataRegFullFlag);
    SDK_ISR_EXIT_BARRIER;
}
/*lint -e957 */

/*******************************************************************************
 * Code
 ******************************************************************************/
DWORD get_fattime(void)
{
    irtc_datetime_t datetime = { 0U };

    /*lint -e9033*/
    /* MISRA 2012 Rule 10.8:
     * The cast from smaller unsigned integer types (uint8_t, uint16_t) to a wider type (uint32_t)
     * is intentional and safe in this context. The values are combined using bitwise OR into a
     * FAT timestamp format that expects 32-bit result. All input ranges are within limits.
     */
    IRTC_GetDatetime(RTC, &datetime);

    return ((DWORD)(datetime.year - 1980U) << 25) |
           ((DWORD)datetime.month << 21) |
           ((DWORD)datetime.day << 16) |
           ((DWORD)datetime.hour << 11) |
           ((DWORD)datetime.minute << 5) |
           ((DWORD)(datetime.second / (uint8_t)2));
    /*lint +e9033*/
}

int CONSOLELOG_Abs(int x)
{
    /*lint -e9027*/
    /* MISRA 2012 Rule 10.1:
     * Comparison with INT_MIN is intentional to detect the edge case
     * where -INT_MIN is undefined behavior. This check prevents incorrect abs().
     */
    if (x == INT_MIN) {
        return INT_MAX;
    }
    return (x < 0) ? -x : x;
    /*lint +e9027*/
}


error_t CONSOLELOG_CreateFile(void)
{
    FRESULT status;
    char fileName[64];					//<! TODO: Vypocitat Maximalni pocet souboru za den na zaklade baud rate
    irtc_datetime_t datetimeGet;		//<! To Store Time in File Meta-Data
    FILINFO fno;						//<! File Meta-Data


    IRTC_GetDatetime(RTC, &datetimeGet);

    /* Generate a New File Name */
    /* @note snprintf() Is Depricated But There Is No Better Equivalent */
    //lint -save -e586
    (void)snprintf(fileName, sizeof(fileName), "%s/%04d%02d%02d_%02d%02d%02d_%u.txt",
    		g_currentDirectory, datetimeGet.year, datetimeGet.month, datetimeGet.day,
             datetimeGet.hour, datetimeGet.minute, datetimeGet.second,
             g_fileCounter++);
    //lint -restore

    /* Open New File */
    /**
     * MISRA Deviation: Rule 10.1
     * Justification: FA_WRITE and FA_CREATE_ALWAYS are standard bitmask flags defined by the FatFs library.
     * These constants are specifically designed to be combined using bitwise OR (|), and the use is safe and intentional.
     */
    /*lint -e9027 */
    status = f_open(&g_fileObject, fileName, (FA_WRITE | FA_CREATE_ALWAYS));
    if (FR_OK != status)
    {
        PRINTF("ERR: Failed to create file %s. Error=%d\r\n", fileName, (uint32_t)status);
        return ERROR_OPEN;
    }
    /*lint +e9027 */

    /* Setup of File Meta-Data */
    /* MISRA Deviation Note:
     * The following expression is intentionally written in a compact and readable form
     * for setting FAT file timestamps. All shifts and bitwise operations are used correctly,
     * even though they trigger MISRA Rule 10.1, 10.3, 10.4, 10.7, 12.2 warnings.
     * These are suppressed here for clarity and maintainability.
     */
    /*lint -e9027 -e9029 -e9032 -e9034 -e9033 -e9053 -e701 -e10.1 -e10.3 -e10.4 -e10.7 -e12.2 */
    fno.fdate = ((datetimeGet.year - 1980) << 9) | (datetimeGet.month << 5) | (datetimeGet.day);
    fno.ftime = (datetimeGet.hour << 11) | (datetimeGet.minute << 5) | (datetimeGet.second / 2);
    /*lint +e9027 +e9029 +e9032 +e9034 +e9033 +e9053 +e701 +e10.1 +e10.3 +e10.4 +e10.7 +e12.2 */

    status = f_utime(fileName, &fno);
    if (FR_OK != status)
    {
        PRINTF("ERR: Failed to Set Meta-Data for %s. Error=%d\r\n", fileName, status);
        status = f_close(&g_fileObject);
        if (FR_OK != status)
        {
            return ERROR_CLOSE;
        }
        return ERROR_FILESYSTEM;
    }

    g_currentFileSize = 0; // Reset file size
#if (true == INFO_ENABLED || true == DEBUG_ENABLED)
    PRINTF("INFO: Created Log %s.\r\n", fileName);
#endif

    return ERROR_NONE;
}

error_t CONSOLELOG_CreateDirectory(void)
{
    FRESULT status;
    irtc_datetime_t datetimeGet;
    char directoryName[32];
    uint32_t counter = 1;

    IRTC_GetDatetime(RTC, &datetimeGet);

    /* Attempt to Create a New Folder With a Unique Name (Date + Cnt) */
    do
    {
        /* @note snprintf() Is Depricated But There Is No Better Equivalent */
        //lint -save -e586
    	(void)snprintf(directoryName, sizeof(directoryName), "/%04d%02d%02d_%u",
                 datetimeGet.year, datetimeGet.month, datetimeGet.day, counter++);
        //lint -restore
        status = f_mkdir(directoryName);
    } while ((FR_EXIST == status) && (counter < 1000UL));

    if (FR_OK != status)
    {
        PRINTF("ERR: Failed To Create Session Directory. Error=%d\r\n", (uint16_t)status);
        return ERROR_FILESYSTEM;
    }

    /* @note snprintf() Is Depricated But There Is No Better Equivalent */
    //lint -save -e586
    (void)snprintf(g_currentDirectory, sizeof(g_currentDirectory), "%s", directoryName);
    //lint -restore

#if (true == INFO_ENABLED || true == DEBUG_ENABLED)
    PRINTF("INFO: Created Directory %s.\r\n", g_currentDirectory);
#endif /* (true == INFO_ENABLED) */
    return ERROR_NONE;
}

REC_config_t CONSOLELOG_GetConfig(void)
{
	return g_config;
}

REC_version_t CONSOLELOG_GetVersion(void)
{
	return g_config.version;
}

uint32_t CONSOLELOG_GetBaudrate(void)
{
	return g_config.baudrate;
}

uint32_t CONSOLELOG_GetFileSize(void)
{
	return g_config.size;
}

uint32_t CONSOLELOG_GetTransferedBytes(void)
{
	return g_bytesTransfered;
}

bool CONSOLELOG_GetFlushCompleted(void)
{
	return g_flushCompleted;
}

void CONSOLELOG_ClearTransferedBytes(void)
{
	g_bytesTransfered = 0;
}

uint32_t CONSOLELOG_GetMaxBytes(void)
{
	return g_config.max_bytes;
}

FRESULT CONSOLELOG_CheckFileSystem(void)
{
    DIR sDir;
    /* Try To Open Root Folder */
    FRESULT sRes = f_opendir(&sDir, "/");
    if (FR_OK == sRes)
    {
        sRes = f_closedir(&sDir);
    }
    return sRes;
}

error_t CONSOLELOG_Init(void)
{
	FRESULT status;
	g_config.version 	= WCT_UNKOWN;
	g_config.baudrate 	= 0;

	/* Logic Disk */
	const TCHAR sLogicDisk[3U] = {SDDISK + '0', ':', '/'};
	BYTE work[FF_MAX_SS];

	/* Mount File System */
	if (FR_OK != f_mount(&g_fileSystem, sLogicDisk, 0U))
	{
		PRINTF("ERR: Mount Volume Failed.\r\n");
		return ERROR_FILESYSTEM;
	}

#if (FF_FS_RPATH >= 2U)

	/* Setup of Actual Work Logic Disk */
	status = f_chdrive((char const *)&sLogicDisk[0U]);
    if (FR_OK != status)
    {
        PRINTF("ERR: Change Drive Failed.\r\n");
        return ERROR_FILESYSTEM;
    }
#else
	#error "ERR: Disabled f_chdrive Function, Set The FF_FS_RPATH To Value Value That Is Equal Or Bigger Then 2."
#endif	/* (FF_FS_RPATH >= 2U) */

#if FF_USE_MKFS

    if (FR_OK != CONSOLELOG_CheckFileSystem())
    {

#if (true == DEBUG_ENABLED)
        PRINTF("DEBUG: File System Not Found. Formatting...\r\n");
#endif /* (true == DEBUG_ENABLED) */

        /* Make File System */
        if (FR_OK != f_mkfs(sLogicDisk, NULL, work, sizeof work))
        {
            PRINTF("ERR: Init File System Failed.\r\n");
            return ERROR_FILESYSTEM;
        }
    }
#else
	#error "ERR: f_mkfs() Function Is Disabled."
#endif 	/* FF_USE_MKFS */

    if (ERROR_NONE != CONSOLELOG_CreateDirectory())
    {
        PRINTF("ERR: Failed to create directory for logs.\r\n");
        return ERROR_FILESYSTEM;
    }

    return ERROR_NONE;
}

error_t CONSOLELOG_Recording(uint32_t file_size)
{
    FRESULT error;
    UINT bytesWritten;               //<! Bytes Written Into SD Card
    irtc_datetime_t datetimeGet;     //<! Actual Time From IRTC
    static uint8_t lastChar = 0;     //<! Last Character From Previous DMA Buffer

    static char timeString[12];
    static uint8_t timeLength = 0;

//    uint32_t localReadIndex  = g_readIndex;
    uint32_t localWriteIndex = g_writeIndex;

    while (g_readIndex != localWriteIndex)
    {
        /* Loads One Char From FIFO And Stores The Char Into Active DMA Buffer */
        uint8_t currentChar = g_fifo[g_readIndex];
        g_readIndex = (g_readIndex + 1UL) % FIFO_SIZE;

        g_activeDmaBuffer[g_dmaIndex++] = currentChar;

        /* Check If The CRLF Is Not Divided Into Two DMA Buffers */
        if (((lastChar == (uint8_t)'\r') && (currentChar == (uint8_t)'\n')) ||
            ((g_dmaIndex >= 2U) &&
             (g_activeDmaBuffer[g_dmaIndex - 2U] == (uint8_t)'\r') &&
             (g_activeDmaBuffer[g_dmaIndex - 1U] == (uint8_t)'\n')))
        {

        	IRTC_GetDatetime(RTC, &datetimeGet);
            /* @note snprintf() Is Depricated But There Is No Better Equivalent */
            //lint -save -e586
            timeLength = (uint8_t)snprintf(timeString, sizeof(timeString), "(%02d:%02d:%02d) ", datetimeGet.hour, datetimeGet.minute, datetimeGet.second);
            //lint -restore

            /* Addition of Time Mark To The DMA Buffer */
            for (uint8_t i = 0; i < timeLength; i++)
            {
                if (BLOCK_SIZE > g_dmaIndex)	// If DMA Buffer is Not Full
                {
                    g_activeDmaBuffer[g_dmaIndex++] = (uint8_t)timeString[i];
                }
                else
                {
                    /* Switch To New DMA Buffer */
                	g_processDmaBuffer = g_activeDmaBuffer;
                    g_dmaBufferReady = true;

                    g_activeDmaBuffer = (g_activeDmaBuffer == g_dmaBuffer1) ? g_dmaBuffer2 : g_dmaBuffer1;
                    g_dmaIndex = 0;

                    /* Continue in Addition of Time Mark */
                    g_activeDmaBuffer[g_dmaIndex++] = (uint8_t)timeString[i];
                }
            }
        }

        lastChar = currentChar; // Current Last Character For Next Buffer

        /* Check If DMA Buffer Is Full */
        if (BLOCK_SIZE == g_dmaIndex)
        {
            g_processDmaBuffer = g_activeDmaBuffer;
            g_dmaBufferReady = true;

            /* Switch on Next DMA Buffer */
            g_activeDmaBuffer = (g_activeDmaBuffer == g_dmaBuffer1) ? g_dmaBuffer2 : g_dmaBuffer1;
            g_dmaIndex = 0;
        }
    }

    /* Process Full DMA Buffer */
    if (g_dmaBufferReady && (NULL != g_processDmaBuffer))
    {
        if (NULL == g_fileObject.obj.fs)
        {
            if (ERROR_NONE != CONSOLELOG_CreateFile())
            {
                PRINTF("ERR: Failed to create new file.\r\n");
                return ERROR_OPEN;
            }
        }

		/**
		* ADMA Error Status (ADMA_ERR_STATUS)
		* 3 bit 	-> 	ADMA Descriptor Error
		* 2 bit 	-> 	ADMA Length Mismatch Error
		* 1-0 bit 	-> 	ADMA Error State (When ADMA Error Occurred)
		*				Field Indicates The State of The ADMA When An
		*				Error Has Occurred During An ADMA Data Transfer.
		*/
        uint32_t stat_reg = g_sd.host->hostController.base->ADMA_ERR_STATUS;
        if (0x0U != (stat_reg & (uint32_t)0xC))
        {
            PRINTF("ERR: Failed to Write Data To File. Error=%d\r\n", ERROR_ADMA);
            (void)f_close(&g_fileObject);
            g_fileObject.obj.fs = NULL;
            return ERROR_ADMA;
        }
        error = f_write(&g_fileObject, g_processDmaBuffer, BLOCK_SIZE, &bytesWritten);
        if (FR_OK != error)
        {
        	return (error_t)error;
        }

        g_currentFileSize += BLOCK_SIZE;
        if (g_currentFileSize >= file_size)
        {
#if (true == INFO_ENABLED || true == DEBUG_ENABLED)
            PRINTF("INFO: File Size Limit Reached. Closing file. (LIMIT: %d, CURRENT %d)\r\n", file_size, g_currentFileSize);
#endif /* (true == INFO_ENABLED || true == DEBUG_ENABLED) */
            (void)f_close(&g_fileObject);
            g_fileObject.obj.fs = NULL;
        }

        g_dmaBufferReady = false;     // Reset Flag of ADMA Buffer
        g_processDmaBuffer = NULL;    // Clear g_processDmaBuffer
    }

    return ERROR_NONE;
}

error_t CONSOLELOG_Flush(void)
{
	FRESULT error;
	UINT bytesWritten;
	int tickDiff 			= 0;
	int64_t lastTick 		= 0;
	int64_t currentTick 	= (int64_t)xTaskGetTickCount();

	/*
	 * __ATOMIC_ACQUIRE - 	Ensures That All Read Values Are Consistent
	 * 						With Pre-Read Operations.
	 * */
	/*
	 * MISRA Deviation: Rule 1.3
	 * Reason: __ATOMIC_ACQUIRE is a compiler-specific macro used for atomic memory ordering
	 *        in GCC/Clang. It is intentionally used here for ensuring that all read values are consistent
	 * 		  with pre-read operations.
	 */
	/*lint -save -e40 */
	lastTick = (int64_t)__atomic_load_n(&g_lastDataTick, __ATOMIC_ACQUIRE);
	/*lint -restore */

	tickDiff = (int)(currentTick - lastTick);
	if ((CONSOLELOG_Abs(tickDiff) > FLUSH_TIMEOUT_TICKS) && (g_dmaIndex > 0U))
	{
#if (true == INFO_ENABLED)
		PRINTF("INFO: Current Ticks = %d.\r\n", currentTick);
		PRINTF("INFO: Last Ticks = %d.\r\n", g_lastDataTick);
		PRINTF("INFO: Flush Triggered. Writing Remaining Data To File.\r\n");
#else
		PRINTF("INFO: Flush Triggered.\r\n");
#endif /* (true == INFO_ENABLED) */

		while (g_dmaIndex < BLOCK_SIZE)			/* Fill Buffer With ' ' */
		{
			g_activeDmaBuffer[g_dmaIndex++] = (uint8_t)' ';
		}

		g_processDmaBuffer 	= g_activeDmaBuffer;
		g_dmaBufferReady 	= true;

		// Switch To Second Buffer
		g_activeDmaBuffer 	= (g_activeDmaBuffer == g_dmaBuffer1) ? g_dmaBuffer2 : g_dmaBuffer1;
		g_dmaIndex 			= 0;

		if (NULL == g_fileObject.obj.fs)
		{
			if (ERROR_NONE != CONSOLELOG_CreateFile())
			{
				PRINTF("ERR: Failed to Create New File During Flush.\r\n");
				return ERROR_RECORD;
			}
		}

#if (CONTROL_LED_ENABLED == true )
		LED_SignalFlush();				/* Signal Flush							*/
		LED_SignalRecordingStop();		/* Signal That Recording Is Not Active	*/

#endif /* (CONTROL_LED_ENABLED == true ) */

		/**
		* ADMA Error Status (ADMA_ERR_STATUS)
		* 3 bit 	-> 	ADMA Descriptor Error
		* 2 bit 	-> 	ADMA Length Mismatch Error
		* 1-0 bit 	-> 	ADMA Error State (When ADMA Error Occurred)
		*				Field Indicates The State of The ADMA When An
		*				Error Has Occurred During An ADMA Data Transfer.
		*/
		uint32_t stat_reg = g_sd.host->hostController.base->ADMA_ERR_STATUS;
		if (0x0U != (stat_reg & (uint32_t)0xC))
		{
			PRINTF("ERR: Failed to Write Data To File During Flush. Error=%d\r\n", ERROR_ADMA);
			(void)f_close(&g_fileObject);
			g_fileObject.obj.fs = NULL;
			return ERROR_ADMA;
		}
		error = f_write(&g_fileObject, g_processDmaBuffer, BLOCK_SIZE, &bytesWritten);
		if (FR_OK != error)
		{
			return (error_t)error;
		}
		g_currentFileSize += BLOCK_SIZE;

#if	(true == INFO_ENABLED || true == DEBUG_ENABLED)
		PRINTF("INFO: Closing File\r\n");
#endif /* (true == INFO_ENABLED || true == DEBUG_ENABLED) */

		(void)f_close(&g_fileObject);
		g_fileObject.obj.fs = NULL;
		g_dmaBufferReady = false;
		g_processDmaBuffer = NULL;
		g_flushCompleted = true;
	}
	return ERROR_NONE;
}

error_t CONSOLELOG_Deinit(void)
{
	FRESULT error;

	/* Stop Generating UART Interrupt */
	UART_Disable();

	/* Close All Opened Files */
    if (NULL != g_fileObject.obj.fs)
    {
        error = f_close(&g_fileObject);
        if (FR_OK != error)
        {
            PRINTF("ERR: Failed to Close File. ERR=%d\r\n", error);
            return ERROR_CLOSE;
        }
    }


	/* Unmount File System From Logic Drive */
	error = f_mount(NULL, "", 0);
	if (FR_OK != error)
	{
		PRINTF("ERR: Failed To Unmount File System. Error=%d\r\n", error);
		return ERROR_RECORD;
	}

#if (true == DEBUG_ENABLED)
	PRINTF("DEBUG: File System De-Inicialized.\r\n");
#endif /* (true == DEBUG_ENABLED) */

	return ERROR_NONE;
}

error_t CONSOLELOG_ReadConfig(void)
{
    FRESULT error;
    DIR dir;           					//<! Opened Directory
    FILINFO fno;       					//<! Information About File
    FIL configFile;    					//<! Opened File
    UINT bytesRead;    					//<! Number of Read Bytes

    error = f_opendir(&dir, "/");
    if (FR_OK != error)
    {
#if (CONTROL_LED_ENABLED == true)
    	LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */

    	PRINTF("ERR: Failed to Open Root Dir. ERR=%d\r\n", error);
        return ERROR_OPEN;
    }

    while (true)
	{
    	error = f_readdir(&dir, &fno);
    	if (FR_OK != error || 0UL == (uint32_t)fno.fname[0])
		{
			break; 	// End of Directory or Error
		}

    	if ((fno.fattrib & (BYTE)AM_DIR) == 0U)	// If Not Directory
    	{
    		if (ERROR_NONE == (uint32_t)strcmp(fno.fname, CONFIG_FILE))	// If .config File
			{
    			PRINTF("DEBUG: Found .config File: %s\r\n", fno.fname);

    			error = f_open(&configFile, CONFIG_FILE, FA_READ);
    			if (FR_OK != error)
				{
#if (CONTROL_LED_ENABLED == true)
    				LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */

    				PRINTF("ERR: Failed to open .config file. ERR=%d\r\n", error);
    				(void)f_closedir(&dir); 	// Close Root Directory
					return ERROR_OPEN;
				}
    			(void)memset(g_dmaBuffer1, 0, sizeof(g_dmaBuffer1));
    			error = f_read(&configFile, g_dmaBuffer1, sizeof(g_dmaBuffer1) - 1UL, &bytesRead);
				if (FR_OK != error)
				{
#if (CONTROL_LED_ENABLED == true)
					LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */

					PRINTF("ERR: Failed To Read .config File. ERR=%d\r\n", error);
					(void)f_close(&configFile);
					(void)f_closedir(&dir);
					return ERROR_READ;
				}

				if (ERROR_NONE != CONSOLELOG_ProccessConfigFile((const char *)g_dmaBuffer1))
				{
#if (CONTROL_LED_ENABLED == true)
					LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */

					PRINTF("ERR: Failed To Read .config File. ERR=%d\r\n", error);
					(void)f_close(&configFile);
					(void)f_closedir(&dir);
					return ERROR_READ;
				}

				// Processed Configuration File
				(void)f_close(&configFile);
				(void)f_closedir(&dir);
				return ERROR_NONE;
			}
    	}
	}
    PRINTF("ERR: .config file not found in root directory.\r\n");
    (void)f_closedir(&dir);
	return ERROR_OPEN;
}

error_t CONSOLELOG_ProccessConfigFile(const char *content)
{
    const char *key = "baudrate=";
    const char *keyFileSize = "file_size=";

    char *found;
    uint32_t baudrate;
    uint32_t value;
    error_t error = ERROR_NONE;

    /* Find Key "baudrate=" */
    found = strstr(content, key);
    if (NULL == found)
    {
#if (CONTROL_LED_ENABLED == true)
    	LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */

        PRINTF("ERR: Key 'baudrate=' Not Found.\r\n");
        return ERROR_READ;
    }

    found += strlen(key);					// Move Pointer Behind "baudrate="

    /* MISRA Deviation Note:
     * Rule: MISRA 2012 Rule 21.7
     * Justification: Use of 'atoi' is intentional in controlled context.
     * The input string 'found' should contain only numeric characters - baudrate.
     * If string is not a number returns zero.
     */
    /*lint -e586 MISRA Deviation: Use of 'atoi' is intentional and input is trusted. */
    baudrate = (uint32_t)atoi(found);
    /*lint +e586 */

    if (0UL == baudrate)
	{
#if (CONTROL_LED_ENABLED == true)
    	LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */

		PRINTF("ERR: Invalid Baudrate Value.\r\n");
		return ERROR_READ;
	}

    switch (baudrate)
	{
		case 230400U:
			g_config.version = WCT_AUTOS2;
			break;
		case 115200U:
			g_config.version = WCT_AUTOS1;
			break;
		default:
#if (CONTROL_LED_ENABLED == true)
			LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */

			PRINTF("ERR: Unsupported Baudrate Value: %d\r\n", baudrate);
			error = ERROR_CONFIG;
			break;
	}
    if (ERROR_NONE == error)
    {
    	g_config.baudrate = baudrate;
    }
    else
    {
    	return error;
    }

    /* Find Key "file_size=" */
    found = strstr(content, keyFileSize);
    if (NULL == found)
    {
#if (CONTROL_LED_ENABLED == true)
    	LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */

        PRINTF("ERR: Key 'file_size=' Not Found.\r\n");
        return ERROR_READ;
    }

    found += strlen(keyFileSize);			// Move Pointer Behind "file_size="

    /* MISRA Deviation Note:
     * Rule: MISRA 2012 Rule 21.7
     * Justification: Use of 'atoi' is intentional in controlled context.
     * The input string 'found' should contain only numeric characters - size of record file.
     */
    /*lint -e586 MISRA Deviation: Use of 'atoi' is intentional and input is trusted. */
    value = (uint32_t)atoi(found);			// Convert Value To INT
    /*lint +e586 */

    if (0UL >= value)
    {
#if (CONTROL_LED_ENABLED == true)
    	LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */

        PRINTF("ERR: Invalid File Size Value.\r\n");
        return ERROR_READ;
    }
    if (0UL != (value % 512UL))
    {
    	/* Round Up To 512 */
        uint32_t roundedValue = ((value + 511UL) / 512UL) * 512UL;
        PRINTF("WARN: File Size %d Is Not a Multiple of 512. Rounding Up to %d.\r\n", value, roundedValue);
        value = roundedValue;
    }

    g_config.size = value;  // Store File Size in Configuration Structure
    g_config.max_bytes = (g_config.baudrate / 1000UL) * RECORD_LED_TIME_INTERVAL;
    PRINTF("DEBUG: File Size Set To %d Bytes.\r\n", g_config.size);

    return ERROR_NONE;
}
