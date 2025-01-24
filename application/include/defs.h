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

#define UART_FIFO_LENGHT		(1u)

#define UART_PRINT_ENABLED		(false)
/**
 * @brief Enables Timer For 1 Sec. Adjustment.
 */
#define CTIMER_ENABLED			(false)

#define IRTC_ENABLED			(true)

#if (CTIMER_ENABLED == IRTC_ENABLED)
	/*#error "ERR: Bad Selection of Time Adjustement Function!"*/
#else
	#if (CTIMER_ENABLED == true)
		#define TIME_ADJUSTEMENT
	#endif
#endif /* (CTIMER_ENABLED == IRTC_ENABLED) */

/**
 * @brief Configuration File.
 */
#define CONFIG_FILE 			"CONFIG"


#endif /* DEFS_H_ */
