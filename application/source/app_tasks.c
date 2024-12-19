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
        xSemaphoreTake(g_TaskMutex, portMAX_DELAY);

        if (1 == usbAttached)
        {
            PRINTF("MSC Task Running!\r\n");
            while (usbAttached)
            {
                USB_DeviceMscAppTask();
                taskYIELD();
            }
        }
        /* Switch Back To record_task, USB Not Connected */
        vTaskSuspend(NULL);
        vTaskResume(recordTaskHandle);
	}
}


void record_task(void *handle)
{
	uint8_t retVal 		= 1;
	uint32_t baudrate 	= 0;

	USB_DeviceModeInit();

#if (true == DEBUG_ENABLED)

	PRINTF("DEBUG: Initialize File System\r\n");

#endif /* (true == DEBUG_ENABLED) */

	/* Initialize File System */
#if 1
	retVal = RECORD_Init();
	if (SUCCESS != retVal)
	{
		return;
	}

	/* Read Configuration File */
	if (0 != RECORD_ReadConfig())
	{
		return;
	}
	baudrate = RECORD_GetBaudrate();
#endif
	baudrate = 320400;

	/* Initialize Application UART */
	UART_Init(baudrate);
	UART_Enable();

	/*
	retVal = RECORD_Deinit();
	if (0 != retVal)
	{
		return;
	}
	 */
    while (1)
    {
        usbAttached = 0;
        if (1 == usbAttached)
        {
            /* Pokud je USB připojeno, pozastavit tuto úlohu a přepnout na msc_task */
            vTaskSuspend(NULL);  			// Pozastavit tuto úlohu
            vTaskResume(mscTaskHandle);  	// Obnovit msc_task
            /* Uloha nema co delat -> Delay */
            /* Jedna uloha nesmi zastavit tu druhou */
        }
        RECORD_Start();
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
