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

#ifndef APP_TASKS_H_
#define APP_TASKS_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "usb_msc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TASK_PRIO		(4u) //<! Task Priorities.

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief Task Responsible for Time Handling.
 */
void rtc_task(void *pvParameters);

#if 0
#if USB_DEVICE_CONFIG_USE_TASK

void USB_DeviceTask(void *handle);

#endif /* USB_DEVICE_CONFIG_USE_TASK */

void APP_task(void *handle);
#endif

#endif /* APP_TASKS_H_ */
