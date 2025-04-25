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

extern SemaphoreHandle_t g_xSemRecord;

extern usb_msc_struct_t g_msc;

extern SemaphoreHandle_t g_xSemMassStorage;
/*******************************************************************************
 * Interrupt Service Routines (ISRs)
 ******************************************************************************/

/*lint -e957 */
/* MISRA 2012 Rule 8.4:
 * Suppress: function 'USB1_HS_IRQHandler' defined without a prototype in scope.
 * USB1_HS_IRQHandler is declared WEAK in startup_mcxn947_cm33_core0.c and overridden here.
 */
void USB1_HS_IRQHandler(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    USB_DeviceEhciIsrFunction(g_msc.deviceHandle);

    if (USB_State(g_msc.deviceHandle) == kUSB_DeviceNotifyAttach)
	{
		(void)xSemaphoreGiveFromISR(g_xSemMassStorage, &xHigherPriorityTaskWoken);
	}
	else
	{
    	(void)xSemaphoreGiveFromISR(g_xSemRecord, &xHigherPriorityTaskWoken);
	}

    /**
     * Switch The Context From ISR To a Higher Priority Task,
     * Without Waiting For The Next Scheduler Tick.
     **/
    /* MISRA 2012 deviation: portYIELD_FROM_ISR expects BaseType_t, which is signed, not bool.
     * Justification: `portYIELD_FROM_ISR()` expects `BaseType_t` as per FreeRTOS convention, but MISRA
     * on the other hand would like conversion to boolean.
     * */
    /*lint -e9036 -e9048 -e9078 */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    /*lint +e9036 +e9048 +e9078 */
}
/*lint -e957 */

/*******************************************************************************
 * Functions
 ******************************************************************************/
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
