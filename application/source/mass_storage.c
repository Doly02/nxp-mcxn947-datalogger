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

extern usb_msc_struct_t g_msc;

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
