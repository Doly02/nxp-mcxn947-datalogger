/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      mainc.c
 *  Author:         Tomas Dolak
 *  Date:           07.08.2024
 *  Description:    Implements Datalogger Application.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           main.c
 *  @author         Tomas Dolak
 *  @date           07.08.2024
 *  @brief          Implements Datalogger Application.
 * ****************************/


#ifndef TIME_H_
#define TIME_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_irtc.h"
#include "rtc_ds3231.h"
#include "error.h"

#include "fsl_lpi2c_cmsis.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Global Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

error_t TIME_InitIRTC(void);

#endif /* TIME_H_ */
