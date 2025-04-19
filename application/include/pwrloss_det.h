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

#include "led.h"		/*TODO: */
#include "defs.h"


#include "fsl_ctimer.h"
/*******************************************************************************
 * Global Definitions
 ******************************************************************************/
#define DEMO_LPCMP_BASE             CMP1
#define DEMO_LPCMP_USER_CHANNEL     2U
#define DEMO_LPCMP_DAC_CHANNEL      7U
#define DEMO_LPCMP_IRQ_ID           HSCMP1_IRQn
#define DEMO_LPCMP_IRQ_HANDLER_FUNC HSCMP1_IRQHandler
#define DEMO_VREF_BASE              VREF0
#define DEMO_SPC_BASE               SPC0

#define CTIMER						CTIMER4         /* Timer 4 */
#define CTIMER_MAT0_OUT				kCTIMER_Match_0 /* Match output 0 */
#define CTIMER_EMT0_OUT				(1u << kCTIMER_Match_0)
#define CTIMER_CLK_FREQ				CLOCK_GetCTimerClkFreq(4U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void PWRLOSS_DetectionInit(void);


#endif /* PWRLOSS_DET_H_ */
