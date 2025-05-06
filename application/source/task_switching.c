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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "mass_storage.h"


/*******************************************************************************
 * Functions
 ******************************************************************************/
/*
 * MISRA Deviation: Rule 8.4
 * Suppress: Function 'USB_State' Defined Without A Prototype In Scope.
 * Justification: The Prototype Is Declared In 'Task_Switching.h' And This Definition Is Consistent With It.
 */
/*lint -e957 */
usb_device_notification_t USB_State(usb_device_struct_t *pDeviceHandle)
{
	usb_device_ehci_state_struct_t *ehciState;
	ehciState = (usb_device_ehci_state_struct_t *)(pDeviceHandle->controllerHandle);

	if ((0U != (ehciState->registerBase->OTGSC & USBHS_OTGSC_BSE_MASK)))
    {
		return kUSB_DeviceNotifyDetach;
    }
	else
    {
		return kUSB_DeviceNotifyAttach;
    }
}
/*lint +e957 */
