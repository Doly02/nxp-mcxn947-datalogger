/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      pwrloss_det.h
 *  Author:         Tomas Dolak
 *  Date:           01.02.2024
 *  Description:    Implements The Logic Of Power Loss Detection.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           pwrloss_det.h
 *  @author         Tomas Dolak
 *  @date           01.02.2024
 *  @brief          Implements The Logic Of Power Loss Detection.
 * ****************************/

#ifndef PWRLOSS_DET_H_
#define PWRLOSS_DET_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_lpcmp.h"
#include "fsl_spc.h"

#include "led.h"
#include "defs.h"


#include "fsl_ctimer.h"
/*******************************************************************************
 * Global Definitions
 ******************************************************************************/

/**
 * @brief Base Address of The Low-Power Comparator (LPCMP).
 * @details Used To Detect Loss of Primary 5V Supply. If The Voltage Drops Below a Defined Threshold,
 *          It Triggers a Transition To Backup Power Mode for Safe Data Preservation.
 */
#define LPCMP_BASE             		CMP1

/**
 * @brief Comparator Input Channel Used For 5V Monitoring.
 */
#define LPCMP_USER_CHANNEL     		0x02

/**
 * @brief Comparator Internal DAC Reference Channel.
 */
#define LPCMP_DAC_CHANNEL      		0x07

/**
 * @brief IRQ Number for The Comparator Peripheral.
 */
#define LPCMP_IRQ_ID           		HSCMP1_IRQn

/**
 * @brief Base Address of The Supply Power Controller (SPC).
 * @brief For Activation Analog Modules.
 */
#define SPC_BASE               		SPC0

/**
 * @brief 	Timer Used To Control Delayed Activation of Power Loss Detection.
 * @details The Comparator is Enabled After a Delay of 5tau, Which Corresponds To The Time Needed
 *          For The Backup Capacitor To Become Fully Charged. This Delay Ensures Valid Detection
 *          of Power Drop Events and Avoids False Triggering During Startup.
 */
#define CTIMER						CTIMER4

/**
 * @brief Timer Match Output Channel 0.
 */
#define CTIMER_MAT0_OUT				kCTIMER_Match_0

/**
 * @brief Timer External Match Output Mask For Channel 0.
 */
#define CTIMER_EMT0_OUT				(1u << kCTIMER_Match_0)

/**
 * @brief Clock Frequency Retrieval Macro For The Timer Instance.
 */
#define CTIMER_CLK_FREQ				CLOCK_GetCTimerClkFreq(4U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/**
 * @brief   Initializes Power Loss Detection Components.
 * @details This Function Configures The Comparator (LPCMP) for Monitoring The 5V Power Rail and
 *          The Timer (CTIMER4) That Delays The Activation of The Comparator Until The Backup
 *          Capacitor Is Fully Charged (~5tau). After Initialization, The Timer is Started
 *          and The Comparator Will Be Enabled Upon Timer Match Event.
 */
void PWRLOSS_DetectionInit(void);


#endif /* PWRLOSS_DET_H_ */
