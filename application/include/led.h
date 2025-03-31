/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      leds.c
 *  Author:         Tomas Dolak
 *  Date:           06.02.2025
 *  Description:    Implements The Logic For LEDs Control.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           leds.c
 *  @author         Tomas Dolak
 *  @date           06.02.2025
 *  @brief          Implements The Logic For LEDs Control.
 * ****************************/

#ifndef LED_H_
#define LED_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_gpio.h"
#include "board.h"
/*******************************************************************************
 * Functions Macros
 ******************************************************************************/
/*
 * @brief Error LED's Port Number.
 */
#define ERROR_LED_PORT				GPIO0

/*
 * @brief 	Error LED's Pin 1.
 * @details	P0_7
 */
#define ERROR_LED_PIN_CONFIG		0x07

/*
 * @brief 	Error LED's Pin 2.
 * @details	P0_9
 */
#define ERROR_LED_PIN_RECORD		0x09

/*
 * @brief 	Error LED's Pin 3.
 * @details	P0_13
 */
#define RECORD_LED_PIN_FLUSH	0x0D


/*
 * @brief 	Recording LED's Port Number.
 * @details Signals That Device Records.
 */
#define RECORD_LED_PORT				GPIO2

/*
 * @brief 	Record LED's Pin.
 * @details	P2_11
 */
#define RECORD_LED_PIN				0x0B

/*
 * @brief 	Recording LED's Port Number.
 * @details Signals That Device Records.
 */
#define BACKUP_POWER_LED_PORT		GPIO4

/*
 * @brief 	Record LED's Pin.
 * @details	P4_17
 */
#define BACKUP_POWER_LED_PIN		0x11
/*******************************************************************************
 * Functions Definitions
 ******************************************************************************/

/*
 * @brief 			Sets Logic 1 on GPIO Pin.
 * @param port_base Pointer to GPIO Instance.
 * @param pin		Pin.
 */
void LED_SetHigh(GPIO_Type *port_base, uint32_t pin);

/*
 * @brief 			Sets Logic 0 on GPIO Pin.
 * @param port_base Pointer to GPIO Instance.
 * @param pin		Pin.
 */
void LED_SetLow(GPIO_Type *port_base, uint32_t pin);

/*
 * @brief Indicates Ready State of The Recording Device.
 */
void LED_SignalReady(void);

/*
 * @brief Signals That Device Is Currently Receiving Bytes (Recording).
 */
void LED_SignalRecording(void);

/*
 * @brief Signals That Device Is Stopped Receiving Bytes (Recording).
 */
void LED_SignalRecordingStop(void);

/*
 * @brief Signals Configuration File Missing or Contains Unexpected Data.
 */
void LED_SignalBackUpPowerAvailable(void);

/*
 * @brief Signals Error During Recording.
 */
void LED_SignalError(void);

/*
 * @brief Signals That Flush Has Been Activated.
 */
void LED_SignalFlush(void);

#endif /* LED_H_ */
