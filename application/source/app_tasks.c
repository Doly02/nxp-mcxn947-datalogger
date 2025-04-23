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
#include "mass_storage.h"

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
 * @brief	TCB (Task Control Block) - Meta-Data of IDLE Task.
 */
static StaticTask_t xIdleTaskTCB;
/*!
 * @brief 	Stack for Static Idle Task.
 */
static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

/*!
 * @brief	TCB (Task Control Block) - Meta-Data of Timer Task.
 */
static StaticTask_t xTimerTaskTCB;

/*!
 * @brief 	Stack for Static Timer Task.
 */
/* MISRA Deviation Note:
 * Rule: MISRA 2012 Rule 10.4 [Required]
 * Justification: The macro 'configTIMER_TASK_STACK_DEPTH' is defined by the FreeRTOS kernel
 * configuration as an integer constant.
 */
/*lint -e9029 MISRA Deviation: standard FreeRTOS macro cast to uint32_t */
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
/*lint +e9029 */

/** @} */ // End of TaskManagement group

/*******************************************************************************
 * Implementation of Functions
 ******************************************************************************/

void msc_task(void *handle)
{
    while (true)
    {
    	/* Wait For Attachment of USB */
        (void)xSemaphoreTake(g_xSemMassStorage, portMAX_DELAY);

        /* De-Inicializace of UART */
        UART_Disable();
#if (true == INFO_ENABLED)
        PRINTF("INFO: Disabled LPUART7\r\n");
#endif /* (true == INFO_ENABLED) */

        while (true)
        {
        	MSC_DeviceMscAppTask();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
	}
}

void record_task(void *handle)
{
	error_t  retVal 			= ERROR_UNKNOWN;
	uint32_t u32Baudrate 		= 0UL;
	uint32_t u32FileSize 		= 0UL;
	uint32_t u32CurrentBytes 	= 0UL;
	uint32_t u32MaxBytes 		= 0UL;
	uint32_t u32FreeSpaceSdCard = 0UL;
	bool bUartInitialized 		= false;

	/* Initialize's The SDHC Card */
	USB_DeviceModeInit();

#if (true == DEBUG_ENABLED)
	PRINTF("DEBUG: Initialize File System\r\n");
#endif /* (true == DEBUG_ENABLED) */

	/* Initialize File System */
	retVal = (error_t)CONSOLELOG_Init();
	if (ERROR_NONE != retVal)
	{
		return;
	}

	/* Read Configuration File */
	if (ERROR_NONE != CONSOLELOG_ReadConfig())
	{
		u32Baudrate = DEFAULT_BAUDRATE;
		u32FileSize = DEFAULT_MAX_FILESIZE;
#if (true == INFO_ENABLED)
		PRINTF("INFO: Default Configuration:\r\n");
#endif
	}
	else
	{
#if (true == INFO_ENABLED)
		PRINTF("INFO: Configuration:\r\n");
#endif
		u32Baudrate = PARSER_GetBaudrate();
		u32FileSize = PARSER_GetFileSize();
	}

#if (true == INFO_ENABLED)
	PRINTF("Baudrate=%d\r\n", u32Baudrate);
	PRINTF("File Size=%d\r\n", u32FileSize);
#endif

    while (true)
    {
        /* Take Task Priority */
        (void)xSemaphoreTake(g_xSemRecord, portMAX_DELAY);

        UART_Init(u32Baudrate);
        UART_Enable();
        bUartInitialized = true;	/*TODO: */

#if (true == DEBUG_ENABLED)
        PRINTF("DEBUG: RECORD Task Running...\r\n");
#endif

        while (true)
        {
            if (ERROR_NONE != CONSOLELOG_Recording(u32FileSize))
            {
                ERR_HandleError();
            }

#if (CONTROL_LED_ENABLED == true)
            u32CurrentBytes = CONSOLELOG_GetTransferedBytes();
            u32MaxBytes = PARSER_GetMaxBytes();
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

#if (true == INFO_ENABLED)
            u32FreeSpaceSdCard = CONSOLELOG_GetFreeSpaceMB();
            if (LOW_MEMORY_MB >= u32FreeSpaceSdCard)
            {
            	LED_SignalLowMemory();
            	PRINTF("DEBUG: Free Space: %d\r\n", u32FreeSpaceSdCard);
            }
#endif /* (true == INFO_ENABLED) */
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

    /* MISRA 2012 deviation: configTIMER_TASK_STACK_DEPTH is defined by FreeRTOS as a signed macro.
     * Conversion to uint32_t is intentional and safe in this context. Fixing the definition is not possible.
     */
    /*lint -e9029 */
    *pulIdleTaskStackSize = (uint32_t)configMINIMAL_STACK_SIZE;
    /*lint +e9029 */
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* MISRA 2012 deviation: configTIMER_TASK_STACK_DEPTH is defined by FreeRTOS as a signed macro.
     * Conversion to uint32_t is intentional and safe in this context. Fixing the definition is not possible.
     */
    /*lint -e9029 */
    *pulTimerTaskStackSize = (uint32_t)configTIMER_TASK_STACK_DEPTH;
    /*lint +e9029 */
}
