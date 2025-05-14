/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      init.c
 *  Author:         Tomas Dolak
 *  Date:           31.03.2025
 *  Description:    Implements Datalogger Application.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           init.c
 *  @author         Tomas Dolak
 *  @date           31.03.2025
 *  @brief          Implements Datalogger Application.
 * ****************************/

#ifndef APP_INIT_H_
#define APP_INIT_H_
/*******************************************************************************
 * Includes
 ******************************************************************************/
/* NXP Board Specific */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_clock.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/* Application Includes */
#include "led.h"				// Control LEDs Module
#include "uart.h"				// UART Module
#include "time.h"				// Time Module
#include "error.h"				// Error Handling
#include "temperature.h"		// Temperature Measurement Module
#include "pwrloss_det.h"		// Power Loss Detection Module

/*******************************************************************************
 * Global Definitions
 ******************************************************************************/

/*******************************************************************************
 * Global Structures
 ******************************************************************************/

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * @brief 	Initializes Board Peripherals And Modules For Proper
 * 			Functionality of The Logger.
 * */
void APP_InitBoard(void);


#endif /* APP_INIT_H_ */
