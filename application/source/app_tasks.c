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
/*******************************************************************************
 * Global Variables.
 ******************************************************************************/
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

/*******************************************************************************
 * Implementation of Functions
 ******************************************************************************/
/*!
 * @brief Task Responsible for Time Handling.
 */
void rtc_task(void *pvParameters)
{
	uint8_t retVal = E_FAULT;
	RTC_time_t actTime;
	RTC_date_t actDate;

    retVal = RTC_GetState();
    if (OSC_STOPPED == retVal)	// If The Oscillator Was Stopped -> Set Time & Date
    {
    	/* Set Default Time & Date (Prepare Variables) */
    	RTC_SetDateDefault(&actDate);
    	RTC_SetTimeDefault(&actTime);
		/* Set Time & Date Into RTC */
		RTC_SetDate(&actDate);
		RTC_SetTime(&actTime);

		RTC_SetOscState(OSC_OK);
    }

    /* Get Current Time */
    memset(&actTime, 0U, sizeof(actTime));
    memset(&actDate, 0U, sizeof(actDate));

    RTC_GetTime(&actTime);
    RTC_GetDate(&actDate);
    /* Print The Time From RTC */
	PRINTF("Current Date: %d\r\n",actDate.day);

	/* Finish The Task */
	vTaskSuspend(NULL);

}

void msc_task(void *handle)
{
	while (1)
	{
        xSemaphoreTake(g_TaskMutex, portMAX_DELAY);

        if (1 == usbAttached)
        {
            PRINTF("MSC Task Running!\r\n");
            while (usbAttached)  // Přidáno: Spustit MSC task, dokud je USB připojeno
            {
                USB_DeviceMscAppTask();
                taskYIELD();  // Přidáno: Umožnit ostatním úlohám běžet
            }
        }
        /* Switch Back To record_task, USB Not Connected */
        vTaskSuspend(NULL);
        vTaskResume(recordTaskHandle);
	}
}


void record_task(void *handle)
{
	USB_DeviceApplicationInit();

#if 0
    while (1)
    {
        PRINTF("Record Task Running!\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));  // Zpoždění, aby nebylo příliš rychlé
    }
#else
    while (1)
    {
        PRINTF("Record Task Enabled!\r\n");
        PRINTF("USB Attached state: %d\r\n", usbAttached);
        usbAttached = 0;
        if (1 == usbAttached)
        {
            /* Pokud je USB připojeno, pozastavit tuto úlohu a přepnout na msc_task */
            vTaskSuspend(NULL);  // Pozastavit tuto úlohu
            vTaskResume(mscTaskHandle);  // Obnovit msc_task
        }
        PRINTF("USB Attached state after if: %d\r\n", usbAttached);

		/* Krátké zpoždění pro uvolnění času ostatním úlohám */
		vTaskDelay(pdMS_TO_TICKS(10));
    }
#endif
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
