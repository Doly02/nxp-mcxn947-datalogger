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

/**
 * @brief LPUART7 IRQ Handler.
 *
 */
void LP_FLEXCOMM7_IRQHandler(void)
{
    uint8_t data;
    uint32_t stat;

    // Check for new data
    stat = LPUART_GetStatusFlags(LPUART7);
    if (kLPUART_RxDataRegFullFlag & stat)
    {
        data = LPUART_ReadByte(LPUART7);

        // Add data to FIFO
        uint16_t nextWriteIndex = (writeIndex + 1) % BUFFER_SIZE;
        if (nextWriteIndex != readIndex) // Check if FIFO is not full
        {
            swFifo[writeIndex] = data;
            writeIndex = nextWriteIndex;

            /* Actualizate Time Of Last Receivement */
            lastDataTick = xTaskGetTickCount();
            flushCompleted = false;
        }
    }

    // Clear interrupt flag
    LPUART_ClearStatusFlags(LPUART7, kLPUART_RxDataRegFullFlag);
    SDK_ISR_EXIT_BARRIER;
}

uint8_t CONSOLELOG_CreateFile(void)
{
    FRESULT error;
    char fileName[32];

    // Generate a new file name
    snprintf(fileName, sizeof(fileName), FILE_NAME_TEMPLATE, g_fileCounter++);

    // Open new file
    error = f_open(&g_fileObject, fileName, (FA_WRITE | FA_CREATE_ALWAYS));
    if (error != FR_OK)
    {
        PRINTF("ERR: Failed to create file %s. Error=%d\r\n", fileName, error);
        return E_FAULT;
    }

    g_currentFileSize = 0; // Reset file size
    PRINTF("INFO: Created new file %s.\r\n", fileName);
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
    UINT bytesWritten;

    while (readIndex != writeIndex)
    {
        /* Transfer From FIFO To Active ADMA Buffer */
        activeDmaBuffer[dmaIndex++] = swFifo[readIndex];
        readIndex = (readIndex + 1) % BUFFER_SIZE;

        if (dmaIndex == BLOCK_SIZE)	/* If Active Buffer Is Full */
        {
        	processDmaBuffer = activeDmaBuffer;
            dmaBufferReady = true;

            /* Switch To Second ADMA Buffer */
            activeDmaBuffer = (activeDmaBuffer == g_dmaBuffer1) ? g_dmaBuffer2 : g_dmaBuffer1;
            dmaIndex = 0; // Reset Index of Active DMA Buffer
        }
    }

    /* Process Full ADMA Buffer */
    if (dmaBufferReady && processDmaBuffer != NULL)
    {
        if (g_fileObject.obj.fs == NULL)
        {
            if (CONSOLELOG_CreateFile() != SUCCESS)
            {
                PRINTF("ERR: Failed to create new file.\r\n");
                return E_FAULT;
            }
        }

        error = f_write(&g_fileObject, processDmaBuffer, BLOCK_SIZE, &bytesWritten);
        if (error != FR_OK || bytesWritten != BLOCK_SIZE)
        {
            PRINTF("ERR: Failed to write data to file. Error=%d\r\n", error);
            f_close(&g_fileObject);
            g_fileObject.obj.fs = NULL;
            return E_FAULT;
        }

        g_currentFileSize += BLOCK_SIZE;

        if (g_currentFileSize >= MAX_FILE_SIZE)	/* MAX Size Detected */
        {
            PRINTF("INFO: File size limit reached. Closing file.\r\n");
            f_close(&g_fileObject);
            g_fileObject.obj.fs = NULL;
        }

        dmaBufferReady = false;    // Reset Flag of ADMA Buffer
        processDmaBuffer = NULL;   // Clear processDmaBuffer
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
		PRINTF("INFO: Flush triggered. Writing remaining data to file.\r\n");

		while (dmaIndex < BLOCK_SIZE)			// Fill Buffer With ' '
		{
			activeDmaBuffer[dmaIndex++] = ' ';
		}

		processDmaBuffer = activeDmaBuffer;
		dmaBufferReady = true;

		// Switch To Second Buffer
		activeDmaBuffer = (activeDmaBuffer == g_dmaBuffer1) ? g_dmaBuffer2 : g_dmaBuffer1;
		dmaIndex = 0;

		if (g_fileObject.obj.fs == NULL)
		{
			if (CONSOLELOG_CreateFile() != SUCCESS)
			{
				PRINTF("ERR: Failed to create new file during flush.\r\n");
				return;
			}
		}

		error = f_write(&g_fileObject, processDmaBuffer, BLOCK_SIZE, &bytesWritten);
		if (error != FR_OK || bytesWritten != BLOCK_SIZE)
		{
			PRINTF("ERR: Failed to write data to file during flush. Error=%d\r\n", error);
			f_close(&g_fileObject);
			g_fileObject.obj.fs = NULL;
			return;
		}

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

	/* De-Initialize UART */

	/* Close All Opened Files */
    if (g_fileObject.obj.fs != NULL)
    {
        error = f_close(&g_fileObject);
        if (error != FR_OK)
        {
            PRINTF("ERR: Failed to Close File. ERR=%d\r\n", error);
            return E_FAULT;
        }
    }


	/* Unmount File System From Logic Drive */
	error = f_mount(NULL, "", 0);
	if (error != FR_OK)
	{
		PRINTF("ERR: Failed to unmount filesystem. Error=%d\r\n", error);
		return E_FAULT;
	}

#if (true == DEBUG_ENABLED)

	PRINTF("DEBUG: Filesystem Deinicialized.\r\n");

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
