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

/*!
 * @brief	Mass Storage Descriptor.
 */
extern usb_msc_struct_t g_msc;


/*******************************************************************************
 * Implementation of Functions
 ******************************************************************************/


/*******************************************************************************
 * Implementation of Functions
 ******************************************************************************/

void msc_task(void *handle)
{
    while (true)
    {

    	/* Wait Here Till USB Is Attached */
    	(void)xSemaphoreTake(g_xSemMassStorage, portMAX_DELAY);

        /* Disable Recording & Close Current Record If It's Opened */
        UART_Disable();
#if (true == INFO_ENABLED)
        PRINTF("INFO: Disabled LPUART7\r\n");
#endif /* (true == INFO_ENABLED) */
        CONSOLELOG_PowerLossFlush();

        while (true)
        {
            MSC_DeviceMscAppTask();

            /* If USB Is Detached Then Switch To Record Task */
            if (USB_State(g_msc.deviceHandle) == kUSB_DeviceNotifyDetach)
            {
#if (true == INFO_ENABLED)
                PRINTF("INFO: MSC Task Ending - USB Detached\r\n");
#endif
                xSemaphoreGive(g_xSemRecord);
                break;
            }

            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}



void record_task(void *handle)
{
    error_t  retVal             = ERROR_UNKNOWN;
    uint32_t u32Baudrate        = 0UL;
    uint32_t u32FileSize        = 0UL;
    uint32_t u32CurrentBytes    = 0UL;
    uint32_t u32MaxBytes        = 0UL;
    uint32_t u32FreeSpaceSdCard = 0UL;

    /* Initialize SD Card and File System */
    USB_DeviceModeInit();
    retVal = CONSOLELOG_Init();
    if (retVal != ERROR_NONE) return;

    if (CONSOLELOG_ReadConfig() != ERROR_NONE)
    {
        u32Baudrate = DEFAULT_BAUDRATE;
        u32FileSize = DEFAULT_MAX_FILESIZE;
    }
    else
    {
        u32Baudrate = PARSER_GetBaudrate();
        u32FileSize = PARSER_GetFileSize();
    }

    while (true)
    {

    	xSemaphoreTake(g_xSemRecord, portMAX_DELAY);

		UART_Init(u32Baudrate);
		UART_Enable();
		PRINTF("INFO: UART Initialized for Record Mode\r\n");

        while (kUSB_DeviceNotifyAttach != USB_State(g_msc.deviceHandle))
        {
			if (ERROR_NONE != CONSOLELOG_Recording(u32FileSize))
			{
#if (CONTROL_LED_ENABLED == true)
				LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */
				ERR_HandleError();
			}

			if (ERROR_NONE != CONSOLELOG_Flush())
			{
#if (CONTROL_LED_ENABLED == true)
				LED_SignalError();
#endif /* (CONTROL_LED_ENABLED == true) */
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

#if (true == INFO_ENABLED)
			u32FreeSpaceSdCard = CONSOLELOG_GetFreeSpaceMB();
			if ((u32FreeSpaceSdCard <= PARSER_GetFreeSpaceLimitMB()) &&
				(0UL != PARSER_GetFreeSpaceLimitMB()))
			{
				LED_SignalLowMemory();
				PRINTF("DEBUG: Free Space: %d MB\r\n", u32FreeSpaceSdCard);
			}
#endif

        }

		xSemaphoreGive(g_xSemMassStorage);
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
