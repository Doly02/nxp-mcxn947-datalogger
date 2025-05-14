/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      init.c
 *  Author:         Tomas Dolak
 *  Date:           31.03.2025
 *  Description:    Implements Data Logger Initialization Phase.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           init.c
 *  @author         Tomas Dolak
 *  @date           31.03.2025
 *  @brief          Implements Data Logger Initialization Phase.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "app_tasks.h"
#include "app_init.h"
/*******************************************************************************
 * Local Definitions
 ******************************************************************************/

/*******************************************************************************
 * Local Structures
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/

void APP_InitBoard(void)
{

    /* Attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);

    /* Attach FRO 12M to FLEXCOMM3 Application UART */
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom3Clk, 2u);
	CLOCK_AttachClk(kFRO_HF_DIV_to_FLEXCOMM3);

	/* Attach FRO 12M To FLEXCOMM2 (I2C for RTC) */
	CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1U);
	CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);
	CLOCK_EnableClock(kCLOCK_Dma0);

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

#if (true == INFO_ENABLED)
	PRINTF("INFO: Data Logger Initialization\r\n");
#endif /* (true == INFO_ENABLED) */

    (void)CLOCK_SetupExtClocking(BOARD_XTAL0_CLK_HZ);
    BOARD_USB_Disk_Config(0x02);

#if (true == TEMPERATURE_MEAS_ENABLED)
    TMP_Init();
#endif /* (true == TEMPERATURE_MEAS_ENABLED) */

#if (true == PWRLOSS_DETECTION_ENABLED)

    PWRLOSS_DetectionInit();
#endif /* (true == PWRLOSS_DETECTION_ENABLED) */
}
