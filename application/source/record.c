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
#include <uart.h>
#include <time.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* buffer size (in byte) for read/write operations */
#define BUFFER_SIZE 		1024U

#define BLOCK_SIZE 			512U

#define MAX_FILE_SIZE 		8192 			// Maximum file size in bytes (8 KB)

#define FLUSH_TIMEOUT_TICKS pdMS_TO_TICKS(3000)
#define FILE_NAME_TEMPLATE 	"/log_%d.txt" 	// File name template
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
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
SDK_ALIGN(uint8_t g_dmaBuffer1[BLOCK_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);

SDK_ALIGN(uint8_t g_dmaBuffer2[BLOCK_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);

/* DMA Buffer State */
static uint8_t* activeDmaBuffer = g_dmaBuffer1;
static uint8_t* processDmaBuffer = NULL;
static uint16_t dmaIndex = 0; 					// Index in the active DMA buffer

static bool dmaBufferReady = false;

static uint16_t g_blockIndex = 0;
/*! @brief Data read from the card */

static TickType_t lastDataTick = 0;
/**
 * @defgroup UART Management
 * @brief Group Contains Variables For Recording From UART.
 * @{
 */

SDK_ALIGN(volatile uint8_t swFifo[BUFFER_SIZE], BOARD_SDMMC_DATA_BUFFER_ALIGN_SIZE);	// FIFO buffer

volatile uint16_t writeIndex 		= 0;       // Index for writing into FIFO
volatile uint16_t readIndex 		= 0;       // Index for reading from FIFO
volatile bool fifoOverflow 			= false;   // Flag indicating FIFO overflow

/** @} */ // End of UART Management group

static uint32_t g_currentFileSize 	= 0; 		// Tracks current file size
static uint16_t g_fileCounter 		= 1; 		// Counter for unique file names

static bool flushCompleted = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
DWORD get_fattime(void)
{
    irtc_datetime_t datetime;
    IRTC_GetDatetime(RTC, &datetime);

    return ((DWORD)(datetime.year - 1980) << 25) |
           ((DWORD)datetime.month << 21) |
           ((DWORD)datetime.day << 16) |
           ((DWORD)datetime.hour << 11) |
           ((DWORD)datetime.minute << 5) |
           ((DWORD)(datetime.second / 2));
}
/**
 * @brief LPUART7 IRQ Handler.
 *
 */
void LP_FLEXCOMM7_IRQHandler(void)
{
    uint8_t data;
    uint32_t stat;

    /* Check For New Data */
    stat = LPUART_GetStatusFlags(LPUART7);
    if (kLPUART_RxDataRegFullFlag & stat)
    {
        data = LPUART_ReadByte(LPUART7);
        /* Add Data To FIFO */
        uint16_t nextWriteIndex = (writeIndex + 1) % BUFFER_SIZE;
        if (nextWriteIndex != readIndex) // Check if FIFO is not full
        {
            swFifo[writeIndex] = data;
            writeIndex = nextWriteIndex;

            /* Update Time Of Last Receiving */
            lastDataTick = xTaskGetTickCount();
            flushCompleted = false;
        }
    }

    /* Clear Interrupt Flag */
    LPUART_ClearStatusFlags(LPUART7, kLPUART_RxDataRegFullFlag);
    SDK_ISR_EXIT_BARRIER;
}

uint8_t CONSOLELOG_CreateFile(void)
{
    FRESULT error;
    char fileName[32];
    irtc_datetime_t datetimeGet;		//<! To Store Time in File Meta-Data
    FILINFO fno;						//<! File Meta-Data

    /* Generate a New File Name */
    snprintf(fileName, sizeof(fileName), FILE_NAME_TEMPLATE, g_fileCounter++);

    /* Open New File */
    error = f_open(&g_fileObject, fileName, (FA_WRITE | FA_CREATE_ALWAYS));
    if (error != FR_OK)
    {
        PRINTF("ERR: Failed to create file %s. Error=%d\r\n", fileName, error);
        return E_FAULT;
    }

    /* Setup of File Meta-Data */
    IRTC_GetDatetime(RTC, &datetimeGet);
    fno.fdate = ((datetimeGet.year - 1980) << 9) | (datetimeGet.month << 5) | (datetimeGet.day);
    fno.ftime = (datetimeGet.hour << 11) | (datetimeGet.minute << 5) | (datetimeGet.second / 2);
    error = f_utime(fileName, &fno);
    if (FR_OK != error)
    {
        PRINTF("ERR: Failed to Set Meta-Data for %s. Error=%d\r\n", fileName, error);
        f_close(&g_fileObject);
        return E_FAULT;
    }

    g_currentFileSize = 0; // Reset file size
    PRINTF("INFO: Created New File %s.\r\n", fileName);
    return SUCCESS;
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

FRESULT CONSOLELOG_CheckFileSystem(void)
{
    DIR dir;
    /* Try To Open Root Folder */
    FRESULT res = f_opendir(&dir, "/");
    if (res == FR_OK)
    {
        f_closedir(&dir);
    }
    return res;
}

uint8_t CONSOLELOG_Init(void)
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
		return E_FAULT;
	}

#if (FF_FS_RPATH >= 2U)

	/* Setup of Actual Work Logic Disk */
    error = f_chdrive((char const *)&driverNumberBuffer[0U]);
    if (error)
    {
        PRINTF("ERR: Change drive failed.\r\n");
        return E_FAULT;
    }
#endif	/* (FF_FS_RPATH >= 2U) */

#if FF_USE_MKFS

    if (FR_OK != CONSOLELOG_CheckFileSystem())
    {

#if (true == DEBUG_ENABLED)
        PRINTF("DEBUG: File system not found. Formatting...\r\n");
#endif /* (true == DEBUG_ENABLED) */

        /* Make File System */
        if (f_mkfs(driverNumberBuffer, 0, work, sizeof work))
        {
            PRINTF("ERR: Init File System Failed.\r\n");
            return E_FAULT;
        }
    }

#endif 	/* FF_USE_MKFS */

    return SUCCESS;
}

uint8_t CONSOLELOG_Recording(void)
{
    FRESULT error;
    UINT bytesWritten;               //<! Bytes Written Into SD Card
    irtc_datetime_t datetimeGet;     //<! Actual Time From IRTC
    static uint8_t lastChar = 0;     //<! Last Character From Previous DMA Buffer

    static char timeString[12];
    static uint8_t timeLength = 0;

    while (readIndex != writeIndex)
    {
        /* Loads One Char From FIFO And Stores The Char Into Active DMA Buffer */
        uint8_t currentChar = swFifo[readIndex];
        readIndex = (readIndex + 1) % BUFFER_SIZE;

        activeDmaBuffer[dmaIndex++] = currentChar;

        /* Check If The CRLF Is Not Divided Into Two DMA Buffers */
        if ((lastChar == '\r' && currentChar == '\n') ||
            (dmaIndex >= 2 &&
             activeDmaBuffer[dmaIndex - 2] == '\r' &&
             activeDmaBuffer[dmaIndex - 1] == '\n'))
        {

        	IRTC_GetDatetime(RTC, &datetimeGet);
            timeLength = (uint8_t)snprintf(timeString, sizeof(timeString), "(%02d:%02d:%02d) ", datetimeGet.hour, datetimeGet.minute, datetimeGet.second);

            /* Addition of Time Mark To The DMA Buffer */
            for (uint8_t i = 0; i < timeLength; i++)
            {
                if (BLOCK_SIZE > dmaIndex)	// If DMA Buffer is Not Full
                {
                    activeDmaBuffer[dmaIndex++] = timeString[i];
                }
                else
                {
                    /* Switch To New DMA Buffer */
                	processDmaBuffer = activeDmaBuffer;
                    dmaBufferReady = true;

                    activeDmaBuffer = (activeDmaBuffer == g_dmaBuffer1) ? g_dmaBuffer2 : g_dmaBuffer1;
                    dmaIndex = 0;

                    /* Continue in Addition of Time Mark */
                    activeDmaBuffer[dmaIndex++] = timeString[i];
                }
            }
        }

        lastChar = currentChar; // Current Last Character For Next Buffer

        /* Check If DMA Buffer Is Full */
        if (BLOCK_SIZE == dmaIndex)
        {
            processDmaBuffer = activeDmaBuffer;
            dmaBufferReady = true;

            /* Switch on Next DMA Buffer */
            activeDmaBuffer = (activeDmaBuffer == g_dmaBuffer1) ? g_dmaBuffer2 : g_dmaBuffer1;
            dmaIndex = 0;
        }
    }

    /* Process Full DMA Buffer */
    if (dmaBufferReady && NULL != processDmaBuffer)
    {
        if (NULL == g_fileObject.obj.fs)
        {
            if (SUCCESS != CONSOLELOG_CreateFile())
            {
                PRINTF("ERR: Failed to create new file.\r\n");
                return E_FAULT;
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
        if (0x0 != (stat_reg & 0xC))
        {
            PRINTF("ERR: Failed to write data to file. Error=%d\r\n", error);
            f_close(&g_fileObject);
            g_fileObject.obj.fs = NULL;
            return E_FAULT;
        }
        error = f_write(&g_fileObject, processDmaBuffer, BLOCK_SIZE, &bytesWritten);

        g_currentFileSize += BLOCK_SIZE;
        if (g_currentFileSize >= MAX_FILE_SIZE)
        {
            PRINTF("INFO: File size limit reached. Closing file.\r\n");
            f_close(&g_fileObject);
            g_fileObject.obj.fs = NULL;
        }

        dmaBufferReady = false;     // Reset Flag of ADMA Buffer
        processDmaBuffer = NULL;    // Clear processDmaBuffer
    }

    return SUCCESS;
}

void CONSOLELOG_Flush(void)
{
	FRESULT error;
	UINT bytesWritten;

	uint32_t currentTick = xTaskGetTickCount();
	if ((currentTick - lastDataTick >= FLUSH_TIMEOUT_TICKS) && dmaIndex > 0)
	{
		PRINTF("INFO: Flush Triggered. Writing Remaining Data To File.\r\n");

		while (dmaIndex < BLOCK_SIZE)			// Fill Buffer With ' '
		{
			activeDmaBuffer[dmaIndex++] = ' ';
		}

		processDmaBuffer = activeDmaBuffer;
		dmaBufferReady = true;

		// Switch To Second Buffer
		activeDmaBuffer = (activeDmaBuffer == g_dmaBuffer1) ? g_dmaBuffer2 : g_dmaBuffer1;
		dmaIndex = 0;

		if (NULL == g_fileObject.obj.fs)
		{
			if (SUCCESS != CONSOLELOG_CreateFile())
			{
				PRINTF("ERR: Failed to Create New File During Flush.\r\n");
				return;
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
		if (0x0 != (stat_reg & 0xC))
		{
			PRINTF("ERR: Failed to Write Data To File During Flush. Error=%d\r\n", error);
			f_close(&g_fileObject);
			g_fileObject.obj.fs = NULL;
			return;
		}
		error = f_write(&g_fileObject, processDmaBuffer, BLOCK_SIZE, &bytesWritten);
		g_currentFileSize += BLOCK_SIZE;

		PRINTF("INFO: Closing File\r\n");
		f_close(&g_fileObject);
		g_fileObject.obj.fs = NULL;

		dmaBufferReady = false;
		processDmaBuffer = NULL;
		flushCompleted = true;
	}
}

uint8_t CONSOLELOG_Deinit(void)
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
            return E_FAULT;
        }
    }


	/* Unmount File System From Logic Drive */
	error = f_mount(NULL, "", 0);
	if (error != FR_OK)
	{
		PRINTF("ERR: Failed To Unmount File System. Error=%d\r\n", error);
		return E_FAULT;
	}

#if (true == DEBUG_ENABLED)

	PRINTF("DEBUG: File System De-Inicialized.\r\n");

#endif /* (true == DEBUG_ENABLED) */

	return SUCCESS;
}


uint8_t CONSOLELOG_ReadConfig(void)
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
        return E_FAULT;
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
    		if (SUCCESS == strcmp(fno.fname, CONFIG_FILE))	// If .config File
			{
    			PRINTF("DEBUG: Found .config File: %s\r\n", fno.fname);

    			error = f_open(&configFile, CONFIG_FILE, FA_READ);
    			if (FR_OK != error)
				{
    				PRINTF("ERR: Failed to open .config file. ERR=%d\r\n", error);
					f_closedir(&dir); 	// Close Root Directory
					return E_FAULT;
				}
    			memset(g_dmaBuffer1, 0, sizeof(g_dmaBuffer1));
    			error = f_read(&configFile, g_dmaBuffer1, sizeof(g_dmaBuffer1) - 1, &bytesRead);
				if (FR_OK != error)
				{
					PRINTF("ERR: Failed To Read .config File. ERR=%d\r\n", error);
					f_close(&configFile);
					f_closedir(&dir);
					return E_FAULT;
				}

				if (SUCCESS != CONSOLELOG_ProccessConfigFile((const char *)g_dmaBuffer1))
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

uint8_t CONSOLELOG_ProccessConfigFile(const char *content)
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
