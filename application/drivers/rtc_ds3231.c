/*
 *		Author: 		Tomas Dolak
 * 		File Name:		rtc_ds3231.c
 *      Description:	RTC DS3231 Driver.
 *		Created on: 	Aug 7, 2024
 *
 *		@author			Tomas Dolak
 * 		@brief			RTC DS3231 Driver.
 * 		@filename		rtc_ds3231.c
 */



/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rtc_ds3231.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define I2C_DATA_LENGTH            	(2) /* MAX is 256 */

/*
 * @brief Other Definitions.
 */
#define BYTE_1						(0U)
#define BYTE_2						(1U)
#define CLK_STATE(x)				(uint8_t)((x >> 7) & 0x1)
/*
 * @brief 	OSF Bit (Oscillator Stop Flag) Indicates Whether The Oscillator Has Stopped.
 * 			If The Oscillator Has Stopped THe OSF Bit is Set To 1.
 */
#define OSF_STATE(x)

/*******************************************************************************
 * Global Variables.
 ******************************************************************************/
AT_NONCACHEABLE_SECTION(uint8_t g_master_txBuff[I2C_DATA_LENGTH]);
AT_NONCACHEABLE_SECTION(uint8_t g_master_rxBuff[I2C_DATA_LENGTH]);
volatile bool g_MasterCompletionFlag = false;
ARM_DRIVER_I2C *i2cDriver;

/*******************************************************************************
 * Implementation of Functions
 ******************************************************************************/
static void I2C_MasterCallback(uint32_t event)
{
    switch (event)
    {
        /* The master has sent a stop transition on the bus */
        case ARM_I2C_EVENT_TRANSFER_DONE:
            g_MasterCompletionFlag = true;
            break;

        default:
            break;
    }
}

uint8_t RTC_Init(ARM_DRIVER_I2C *pI2c)
{
	/* Initialize Peripherals */
	i2cDriver = pI2c;
	/* Initialize the I2C Bus */
	i2cDriver->Initialize(I2C_MasterCallback);
	i2cDriver->PowerControl(ARM_POWER_FULL);

    /* Change the default baudrate configuration */
	i2cDriver->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);

	/* Disable Alarms */
	RTC_CtrlAlarm1(ALARM_DISABLED);
	RTC_CtrlAlarm2(ALARM_DISABLED);
	RTC_ClearFlagAlarm1();
	RTC_ClearFlagAlarm2();

	/* Set Interrupt Mode */
	RTC_SetInterruptMode(ALARM_INTERRUPT);

	return SUCCESS;

}

/*
 * @brief General Functions.
 */
static uint8_t RTC_ConvertToBCD(uint8_t dec)
{
	/* BCD Is Not Defined For Numbers Greater Then 99 */
	 assert(dec < 99U);

	 return (dec % 10 + ((dec / 10) << 4));
}

static uint8_t RTC_ConvertToDEC(uint8_t bcd)
{
	return (((bcd & 0xf0) >> 4) * 10) + (bcd & 0x0f);
}

RTC_osc_state_t RTC_GetState(void)
{
	uint8_t bOscState = 0xFFu;		//<! Oscillator State
	bOscState = RTC_Read(DS3231_REG_STATUS);
	bOscState = ((bOscState >> 7) & 0xFFu);

	if (0x01u == bOscState)
	{
		/*
		 * A logic 1 in this bit indicates that the oscillator either is stopped or
		 * was stopped for some period and may be used to judge
		 * the validity of the time-keeping data.
		 *
		 * This bit is set to logic 1 any time that the oscillator stops.
		 * The following are examples of conditions that can cause the OSF bit to be set:
		 *
		 * 1) The first time power is applied.
		 * 2) The voltages present on both VCC and VBAT are insufficient to support oscillation.
		 * 3) The EOSC bit is turned off in battery-backed mode.
		 * 4) External influences on the crystal (i.e., noise, leakage, etc.).
		 *
		 * This bit remains at logic 1 until written to logic 0.
		 */
		return OSC_INTERRUPTED;
	}
	else
		return OSC_OK;
}

void RTC_SetOscState(RTC_osc_state_t state)
{
	if (OSC_OK == state)
	{
		// Clear The Flag
		// Set The Last Bit Of The Register To 0 By And With 127 (01111111b)
		RTC_Write(DS3231_REG_STATUS, ( 0x7Fu & RTC_Read(DS3231_REG_STATUS)));
	}
	else if (OSC_INTERRUPTED == state)
	{
		RTC_Write(DS3231_REG_STATUS, ( 0x80u | RTC_Read(DS3231_REG_STATUS)));
	}
	else
	{
		; // MISRA Compliance :)
	}
}



/*
 * @brief RTC Specific Functions.
 */
void RTC_Write(uint8_t regAddress, uint8_t val)
{
	g_master_txBuff[BYTE_1] = regAddress;
	g_master_txBuff[BYTE_2] = val;

	/* Transmit Data To The RTC Slave */
	i2cDriver->MasterTransmit(DS3231_ADDR_I2C, g_master_txBuff, I2C_DATA_LENGTH, false);

	/*wait for master complete*/
    while (!g_MasterCompletionFlag)
    {
    }

    /*  Reset master completion flag to false. */
    g_MasterCompletionFlag = false;

	return;
}

