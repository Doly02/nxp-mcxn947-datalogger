/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      record.c
 *  Author:         Tomas Dolak
 *  Date:           18.11.2024
 *  Description:    File Includes Operation For Recoding Mode.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           record.c
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
#define CIRCULAR_BUFFER_SIZE 		1024U

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

#define GET_CURRENT_TIME_MS()  (xTaskGetTickCount() * portTICK_PERIOD_MS)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
/**
 * @defgroup 	Recording Buffers and Recording Management
 * @brief 		Group Contains Variables For Management of Recording.
 * @details 	Includes DMA Buffers For Recording, Indexes, Pointers, ...
 * @{
 */

/**
 * @brief 	File System Object.
 */
static FATFS g_fileSystem;

/**
 * @brief 	File Object.
 * @details Pointer to Current Opened File.
 */
static FIL g_fileObject;

/**
 * @brief	Name Of The Folder Where The Files (Logs)
 * 			From The Current Session Are Stored.
 */
static char g_u8CurrentDirectory[32];


/**
 * @brief 	Buffer For Multi-Buffering - In Particular Dual-Buffering,
 * 			One Is Always Filled, The Other Is Processed.
 * @details Must Be Aligned on Multiple of 512B, Since SDHC/SDXC Card Uses 512-Byte Fixed
 * 			Block Length. The Address of The R/W Buffer Should Be Also Align To The Specific DMA
 * 			Data Buffer Address Align Value. At The Same Time Buffer Address/Size Should Be Aligned To The Cache
 * 			Line Size.
 */
