/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      uart.h
 *  Author:         Tomas Dolak
 *  Date:           25.11.2024
 *  Description:    Header File For The UART Peripheral Support.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           uart.h
 *  @author         Tomas Dolak
 *  @date           25.11.2024
 *  @brief          Header File For The UART Peripheral Support.
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

#include "fsl_debug_console.h"
#include "fsl_clock.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * @brief Frequency of LPUART7.
 */
#define LPUART3_CLK_FREQ   CLOCK_GetLPFlexCommClkFreq(3u)


/*******************************************************************************
 * Declarations
 ******************************************************************************/

/**
 * @brief 		Initializes LPUART For Recording.
 */
void UART_Init(uint32_t baudrate);

/**
 * @brief 		Prints Character on The Terminal.
 *
 * @param[in]	ch Character in uint8_t.
 */
void UART_Print(uint8_t ch);

/**
 * @brief 		Enables Interrupt For Application LPUART.
 */
void UART_Enable(void);

/**
 * @brief 		Disables Interrupt For Application LPUART.
 */
void UART_Disable(void);

/**
 * @brief 		De-Initialize LPUART.
 * @details		The Pins Should Be De-Initialized After This Function.
 */
void UART_Deinit(void);

#endif /* UART_H_ */

