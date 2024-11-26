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
 * Includes
 ******************************************************************************/
#define BUFFER_SIZE				(64U)

/**
 * @brief 	When configured for LPI2C/LPUART, two 8x16 FIFOs are supported,
 * 			one for each module.
 * */
#define FIFO_BUFFER_LENGHT		(16u)
/*******************************************************************************
 * Code
 ******************************************************************************/
#if (true == UART_FIFO_ENABLED)

/**
 * @brief Rx Fifo IRQ Handler.
 * */
void LP_FLEXCOMM7_IRQHandler(void)
{
    while (LPUART_GetRxFifoCount(LPUART7) > 0)
    {
    	/* Read Byte */
        uint8_t data = LPUART_ReadByte(LPUART7);
        UART_Print(data);

    }

    PRINTF(" ");
    /* Control of Overflow FIFO -> Prevention From Data Loss */
    if (LPUART_GetStatusFlags(LPUART7) & kLPUART_RxOverrunFlag)
    {
    	/* Create FIFO Overflow */
        LPUART_ClearStatusFlags(LPUART7, kLPUART_RxOverrunFlag);
        PRINTF("ERR: RX FIFO Overflow!\r\n");
    }

    /*
    if (LPUART_GetStatusFlags(LPUART7) & kLPUART_RxDataRegFullFlag)
    {
        LPUART_ClearStatusFlags(LPUART7, kLPUART_RxDataRegFullFlag);
    }
    */
    SDK_ISR_EXIT_BARRIER;
}

#else

void LP_FLEXCOMM7_IRQHandler(void)
{
    static uint16_t bufferPos = 0; 		// Index For Ring Buffer
    uint8_t data;

    /* If New Data */
    if ((kLPUART_RxDataRegFullFlag) & LPUART_GetStatusFlags(LPUART7))
    {
        data = LPUART_ReadByte(LPUART7);

        /* Buffer Isn't Full . */
        if (((bufferPos + 1) % DEMO_RING_BUFFER_SIZE) != bufferPos)
        {
            demoRingBuffer[bufferPos] = data;
            bufferPos++;
            bufferPos %= DEMO_RING_BUFFER_SIZE;
        }
        else
        {
            /* Process Buffer */
        }
    }
    SDK_ISR_EXIT_BARRIER;
}

#endif /* (true == UART_FIFO_ENABLED) */

void UART_Print(uint8_t ch)
{
	PRINTF("%c", (char)ch);
}

void UART_Init(void)
{
    lpuart_config_t config;

    config.baudRate_Bps = 115200U;
    config.parityMode = kLPUART_ParityDisabled;
    config.stopBitCount = kLPUART_OneStopBit;

#if (true == UART_FIFO_ENABLED)
    config.txFifoWatermark = 15;
    config.rxFifoWatermark = 0;
#else
    config.txFifoWatermark = 0;
	config.rxFifoWatermark = 0;

#endif /* (true == UART_FIFO_ENABLED) */

    config.enableTx = false;
    config.enableRx = true;

    LPUART_Init(LPUART7, &config, LPUART_CLK_FREQ);

    uint32_t rxFifoSize = 1U << ((LPUART7->PARAM & LPUART_PARAM_RXFIFO_MASK) >> LPUART_PARAM_RXFIFO_SHIFT);
    PRINTF("RX FIFO Size: %u bytes\r\n", rxFifoSize);
}
