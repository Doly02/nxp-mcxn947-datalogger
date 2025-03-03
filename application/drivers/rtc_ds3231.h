/*
 *		Author: 		Tomas Dolak
 * 		File Name:		rtc_DS3231.h
 *      Description:	Header File to RTC DS3231 Driver.
 *		Created on: 	Aug 7, 2024
 *
 *		@author			Tomas Dolak
 * 		@brief			Header File to RTC DS3231 Driver.
 * 		@filename		rtc_DS3231.h
 */

#ifndef RTC_DS3231_H_
#define RTC_DS3231_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>

#include "defs.h"

#include "fsl_lpi2c.h"
#include "fsl_lpi2c_edma.h"
#include "fsl_edma.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define E_FAULT 					1
/**
 * @brief 	I2C DMA Channel For Transmission.
 */
#define LPI2C_TX_DMA_CHANNEL       	0U

/**
 * @brief 	I2C DMA Channel For Reception.
 */
#define LPI2C_RX_DMA_CHANNEL    	1U

/**
 * @brief	Connection Between DMA Channel 0 and LP_FLEXCOMM2 Tx.
 */
#define LPI2C_TX_CHANNEL 			kDma0RequestMuxLpFlexcomm2Tx

/**
 * @brief	Connection Between DMA Channel 0 and LP_FLEXCOMM2 Rx.
 */
#define LPI2C_RX_EDMA_CHANNEL  		kDma0RequestMuxLpFlexcomm2Rx

/**
 * @brief Points To I2C Peripheral Unit (Specifically LPI2C2 Instance).
 */
#define I2C_MASTER 					((LPI2C_Type *)LPI2C2_BASE)

/*
 * @brief Alarm Interrupt Enable Bits.
 */
#define DS3231_A1IE				(0x00u)
#define DS3231_A2IE				(0x1u)
/*
 * @brief Interrupt Control Bit.
 */
#define DS3231_INTCN			(0x2u)
/*
 * @brief 	Alarm 1 & Alarm 2 Flags.
 * @details A Logic 1 in The Alarm 'x' Flag Bit Indicates That The Time Matched The Alarm 'x' Registers.
 */
#define DS3231_A1F				(0x0u)
#define DS3231_A2F				(0x1u)

#define ALARM_DISABLED			(0x0u)
/*
 * @brief Indicates That Oscillator Has Stopped & Time Has To Updated.
 */
#define OSC_STOPPED				(0x00u)

/*
 * @brief Registers Addresses of DS3231.
 */
#define DS3231_ADDR_SEC			(0x00U)
#define DS3231_ADDR_MIN			(0x01U)
#define DS3231_ADDR_HRS			(0x02U)
#define DS3231_ADDR_DAY			(0x03U)

#define DS3231_ADDR_DATE		(0x04U)
#define DS3231_ADDR_MONTH		(0x05U)
#define DS3231_ADDR_YEAR		(0x06U)
#define DS3231_ADDR_CENT		(0x07U)
/*
 * @brief Address of Status Register.
 */
#define DS3231_REG_STATUS		(0x0Fu)
/*
 * @brief Address of Control Register.
 */

#define DS3231_REG_CTRL 		(0x0Eu)
/*
 * @brief Address of I2C Slave.
 */
#define DS3231_ADDR_I2C			(0x68U)	//<! The Slave Address Byte Contains In 7-bit: 1101000

/*
 * @brief Definition of Dates.
 */
#define SUNDAY					(0x1)
#define MONDAY					(0x2)
#define TUESDAY					(0x3)
#define WEDNESDAY				(0x4)
#define THURSDAY				(0x5)
#define FRIDAY					(0x6)
#define SATURDAY				(0x7)

/*
 * @brief Definitions For Time Format Handling.
 */
#define TIM_CYCLE_12H			(0x0)
#define TIM_CYCLE_12H_AM		(0x0)
#define TIM_CYCLE_12H_PM		(0x1)
#define TIM_CYCLE_24H			(0x2)

/*
 * @brief Application Configurable Items.
 */


/**
 * @brief 	Desired Baud Rate For I2C Bus.
 * @details Frequency - 100kHz (Up To 400kHz -> Defined By DS3231).
 */
#define I2C_BAUDRATE               	100000U


#define APP_SUCCESS					0
/*******************************************************************************
 * Structures
 ******************************************************************************/
/*
 * @brief Structure for Keeping Time.
 */
typedef struct
{
	uint8_t format;			//<! Time Format (e.g. AM/PM or 24H Cycle)
	uint8_t sec;
	uint8_t min;
	uint8_t hrs;

} RTC_time_t;

