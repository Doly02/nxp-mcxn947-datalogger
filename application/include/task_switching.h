/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      task_switching.c
 *  Author:         Tomas Dolak
 *  Date:           14.09.2024
 *  Description:    Includes Implementation of Task Switching (Record / USB Mass Storage).
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           task_switching.c
 *  @author         Tomas Dolak
 *  @date           14.09.2024
 *  @brief          Includes Implementation of Task Switching (Record / USB Mass Storage).
 * ****************************/

#ifndef TASK_SWITCHING_H_
#define TASK_SWITCHING_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "usb_device_dci.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
/**
 * @brief	Mass Storage Descriptor.
 */
extern usb_msc_struct_t g_msc;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/**
 * @brief 		Checks If Application USB Is Attached To Digital Data Logger.
 * @details		Checks Thru USB OTG (USB On-To-Go) SC Register.
 * @param[in]	pDeviceHandle Pointer to USB Device Handle (e.g. Mass Storage Handle).
 *
 * @returns		kUSB_DeviceNotifyAttach if USB Is Attached Otherwise Returns kUSB_DeviceNotifyDetach.
 */
usb_device_notification_t USB_State(usb_device_struct_t *pDeviceHandle);


#endif /* TASK_SWITCHING_H_ */
