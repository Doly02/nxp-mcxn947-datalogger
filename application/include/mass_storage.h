/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      mass_storage.h
 *  Author:         Tomas Dolak
 *  Date:           06.02.2025
 *  Description:    Header File For USB Mass Storage.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           mass_storage.h
 *  @author         Tomas Dolak
 *  @date           06.02.2025
 *  @brief          Header File For USB Mass Storage.
 * ****************************/

#ifndef MASS_STORAGE_H_
#define MASS_STORAGE_H_


/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "disk.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * Function Declarations
 ******************************************************************************/
usb_device_notification_t USB_State(usb_device_struct_t *pDeviceHandle);

void MSC_DeviceMscApp(void);

void MSC_DeviceMscAppTask(void);

#endif /* MASS_STORAGE_H_ */
