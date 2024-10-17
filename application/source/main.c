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

/*******************************************************************************
 * Includes
 ******************************************************************************/

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_clock.h"
#include "fsl_lpi2c_cmsis.h"
#include "rtc_ds3231.h"

#include "app_tasks.h"
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
 * Global Variables
 ******************************************************************************/
/*!
 * @brief 	Buffer for Static Stack of Mass Storage Task.
 */
static StackType_t mscTaskStack[MSC_STACK_SIZE];
/*!
 * @brief 	TCB (Task Control Block) - Metadata of Mass Storage Task.
 * @details Includes All The Information Needed to Manage The Task Such As Job Status,
 * 			Job Stack Pointer, Values of Variables During Context Switching.
 */
static StaticTask_t mscTaskTCB;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

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

void APP_HandleError(void)
{
	while(1)
	{
		; /* Error State */
	}
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

    /* attach FRO HF to USDHC */
    CLOCK_SetClkDiv(kCLOCK_DivUSdhcClk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_USDHC);

    /* Enables the clock for GPIO0 */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    /* Enables the clock for GPIO2 */
    CLOCK_EnableClock(kCLOCK_Gpio2);

    BOARD_InitBootPins();
    BOARD_PowerMode_OD();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

#if (true == RTC_ENABLED)

    /* Initialize DMA */
	edma_config_t edmaConfig = { 0U };
	EDMA_GetDefaultConfig(&edmaConfig);
	EDMA_Init(EXAMPLE_LPI2C_DMA_BASEADDR, &edmaConfig);

#endif /* (true == RTC_ENABLED) */

    CLOCK_SetupExtClocking(BOARD_XTAL0_CLK_HZ);
    BOARD_USB_Disk_Config(USB_DEVICE_INTERRUPT_PRIORITY);

	return;
}
 
/*!
 * @brief Application Entry Point.
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

    if (pdPASS != xTaskCreate(rtc_task, "rtc_task", configMINIMAL_STACK_SIZE + 100, NULL, TASK_PRIO, NULL))
    {
        PRINTF("RTC Task Creation Failed!\r\n");
        APP_HandleError();
    }
#endif /* (true == RTC_ENABLED) */

#if (true == MSC_ENABLED)

    if (pdPASS != xTaskCreateStatic(msc_task, "msc_task", MSC_STACK_SIZE, NULL, TASK_PRIO, mscTaskStack, &mscTaskTCB ))
    {
    	PRINTF("MSC Task Creation Failed!\r\n");
    	APP_HandleError();
    }

#endif /* (true == MSC_ENABLED) */

    vTaskStartScheduler();
    while (1 == 1)
    {
    	;
    }

    return SUCCESS;
}
