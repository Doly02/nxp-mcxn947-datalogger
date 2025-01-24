/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      ctimer.c
 *  Author:         Tomas Dolak
 *  Date:           18.11.2024
 *  Description:    Implements Datalogger Application.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           ctimer.c
 *  @author         Tomas Dolak
 *  @date           18.11.2024
 *  @brief          Implements Datalogger Application.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "ctimer.h"

#if 0
/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief Variable That Will Simulate Time By Timer.
 * */
static RTC_time_t actTime;

/**
 * @brief Variable That Will Simulate Date By Timer.
 * */
static RTC_date_t actDate;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * @brief Functions of I2C That Are Used For Correct Work of RTC.
 */
uint32_t LPI2C2_GetFreq(void)
{
    return LPI2C_CLOCK_FREQUENCY;
}

void CTIMER0_IRQHandler(void)
{
	CTIMER_ClearStatusFlags(CTIMER0, kCTIMER_Match0Flag);
	TIMER_UpdateSimulatedTime();
	PRINTF("\r\nTime: %02d:%02d:%02d \r\n",
	           actTime.hrs, actTime.min, actTime.sec);
}

void TIMER_Init(void)
{
	uint8_t retVal = E_FAULT;
	ctimer_config_t config;
    ctimer_match_config_t matchConfig;

    /* Enable The CTimer */
    CLOCK_SetClkDiv(kCLOCK_DivCtimer0Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_CTIMER0);

    CTIMER_GetDefaultConfig(&config);
    config.prescale = 48 - 1;

    CTIMER_Init(CTIMER0, &config);

    matchConfig.enableInterrupt = true;         		// Enable Interrupt If Match
	matchConfig.enableCounterReset = true;      		// Reset Timer If Match
	matchConfig.enableCounterStop = false;      		// Count After Match
	matchConfig.matchValue = 1000000 - 1;               // Match Value (1M/cycles -> 1Hz)
	matchConfig.outControl = kCTIMER_Output_NoAction; 	// Output Pin Off

	/* Setup Match Register */
	CTIMER_SetupMatch(CTIMER0, kCTIMER_Match_0, &matchConfig);

	/* Enable IRQ */
	EnableIRQ(CTIMER0_IRQn);

	retVal = RTC_Init(&I2C_MASTER);
	if (SUCCESS != retVal)
	{
		PRINTF("ERR: Init. RTC Failed\r\n");
	}

	/* Load The State of Real-Time Circuit */
	retVal = RTC_GetState();
	if (OSC_STOPPED == retVal)	// If The Oscillator Was Stopped -> Set Time & Date
	{
		/* Set Default Time & Date (Prepare Variables) */
		RTC_SetDateDefault(&actDate);
		RTC_SetTimeDefault(&actTime);

		/* Set Time & Date Into RTC */
		RTC_SetDate(&actDate);
		RTC_SetTime(&actTime);

		PRINTF("ERR: Oscillator Has Been Stopped!\r\n");
		RTC_SetOscState(OSC_OK);
	}


	memset(&actTime, 0U, sizeof(actTime));
	memset(&actDate, 0U, sizeof(actDate));

	RTC_GetTime(&actTime);
	RTC_GetDate(&actDate);

	/* Start Timer */
	CTIMER_StartTimer(CTIMER0);

}

void TIMER_UpdateSimulatedTime(void)
{
	actTime.sec++;
	if (60 <= actTime.sec)
	{
		actTime.sec = 0;
		actTime.min++;
		if (60 <= actTime.min)
		{
			actTime.min = 0;
			actTime.hrs++;
			if (24 <= actTime.hrs)
			{
				actTime.hrs = 0;
				actDate.date++;
				if (actDate.date > TIMER_GetDaysInMonth(actDate.month, actDate.year))
				{
					actDate.date = 1;
					actDate.month++;
					if (actDate.month > 12)
					{
						actDate.month = 1;
						actDate.year++;
					}
				}
			}
		}
	}
}

uint8_t TIMER_GetDaysInMonth(uint8_t month, uint8_t year)
{
    static const uint8_t daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (month == 2) // Febluary
    {
        /* Leap Year */
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
        {
            return 29;
        }
    }
    return daysInMonth[month - 1];
}
#endif
