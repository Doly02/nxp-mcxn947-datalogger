/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      mainc.c
 *  Author:         Tomas Dolak
 *  Date:           07.08.2024
 *  Description:    Implements The Logic Of Time-Keeping.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           main.c
 *  @author         Tomas Dolak
 *  @date           07.08.2024
 *  @brief          Implements The Logic Of Time-Keeping.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_irtc.h"
#include "rtc_ds3231.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"

/* MISRA Deviation: Rule 21.10
 * Suppress: Use Of Banned Standard Header 'Time.h'.
 * Justification: "time.h" is Intentionally Used For Management of Time Stamps and Other
 * Time-Related Operations in a Controlled and Limited Scope. Usage of "time.h" Usage is
 * Intentional and Controlled.
 */

/*lint -e829 */
#include "time.h"
/*lint +e829 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * @brief Base Year of IRTC.
 */
#define INTERNAL_RTC_BASE_YEAR 			2112

/**
 * @brief The Year Range Is Between 0 and 99.
 */
#define EXTERNAL_RTC_TIME_OFSET			2000
/*******************************************************************************
 * Global Variables
 ******************************************************************************/

error_t TIME_InitIRTC(void)
{
	irtc_config_t irtcCfg;
	status_t status = 2;
	irtc_datetime_t time;
	RTC_time_t rtc_time;
	RTC_date_t rtc_date;

	irtcCfg.alrmMatch 			= kRTC_MatchSecMinHr;
	irtcCfg.clockSelect 		= kIRTC_Clk16K;
	irtcCfg.disableClockOutput  = true;
    if (kStatus_Fail == IRTC_Init(RTC, &irtcCfg))
    {
    	PRINTF("ERR: Init. Internal-RTC Failed\r\n");

    	/* MISRA Deviation Note:
		 * Rule: MISRA 2012 Rule 1.3 [Required]
		 * Suppress: Identifier Declared via Indirect Header Include.
		 * Justification: The Macro 'ERROR_IRTC' is Defined in "error.h", Which is Included Indirectly
		 * via "time.h". This Deviation is Safe and Intentional.
		 */
		/*lint -e40 */
		return ERROR_IRTC;
		/*lint +e40 */
    }

	/* MISRA Deviation Note:
	 * Rule: MISRA 2012 Rule 1.3 [Required]
	 * Suppress: Identifier Declared via Indirect Header Include.
	 * Justification: The Macro 'ERROR_IRTC' is Defined in "error.h", Which is Included Indirectly
	 * via "time.h". This Deviation is Safe and Intentional.
	 */
	/*lint -e40 */
	if (ERROR_NONE != RTC_Init())
	{
		PRINTF("ERR: Init. External-RTC Failed\r\n");

		return ERROR_IRTC;
	}
	/*lint +e40 */

	/* Load The State of Real-Time Circuit */
	if (OSC_STOPPED == RTC_GetState())	// If The Oscillator Was Stopped -> Set Time & Date
	{
		/* Set Default Time & Date (Prepare Variables) */
		RTC_SetDateDefault(&rtc_date);
		RTC_SetTimeDefault(&rtc_time);

		/* Set Time & Date Into RTC */
		RTC_SetDate(&rtc_date);
		RTC_SetTime(&rtc_time);

		PRINTF("ERR: Oscillator Has Been Stopped!\r\n");
		RTC_SetOscState(OSC_OK);
		/* This Error Is Not Critical, So It's Possible To Continue */
	}


	(void)memset(&rtc_time, 0U, sizeof(rtc_time));
	(void)memset(&rtc_date, 0U, sizeof(rtc_date));

	RTC_GetTime(&rtc_time);
	RTC_GetDate(&rtc_date);

	time.day 	= (uint8_t) rtc_date.date;
	time.month 	= (uint8_t) rtc_date.month;
	time.year	= (uint16_t)(2000 + rtc_date.year);

	/* DS3231 Used 0x1 (Sunday) To 0x7 (Saturday)
	 * IRTC Uses 0x0 (Sunday) To 0x6 (Saturday)
	 * */
	time.weekDay = (uint8_t) (rtc_date.day - 1);

	time.hour	= (uint8_t) rtc_time.hrs;
	time.minute	= (uint8_t) rtc_time.min;
	time.second = (uint8_t) rtc_time.sec;

    status = IRTC_SetDatetime(RTC, &time);
    if (0 != status)
    {

    	/* MISRA Deviation Note:
    	 * Rule: MISRA 2012 Rule 1.3 [Required]
    	 * Suppress: Identifier Declared via Indirect Header Include.
    	 * Justification: The Macro 'ERROR_IRTC' is Defined in "error.h", Which is Included Indirectly
    	 * via "time.h". This Deviation is Safe and Intentional.
    	 */
		/*lint -e40 */
		return ERROR_IRTC;
		/*lint +e40 */
    }


	/* MISRA Deviation Note:
	 * Rule: MISRA 2012 Rule 1.3 [Required]
	 * Suppress: Identifier Declared via Indirect Header Include.
	 * Justification: The Macro 'ERROR_NONE' is Defined in "error.h", Which is Included Indirectly
	 * via "time.h". This Deviation is Safe and Intentional.
	 */
	/*lint -e40 */
	return ERROR_NONE;
	/*lint +e40 */
}
