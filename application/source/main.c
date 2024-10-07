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
#include "app_tasks.h"
#include "usb_msc.h"
#include "fsl_spc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define LENGHT 						(9U)
/*
 * @brief I2C Definitions.
 */
#define I2C_MASTER         			Driver_I2C2
#define LPI2C_DMA_BASEADDR 			(DMA0)
#define LPI2C_CLOCK_FREQUENCY      	CLOCK_GetLPFlexCommClkFreq(2u)
#define LPI2C_DMA_CLOCK    			kCLOCK_Dma0
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* Data structure of msc device, store the information ,such as class handle */
usb_msc_struct_t g_msc;
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

void APP_SetPowModeToOverDrive(void)
{
    spc_active_mode_dcdc_option_t opt = {
        .DCDCVoltage       = kSPC_DCDC_OverdriveVoltage,		//<! DCDC Regulator Voltage (Safe-Mode 1.2V).
        .DCDCDriveStrength = kSPC_DCDC_NormalDriveStrength,		//<! DCDC Regulator Driver Strenght.
    };
    SPC_SetActiveModeDCDCRegulatorConfig(SPC0, &opt);

    spc_sram_voltage_config_t cfg = {
        .operateVoltage       = kSPC_sramOperateAt1P2V, 		//<! SRAM Will Operates On 1.2V.
        .requestVoltageUpdate = true,
    };
    /**
     * SPC0 = System Power Controller Interrupt
     */
    SPC_SetSRAMOperateVoltage(SPC0, &cfg);
}


/*
 * TODO: Check The Right Purpose of The Function (Enablement of the USB MSC)!
 * @brief 	Initialize The Board For Data Logger Application.
 * @details	Function Setup Clocks, Pins And Peripherals For Real-Time Circuit (eDMA, I2C), SD Card, USB Mass Storage.
 *			(USB Mass Storage Function Is Initialized Later And Only When The MSC Mode Is Enabled).
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


#if (true == MSC_ENABLED)

    /* Attach FRO HF to USDHC */
    CLOCK_SetClkDiv(kCLOCK_DivUSdhcClk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_USDHC);

    /* Enable The Clock For GPIO0 and GPIO2 */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio2);

	/* Set DCDC To Normal Behavior And Set Safe Voltage Level For SRAM. */
	APP_SetPowModeToOverDrive();

#endif /* (true == MSC_ENABLED) */

	/* Enable DMA Clock */
	CLOCK_EnableClock(LPI2C_DMA_CLOCK);

	BOARD_InitPins();
	BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

#if (true == MSC_ENABLED)

    /* Setup External Oscillator */
    CLOCK_SetupExtClocking(BOARD_XTAL0_CLK_HZ);
    /* Configure SD Card */
    BOARD_USB_Disk_Config(USB_DEVICE_INTERRUPT_PRIORITY);

#endif /* (true == MSC_ENABLED) */

#if (true == RTC_ENABLED)
    /* Initialize DMA */
	edma_config_t edmaConfig = { 0U };
	EDMA_GetDefaultConfig(&edmaConfig);
	EDMA_Init(LPI2C_DMA_BASEADDR, &edmaConfig);
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

	
    if (xTaskCreate(rtc_task, "rtc_task", configMINIMAL_STACK_SIZE + 100, NULL, TASK_PRIO, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!\r\n");
        APP_HandleError();
    }
#endif /* (true == RTC_ENABLED) */

    if (xTaskCreate(APP_task,                       /* pointer to the task */
                        (char const *)"app task",       /* task name for kernel awareness debugging */
                        5000L / sizeof(portSTACK_TYPE), /* task stack size */
                        &g_msc,                         /* optional task startup argument */
                        3,                              /* initial priority */
                        &g_msc.application_task_handle  /* optional task handle to create */
                        ) != pdPASS)
        {
            usb_echo("app task create failed!\r\n");
    #if (defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__))
            return 1;
#else
		return;
#endif
        }
        vTaskStartScheduler();

#if (defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__))
	return 1;
#endif
#if (true == RTC_ENABLED)
    for (;;)
        ;
    return 1;
#endif /* (true == RTC_ENABLED) */
}

