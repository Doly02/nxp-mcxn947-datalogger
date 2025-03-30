/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      gpio.c
 *  Author:         Tomas Dolak
 *  Date:           06.02.2025
 *  Description:    Implements The Logic For GPIO Control.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           gpio.c
 *  @author         Tomas Dolak
 *  @date           06.02.2025
 *  @brief          Implements The Logic For GPIO Control.
 * ****************************/

#ifndef GPIO_H_
#define GPIO_H_

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
#define ERROR_LED_PORT			GPIO0

/*
 * @brief 	Error LED's Pin 1.
 * @details	P0_7
 */
#define ERROR_LED_PIN1			0x07

/*
 * @brief 	Error LED's Pin 2.
 * @details	P0_9
 */
#define ERROR_LED_PIN2			0x09

/*
 * @brief 	Error LED's Pin 3.
 * @details	P0_13
 */
#define ERROR_LED_PIN3			0x0D


/*
 * @brief 	Recording LED's Port Number.
 * @details Signals That Device Records.
 */
#define RECORD_LED_PORT			GPIO2

/*
 * @brief 	Record LED's Pin.
 * @details	P2_11
 */
#define RECORD_LED_PIN			0x0B

/*
 * @brief 	Time Interval Between LED Blinking.
 * @details	In Seconds.
 */
#define RECORD_LED_TIME_INTERVAL 2

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
void GPIO_ConfigureGpioPins(void);

void GPIO_SetHigh(GPIO_Type *port_base, uint32_t pin);

void GPIO_SetLow(GPIO_Type *port_base, uint32_t pin);

void GPIO_SignalRecording(void);

void GPIO_SignalError(void);

#endif /* GPIO_H_ */
