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

/* Freescale Includes. */
#include <led.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_clock.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/* FreeRTOS Includes. */
#include "semphr.h"

/* Application Includes */
#include "app_tasks.h"
#include "uart.h"
#include "time.h"
#include "error.h"
#include "temperature.h"
#include "pwrloss_det.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

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

static StackType_t recordTaskStack[RECORD_STACK_SIZE];

static StaticTask_t recordTaskTCB;

usb_msc_struct_t g_msc;

TaskHandle_t mscTaskHandle 		= NULL;
TaskHandle_t recordTaskHandle 	= NULL;

SemaphoreHandle_t g_TaskMutex;

uint8_t volatile usbAttached;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
 * @brief Functions of DMA That Are Used For Correct Work of RTC.
 */
void APP_InitBoard(void)
{
	irtc_config_t irtcCfg;
	edma_config_t edmaConfig = { 0U };

    /* Attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);

    /* Attach FRO 12M to FLEXCOMM3 Application UART */
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom3Clk, 2u);
	CLOCK_AttachClk(kFRO_HF_DIV_to_FLEXCOMM3);

#if (true == RTC_ENABLED)
	/* Attach FRO 12M To FLEXCOMM2 (I2C for RTC) */
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1U);
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);
	CLOCK_EnableClock(kCLOCK_Dma0);
#endif /* (true == RTC_ENABLED) */

    /* Attach FRO HF to USDHC */
    CLOCK_SetClkDiv(kCLOCK_DivUSdhcClk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_USDHC);

    /* Enables the clock for GPIO0 */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    /* Enables the clock for GPIO2 */
    CLOCK_EnableClock(kCLOCK_Gpio2);
    /* Enables the clock for GPIO4 */
	CLOCK_EnableClock(kCLOCK_Gpio4);

#if (true == TEMPERATURE_MEAS_ENABLED)
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom5Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM5);
    CLOCK_EnableClock(kCLOCK_Dma1);

    CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_CTIMER0);
#endif /* (true == TEMPERATURE_MEAS_ENABLED) */

#if (true == PWRLOSS_DETECTION_ENABLED)
    /* Attach FRO 12M to CMP1 */
    CLOCK_SetClkDiv(kCLOCK_DivCmp1FClk, 1U);
    CLOCK_AttachClk(kFRO12M_to_CMP1F);

    /* Attach FRO HF clock for CTIMER4 */
    CLOCK_SetClkDiv(kCLOCK_DivCtimer4Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_CTIMER4);

#endif /* (true == PWRLOSS_DETECTION_ENABLED) */

	BOARD_InitPins();
    BOARD_PowerMode_OD();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
	PRINTF("Initialization!\r\n");

#if (true == RTC_ENABLED)

    /* Initialize DMA */
	EDMA_GetDefaultConfig(&edmaConfig);
	EDMA_Init(LPI2C_DMA_BASEADDR, &edmaConfig);

#endif /* (true == RTC_ENABLED) */

    CLOCK_SetupExtClocking(BOARD_XTAL0_CLK_HZ);
    BOARD_USB_Disk_Config(USB_DEVICE_INTERRUPT_PRIORITY);

#if (true == IRTC_ENABLED)
    CLOCK_SetupClk16KClocking(kCLOCK_Clk16KToVbat | kCLOCK_Clk16KToMain);
    TIME_InitIRTC();
#endif /* (true == IRTC_ENABLED) */

#if (true == CONTROL_LED_ENABLED)
    LED_SetLow(GPIO0, 7);
    LED_SetLow(GPIO0, 9);
    LED_SetLow(GPIO0, 13);
    LED_SetLow(GPIO2, 11);
    LED_SetLow(GPIO4, 17);
#endif /* (true == CONTROL_LED_ENABLED) */

#if (true == TEMPERATURE_MEAS_ENABLED)
    TMP_Init();
#endif /* (true == TEMPERATURE_MEAS_ENABLED) */

#if (true == PWRLOSS_DETECTION_ENABLED)
    PWRLOSS_DetectionInit();
#endif /* (true == PWRLOSS_DETECTION_ENABLED) */

}
 
/*!
 * @brief Application Entry Point.
 */
int main(void)
{
	usbAttached = 0;

	g_TaskMutex = xSemaphoreCreateBinary();
	if (NULL == g_TaskMutex)
	{
        PRINTF("ERR: Failed to Create Semaphore!\n");
        ERR_HandleError();
	}

	/* Release Semaphore For record_task */
	xSemaphoreGive(g_TaskMutex);

    /* Initialize board hardware. */
	APP_InitBoard();

    recordTaskHandle = xTaskCreateStatic(
    			  record_task,       		/* Function That Implements The Task. 		*/
                  "record_task",          	/* Text Name For The Task. 					*/
				  MSC_STACK_SIZE,      		/* Number of Indexes In The xStack Array. 	*/
                  NULL,    					/* Parameter Passed Into The Task. 			*/
				  TASK_PRIO,				/* Priority at Which The Task Is Created. 	*/
				  &recordTaskStack[0],      /* Array To Use As The Task's Stack.		*/
                  &recordTaskTCB );
    if (NULL == recordTaskHandle)
    {
    	PRINTF("ERR: MSC Task Creation Failed!\r\n");
    	ERR_HandleError();
    }


#if (true == MSC_ENABLED)

    mscTaskHandle = xTaskCreateStatic(
    			  msc_task,       			/* Function That Implements The Task. 		*/
                  "msc_task",          		/* Text Name For The Task. 					*/
				  MSC_STACK_SIZE,      		/* Number of Indexes In The xStack Array. 	*/
                  NULL,    					/* Parameter Passed Into The Task. 			*/
				  TASK_PRIO - 1,			/* Priority at Which The Task Is Created. 	*/
				  &mscTaskStack[0],         /* Array To Use As The Task's Stack.		*/
                  &mscTaskTCB );
    if (NULL == mscTaskHandle)
    {
    	PRINTF("ERR: MSC Task Creation Failed!\r\n");
    	ERR_HandleError();
    }

#endif /* (true == MSC_ENABLED) */

    vTaskStartScheduler();
    while (1 == 1)
    {
    	;
    }

    return ERROR_OUT_OF_CYCLE;

}
