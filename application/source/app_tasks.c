/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      tasks.c
 *  Author:         Tomas Dolak
 *  Date:           14.09.2024
 *  Description:    Includes Implementation of Task For FreeRTOS.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           tasks.c
 *  @author         Tomas Dolak
 *  @date           14.09.2024
 *  @brief          Includes Implementation of Task For FreeRTOS.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "app_tasks.h"
#include "rtc_ds3231.h"
#include "semphr.h"

#include "record.h"
/*******************************************************************************
 * Global Variables.
 ******************************************************************************/
/**
 * @defgroup 	TaskManagement Task Management
 * @brief 		Group Contains Task Management Variables and Metadata.
 * @{
 */

/*!
 * @brief	TCB (Task Control Block) - Metadata of IDLE Task.
 */
static StaticTask_t xIdleTaskTCB;
/*!
 * @brief 	Buffer for Static Idle Task.
 */
static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

static StaticTask_t xTimerTaskTCB;

static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

//extern SemaphoreHandle_t g_TaskMutex;

extern SemaphoreHandle_t g_xSemRecord;

extern SemaphoreHandle_t g_xSemMassStorage;

extern TaskHandle_t g_xMscTaskHandle;

extern TaskHandle_t g_xRecordTaskHandle;

/** @} */ // End of TaskManagement group

/*******************************************************************************
 * Implementation of Functions
 ******************************************************************************/

void msc_task(void *handle)
{
    while (1)
    {
    	/* Wait For Attachment of USB */
        xSemaphoreTake(g_xSemMassStorage, portMAX_DELAY);

        /* De-Inicializace of UART */
        UART_Disable();
#if (true == INFO_ENABLED)
        PRINTF("INFO: Disabled LPUART7\r\n");
#endif /* (true == INFO_ENABLED) */

        while (1)
        {
            USB_DeviceMscAppTask();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
	}
}

void record_task(void *handle)
{
	error_t u16RetVal 			= ERROR_UNKNOWN;
	uint32_t u32Baudrate 		= 0U;
	uint32_t u32FileSize 		= 0U;
	uint32_t u32CurrentBytes 	= 0U;
	uint32_t u32MaxBytes 		= 0U;
	bool bUartInitialized 		= false;

	/* Initialize's The SDHC Card */
	USB_DeviceModeInit();

#if (true == DEBUG_ENABLED)
	PRINTF("DEBUG: Initialize File System\r\n");
#endif /* (true == DEBUG_ENABLED) */

	/* Initialize File System */
	u16RetVal = (error_t)CONSOLELOG_Init();
	if (ERROR_NONE != u16RetVal)
	{
		return;
	}

	/* Read Configuration File */
	if (ERROR_NONE != CONSOLELOG_ReadConfig())
	{
		u32Baudrate = DEFAULT_BAUDRATE;
		u32FileSize = DEFAULT_MAX_FILESIZE;
#if (true == INFO_ENABLED)
		PRINTF("INFO: Configuration File (config) Not Found\r\n");
		PRINTF("INFO: Default Configuration:\r\n");
#endif
	}
	else
	{
#if (true == INFO_ENABLED)
		PRINTF("INFO: Configuration File Found\r\n");
		PRINTF("INFO: Configuration:\r\n");
#endif
		u32Baudrate = CONSOLELOG_GetBaudrate();
		u32FileSize = CONSOLELOG_GetFileSize();
	}

#if (true == INFO_ENABLED)
	PRINTF("Baudrate=%d\r\n", u32Baudrate);
	PRINTF("File Size=%d\r\n", u32FileSize);
#endif

    while (1)
    {
        /* Take Task Priority */
        xSemaphoreTake(g_xSemRecord, portMAX_DELAY);

#if (true == INFO_ENABLED)
		PRINTF("INFO: Initializing UART...\r\n");
#endif /* (true == INFO_ENABLED) */

        UART_Init(u32Baudrate);
        UART_Enable();
        bUartInitialized = true;

#if (true == DEBUG_ENABLED)
        PRINTF("DEBUG: RECORD Task Running...\r\n");
#endif

        while (1)
        {
            if (ERROR_NONE != CONSOLELOG_Recording(u32FileSize))
            {
                ERR_HandleError();
            }

#if (CONTROL_LED_ENABLED == true)
            u32CurrentBytes = CONSOLELOG_GetTransferedBytes();
            u32MaxBytes = CONSOLELOG_GetMaxBytes();
            if (u32CurrentBytes >= u32MaxBytes)
            {
                LED_SignalRecording();
                CONSOLELOG_ClearTransferedBytes();
                LED_ClearSignalFlush();
            }
#endif

            if (ERROR_NONE != CONSOLELOG_Flush())
            {
                ERR_HandleError();
            }

            vTaskDelay(pdMS_TO_TICKS(100));
        }
	}
}

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
