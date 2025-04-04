/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      task_switching.c
 *  Author:         Tomas Dolak
 *  Date:           14.09.2024
 *  Description:    Includes Implementation of Task For FreeRTOS.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           task_switching.c
 *  @author         Tomas Dolak
 *  @date           14.09.2024
 *  @brief          Includes Implementation of Task For FreeRTOS.
 * ****************************/

#ifndef TASK_SWITCHING_H_
#define TASK_SWITCHING_H_

#include "usb_device_dci.h"
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
usb_device_notification_t USB_State(usb_device_struct_t *pDeviceHandle);


#endif /* TASK_SWITCHING_H_ */
