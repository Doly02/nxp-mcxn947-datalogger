/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      usb_vbus_detection.c
 *  Author:         Tomas Dolak
 *  Date:           19.10.2024
 *  Description:    Implements USB VBUS Detection.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           usb_vbus_detection.c
 *  @author         Tomas Dolak
 *  @date           19.10.2024
 *  @brief          Implements USB VBUS Detection.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "usb_vbus_detection.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Definition of Pins */
#define USB0_VBUS_DET_PIN 		12U
#define USB0_VBUS_DET_PORT 		4
#define USB0_VBUS_DET_GPIO 		GPIO4
#define USB0_VBUS_IRQ 			GPIO40_IRQn

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
#if 0
usb_device_notification_t USB_State(usb_device_struct_t *pDeviceHandle)
{
	usb_device_ehci_state_struct_t *ehciState;
	ehciState = (usb_device_ehci_state_struct_t *)(handle->controllerHandle);
	if (0U != ehciState->isResetting)
	{
		return kUSB_DeviceNotifyAttach;
	}
	return kUSB_DeviceNotifyDetach;
}
#endif
