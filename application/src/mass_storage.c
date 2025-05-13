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

extern SemaphoreHandle_t g_xSemRecord;

extern SemaphoreHandle_t g_xSemMassStorage;

/*******************************************************************************
 * Interrupt Service Routines (ISRs)
 ******************************************************************************/

/*lint -e957 */
/* Rule: MISRA 2012 Rule 8.4 [Required]
 * Suppress: Function 'USB1_HS_IRQHandler' Defined Without a Prototype in Scope.
 * Justification: USB1_HS_IRQHandler is Declared WEAK in startup_mcxn947_cm33_core0.c and Overridden Here.
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
    /*
     * MISRA Deviation: Rule 10.3, Rule 14.4, Rule 11.4
     * Suppress: Conversion From Boolean To Signed Type, Use Of Non-Boolean In Conditional Context, And Pointer-To-Integer Conversion.
     * Justification: `portYIELD_FROM_ISR()` Expects `BaseType_t` as Per FreeRTOS Convention, But MISRA
     * on The Other Hand Would Like Conversion To Boolean.
     */
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
    /**
     * MISRA Deviation: Rule 2.2
     * Justification: Function 'MSC_DeviceMscApp' is Called Periodically and Allows To Expand USB Mass Storage.
     */
    /*lint -save -e522 */
    MSC_DeviceMscApp();
    /*lint +e522 */
}
