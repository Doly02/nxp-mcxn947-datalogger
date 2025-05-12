/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      pwrloss_det.c
 *  Author:         Tomas Dolak
 *  Date:           01.02.2024
 *  Description:    Implements The Logic Of Power Loss Detection.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           pwrloss_det.c
 *  @author         Tomas Dolak
 *  @date           01.02.2024
 *  @brief          Implements The Logic Of Power Loss Detection.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "pwrloss_det.h"

#include "record.h"
/*******************************************************************************
 * Local Definitions
 ******************************************************************************/
/*
 * @brief 	Reference Voltage.
 * @details Maximal Value In DAC (3V3)
 */
#define REF_VOLTAGE			0xFFu

/*
 * @brief 	Trigger Voltage.
 * @details 2.91V
 */
#define TRIGGER_VOLTAGE		0xE1

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * Interrupt Service Routines (ISRs)
 ******************************************************************************/

/*
 * @brief 	Signals based on the tau value of the capacitor charged to 99%, i.e. 5x tau.
 * 			Sets the detection to the defined state at this period of time.
 */

/*lint -e957 */
/* MISRA 2012 Rule 8.4:
 * Suppress: Function 'CTIMER4_IRQHandler' Defined Without a Prototype in Scope.
 * Justification: CTIMER4_IRQHandler is Declared WEAK in startup_mcxn947_cm33_core0.c and Overridden Here.
 */
void CTIMER4_IRQHandler(void)
{
#if (true == PWRLOSS_TEST_GPIOS)
	LED_SetHigh(GPIO0, 15);
	LED_SetLow(GPIO0, 23);
#endif /* (true == PWRLOSS_TEST_GPIOS) */

	/**
	 * MISRA Deviation: Rule 10.3 [Required]
	 * Suppress: Conversion From enum To unsigned32.
	 * Justification: The Enum Value is Part of The NXP SDK
	 * and is Intentionally Used in Context as a Bitmask Flag For Hardware Status Registers.
	 */

	/*lint -e9034 */
	CTIMER_ClearStatusFlags(CTIMER4, kCTIMER_Match0Flag);
	/*lint +e9034 */

	/* Signal To The User That Back-Up Power Is Available */
	LED_SignalBackUpPowerAvailable();
	CTIMER_StopTimer(CTIMER);
	(void)DisableIRQ(CTIMER4_IRQn);

    /* Enable the interrupt. */
	(void)IRQ_ClearPendingIRQ(HSCMP1_IRQn);
	LPCMP_ClearStatusFlags(CMP1, (uint32_t)(kLPCMP_OutputRisingEventFlag | kLPCMP_OutputFallingEventFlag));

	LPCMP_EnableInterrupts(CMP1, (uint32_t)kLPCMP_OutputFallingInterruptEnable);
//    (void)IRQ_ClearPendingIRQ(HSCMP1_IRQn);

    (void)EnableIRQWithPriority(HSCMP1_IRQn, PWRLOSS_DET_PRIO);
}
/*lint +e957 */

/*
 * @brief	Response To Loss of Supply Voltage.
 */

/*lint -e957 */
/* MISRA 2012 Rule 8.4:
 * Suppress: Function 'HSCMP1_IRQHandler' Defined Without a Prototype in Scope.
 * Justification: HSCMP1_IRQHandler is Declared WEAK in startup_mcxn947_cm33_core0.c and Overridden Here.
 */
void HSCMP1_IRQHandler(void)
{
	LPCMP_ClearStatusFlags(LPCMP_BASE, (uint32_t)kLPCMP_OutputFallingEventFlag);


	UART_Disable();						/* Disable Character Reception			*/
	(void)CONSOLELOG_PowerLossFlush();	/* Flush Data From Buffer To SDHC Card 	*/

#if (true == PWRLOSS_TEST_GPIOS)
	LED_SetHigh(GPIO0, 23);				/* Signal Power Loss 					*/
	LED_SetLow(GPIO0, 15);
#endif /* (true == PWRLOSS_TEST_GPIOS) */

	SDK_ISR_EXIT_BARRIER;

}
/*lint +e957 */

