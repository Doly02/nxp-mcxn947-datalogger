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
/*******************************************************************************
 * Global Variables.
 ******************************************************************************/

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



#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTask(void *handle)
{
    while (1U)
    {
        USB_DeviceTaskFn(handle);
    }
}
#endif

void APP_task(void *handle)
{
    USB_DeviceApplicationInit();

#if USB_DEVICE_CONFIG_USE_TASK
    if (g_msc.deviceHandle)
    {
        if (xTaskCreate(USB_DeviceTask,                  /* pointer to the task */
                        (char const *)"usb device task", /* task name for kernel awareness debugging */
                        5000L / sizeof(portSTACK_TYPE),  /* task stack size */
                        g_msc.deviceHandle,              /* optional task startup argument */
                        5,                               /* initial priority */
                        &g_msc.device_task_handle        /* optional task handle to create */
                        ) != pdPASS)
        {
            usb_echo("usb device task create failed!\r\n");
            return;
        }
    }
#endif

    while (1)
    {
    }
}
