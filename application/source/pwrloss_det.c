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
#define TRIGGER_VOLTAGE		0xE1u

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
static volatile uint32_t g_u32Cnt = 0UL;

/*******************************************************************************
 * Interrupt Service Routines (ISRs)
 ******************************************************************************/

/*
 * @brief 	Signals based on the tau value of the capacitor charged to 99%, i.e. 5x tau.
 * 			Sets the detection to the defined state at this period of time.
 */

/*lint -e957 */
/* MISRA 2012 Rule 8.4:
 * Suppress: function 'CTIMER4_IRQHandler' defined without a prototype in scope.
 * CTIMER4_IRQHandler is declared WEAK in startup_mcxn947_cm33_core0.c and overridden here.
 */
void CTIMER4_IRQHandler(void)
{
	/// LED_SetHigh(GPIO0, 15);
	LED_SetLow(GPIO0, 23);
	g_u32Cnt = 1;

	/* MISRA Deviation Note:
	 * Rule: MISRA 2012 Rule 10.3 [Required]
	 * Justification: The enum value is part of the NXP SDK
	 * and is intentionally used in this context as a bitmask flag for hardware status registers.
	 */

	/*lint -e9034 MISRA Deviation: conversion from enum to unsigned32 */
	CTIMER_ClearStatusFlags(CTIMER4, kCTIMER_Match0Flag);
	/*lint +e9034 */

	/* Signal To The User That Back-Up Power Is Available */
	LED_SignalBackUpPowerAvailable();
	CTIMER_StopTimer(CTIMER);
	(void)DisableIRQ(CTIMER4_IRQn);
}
/*lint -e957 */

/*
 * @brief	Response To Loss of Supply Voltage.
 */

/*lint -e957 */
/* MISRA 2012 Rule 8.4:
 * Suppress: function 'HSCMP1_IRQHandler' defined without a prototype in scope.
 * HSCMP1_IRQHandler is declared WEAK in startup_mcxn947_cm33_core0.c and overridden here.
 */
void HSCMP1_IRQHandler(void)
{
	LPCMP_ClearStatusFlags(DEMO_LPCMP_BASE, (uint32_t)kLPCMP_OutputFallingEventFlag);
	if (1UL == g_u32Cnt)
	{
		LED_SetHigh(GPIO0, 23);				/* Signal Power Loss 					*/
		UART_Disable();						/* Disable Character Reception			*/
    	(void)CONSOLELOG_PowerLossFlush();	/* Flush Data From Buffer To SDHC Card 	*/
	}
	g_u32Cnt++;

	SDK_ISR_EXIT_BARRIER;

}
/*lint -e957 */

/*******************************************************************************
 * Functions
 ******************************************************************************/
void PWRLOSS_DetectionInit(void)
{

    lpcmp_config_t mLpcmpCfg;
    lpcmp_dac_config_t mLpcmpDacConfigStruct;
    ctimer_config_t config;
    ctimer_match_config_t matchConfig0;			/* Match Configuration for Channel 0 */

    uint32_t u32TimerClkFreq; 					/* Timer Frequency 	*/
    uint32_t u32Match;							/* Match Value 		*/


    /* Enable CMP1, CMP1_DAC and VREF. */
    /* MISRA Deviation Note:
     * Rule: MISRA 2012 Rule 10.1 [Required]
     * Justification: Enum values 'kSPC_controlCmp1' and 'kSPC_controlCmp1Dac' are defined as bitmask flags
     * in the provided NXP SDK, and are intended to be combined using bitwise OR '|'.
     */
    /*lint -e9027 -e655 -e641 MISRA Deviation: bitwise operation on enum values is intentional and safe */
    SPC_EnableActiveModeAnalogModules(DEMO_SPC_BASE, (uint32_t)(kSPC_controlCmp1 | kSPC_controlCmp1Dac));
    /*lint +e9027 +e655 +e641 */

    /*
     *   k_LpcmpConfigStruct->enableStopMode      	= false;
     *   k_LpcmpConfigStruct->enableOutputPin     	= false;
     *   k_LpcmpConfigStruct->useUnfilteredOutput 	= false;
     *   k_LpcmpConfigStruct->enableInvertOutput  	= false;
     *   k_LpcmpConfigStruct->hysteresisMode      	= kLPCMP_HysteresisLevel0;
     *   k_LpcmpConfigStruct->powerMode           	= kLPCMP_LowSpeedPowerMode;
     *   k_LpcmpConfigStruct->functionalSourceClock = kLPCMP_FunctionalClockSource0;
     */
    LPCMP_GetDefaultConfig(&mLpcmpCfg);
    mLpcmpCfg.hysteresisMode = kLPCMP_HysteresisLevel2;

    /* Init the LPCMP module. */
    LPCMP_Init(CMP1, &mLpcmpCfg);

    /* Configure the internal DAC to output half of reference voltage. */
    mLpcmpDacConfigStruct.enableLowPowerMode		= false;
    mLpcmpDacConfigStruct.referenceVoltageSource	= kLPCMP_VrefSourceVin1;
    mLpcmpDacConfigStruct.DACValue					= 0xE1;
    LPCMP_SetDACConfig(DEMO_LPCMP_BASE, &mLpcmpDacConfigStruct);

    /* Configure LPCMP input channels. */
    LPCMP_SetInputChannels(CMP1, 0x02, 0x07);

    /* Enable the interrupt. */
    (void)EnableIRQ(HSCMP1_IRQn);
    //(void)EnableIRQWithPriority(HSCMP1_IRQn, POWERLOSS_DET_PRIO);
    LPCMP_EnableInterrupts(CMP1, (uint32_t)kLPCMP_OutputFallingInterruptEnable);

    CTIMER_GetDefaultConfig(&config);

    CTIMER_Init(CTIMER, &config);

    u32TimerClkFreq = CLOCK_GetCTimerClkFreq(4U);
    u32Match = u32TimerClkFreq * 25UL;	/* 25 Seconds*/

    /* Configuration 0 */
    matchConfig0.enableCounterReset = true;
    matchConfig0.enableCounterStop  = false;
    matchConfig0.matchValue         = u32Match;
    matchConfig0.outControl         = kCTIMER_Output_NoAction;
    matchConfig0.outPinInitState    = false;
    matchConfig0.enableInterrupt    = true;

    /*
     * Macros CTIMER_MAT0_OUT and CTIMER_MAT1_OUT are nominal match output, instead of
     * hardware MR0 and MR1 register match output.
     * So CTIMER_MAT0_OUT can be defined as kCTIMER_Match_1, CTIMER_MAT1_OUT can be defined
     * as kCTIMER_Match_3, which means they are MR1 and MR3 register match output.
     */
    CTIMER_SetupMatch(CTIMER, CTIMER_MAT0_OUT, &matchConfig0);

    (void)EnableIRQWithPriority(CTIMER4_IRQn, 0x2);

    CTIMER_StartTimer(CTIMER);

    LED_SetLow(GPIO0, 23);
    LED_SetLow(GPIO0, 15);


	return;
}
