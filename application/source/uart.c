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
#if (true == UART_FIFO_ENABLED)



#else

volatile uint8_t buffer1[BUFFER_SIZE] 	= {0U};
volatile uint8_t buffer2[BUFFER_SIZE]	= {0U};

volatile uint8_t *activeBuffer 			= buffer1;
volatile uint8_t *processBuffer 		= NULL;
volatile bool bufferReady 				= false;

volatile uint8_t index 					= 0;

#endif /* (true == UART_FIFO_ENABLED) */
/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief LPUART7 IRQ Handler.
 *
 */
void LP_FLEXCOMM7_IRQHandler(void)
{
    uint8_t data;
    uint32_t stat;

    /* If New Data Arrived. */
    stat = LPUART_GetStatusFlags(LPUART7);
    if (kLPUART_RxDataRegFullFlag & stat)
    {
        data = LPUART_ReadByte(LPUART7);
        activeBuffer[index++] = data;

        if (index >= BUFFER_SIZE) // Buffer Is Full
        {
        	/* Switch The Buffer */
        	if (activeBuffer == buffer1)
            {
                activeBuffer = buffer2;
                processBuffer = buffer1;
            }
            else
            {
                activeBuffer = buffer1;
                processBuffer = buffer2;
            }

            index = 0;
            bufferReady = true; // Buffer Is Ready
        }
    }

    LPUART_ClearStatusFlags(LPUART7, kLPUART_RxDataRegFullFlag);
    SDK_ISR_EXIT_BARRIER;
}

void UART_Print(uint8_t ch)
{
	PRINTF("%c", (char)ch);
}

void UART_Init(void)
{
    lpuart_config_t config;

    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = 230400U;
    config.parityMode 	= kLPUART_ParityDisabled;
    config.stopBitCount = kLPUART_OneStopBit;

#if (true == UART_FIFO_ENABLED)
    config.rxFifoWatermark = UART_FIFO_LENGHT;

#else
	config.rxFifoWatermark = 0;

#endif /* (true == UART_FIFO_ENABLED) */
    config.txFifoWatermark = 0;

    config.isMsb        = false;
    config.enableTx     = false;
    config.enableRx     = true;

    LPUART_Init(LPUART7, &config, LPUART7_CLK_FREQ);

    DisableIRQ(LP_FLEXCOMM7_IRQn);

#if (true == DEBUG_ENABLED)

    uint32_t rxFifoSize = 1U << ((LPUART7->PARAM & LPUART_PARAM_RXFIFO_MASK) >> LPUART_PARAM_RXFIFO_SHIFT);
    PRINTF("DEBUG: Number of Characters in Rx FIFO: %u bytes\r\n", rxFifoSize);

#endif /* (true == DEBUG_ENABLED) */
}

void UART_Enable(void)
{
    LPUART_EnableInterrupts(LPUART7, kLPUART_RxDataRegFullInterruptEnable);
    EnableIRQWithPriority(LP_FLEXCOMM7_IRQn, LPUART_PRIO);

}

void UART_Disable(void)
{
	DisableIRQ(LP_FLEXCOMM7_IRQn);
}
