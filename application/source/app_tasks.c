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
            taskYIELD(); // Vrať plánovač k jiným úlohám
            continue;
		}

        // xSemaphoreTake(g_TaskMutex, portMAX_DELAY);

        if (1 == usbAttached)
        {
            PRINTF("MSC Task Running!\r\n");
            while (usbAttached)
            {
                USB_DeviceMscAppTask();
                taskYIELD();
            }
        }
	}
}


void record_task(void *handle)
{
	assert(NULL != handle);

	error_t u16RetVal 		= ERROR_UNKNOWN;
	uint32_t u32Baudrate 	= 0;
	bool bUartInitialized 	= false;

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
	/*
	if (0 != CONSOLELOG_ReadConfig())
	{
		return;
	}
	u32Baudrate = CONSOLELOG_GetBaudrate(); */
	u32Baudrate = 320400;

	/* Initialize Application UART */

    while (1)
    {
        if (1 == usbAttached)
        {
            taskYIELD(); // Return Scheduler To Other Tasks
            continue;
        }
        if (false == bUartInitialized)
        {
#if (true == DEBUG_ENABLED)
            PRINTF("INFO: Reinitializing UART...\r\n");
#endif /* (true == DEBUG_ENABLED) */

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

#if (true == DEBUG_ENABLED)
        	PRINTF("INFO: Disabled LPUART7\r\n");
#endif /* (true == DEBUG_ENABLED) */
            /* Uloha nema co delat -> Delay */
            /* Jedna uloha nesmi zastavit tu druhou */
        }
        if (ERROR_NONE != (error_t)CONSOLELOG_Recording())
        {
        	/* Look At The Error */
        	ERR_HandleError();
        }

        CONSOLELOG_Flush();
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
