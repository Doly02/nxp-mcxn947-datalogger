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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "fsl_gpio.h"

#include "gpio.h"

/*******************************************************************************
 * Functions
 ******************************************************************************/

void GPIO_SetHigh(GPIO_Type *port_base, uint32_t pin)
{
	port_base->PSOR = (1U << pin);
}

void GPIO_SetLow(GPIO_Type *port_base, uint32_t pin)
{
	port_base->PCOR = (1U << pin);
}

void GPIO_SignalRecording(void)
{
	GPIO_PortToggle(RECORD_LED_PORT, 1u << RECORD_LED_PIN);
}

void GPIO_SignalRecordingStop(void)
{
	GPIO_SetHigh(RECORD_LED_PORT, RECORD_LED_PIN);
}

void GPIO_SignalConfigError(void)
{
	GPIO_SetHigh(ERROR_LED_PORT, ERROR_LED_PIN_CONFIG);
}

void GPIO_SignalRecordError(void)
{
	GPIO_SetHigh(ERROR_LED_PORT, ERROR_LED_PIN_RECORD);
}

void GPIO_SignalFlush(void)
{
	GPIO_SetHigh(ERROR_LED_PORT, RECORD_LED_PIN_FLUSH);
}
