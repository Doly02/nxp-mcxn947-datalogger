/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      uart.c
 *  Author:         Tomas Dolak
 *  Date:           25.11.2024
 *  Description:    Implements The UART Peripheral Support.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           uart.c
 *  @author         Tomas Dolak
 *  @date           25.11.2024
 *  @brief          Implements The UART Peripheral Support.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "uart.h"
#include "defs.h"

#include "parser.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

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
    config.baudRate_Bps  = baudrate;
    config.isMsb         = false;
    config.parityMode    = PARSER_GetParity();
    config.dataBitsCount = PARSER_GetDataBits();
    config.stopBitCount  = PARSER_GetStopBits();

    config.enableTx      = false;
    config.enableRx      = true;

#if (true == UART_FIFO_ENABLED)
    config.rxFifoWatermark = UART_FIFO_LENGHT;

#else
	config.rxFifoWatermark = 0;

#endif /* (true == UART_FIFO_ENABLED) */
    config.txFifoWatermark = 0;

    uint32_t clk = LPUART3_CLK_FREQ;

    /**
     * MISRA Deviation: Rule 11.4 [Required]
     * Suppress: Conversion Between Object Pointer Type 'LPUART_Type *' and Integer Type 'Unsigned Int'.
     * Justification: LPUART3 Is a Hardware Peripheral Base Address Defined In The NXP SDK.
     * This Code Follows The Usage Pattern Provided By The NXP SDK.
     */
    /*lint -e9078 */
    (void)LPUART_Init(LPUART3, &config, clk);
    /*lint +e9078 */
}

void UART_Enable(void)
{
	(void)DisableIRQ(LP_FLEXCOMM3_IRQn);

    /* Enable RX interrupt. */
    /**
     * MISRA Deviation Note:
     * Rule 10.3: Cannot Assign 'enum' to a Different Essential Type Such As 'unsigned32'. [Required]
     * Rule 11.4: Conversion Between Object Pointer Type and Integer Type. [Required]
     * Justification: This Code Follows The Usage Pattern Provided By The NXP SDK.
     */
	/*lint -e9034 -e9078 */
    LPUART_EnableInterrupts(LPUART3, (uint32_t)kLPUART_RxDataRegFullInterruptEnable);
    /*lint +e9034 +e9078 */
    (void)EnableIRQWithPriority(LP_FLEXCOMM3_IRQn, UART_RECEIVE_PRIO);
}

void UART_Disable(void)
{
	(void)DisableIRQ(LP_FLEXCOMM3_IRQn);
}

void UART_Deinit(void)
{
	(void)DisableIRQ(LP_FLEXCOMM3_IRQn);
	LPUART_DisableInterrupts(LPUART3, (uint32_t)kLPUART_RxDataRegFullInterruptEnable);

	LPUART_Deinit(LPUART3);

	/* Pins Are De-Initialized Together in pin_mux.c */
}
