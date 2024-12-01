/*
 *		Author: 		Tomas Dolak
 * 		File Name:		rtc_ds1307.c
 *      Description:	RTC DS1307 Driver.
 *		Created on: 	Aug 7, 2024
 *
 *		@author			Tomas Dolak
 * 		@brief			RTC DS1307 Driver.
 * 		@filename		rtc_ds1307.c
 */

#ifndef DEFS_H_
#define DEFS_H_

#define SUCCESS					(0U)
#define E_FAULT					(1U)
#define NOT_IMPLEMENTED			#error "This Feature Is Not Implemented!"

#define RTC_ENABLED				(true)
/*
 * @brief Enables Mass Storage Functionality.
 * */
#define MSC_ENABLED				(true)

#define MSC_STACK_SIZE			(5000L / sizeof(portSTACK_TYPE))

#define RECORD_STACK_SIZE		(5000L / sizeof(portSTACK_TYPE))

#define DEBUG_ENABLED			(true)

#define UART_FIFO_ENABLED		(true)
/**
 * @brief Enables Timer For 1 Sec. Adjustment.
 */
#define TIMER_ENABLED			(false)
/*
 * @brief Enables VBUS Detection on USB-C0.
 */
#define USB0_DET_PIN_ENABLED 	(false)

/**
 * @brief Configuration File.
 */
#define CONFIG_FILE "config"


#endif /* DEFS_H_ */
