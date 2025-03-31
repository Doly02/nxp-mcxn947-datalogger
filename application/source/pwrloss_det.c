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

#include "fsl_debug_console.h"
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
volatile uint32_t cnt = 0U;

/*******************************************************************************
 * Interrupt Service Routines (ISRs)
 ******************************************************************************/

/*
 * @brief 	Signals based on the tau value of the capacitor charged to 99%, i.e. 5x tau.
 * 			Sets the detection to the defined state at this period of time.
 */
void CTIMER4_IRQHandler(void)
{
	LED_SetHigh(GPIO0, 15);
	LED_SetLow(GPIO0, 23);
	cnt = 1;

	CTIMER_ClearStatusFlags(CTIMER4, kCTIMER_Match0Flag);

	/* Signal To The User That Back-Up Power Is Available */
	LED_SignalBackUpPowerAvailable();
	CTIMER_StopTimer(CTIMER);
	DisableIRQ(CTIMER4_IRQn);
}

/*
 * @brief	Response To Loss of Supply Voltage.
 */
void HSCMP1_IRQHandler(void)
{
	/*? LPCMP_ClearStatusFlags(DEMO_LPCMP_BASE, kLPCMP_OutputFallingInterruptEnable); kLPCMP_OutputFallingEventFlag */
	LPCMP_ClearStatusFlags(DEMO_LPCMP_BASE, kLPCMP_OutputFallingEventFlag);
	if (1 == cnt)
	{
		LED_SetHigh(GPIO0, 23);	/* Signal Power Loss 					*/
		UART_Disable();				/* Disable Character Reception			*/
    	CONSOLELOG_Flush();			/* Flush Data From Buffer To SDHC Card 	*/
	}
	cnt++;

	SDK_ISR_EXIT_BARRIER;

}

/*******************************************************************************
 * Functions
 ******************************************************************************/
void PWRLOSS_DetectionInit(void)
{

    lpcmp_config_t mLpcmpCfg;
    lpcmp_dac_config_t mLpcmpDacConfigStruct;
    ctimer_config_t config;
    ctimer_match_config_t matchConfig0;			/* Match Configuration for Channel 0 */

    /* enable CMP1, CMP1_DAC and VREF. */
    SPC_EnableActiveModeAnalogModules(DEMO_SPC_BASE, (kSPC_controlCmp1 | kSPC_controlCmp1Dac));

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
    EnableIRQ(HSCMP1_IRQn);
    //EnableIRQWithPriority(HSCMP1_IRQn, POWERLOSS_DET_PRIO);
    LPCMP_EnableInterrupts(CMP1, kLPCMP_OutputFallingInterruptEnable);

    CTIMER_GetDefaultConfig(&config);

    CTIMER_Init(CTIMER, &config);

    uint32_t timerClkFreq = CLOCK_GetCTimerClkFreq(4U);
    uint32_t match = timerClkFreq * 25;	/* 25 Seconds*/

    /* Configuration 0 */
    matchConfig0.enableCounterReset = true;
    matchConfig0.enableCounterStop  = false;
    matchConfig0.matchValue         = match;
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

    EnableIRQWithPriority(CTIMER4_IRQn, 0x2);

    CTIMER_StartTimer(CTIMER);

    LED_SetLow(GPIO0, 23);
    LED_SetLow(GPIO0, 15);


	return;
}
