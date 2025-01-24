/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      defs.h
 *  Author:         Tomas Dolak
 *  Date:           22.09.2024
 *  Description:    Header File Providing Definitions To Set Up The Project.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           defs.h
 *  @author         Tomas Dolak
 *  @date           22.09.2024
 *  @brief          Header File Providing Definitions To Set Up The Project.
 * ****************************/

#ifndef DEFS_H_
#define DEFS_H_

#define NOT_IMPLEMENTED			#error "This Feature Is Not Implemented!"

/**
 * @brief 	Defines The Stack Size For Mass Storage Task.
 */
#define MSC_STACK_SIZE			(5000L / sizeof(portSTACK_TYPE))

/**
 * @brief 	Defines The Stack Size For Recording Task.
 */
#define RECORD_STACK_SIZE		(5000L / sizeof(portSTACK_TYPE))

/*
 * @brief Enables/Disables Logic For External Real-Time Circuit DS3231.
 */
#define RTC_ENABLED				(true)

/*
 * @brief Enables/Disables Mass Storage Functionality.
 * */
#define MSC_ENABLED				(true)

/**
 * @brief 	Enables/Disables Debug Mode
 * @details If Debug Mode Is Enabled Then Extended Logs Are Printed Into Debug Console.
 */
#define DEBUG_ENABLED			(true)

/**
 * @brief Enables/Disables HW FIFO Queue on Application LPUART.
 */
#define UART_FIFO_ENABLED		(true)

/**
 * @brief Defines The Size of HW FIFO Queue.
 */
#define UART_FIFO_LENGHT		(1u)

/**
 * @brief Enables/Disables Print of Received Bytes From Application LPUART To Console.
 */
#define UART_PRINT_ENABLED		(false)
/**

 * @brief Enables Timer For 1 Sec. Adjustment.
 */
#define CTIMER_ENABLED			(false)

/**
 * @brief Enables/Disables Logic For Internal Real-Time Circuit.
 */
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
