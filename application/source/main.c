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

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_clock.h"
#include "fsl_lpi2c_cmsis.h"
#include "rtc_ds3231.h"
#include "defs.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Task priorities. */
#define hello_task_PRIORITY (configMAX_PRIORITIES - 1)

#define LENGHT 				(9U)
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
static void rtc_task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * @brief Functions of I2C That Are Used For Correct Work of RTC.
 */
uint32_t LPI2C2_GetFreq(void)
{
    return LPI2C_CLOCK_FREQUENCY;
}
/*
 * @brief Functions of DMA That Are Used For Correct Work of RTC.
 */
void APP_InitBoard(void)
{
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

#if (true == RTC_ENABLED)
	/* Attach FRO 12M To FLEXCOMM2 (I2C for RTC) */
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1U);
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);
#endif /* (true == RTC_ENABLED) */

	/* Enable DMA Clock */
	CLOCK_EnableClock(EXAMPLE_LPI2C_DMA_CLOCK);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

#if (true == RTC_ENABLED)
    /* Initialize DMA */
	edma_config_t edmaConfig = { 0U };
	EDMA_GetDefaultConfig(&edmaConfig);
	EDMA_Init(EXAMPLE_LPI2C_DMA_BASEADDR, &edmaConfig);
#endif /* (true == RTC_ENABLED) */
	return;
}

void APP_HandleError(void)
{
	while(1)
	{
		; /* Error State */
	}
} 
 

/*!
 * @brief Application entry point.
 */
int main(void)
{
	uint8_t retVal = E_FAULT;

    /* Initialize board hardware. */
	APP_InitBoard();

#if (true == RTC_ENABLED)
	/* Initialize Real-Time Circuit */
    retVal = RTC_Init(&I2C_MASTER);
    if (E_FAULT == retVal)
    {
    	PRINTF("Initialization of RTC Has Failed\r\n");
    	APP_HandleError();
    }

	
    if (xTaskCreate(rtc_task, "rtc_task", configMINIMAL_STACK_SIZE + 100, NULL, hello_task_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        APP_HandleError();
    }
#endif /* (true == RTC_ENABLED) */
    vTaskStartScheduler();
    for (;;)
        ;
}

/*!
 * @brief Task Responsible for Time Handling.
 */
static void rtc_task(void *pvParameters)
{
	uint8_t retVal = E_FAULT;
	RTC_time_t actTime;
	RTC_date_t actDate;

    retVal = RTC_GetState();
    if (OSC_STOPPED == retVal)	// If The Oscillator Was Stopped -> Set Time & Date
    {
    	/* Set Default Time & Date (Prepare Variables) */
    	RTC_SetDateDefault(&actDate);
    	RTC_SetTimeDefault(&actTime);
		/* Set Time & Date Into RTC */
		RTC_SetDate(&actDate);
		RTC_SetTime(&actTime);

		RTC_SetOscState(OSC_OK);
    }

    /* Get Current Time */
    memset(&actTime, 0U, sizeof(actTime));
    memset(&actDate, 0U, sizeof(actDate));

    RTC_GetTime(&actTime);
    RTC_GetDate(&actDate);
    /* Print The Time From RTC */
	PRINTF("Current Date: %d\r\n",actDate.day);

	/* Finish The Task */
	vTaskSuspend(NULL);

}
