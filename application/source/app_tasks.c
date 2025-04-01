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
 * @defgroup TaskManagement Task Management
 * @brief Group Contains Task Management Variables and Metadata.
 * @{
 */

extern volatile uint8_t usbAttached;
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

extern SemaphoreHandle_t g_TaskMutex;

extern TaskHandle_t mscTaskHandle;

extern TaskHandle_t recordTaskHandle;

/** @} */ // End of TaskManagement group

/*******************************************************************************
 * Implementation of Functions
 ******************************************************************************/

void msc_task(void *handle)
{
	while (1)
	{

		if (0 == usbAttached)
		{
			/* Put Back Scheduler To Record Task */
            taskYIELD();
            continue;
		}

        if (xSemaphoreTake(g_TaskMutex, portMAX_DELAY) == pdTRUE)
        {

#if (true == DEBUG_ENABLED)
            PRINTF("DEBUG: MSC Task Running!\r\n");
#endif /* (true == DEBUG_ENABLED) */

            while (true == usbAttached)
            {
                USB_DeviceMscAppTask();
                taskYIELD();
            }
            xSemaphoreGive(g_TaskMutex); // After Un-Plug of USB, Release of Semaphore For record_task
        }
	}
}

void record_task(void *handle)
{
	error_t u16RetVal 			= ERROR_UNKNOWN;
	uint32_t u32Baudrate 		= 0;
	uint32_t u32FileSize 		= 0;
	uint32_t u32CurrentBytes 	= 0;
	uint32_t u32MaxBytes 		= 0;
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
		if (usbAttached == 1)
		{
			// USB connected - wait for it to disconnect
			taskYIELD();
			continue;
		}

		// Wait for the traffic light - only one task can run
		if (xSemaphoreTake(g_TaskMutex, portMAX_DELAY) == pdTRUE)
		{
			if (bUartInitialized == false)
			{
				UART_Init(u32Baudrate);
				UART_Enable();
				LED_ClearSignalFlush(); // resets the LED signalling
				bUartInitialized = true;
			}

			// Loop to record data while USB is not connected
			while (usbAttached == 0)
			{
				if (ERROR_NONE != (error_t)CONSOLELOG_Recording(u32FileSize))
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
				}
#endif

				if (ERROR_NONE != (error_t)CONSOLELOG_Flush())
				{
					ERR_HandleError();
				}

				// Check if the USB was connected during the run, exit record mode
				if (usbAttached == 1)
				{
#if (true == DEBUG_ENABLED)
					PRINTF("DEBUG: USB připojeno během záznamu – přepínám režim.\r\n");
#endif
					break; // break the loop and release the mutex
				}

				vTaskDelay(pdMS_TO_TICKS(100));
			}

			UART_Disable();
			bUartInitialized = false;

			// Release the mutex before switching to MSC mode
			xSemaphoreGive(g_TaskMutex);
		}
	}
}

#if 0
void record_task(void *handle)
{

	error_t u16RetVal 			= ERROR_UNKNOWN;
	uint32_t u32Baudrate 		= 0;
	uint32_t u32FileSize 		= 0;
    uint32_t u32CurrentBytes 	= 0;
    uint32_t u32MaxBytes 		= 0;
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
#endif /* (true == INFO_ENABLED) */
	}
	else
	{
#if (true == INFO_ENABLED)
		PRINTF("INFO: Configuration File Found\r\n");
		PRINTF("INFO: Configuration:\r\n");
#endif /* (true == INFO_ENABLED) */
		u32Baudrate = CONSOLELOG_GetBaudrate();
		u32FileSize = CONSOLELOG_GetFileSize();
	}
#if (true == INFO_ENABLED)
	PRINTF("Baudrate=%d\r\n", u32Baudrate);
	PRINTF("File Size=%d\r\n", u32FileSize);
#endif /* (true == INFO_ENABLED) */

    while (1)
    {
        if (1 == usbAttached)
        {
        	/* Return Scheduler To Other Tasks */
            taskYIELD();
            continue;
        }
        if (false == bUartInitialized)
        {
#if (true == INFO_ENABLED)
            PRINTF("INFO: Initializing UART...\r\n");
#endif /* (true == INFO_ENABLED) */

            UART_Init(u32Baudrate);
            UART_Enable();
            bUartInitialized = true;
        }
        usbAttached = 0;
        if (1 == usbAttached)
        {
        	/* Stop LPUART */
        	UART_Disable();
        	bUartInitialized = false;

#if (true == INFO_ENABLED)
        	PRINTF("INFO: Disabled LPUART7\r\n");
#endif /* (true == INFO_ENABLED) */

        	/*@note If The Task Has No Things To Do -> Delay
        	 *		One Task Cannot Stop Another Task
        	 */
        }
        if (ERROR_NONE != (error_t)CONSOLELOG_Recording(u32FileSize))
        {
        	/* Look At The Error */
        	ERR_HandleError();
        }

#if (CONTROL_LED_ENABLED == true)
        u32CurrentBytes = CONSOLELOG_GetTransferedBytes();
        u32MaxBytes 	= CONSOLELOG_GetMaxBytes();
        if (u32CurrentBytes >= u32MaxBytes)
        {
        	LED_SignalRecording();
        	CONSOLELOG_ClearTransferedBytes();
        }
#endif /* (CONTROL_LED_ENABLED == true) */

        if (ERROR_NONE != (error_t)CONSOLELOG_Flush())
        {
        	/* Look At The Error */
        	ERR_HandleError();
        }

    }
}
#endif

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
