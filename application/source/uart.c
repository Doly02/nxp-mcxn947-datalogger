/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      uart.c
 *  Author:         Tomas Dolak
 *  Date:           25.11.2024
 *  Description:    Implements the UART peripheral support.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           uart.c
 *  @author         Tomas Dolak
 *  @date           25.11.2024
 *  @brief          Implements the UART peripheral support.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "uart.h"
#include "defs.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LPUART_PRIO 			(7U)

#if (true == UART_FIFO_ENABLED)

/**
 * @brief 	When configured for LPI2C/LPUART, two 8x16 FIFOs are supported,
 * 			one for each module.
 */
#define FIFO_BUFFER_LENGHT		(16U)


#else

#define BUFFER_SIZE 			(200)

#endif /* (true == UART_FIFO_ENABLED) */

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void UART_Print(uint8_t ch)
{
	PRINTF("%c", (char)ch);
}

void UART_Init(uint32_t baudrate)
{
    lpuart_config_t config;

    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps 	= 230400;
    config.isMsb        	= false;

    config.enableTx     	= false;
    config.enableRx     	= true;

#if (true == UART_FIFO_ENABLED)
    config.rxFifoWatermark = UART_FIFO_LENGHT;

#else
	config.rxFifoWatermark = 0;

#endif /* (true == UART_FIFO_ENABLED) */
    config.txFifoWatermark = 0;

    uint32_t clk = LPUART7_CLK_FREQ;
    LPUART_Init(LPUART7, &config, clk);

}

void UART_Enable(void)
{
    DisableIRQ(LP_FLEXCOMM7_IRQn);

    /* Enable RX interrupt. */
    LPUART_EnableInterrupts(LPUART7, kLPUART_RxDataRegFullInterruptEnable);
    EnableIRQWithPriority(LP_FLEXCOMM7_IRQn, LPUART_PRIO);
}

void UART_Disable(void)
{
	DisableIRQ(LP_FLEXCOMM7_IRQn);
}