/*
 * @brief Structure for Keeping Date.
 */
typedef struct
{
	uint8_t date;
	uint8_t day;
	uint8_t month;
	uint8_t year;

} RTC_date_t;

/*
 * @brief An Enum to Capture The State of The Oscillator,
 */
typedef enum
{
	OSC_OK = 0,
	OSC_INTERRUPTED		//<! RTC Oscillator Was Interrupted -> Need to Update the Time

} RTC_osc_state_t;

/*
 * @brief An Enum For Interrupt Mode.
 */
typedef enum
{
	SQUARE_WAVE_INTERRUPT = 0,
	ALARM_INTERRUPT

}RTC_interrupt_mode_t;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*
 * @brief Real-Time Circuit Related Functions.
 */

/*
 * @brief 				Initialize The RTC DS3231.
 * @param pI2c			Pointer To I2C Structure.
 * @details				Before Calling of RTC_Init Function Is Important To Prepare I2C
 * 						(To Keep The Driver As Universal As Possible, e.g To Use I2C With DMA,
 * 						Interrupt/Polling Mode,...).
 */
uint8_t RTC_Init(void);
/*
 * @brief 				Converts Numbers From Decimal Base To BCD Base.
 * @param dec			Decimal Number.
 * @return				Number in Binary Coded Decimal.
 */
static uint8_t RTC_ConvertToBCD(uint8_t dec);

/*
 * @brief 				Converts Numbers From BCD Base To Decimal Base.
 * @param bcd			Binary-Coded Decimal Number.
 * @return				Number in Decimal Base.
 */
static uint8_t RTC_ConvertToDEC(uint8_t bcd);

/*
 * @brief 				This Function Checks If The Oscillator Is Still Running.
 * @returns				Function Returns 1 If The Oscillator Is Running. If The Oscillator Has Stopped
 * 						Returns 0.
 */
uint8_t RTC_GetState(void);

/*
 * @brief 				Sets The Oscillator Stop Flag (OSF).
 * @param state 		State of The Flag.
 */
void RTC_SetOscState(RTC_osc_state_t state);

/*
 * @brief 				Writes Value Into RTC Registers.
 * @param regAddress 	Address of Register To Which Will Be Value Written.
 * @param val			Value That Will Be Writen Into Register.
 * @return				void
 */
uint8_t RTC_Write(uint8_t regAddress, uint8_t val);

/*
 * @brief				Reads Value From RTC Register.
 * @param regAddress 	Address of Register From Which Will Be Read.
 * @return 				Read Value.
 */
uint8_t RTC_Read(uint8_t regAddress);


/*
 * @brief 				Sets Time.
 * @param pTime			Pointer To Time Structure.
 */
void RTC_SetTime(RTC_time_t *pTime);

/*
 * @brief 				Gets Time.
 * @param pTime			Pointer To Time Structure.
 */
void RTC_GetTime(RTC_time_t *pTime);

/*
 * @brief 				Sets Date.
 * @param pDate			Pointer To Date Structure.
 */
void RTC_SetDate(RTC_date_t *pDate);

/*
 * @brief 				Gets Date.
 * @param pDate			Pointer To Date Structure.
 */
void RTC_GetDate(RTC_date_t *pDate);

/*
 * @brief				Sets The INTCN Bit in Control Register.
 * @param mode			Interrupt Mode.
 */
void RTC_SetInterruptMode(RTC_interrupt_mode_t mode);

/*
 * @brief 				Enables/Disables The Alarm 1.
 * @param enable		Specifies If The Alarm Will Be Enabled or Not.
 */
void RTC_CtrlAlarm1(uint8_t enable);

/*
 * @brief				Clears The A1F Flag in Control Register.
 */
void RTC_ClearFlagAlarm1(void);
/*
 * @brief 				Enables/Disables The Alarm 2.
 * @param enable		Specifies If The Alarm Will Be Enabled or Not.
 */
void RTC_CtrlAlarm2(uint8_t enable);

/*
 * @brief				Clears The A1F Flag in Control Register.
 */
void RTC_ClearFlagAlarm2(void);

/*
 * @brief				Sets Time To Default.
 * @param pTime			Pointer to Time Structure That Will Be Configured To Default.
 */
void RTC_SetTimeDefault(RTC_time_t *pTime);

/*
 * @brief				Sets Date To Default.
 * @param pDate			Pointer to Date Structure That Will Be Configured To Default.
 */
void RTC_SetDateDefault(RTC_date_t *pDate);

#endif /* RTC_DS3231_H_ */
