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

extern usb_msc_struct_t g_msc;

extern SemaphoreHandle_t g_xSemRecord;

extern SemaphoreHandle_t g_xSemMassStorage;

static bool bMscInitialized = false;

/*******************************************************************************
 * Interrupt Service Routines (ISRs)
 ******************************************************************************/
void USB1_HS_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (false == bMscInitialized)
    {
        bMscInitialized = true;
    }
    USB_DeviceEhciIsrFunction(g_msc.deviceHandle);

    if (USB_State(g_msc.deviceHandle) == kUSB_DeviceNotifyAttach)
    {
    	/* USB Attached, Activation of Mass Storage Task */
        xSemaphoreGiveFromISR(g_xSemMassStorage, &xHigherPriorityTaskWoken);
    }
    else
    {
        /* USB Detached, Activation of Record Task */
        xSemaphoreGiveFromISR(g_xSemRecord, &xHigherPriorityTaskWoken);
    }

    /**
     * Switch The Context From ISR To a Higher Priority Task,
     * Without Waiting For The Next Scheduler Tick.
     **/
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*******************************************************************************
 * Functions
 ******************************************************************************/
usb_device_notification_t USB_State(usb_device_struct_t *pDeviceHandle)
{
	usb_device_ehci_state_struct_t *ehciState;
	ehciState = (usb_device_ehci_state_struct_t *)(pDeviceHandle->controllerHandle);

	if (0U != (ehciState->registerBase->OTGSC & USBHS_OTGSC_BSVIS_MASK))
    {
		return kUSB_DeviceNotifyDetach;
    }
	if (0U != (ehciState->registerBase->OTGSC & USBHS_OTGSC_BSV_MASK))
    {
		return kUSB_DeviceNotifyAttach;
    }
	return kUSB_DeviceNotifyDetach;
}