SDK_ALIGN(static uint8_t g_au8DmaBuffer1[BLOCK_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);

/**
 * @brief 	Buffer For Multi-Buffering - In Particular Dual-Buffering,
 * 			One Is Always Filled, The Other Is Processed.
 * @details Must Be Aligned on Multiple of 512B, Since SDHC/SDXC Card Uses 512-Byte Fixed
 * 			Block Length. The Address of The R/W Buffer Should Be Also Align To The Specific DMA
 * 			Data Buffer Address Align Value. At The Same Time Buffer Address/Size Should Be Aligned To The Cache
 * 			Line Size.
 */
SDK_ALIGN(static uint8_t g_au8DmaBuffer2[BLOCK_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);

/**
 * @brief 	Back Buffer Which Serves For Data Collection From Circular Buffer
 * 			And Is Used For Data-Processing (Time Stamps Are Inserted To This Buffer).
 */
static uint8_t* g_pu8BackDmaBuffer 		= g_au8DmaBuffer1;

/**
 * @brief 	Front Buffer Which Serves For Storing Data Into SD Card.
 */
static uint8_t* g_pu8FrontDmaBuffer 	= NULL;

/**
 * @brief 	Pointer on Current Back DMA Buffer Into Which The Time Stamps Are Inserted.
 */
static uint16_t g_u16BackDmaBufferIdx	= 0;

/**
 * @brief 	Indicates That Collection Buffer (Back Buffer) Is Full and Ready To Swap.
 */
static bool g_bBackDmaBufferReady		= false;

/**
 * @brief 	Value of Ticks When Last Character Was Received Thru LPUART.
 */
static TickType_t g_lastDataTick 		= 0;

/**
 * @brief	Tracks Current File Size.
 */
static uint32_t g_u32CurrentFileSize 	= 0;

/**
 * @brief	Counter For Unique File Names.
 */
static uint16_t g_u32FileCounter 		= 1;

/**
 * @brief 	Flush Completed Flag.
 * @details If No Data of The LPUART Periphery Are Received Within The `FLUSH_TIMEOUT_TICKS`
 * 			Interval, The Data Collected So Far In The Buffer Are Flushed To The File.
 */
static bool g_bFlushCompleted 			= false;

/**
 * @brief	Transferred Bytes Between Blinking LEDs.
 */
static uint32_t g_u32BytesTransfered	= 0U;

/** @} */ // End of Recording Buffers and Recording Management

/**
 * @defgroup 	UART Management
 * @brief 		Group Contains Variables For Recording From UART.
 * @details 	Data From UART Are Stored Into FIFO (Circular Buffer).
 * @{
 */

/**
 * @brief 	Circular Buffer For Reception of Data From UART Interrupt Service Routine.
 * @details Filled in LP_FLEXCOMM3_IRQHandler Interrupt Service Routine.
 */
SDK_ALIGN(static volatile uint8_t g_au8CircBuffer[CIRCULAR_BUFFER_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);

/**
 * @brief	Index For Writing Into FIFO.
 */
static volatile uint32_t g_u32WriteIndex = 0;

/**
 * @brief 	Index For Reading From FIFO.
 */
static volatile uint32_t g_u32ReadIndex = 0;

/** @} */ // End of UART Management Group


/*******************************************************************************
 * Interrupt Service Routines (ISRs)
 ******************************************************************************/
/**
 * @brief LPUART3 IRQ Handler.
 *
 */

/* MISRA Deviation: Rule 8.4 [Required]
 * Suppress: Function 'LP_FLEXCOMM3_IRQHandler' Defined Without a Prototype in Scope.
 * Justification: LP_FLEXCOMM3_IRQHandler is Declared WEAK in startup_mcxn947_cm33_core0.c and Overridden Here.
 */
/*lint -e957 */
void LP_FLEXCOMM3_IRQHandler(void)
{
    uint8_t u8Data;
    uint32_t u32Stat;
    uint32_t u32NextWriteIndex;

    /* Check For New Data */
    u32Stat = LPUART_GetStatusFlags(LPUART3);
    if (0U != ((uint32_t)kLPUART_RxDataRegFullFlag & u32Stat))
    {
    	u8Data = LPUART_ReadByte(LPUART3);
        /* Add Data To FIFO */
    	u32NextWriteIndex = ((g_u32WriteIndex + 1UL) % CIRCULAR_BUFFER_SIZE);
        if (u32NextWriteIndex != g_u32ReadIndex) // Check if FIFO is not full
        {
        	g_au8CircBuffer[g_u32WriteIndex] = u8Data;
            g_u32WriteIndex = u32NextWriteIndex;

            /* Update Time Of Last Receiving */
            g_lastDataTick = xTaskGetTickCount();
            g_bFlushCompleted = false;
        }
        g_u32BytesTransfered++;
    }

    /* Clear Interrupt Flag */
    (void)LPUART_ClearStatusFlags(LPUART3, (uint32_t)kLPUART_RxDataRegFullFlag);
    SDK_ISR_EXIT_BARRIER;
}
/*lint +e957 */

/*******************************************************************************
 * Code
 ******************************************************************************/
DWORD get_fattime(void)
{
    irtc_datetime_t datetime = { 0U };

    /**
     * MISRA Deviation: Rule 10.8
     * Suppress: Conversion From Smaller Unsigned Integer Types to a Wider Unsigned Type.
     * Justification: The Cast From 'Uint8_t' and 'Uint16_t' to 'Uint32_t' is Intentional and Safe in This Context.
     * The Values are Combined Using Bitwise OR Into a FAT Time Stamp Format That Expects 32-Bit Result.
     * All Input Ranges are Within Limits.
     */
    /*lint -e9033*/
    IRTC_GetDatetime(RTC, &datetime);

    return ((DWORD)(datetime.year - 1980U) << 25) |
           ((DWORD)datetime.month << 21) |
           ((DWORD)datetime.day << 16) |
           ((DWORD)datetime.hour << 11) |
           ((DWORD)datetime.minute << 5) |
           ((DWORD)(datetime.second / (uint8_t)2));
    /*lint +e9033*/
}

uint32_t CONSOLELOG_GetFreeSpaceMB(void)
{
    const TCHAR sLogicDisk[3U] = {SDDISK + '0', ':', '/'};
    DWORD freClusters;
    FATFS *fs;
    FRESULT res;
    uint64_t u64FreeSectors;

    res = f_getfree(sLogicDisk, &freClusters, &fs);
    if (FR_OK != res)
    {
        PRINTF("ERR: Failed To Get Free Space ERR=%d\r\n", (int)res);
        return 0;
    }

    u64FreeSectors = (uint64_t)freClusters * fs->csize;

    /* 512B * sector [KB] -> /2 [MB] */
    return (uint32_t)(u64FreeSectors / 2048UL);
}

error_t CONSOLELOG_CreateFile(void)
{
    FRESULT status;
    char u8FileName[64];				//<! File Name
    irtc_datetime_t datetimeGet;		//<! To Store Time in File Meta-Data
    FILINFO fno;						//<! File Meta-Data


    IRTC_GetDatetime(RTC, &datetimeGet);

    /* Generate a New File Name */
    /**
     * MISRA Deviation: Rule 21.6
     * Suppress: Use Of Standard Library Function 'Snprintf'.
     * Justification: 'Snprintf' Is Deprecated But There Is No Better Equivalent For Safe String Formatting.
     */
    /*lint -e586*/
    (void)snprintf(u8FileName, sizeof(u8FileName), "%s/%04d%02d%02d_%02d%02d%02d_%u.txt",
    		g_u8CurrentDirectory, datetimeGet.year, datetimeGet.month, datetimeGet.day,
             datetimeGet.hour, datetimeGet.minute, datetimeGet.second,
             g_u32FileCounter++);
    /*lint +e586 */

    /* Open New File */
    /**
     * MISRA Deviation: Rule 10.1 [Required]
     * Suppress: Bitwise Operation on Composite Constant Expression.
     * Justification: FA_WRITE and FA_CREATE_ALWAYS are Standard Bitmask Flags Defined By The FatFs Library.
     * These Constants Are Designed To Be Combined Using Bitwise OR, and The Use is Safe and Intentional.
     */
    /*lint -e9027 */
    status = f_open(&g_fileObject, u8FileName, (FA_WRITE | FA_CREATE_ALWAYS));
    if (FR_OK != status)
    {
        PRINTF("ERR: Failed to create file %s. Error=%d\r\n", u8FileName, (uint32_t)status);
        return ERROR_OPEN;
    }
    /*lint +e9027 */

    /* Setup of File Meta-Data */
    /**
     * MISRA Deviation Note:
     * The Following Expression is Intentionally Written in a Compact and Readable Form
     * For Setting FAT File Time Stamps. All Shifts and Bitwise Operations Are Used Correctly,
     * Even Though They Trigger MISRA Rule 10.1, 10.3, 10.4, 10.7, 12.2 Warnings.
     * These are Suppressed Here For Clarity and Maintainability.
     */
    /*lint -e9027 -e9029 -e9032 -e9034 -e9033 -e9053 -e701 -e10.1 -e10.3 -e10.4 -e10.7 -e12.2 */
    fno.fdate = ((datetimeGet.year - 1980) << 9) | (datetimeGet.month << 5) | (datetimeGet.day);
    fno.ftime = (datetimeGet.hour << 11) | (datetimeGet.minute << 5) | (datetimeGet.second / 2);
    /*lint +e9027 +e9029 +e9032 +e9034 +e9033 +e9053 +e701 +e10.1 +e10.3 +e10.4 +e10.7 +e12.2 */

    status = f_utime(u8FileName, &fno);
    if (FR_OK != status)
    {
        PRINTF("ERR: Failed to Set Meta-Data for %s. Error=%d\r\n", u8FileName, status);
        status = f_close(&g_fileObject);
        if (FR_OK != status)
        {
            return ERROR_CLOSE;
        }
        return ERROR_FILESYSTEM;
    }

    g_u32CurrentFileSize = 0; // Reset file size
#if (true == INFO_ENABLED)
    PRINTF("INFO: Created Log %s.\r\n", u8FileName);
#endif /* (true == INFO_ENABLED) */

    return ERROR_NONE;
}

error_t CONSOLELOG_CreateDirectory(void)
{
    FRESULT status;
    irtc_datetime_t datetimeGet;
    char u8DirectoryName[32];
    uint32_t counter = 1;

    IRTC_GetDatetime(RTC, &datetimeGet);

    /* Attempt to Create a New Folder With a Unique Name (Date + Cnt) */
    do
    {
        /**
         * MISRA Deviation: Rule 21.6 [Advisory]
         * Suppress: Use Of Standard Library Function 'snprintf()' Which Is Not Fully Bounded In All Environments.
         * Justification: The Use Of 'snprintf()' Is Intentional And Acceptable In This Context,
         * The Format String And All Input Values Are Controlled and Predictable.
         */
    	/*lint -e586 */
    	(void)snprintf(u8DirectoryName, sizeof(u8DirectoryName), "/%04d%02d%02d_%u",
                 datetimeGet.year, datetimeGet.month, datetimeGet.day, counter++);
    	/*lint +e586 */
        status = f_mkdir(u8DirectoryName);
    } while ((FR_EXIST == status) && (counter < 1000UL));

    if (FR_OK != status)
    {
        PRINTF("ERR: Failed To Create Session Directory. Error=%d\r\n", (uint16_t)status);
        return ERROR_FILESYSTEM;
    }

    /**
     * MISRA Deviation: Rule 21.6 [Advisory]
     * Suppress: Use Of Standard Library Function 'snprintf()' Which Is Not Fully Bounded In All Environments.
     * Justification: The Use Of 'snprintf()' Is Intentional And Acceptable In This Context,
     * The Format String And All Input Values Are Controlled and Predictable.
     */
    /*lint -e586 */
    (void)snprintf(g_u8CurrentDirectory, sizeof(g_u8CurrentDirectory), "%s", u8DirectoryName);
    /*lint +e586 */

#if (true == INFO_ENABLED)
    PRINTF("INFO: Created Directory %s.\r\n", g_u8CurrentDirectory);
#endif /* (true == INFO_ENABLED) */
    return ERROR_NONE;
}

uint32_t CONSOLELOG_GetTransferedBytes(void)
{
	return g_u32BytesTransfered;
}

bool CONSOLELOG_GetFlushCompleted(void)
{
	return g_bFlushCompleted;
}

void CONSOLELOG_ClearTransferedBytes(void)
{
	g_u32BytesTransfered = 0;
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

	PARSER_ClearConfig();

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
        PRINTF("DEBUG: File System Not Found. Creating FAT File System...\r\n");
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
    UINT bytesWritten;               //<! Bytes Written Into SD Card
    irtc_datetime_t datetimeGet;     //<! Actual Time From IRTC
    static uint8_t u8LastChar = 0;     //<! Last Character From Previous DMA Buffer

    static char timeString[12];
    static uint8_t u8TimeLength = 0;

    uint32_t u32LocalWriteIndex = g_u32WriteIndex;

    while (g_u32ReadIndex != u32LocalWriteIndex)
    {
        /* Loads One Char From FIFO And Stores The Char Into Active DMA Buffer */
        uint8_t currentChar = g_au8CircBuffer[g_u32ReadIndex];
        g_u32ReadIndex = (g_u32ReadIndex + 1UL) % CIRCULAR_BUFFER_SIZE;

        g_pu8BackDmaBuffer[g_u16BackDmaBufferIdx++] = currentChar;

        /* Check If The CRLF Is Not Divided Into Two DMA Buffers */
        if (((u8LastChar == (uint8_t)'\r') && (currentChar == (uint8_t)'\n')) ||
            ((g_u16BackDmaBufferIdx >= 2U) &&
             (g_pu8BackDmaBuffer[g_u16BackDmaBufferIdx - 2U] == (uint8_t)'\r') &&
             (g_pu8BackDmaBuffer[g_u16BackDmaBufferIdx - 1U] == (uint8_t)'\n')))
        {

        	IRTC_GetDatetime(RTC, &datetimeGet);
            /* @note snprintf() Is Depricated But There Is No Better Equivalent */
            //lint -save -e586
        	u8TimeLength = (uint8_t)snprintf(timeString, sizeof(timeString), "(%02d:%02d:%02d) ", datetimeGet.hour, datetimeGet.minute, datetimeGet.second);
            //lint -restore

            /* Addition of Time Mark To The DMA Buffer */
            for (uint8_t i = 0; i < u8TimeLength; i++)
            {
                if (BLOCK_SIZE > g_u16BackDmaBufferIdx)	// If DMA Buffer is Not Full
                {
                	g_pu8BackDmaBuffer[g_u16BackDmaBufferIdx++] = (uint8_t)timeString[i];
                }
                else
                {
                    /* Switch To New DMA Buffer */
                	g_pu8FrontDmaBuffer = g_pu8BackDmaBuffer;
                	g_bBackDmaBufferReady = true;

                	g_pu8BackDmaBuffer = (g_pu8BackDmaBuffer == g_au8DmaBuffer1) ? g_au8DmaBuffer2 : g_au8DmaBuffer1;
                    g_u16BackDmaBufferIdx = 0;

                    /* Continue in Addition of Time Mark */
                    g_pu8BackDmaBuffer[g_u16BackDmaBufferIdx++] = (uint8_t)timeString[i];
                }
            }
        }

        u8LastChar = currentChar; // Current Last Character For Next Buffer

        /* Check If DMA Buffer Is Full */
        if (BLOCK_SIZE == g_u16BackDmaBufferIdx)
        {
        	g_pu8FrontDmaBuffer = g_pu8BackDmaBuffer;
        	g_bBackDmaBufferReady = true;

            /* Switch on Next DMA Buffer */
        	g_pu8BackDmaBuffer = (g_pu8BackDmaBuffer == g_au8DmaBuffer1) ? g_au8DmaBuffer2 : g_au8DmaBuffer1;
            g_u16BackDmaBufferIdx = 0;
        }
    }

    /* Process Full DMA Buffer */
    if (g_bBackDmaBufferReady && (NULL != g_pu8FrontDmaBuffer))
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
        (void)f_write(&g_fileObject, g_pu8FrontDmaBuffer, BLOCK_SIZE, &bytesWritten);
        g_u32CurrentFileSize += BLOCK_SIZE;
        if (g_u32CurrentFileSize >= file_size)
        {
#if (true == INFO_ENABLED)
            PRINTF("INFO: File Size Limit Reached. Closing file. (LIMIT: %d, CURRENT %d)\r\n", file_size, g_u32CurrentFileSize);
#endif /* (true == INFO_ENABLED */
            (void)f_close(&g_fileObject);
            g_fileObject.obj.fs = NULL;
        }

        g_bBackDmaBufferReady = false;   // Reset Flag of ADMA Buffer
        g_pu8FrontDmaBuffer = NULL;    	// Clear g_pu8FrontDmaBuffer
    }

    return ERROR_NONE;
}

error_t CONSOLELOG_Flush(void)
{
	FRESULT error;
	UINT bytesWritten;
	TickType_t LastTick 	= 0;
	TickType_t CurrentTick 	= xTaskGetTickCount();

	/*
	 * __ATOMIC_ACQUIRE - 	Ensures That All Read Values Are Consistent
	 * 						With Pre-Read Operations.
	 * */
	/*
	 * MISRA Deviation: Rule 1.3
	 * Suppress: Use of Compiler-Specific Intrinsic With Side Effects.
	 * Reason: __ATOMIC_ACQUIRE Is A Compiler-Specific Macro Used For Atomic Memory Ordering
	 *         In GCC/Clang. It Is Intentionally Used Here For Ensuring That All Read Values Are Consistent
	 *         With Pre-Read Operations.
	 */
	/*lint -e40 */
	LastTick = (TickType_t)__atomic_load_n(&g_lastDataTick, __ATOMIC_ACQUIRE);
	/*lint +e40 */

	if ((CurrentTick > LastTick) &&
	    ((CurrentTick - LastTick) > FLUSH_TIMEOUT_TICKS) &&
	    (g_u16BackDmaBufferIdx > 0U))
	{
#if (true == INFO_ENABLED)
		PRINTF("INFO: Current Ticks = %d.\r\n", CurrentTick);
		PRINTF("INFO: Last Ticks = %d.\r\n", LastTick);
		PRINTF("INFO: Flush Triggered. Writing Remaining Data To File.\r\n");
#elif (true == DEBUG_ENABLED)
		PRINTF("DEBUG: Flush Triggered.\r\n");
#endif /* (true == INFO_ENABLED) */

		while (g_u16BackDmaBufferIdx < BLOCK_SIZE)			/* Fill Buffer With ' ' */
		{
			g_pu8BackDmaBuffer[g_u16BackDmaBufferIdx++] = (uint8_t)' ';
		}

		g_pu8FrontDmaBuffer 	= g_pu8BackDmaBuffer;
		g_bBackDmaBufferReady 	= true;

		// Switch To Second Buffer
		g_pu8BackDmaBuffer 	= (g_pu8BackDmaBuffer == g_au8DmaBuffer1) ? g_au8DmaBuffer2 : g_au8DmaBuffer1;
		g_u16BackDmaBufferIdx	= 0;

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
		error = f_write(&g_fileObject, g_pu8FrontDmaBuffer, BLOCK_SIZE, &bytesWritten);
		if (FR_OK != error)
		{
			return (error_t)error;
		}
		g_u32CurrentFileSize += BLOCK_SIZE;

#if	(true == INFO_ENABLED)
		PRINTF("INFO: Closing File\r\n");
#endif /* (true == INFO_ENABLED) */

		(void)f_close(&g_fileObject);
		g_fileObject.obj.fs 	= NULL;
		g_bBackDmaBufferReady 	= false;
		g_pu8FrontDmaBuffer 	= NULL;
		g_bFlushCompleted 		= true;

		UART_Disable();
		UART_Enable();
	}
	else if ((CurrentTick > LastTick) &&
	    ((CurrentTick - LastTick) > FLUSH_TIMEOUT_TICKS))
	{
		UART_Disable();
		UART_Enable();
	}
	else
	{
		; /* To Avoid MISRA 2012 Rule 15.7 */
	}
	return ERROR_NONE;
}

error_t CONSOLELOG_PowerLossFlush(void)
{
	UINT bytesWritten;

	/* Finish The Receiving of New Data */
	UART_Disable();

	if (g_u16BackDmaBufferIdx > 0U)
	{
#if (true == INFO_ENABLED)
		PRINTF("INFO: Pwrloss Flush Triggered.\r\n");
#endif /* (true == INFO_ENABLED) */

		while (g_u16BackDmaBufferIdx < BLOCK_SIZE)			/* Fill Buffer With ' ' */
		{
			g_pu8BackDmaBuffer[g_u16BackDmaBufferIdx++] = (uint8_t)' ';
		}

		g_pu8FrontDmaBuffer 		= g_pu8BackDmaBuffer;
		g_bBackDmaBufferReady 	= true;

		// Switch To Second Buffer
		g_pu8BackDmaBuffer 	= (g_pu8BackDmaBuffer == g_au8DmaBuffer1) ? g_au8DmaBuffer2 : g_au8DmaBuffer1;
		g_u16BackDmaBufferIdx	= 0;

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
		(void)f_write(&g_fileObject, g_pu8FrontDmaBuffer, BLOCK_SIZE, &bytesWritten);
		g_u32CurrentFileSize += BLOCK_SIZE;

#if	(true == INFO_ENABLED)
		PRINTF("INFO: Closing File\r\n");
#endif /* (true == INFO_ENABLED) */

		(void)f_close(&g_fileObject);
		g_fileObject.obj.fs 	= NULL;
		g_bBackDmaBufferReady 	= false;
		g_pu8FrontDmaBuffer 	= NULL;
		g_bFlushCompleted 		= true;
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
    		if (ERROR_NONE == (uint32_t)strcmp(fno.fname, CONFIG_FILE))	// If config File
			{
#if (true == DEBUG_ENABLED)
    			PRINTF("DEBUG: Found .config File: %s\r\n", fno.fname);
#endif /* (true == DEBUG_ENABLED) */
    			error = f_open(&configFile, CONFIG_FILE, FA_READ);
    			if (FR_OK != error)
				{
#if (CONTROL_LED_ENABLED == true)
    				LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */

    				PRINTF("ERR: Failed To Open .config File. ERR=%d\r\n", error);
    				(void)f_closedir(&dir); 	// Close Root Directory
					return ERROR_OPEN;
				}
    			(void)memset(g_au8DmaBuffer1, 0, sizeof(g_au8DmaBuffer1));
    			error = f_read(&configFile, g_au8DmaBuffer1, sizeof(g_au8DmaBuffer1) - 1UL, &bytesRead);
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

				if (ERROR_NONE != CONSOLELOG_ProccessConfigFile((const char *)g_au8DmaBuffer1))
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
    error_t error;

    error = PARSER_ParseBaudrate(content);
    if (error != ERROR_NONE)
    {
#if (CONTROL_LED_ENABLED == true)
        LED_SignalError();
#endif
        return error;
    }

    error = PARSER_ParseFileSize(content);
    if (error != ERROR_NONE)
    {
#if (CONTROL_LED_ENABLED == true)
        LED_SignalError();
#endif
        return error;
    }

    error = PARSER_ParseStopBits(content);
    if (error != ERROR_NONE)
    {
#if (CONTROL_LED_ENABLED == true)
        LED_SignalError();
#endif
        return error;
    }

    error = PARSER_ParseDataBits(content);
    if (error != ERROR_NONE)
    {
#if (CONTROL_LED_ENABLED == true)
        LED_SignalError();
#endif
        return error;
    }

    error = PARSER_ParseParity(content);
    if (error != ERROR_NONE)
    {
#if (CONTROL_LED_ENABLED == true)
        LED_SignalError();
#endif
        return error;
    }

    error = PARSER_ParseFreeSpace(content);
	if (error != ERROR_NONE)
	{
#if (CONTROL_LED_ENABLED == true)
		LED_SignalError();
#endif
		return error;
	}
    return ERROR_NONE;
}