uint8_t RTC_Read(uint8_t regAddress)
{
	uint8_t ui8ReceivedData = 0xFFU;

	i2cDriver->MasterTransmit(DS3231_ADDR_I2C, &regAddress, 1, false);
    while (!g_MasterCompletionFlag)
    {
    }
    /*  Reset master completion flag to false. */
    g_MasterCompletionFlag = false;

    i2cDriver->MasterReceive(DS3231_ADDR_I2C, &ui8ReceivedData, 1, false);

	/*wait for master complete*/
    while (!g_MasterCompletionFlag)
    {
    }

    /*  Reset master completion flag to false. */
    g_MasterCompletionFlag = false;
	return ui8ReceivedData;
}


void RTC_SetTime(RTC_time_t *pTime)
{
	uint8_t ui8BcdNum = 0xFF;

	/* Set Seconds */
	ui8BcdNum = RTC_ConvertToBCD(pTime->sec);
	RTC_Write(DS3231_ADDR_SEC, ui8BcdNum);

	/* Set Minutes */
	RTC_Write(DS3231_ADDR_MIN, RTC_ConvertToBCD(pTime->min));

	/* Write Value of Hours Into RTC Register (In 24H Format) */
	ui8BcdNum = RTC_ConvertToBCD(pTime->hrs  & 0x3Fu);
	RTC_Write(DS3231_ADDR_HRS, ui8BcdNum);

	return;
}


void RTC_GetTime(RTC_time_t *pTime)
{
	uint8_t ui8Val;
	uint8_t ui8Format;

	ui8Val = RTC_Read(DS3231_ADDR_SEC);
	ui8Val = RTC_ConvertToDEC(ui8Val);
	pTime->sec = ui8Val;

	ui8Val = RTC_Read(DS3231_ADDR_MIN);
	ui8Val = RTC_ConvertToDEC(ui8Val);
	pTime->min = ui8Val;

	ui8Val = RTC_Read(DS3231_ADDR_HRS);
	/* Convert From Binary Coded Decimal */
	pTime->hrs = RTC_ConvertToDEC(ui8Val);		//<! Hours In 24H Format.

	return;
}

void RTC_SetDate(RTC_date_t *pDate)
{
	RTC_Write(DS3231_ADDR_DATE, RTC_ConvertToBCD(pDate->date));
	RTC_Write(DS3231_ADDR_DAY, RTC_ConvertToBCD(pDate->day));
	RTC_Write(DS3231_ADDR_MONTH, RTC_ConvertToBCD(pDate->month));
	RTC_Write(DS3231_ADDR_YEAR, RTC_ConvertToBCD(pDate->year));

	return;
}

void RTC_GetDate(RTC_date_t *pDate)
{
	uint8_t ui8RegVal = 0xFFu;
	ui8RegVal = RTC_Read(DS3231_ADDR_DATE);
	pDate->date = RTC_ConvertToDEC(ui8RegVal);

	ui8RegVal = RTC_Read(DS3231_ADDR_DAY);
	pDate->day = RTC_ConvertToDEC(ui8RegVal);

	ui8RegVal = RTC_Read(DS3231_ADDR_MONTH);
	pDate->month = RTC_ConvertToDEC(ui8RegVal);

	ui8RegVal = RTC_Read(DS3231_ADDR_YEAR);
	pDate->year = RTC_ConvertToDEC(ui8RegVal);

	return;
}
/*
 * -----------------------------------------------------------------------------------------
 */
void RTC_SetInterruptMode(RTC_interrupt_mode_t mode)
{
	uint8_t ctrl = RTC_Read(DS3231_REG_CTRL);
	RTC_Write(DS3231_REG_CTRL, (ctrl & 0xfb) | ((mode & 0x01) << DS3231_INTCN));
}

void RTC_CtrlAlarm1(uint8_t enable)
{
	uint8_t ctrl = RTC_Read(DS3231_REG_CTRL);
	RTC_Write(DS3231_REG_CTRL,(ctrl & 0xfe) | ((enable & 0x01) << DS3231_A1IE));
	RTC_SetInterruptMode(ALARM_INTERRUPT);

}
void RTC_ClearFlagAlarm1(void)
{
	uint8_t status = (RTC_Read(DS3231_REG_STATUS) & 0xfe);
	RTC_Write(DS3231_REG_STATUS, status & ~(0x01 << DS3231_A1F));
}

void RTC_CtrlAlarm2(uint8_t enable)
{
	uint8_t ctrl = RTC_Read(DS3231_REG_CTRL);
	RTC_Write(DS3231_REG_CTRL, (ctrl & 0xfd) | ((enable & 0x01) << DS3231_A2IE));
	RTC_SetInterruptMode(ALARM_INTERRUPT);
}

void RTC_ClearFlagAlarm2(void)
{
	uint8_t status = (RTC_Read(DS3231_REG_STATUS) & 0xfd);
	RTC_Write(DS3231_REG_STATUS, status & ~(0x01 << DS3231_A2F));
}

/*
 * -----------------------------------------------------------------------------------------
 */
void RTC_SetTimeDefault(RTC_time_t *pTime)
{
	pTime->format = TIM_CYCLE_24H;
	pTime->hrs = 10;
	pTime->min = 48;
	pTime->sec = 48;
}

void RTC_SetDateDefault(RTC_date_t *pDate)
{
	pDate->date = 6;
	pDate->month = 9;
	pDate->year = 2;
	pDate->day = FRIDAY;
}

