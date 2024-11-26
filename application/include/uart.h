/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      uart.h
 *  Author:         Tomas Dolak
 *  Date:           25.11.2024
 *  Description:    Implements the UART peripheral support.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           uart.h
 *  @author         Tomas Dolak
 *  @date           25.11.2024
 *  @brief          Implements the UART peripheral support.
 * ****************************/

#ifndef UART_H_
#define UART_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_lpuart.h"

#include "fsl_clock.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LPUART7_CLK_FREQ   CLOCK_GetLPFlexCommClkFreq(7u)
#define LPUART7_IRQn       LP_FLEXCOMM7_IRQn


/*******************************************************************************
 * Declarations
 ******************************************************************************/
void UART_Init(void);

void UART_Print(uint8_t ch);

#endif /* UART_H_ */

