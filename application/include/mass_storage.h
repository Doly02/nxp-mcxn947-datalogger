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

/**
 * @brief Process Extension to Mass Storage.
 */
void MSC_DeviceMscApp(void);

/**
 * @brief 	Handles Mass Storage Application.
 * @details Communication and Data Transport Is Handled By USB1_HS ISR.
 */
void MSC_DeviceMscAppTask(void);

#endif /* MASS_STORAGE_H_ */
