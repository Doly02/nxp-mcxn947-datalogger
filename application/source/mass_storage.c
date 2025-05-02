/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      mass_storage.c
 *  Author:         Tomas Dolak
 *  Date:           06.02.2025
 *  Description:    Implements USB Mass Storage Application.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           mass_storage.c
 *  @author         Tomas Dolak
 *  @date           06.02.2025
 *  @brief          Implements USB Mass Storage Application.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "mass_storage.h"
#include "task_switching.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

extern usb_msc_struct_t g_msc;

extern SemaphoreHandle_t g_xSemRecord;

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
 * Code
 ******************************************************************************/
void MSC_DeviceMscApp(void)
{
    /* USB Mass Storage Expansion Can Be Located Here */
    return;
}

void MSC_DeviceMscAppTask(void)
{
    if (0u < g_msc.read_write_error)
    {
        return;
    }
    /*!
     * MISRA Deviation: Rule 2.2
     * Justification: Function 'MSC_DeviceMscApp' is called periodically and allows to expand Mass Storage.
     */
    /*lint -save -e522 */
    MSC_DeviceMscApp();
    /*lint +e522 */
}
