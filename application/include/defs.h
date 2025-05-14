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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdbool.h>


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define NOT_IMPLEMENTED				#error "This Feature Is Not Implemented!"

/**
 * @brief 	Defines The Stack Size For Mass Storage Task.
 */
#define MSC_STACK_SIZE        ((uint32_t)(5000UL / (uint32_t)sizeof(portSTACK_TYPE)))

/**
 * @brief 	Defines The Stack Size For Recording Task.
 */
#define RECORD_STACK_SIZE     ((uint32_t)(5000UL / (uint32_t)sizeof(portSTACK_TYPE)))

/**
 * @brief Enables/Disables Mass Storage Functionality.
 * */
#define MSC_ENABLED					(true)

/**
 * @brief 	Enables/Disables Debug Mode
 * @details If Debug Mode Is Enabled Then Extended Logs Are Printed Into Debug Console.
 */
#define INFO_ENABLED				(false)

/**
 * @brief 	Enables/Disables Debug Mode
 * @details If Debug Mode Is Enabled Then Extended Logs Are Printed Into Debug Console.
 */
#define DEBUG_ENABLED				(false)

/**
 * @brief Enables/Disables HW FIFO Queue on Application LPUART.
 */
#define UART_FIFO_ENABLED			(true)

/**
 * @brief Defines The Size of HW FIFO Queue.
 */
#define UART_FIFO_LENGHT			(4u)

/**
 * @brief Enables/Disables Print of Received Bytes From Application LPUART To Console.
 */
#define UART_PRINT_ENABLED			(false)

/**
 * @brief Enables/Disables Power Loss Detection.
 */
#define PWRLOSS_DETECTION_ENABLED	(true)

/**
 * @brief Enables/Disables GPIO For Testing Power Loss Detection.
 */
#define PWRLOSS_TEST_GPIOS			(false)

/**
 * @brief 	Constant 5 Tau, When Back-Up Power Capacitor is Charged To 99%.
 * @details In Seconds (It's Actualy 16.5s).
 */
#define TAU5						16.5

/**
 * @brief 	Time Interval When Power Loss Detection Became Active.
 * @details	In Seconds.
 */
#define PWRLOSS_DET_ACTIVE_IN_TIME	TAU5

/**
 * @brief Priority of LP_FLEXCOMM Interrupt (UART) For Rx Of Recorded Data.
 */
#define UART_RECEIVE_PRIO			(6U)

/**
 * @brief Priority of Comparator Interrupt For Power Loss Detection.
 */
#define PWRLOSS_DET_PRIO			(5U)

/**
 * @brief Priority of Comparator Interrupt For Power Loss Detection.
 */
#define PWRLOSS_TIMER_PRIO			(7U)


/**
 * @brief Enables/Disables Temperature Recording.
 */
#define TEMPERATURE_MEAS_ENABLED	(false)

/**
 * @brief Enables/Disables Signaling By LEDs.
 */
#define CONTROL_LED_ENABLED			(true)


/**
 * @brief	Default Maximal File Size If The Configuration File Could
 * 			Not Be Read Properly.
 */
#define DEFAULT_MAX_FILESIZE		8192

/**
 * @brief Configuration File.
 */
#define CONFIG_FILE 				"config"

/**
 * @brief 	Default Baud Rate If The Configuration File Could Not Be
 * 			Read Properly.
 */
#define DEFAULT_BAUDRATE			230400UL

/**
 * @brief	Default Number of Data Bits If The Configuration File Could
 * 			Not Be Read Properly.
 */
#define DEFAULT_DATA_BITS			kLPUART_EightDataBits

/**
 * @brief	Default Number of Stop Bits If The Configuration File Could
 * 			Not Be Read Properly.
 */
#define DEFAULT_STOP_BITS			kLPUART_OneStopBit

/**
 * @brief	Default UART Parity If The Configuration File Could
 * 			Not Be Read Properly.
 */
#define DEFAULT_PARITY				kLPUART_ParityDisabled

/**
 * @brief	Defines The Threshold Level of Free Memory on The SD card,
 * 			Below Which The Lack of Memory is Indicated.
 * @details In Megabytes.
 */
#define DEFAULT_FREE_SPACE			50UL

#endif /* DEFS_H_ */
