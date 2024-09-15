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

#ifndef TASKS_H_
#define TASKS_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "usb_msc.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if USB_DEVICE_CONFIG_USE_TASK

void USB_DeviceTask(void *handle);

#endif /* USB_DEVICE_CONFIG_USE_TASK */

void APP_task(void *handle);

#endif /* TASKS_H_ */
