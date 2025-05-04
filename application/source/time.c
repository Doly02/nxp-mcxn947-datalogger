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
 * Justification: <time.h> is intentionally used for management of timestamps and other
 * time-related operations in a controlled and limited scope. Usage of "time.h" usage is
 * intentional and controlled.
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


	irtcCfg.alrmMatch 			= kRTC_MatchSecMinHr;
	irtcCfg.clockSelect 		= kIRTC_Clk16K;
	irtcCfg.disableClockOutput  = true;
    if (kStatus_Fail == IRTC_Init(RTC, &irtcCfg))
    {
    	PRINTF("ERR: Init. Internal-RTC Failed\r\n");

    	/* MISRA Deviation Note:
		 * Rule: MISRA 2012 Rule 1.3 [Required]
		 * Justification: The macro 'ERROR_IRTC' is defined in 'error.h', which is included indirectly
		 * via 'time.h'. This deviation is safe and intentional.
		 */
		/*lint -e40 MISRA Deviation: identifier declared via indirect header include */
		return ERROR_IRTC;
		/*lint +e40 */
    }

	/* MISRA Deviation Note:
	 * Rule: MISRA 2012 Rule 1.3 [Required]
	 * Justification: The macros 'ERROR_IRTC' and 'ERROR_NONE' are defined in 'error.h', which is included indirectly
	 * via 'time.h'. This deviation is safe and intentional.
	 */
	/*lint -e40 MISRA Deviation: identifier declared via indirect header include */
	if (ERROR_NONE != RTC_Init())
	{
		PRINTF("ERR: Init. External-RTC Failed\r\n");

		return ERROR_IRTC;
	}
	/*lint +e40 */

	/* MISRA Deviation Note:
	 * Rule: MISRA 2012 Rule 1.3 [Required]
	 * Justification: The macro 'ERROR_NONE' is defined in 'error.h', which is included indirectly
	 * via 'time.h'. This deviation is safe and intentional.
	 */
	/*lint -e40 MISRA Deviation: identifier declared via indirect header include */
	return ERROR_NONE;
	/*lint +e40 */

}


error_t TIME_SetTime(void)
{

	irtc_datetime_t time;
	RTC_time_t rtc_time;
	RTC_date_t rtc_date;
	status_t status = 2;

	uint16_t year;
	uint8_t month, day, date, weekDay, hour, minute, second;

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

	day 	= (uint8_t) rtc_date.date;
	weekDay = (uint8_t) rtc_date.day;
	month 	= (uint8_t) rtc_date.month;
	year	= (uint16_t)(2000 + rtc_date.year);

	hour	= (uint8_t) rtc_time.hrs;
	minute	= (uint8_t) rtc_time.min;
	second = (uint8_t) rtc_time.sec;

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

#if (true == DEBUG_ENABLED)
	uint32_t u32year = 2000 + rtc_date.year;
	PRINTF("DEBUG: External RTC=%d/%d/%d %d:%d:%2d\r\n", u32year, rtc_date.month, rtc_date.date,
			rtc_time.hrs, rtc_time.min, rtc_time.sec);
#endif /* (true == DEBUG_ENABLED) */

    status = IRTC_SetDatetime(RTC, &time);
    if (0 != status)
    {
		/* MISRA Deviation Note:
		 * Rule: MISRA 2012 Rule 1.3 [Required]
		 * Justification: The macro 'ERROR_IRTC' is defined in 'error.h', which is included indirectly
		 * via 'time.h'. This deviation is safe and intentional.
		 */
		/*lint -e40 MISRA Deviation: identifier declared via indirect header include */
		return ERROR_IRTC;
		/*lint +e40 */
    }


#if (true == DEBUG_ENABLED)
    IRTC_GetDatetime(RTC, &time);
	PRINTF("DEBUG: Internal RTC=%d/%d/%d %d:%d:%2d\r\n", time.year, time.month, time.day,
			time.hour, time.minute, time.second);
#endif /* (true == DEBUG_ENABLED) */

	/* MISRA Deviation Note:
	 * Rule: MISRA 2012 Rule 1.3 [Required]
	 * Justification: The macro 'ERROR_NONE' is defined in 'error.h', which is included indirectly
	 * via 'time.h'. This deviation is safe and intentional.
	 */
	/*lint -e40 MISRA Deviation: identifier declared via indirect header include */
	return ERROR_NONE;
	/*lint +e40 */
}
