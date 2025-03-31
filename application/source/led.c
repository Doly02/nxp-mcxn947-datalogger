/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      gpio.c
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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <led.h>
#include "fsl_gpio.h"


/*******************************************************************************
 * Functions
 ******************************************************************************/

void LED_SetHigh(GPIO_Type *port_base, uint32_t pin)
{
	port_base->PSOR = (1U << pin);
}

void LED_SetLow(GPIO_Type *port_base, uint32_t pin)
{
	port_base->PCOR = (1U << pin);
}

void LED_SignalRecording(void)
{
	GPIO_PortToggle(RECORD_LED_PORT, 1u << RECORD_LED_PIN);
}

void LED_SignalRecordingStop(void)
{
	LED_SetHigh(RECORD_LED_PORT, RECORD_LED_PIN);
}

void LED_SignalConfigError(void)
{
	LED_SetHigh(ERROR_LED_PORT, ERROR_LED_PIN_CONFIG);
}

void LED_SignalRecordError(void)
{
	LED_SetHigh(ERROR_LED_PORT, ERROR_LED_PIN_RECORD);
}

void LED_SignalFlush(void)
{
	LED_SetHigh(ERROR_LED_PORT, RECORD_LED_PIN_FLUSH);
}
