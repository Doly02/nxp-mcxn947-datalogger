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
#include "time.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define INTERNAL_RTC_BASE_YEAR 			2112

/**
 * @brief The Year Range Is Between 0 and 99.
 */
#define EXTERNAL_RTC_TIME_OFSET			2000
/*******************************************************************************
 * Global Variables
 ******************************************************************************/

uint32_t LPI2C2_GetFreq(void)
{
    return CLOCK_GetLPFlexCommClkFreq(2u);
}

uint8_t ConvertTo24HourFormat(uint8_t hours)
{
	if (hours == 12) {
		hours = 0;
	}

    return hours;
}

error_t TIME_InitIRTC(void)
{
	irtc_datetime_t time;
	irtc_config_t irtcCfg;
	RTC_time_t rtc_time;
	RTC_date_t rtc_date;
	status_t status = 2;

    /**
     * irtcCfg.wakeupSelect = true;
     * irtcCfg.timerStdMask = false;
     * irtcCfg.alrmMatch 	= kRTC_MatchSecMinHr;
     */
    IRTC_GetDefaultConfig(&irtcCfg);
    if (kStatus_Fail == IRTC_Init(RTC, &irtcCfg))
    {
    	/*TODO: Set ERROR */
    	PRINTF("ERR: Init. Internal-RTC Failed\r\n");
    	return ERROR_IRTC;
    }

	if (ERROR_NONE != RTC_Init(&I2C_MASTER))
	{
		PRINTF("ERR: Init. External-RTC Failed\r\n");
		/*TODO: Set ERROR */
		return ERROR_IRTC;
	}

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


	memset(&rtc_time, 0U, sizeof(rtc_time));
	memset(&rtc_date, 0U, sizeof(rtc_date));

	RTC_GetTime(&rtc_time);
	RTC_GetDate(&rtc_date);

	time.day 	= (uint8_t) rtc_date.date;
	time.month 	= (uint8_t) rtc_date.month;
	time.year	= (uint16_t)(2000 + rtc_date.year);

	time.hour	= (uint8_t) rtc_time.hrs;
	time.minute	= (uint8_t) rtc_time.min;
	time.second = (uint8_t) rtc_time.sec;


#if (true == DEBUG_ENABLED)
	uint32_t year = 2000 + rtc_date.year;
	PRINTF("DEBUG: External RTC=%d/%d/%d %d:%d:%2d\r\n", year, rtc_date.month, rtc_date.date,
			rtc_time.hrs, rtc_time.min, rtc_time.sec);
#endif /* (true == DEBUG_ENABLED) */

    status = IRTC_SetDatetime(RTC, &time);
    if (0 != status)
    {
    	return ERROR_IRTC;
    }

    // SDK_DelayAtLeastUs(3, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    IRTC_GetDatetime(RTC, &time);

    uint32_t hrs = ConvertTo24HourFormat(time.hour);

#if (true == DEBUG_ENABLED)
	PRINTF("DEBUG: Internal RTC=%d/%d/%d %d:%d:%2d\r\n", time.year, time.month, time.day,
			hrs, time.minute, time.second);
#endif /* (true == DEBUG_ENABLED) */

	return ERROR_NONE;
}
