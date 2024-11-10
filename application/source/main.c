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

#include "semphr.h"
#if (true == USB0_DET_PIN_ENABLED)

#include "usb_vbus_detection.h"

#endif /* (true == USB0_DET_PIN_ENABLED) */
/*******************************************************************************
 * Definitions
 ******************************************************************************/

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

static StackType_t recordTaskStack[RECORD_STACK_SIZE];

static StaticTask_t recordTaskTCB;

usb_msc_struct_t g_msc;

TaskHandle_t mscTaskHandle = NULL;
TaskHandle_t recordTaskHandle = NULL;

#if 1

SemaphoreHandle_t g_TaskMutex;

uint8_t volatile usbAttached;

#endif
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

void APP_UsbInit(void)
{
	USB_DeviceClockInit();
    g_msc.speed                      = USB_SPEED_FULL;
    g_msc.attach                     = 0;
    g_msc.deviceHandle               = NULL;

    if (kStatus_USB_Success != USB_DeviceInit(CONTROLLER_ID, USB_DeviceCallback, &g_msc.deviceHandle))
    {
        usb_echo("USB device mass storage init failed\r\n");
        return;
    }

    USB_DeviceIsrEnable();
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

    /* Enables the clock for GPIO4 */
	CLOCK_EnableClock(kCLOCK_Gpio4);

    BOARD_InitBootPins();
    BOARD_PowerMode_OD();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
	PRINTF("Initialization!\r\n");

#if (true == USB0_DET_PIN_ENABLED)
    InitUSBVbusInterrupt();
#endif /* (true == USB0_DET_PIN_ENABLED) */

#if (true == RTC_ENABLED)

    /* Initialize DMA */
	edma_config_t edmaConfig = { 0U };
	EDMA_GetDefaultConfig(&edmaConfig);
	EDMA_Init(EXAMPLE_LPI2C_DMA_BASEADDR, &edmaConfig);

#endif /* (true == RTC_ENABLED) */

    CLOCK_SetupExtClocking(BOARD_XTAL0_CLK_HZ);
    BOARD_USB_Disk_Config(USB_DEVICE_INTERRUPT_PRIORITY);

    // APP_UsbInit();
}
 
/*!
 * @brief Application Entry Point.
 */
int main(void)
{

	uint8_t ui8RetVal = E_FAULT;
	usbAttached = 0;

	g_TaskMutex = xSemaphoreCreateBinary();
	if (NULL == g_TaskMutex)
	{
        PRINTF("Failed to Create Semaphore!\n");
        APP_HandleError();
	}
	/* Release Semaphore For record_task */
	xSemaphoreGive(g_TaskMutex);

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

    recordTaskHandle = xTaskCreateStatic(
    			  record_task,       		/* Function That Implements The Task. 		*/
                  "record_task",          	/* Text Name For The Task. 					*/
				  MSC_STACK_SIZE,      		/* Number of Indexes In The xStack Array. 	*/
                  NULL,    					/* Parameter Passed Into The Task. 			*/
				  TASK_PRIO,				/* Priority at Which The Task Is Created. 	*/
				  &recordTaskStack[0],         /* Array To Use As The Task's Stack.		*/
                  &recordTaskTCB );
    if (NULL == recordTaskHandle)
    {
    	PRINTF("MSC Task Creation Failed!\r\n");
    	APP_HandleError();
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
    	PRINTF("MSC Task Creation Failed!\r\n");
    	APP_HandleError();
    }

#endif /* (true == MSC_ENABLED) */

    vTaskStartScheduler();
    while (1 == 1)
    {
    	;
    }

    return ui8RetVal;

}
