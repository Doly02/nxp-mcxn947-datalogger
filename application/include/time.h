/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      time.c
 *  Author:         Tomas Dolak
 *  Date:           11.11.2024
 *  Description:    Implements The Logic Of Time-Keeping.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           time.c
 *  @author         Tomas Dolak
 *  @date           11.11.2024
 *  @brief          Implements The Logic Of Time-Keeping.
 * ****************************/


#ifndef TIME_H_
#define TIME_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_irtc.h"
#include "rtc_ds3231.h"
#include "fsl_debug_console.h"
#include "error.h"

/*******************************************************************************
 * Global Definitions
 ******************************************************************************/

/**
 * @brief I2C Definitions.
 */
#define LPI2C_DMA_BASEADDR 	(DMA0)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/**
 * @brief 	Initialize Internal And External Real-Time Circuits And Passes Timestamp
 * 			Information From The External To The Internal.
 */
error_t TIME_InitIRTC(void);

#endif /* TIME_H_ */