/*******************************************************************************
 * Functions
 ******************************************************************************/
void PWRLOSS_DetectionInit(void)
{

    lpcmp_config_t mLpcmpCfg = { 0U };
    lpcmp_dac_config_t mLpcmpDacConfigStruct;
    ctimer_config_t config;
    ctimer_match_config_t matchConfig0;			/* Match Configuration for Channel 0 */

    uint32_t u32TimerClkFreq; 					/* Timer Frequency 	*/
    uint32_t u32Match;							/* Match Value 		*/


    /* Enable CMP1, CMP1_DAC and VREF. */
    /* MISRA Deviation Note:
     * Rule: MISRA 2012 Rule 10.1 [Required]
     * Suppress: Bitwise Operation on Composite Enum Values And Conversion To Unsigned Integer.
     * Justification: Enum values 'kSPC_controlCmp1' and 'kSPC_controlCmp1Dac' are defined as Bitmask Flags
     * in The Provided NXP SDK, and are Intended to Be Combined Using Bitwise OR.
     */
    /*lint -e9027 -e655 -e641 */
    SPC_EnableActiveModeAnalogModules(SPC_BASE, (uint32_t)(kSPC_controlCmp1 | kSPC_controlCmp1Dac));
    /*lint +e9027 +e655 +e641 */

    mLpcmpCfg.enableOutputPin     = false;
    mLpcmpCfg.useUnfilteredOutput = false;
    mLpcmpCfg.enableInvertOutput  = false;
    mLpcmpCfg.hysteresisMode      = kLPCMP_HysteresisLevel0;
    mLpcmpCfg.powerMode           = kLPCMP_LowSpeedPowerMode;
    mLpcmpCfg.functionalSourceClock = kLPCMP_FunctionalClockSource0;
    mLpcmpCfg.hysteresisMode = kLPCMP_HysteresisLevel2;

    /* Init the LPCMP module. */
    LPCMP_Init(LPCMP_BASE, &mLpcmpCfg);

    /* Configure the internal DAC to output half of reference voltage. */
    mLpcmpDacConfigStruct.enableLowPowerMode		= false;
    mLpcmpDacConfigStruct.referenceVoltageSource	= kLPCMP_VrefSourceVin1;
    mLpcmpDacConfigStruct.DACValue					= TRIGGER_VOLTAGE;
    LPCMP_SetDACConfig(LPCMP_BASE, &mLpcmpDacConfigStruct);

    /* Configure LPCMP input channels. */
    LPCMP_SetInputChannels(LPCMP_BASE, LPCMP_USER_CHANNEL, LPCMP_DAC_CHANNEL);

    CTIMER_GetDefaultConfig(&config);

    CTIMER_Init(CTIMER, &config);

    u32TimerClkFreq = CTIMER_CLK_FREQ;
    u32Match = (u32TimerClkFreq * PWRLOSS_DET_ACTIVE_IN_TIME);	/* 25 Seconds*/

    /* Configuration 0 */
    matchConfig0.enableCounterReset = true;
    matchConfig0.enableCounterStop  = false;
    matchConfig0.matchValue         = u32Match;
    matchConfig0.outControl         = kCTIMER_Output_NoAction;
    matchConfig0.outPinInitState    = false;
    matchConfig0.enableInterrupt    = true;

    CTIMER_SetupMatch(CTIMER, CTIMER_MAT0_OUT, &matchConfig0);

    (void)EnableIRQWithPriority(CTIMER4_IRQn, PWRLOSS_TIMER_PRIO);

    CTIMER_StartTimer(CTIMER);

#if (true == PWRLOSS_TEST_GPIOS)
    LED_SetLow(GPIO0, 23);
    LED_SetLow(GPIO0, 15);
#endif /* (true == PWRLOSS_TEST_GPIOS) */

	return;
}
