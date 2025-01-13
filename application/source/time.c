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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "time.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Global Variables
 ******************************************************************************/

uint32_t LPI2C2_GetFreq(void)
{
    return CLOCK_GetLPFlexCommClkFreq(2u);
}

uint8_t TIME_InitIRTC(void)
{
	irtc_datetime_t time;
	irtc_config_t irtcCfg;
	RTC_time_t rtc_time;
	RTC_date_t rtc_date;

    /**
     * irtcCfg.wakeupSelect = true;
     * irtcCfg.timerStdMask = false;
     * irtcCfg.alrmMatch 	= kRTC_MatchSecMinHr;
     */
    IRTC_GetDefaultConfig(&irtcCfg);
    if (kStatus_Fail == IRTC_Init(RTC, &irtcCfg))
    {
        return 1;
    }

	if (SUCCESS != RTC_Init(&I2C_MASTER))
	{
		PRINTF("ERR: Init. RTC Failed\r\n");
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
	}


	memset(&rtc_time, 0U, sizeof(rtc_time));
	memset(&rtc_date, 0U, sizeof(rtc_date));

	RTC_GetTime(&rtc_time);
	RTC_GetDate(&rtc_date);
#if 0
	time.day 	= (uint8_t) rtc_date.day;
	time.month 	= (uint8_t) rtc_date.month;
	time.year	= (uint16_t)rtc_date.year;

	time.hour	= (uint8_t) rtc_time.hrs;
	time.minute	= (uint8_t) rtc_time.min;
	time.second = (uint8_t) rtc_time.sec;
#else
	time.day 	= (uint8_t) 2;
	time.month 	= (uint8_t) 2;
	time.year	= (uint16_t)2;

	time.hour	= (uint8_t) 3;
	time.minute	= (uint8_t) 3;
	time.second = (uint8_t) 3;
#endif
    IRTC_SetDatetime(RTC, &time);

    IRTC_GetDatetime(RTC, &time);
	PRINTF("\r\nAlarm Time is %d/%d/%d %d:%d:%2d\r\n", time.year, time.month, time.day,
			time.hour, time.minute, time.second);

}
