/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      ctimer.h
 *  Author:         Tomas Dolak
 *  Date:           18.11.2024
 *  Description:    Implements Datalogger Application.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           ctimer.h
 *  @author         Tomas Dolak
 *  @date           18.11.2024
 *  @brief          Implements Datalogger Application.
 * ****************************/


#ifndef CTIMER_H_
#define CTIMER_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#if 0
#include "fsl_ctimer.h"
#include "fsl_lpi2c_cmsis.h"
#include "rtc_ds3231.h"
#include "fsl_debug_console.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * @brief I2C Definitions.
 */
#define I2C_MASTER         			Driver_I2C2
#define EXAMPLE_LPI2C_DMA_BASEADDR 	(DMA0)
#define LPI2C_CLOCK_FREQUENCY      	CLOCK_GetLPFlexCommClkFreq(2u)
#define EXAMPLE_LPI2C_DMA_CLOCK    	kCLOCK_Dma0

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void TIMER_Init(void);

void TIMER_UpdateSimulatedTime(void);

uint8_t TIMER_GetDaysInMonth(uint8_t month, uint8_t year);

#endif

#endif /* CTIMER_H_ */
