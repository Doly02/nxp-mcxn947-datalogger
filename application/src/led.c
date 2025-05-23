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

/*******************************************************************************
 * Functions
 ******************************************************************************/

void LED_SetHigh(GPIO_Type *port_base, uint32_t pin)
{
	port_base->PSOR = (1UL << pin);
}

void LED_SetLow(GPIO_Type *port_base, uint32_t pin)
{
	port_base->PCOR = (1UL << pin);
}

void LED_SignalReady(void)
{
	LED_SetHigh(GPIO0, 7);
    LED_SetHigh(GPIO0, 9);
    LED_SetHigh(GPIO0, 13);
    LED_SetHigh(GPIO2, 11);
    LED_SetHigh(GPIO4, 17);
	SDK_DelayAtLeastUs(1000, CLOCK_GetCoreSysClkFreq());
    LED_SetLow(GPIO0, 7);
    LED_SetLow(GPIO0, 9);
    LED_SetLow(GPIO0, 13);
    LED_SetLow(GPIO2, 11);
    LED_SetLow(GPIO4, 17);
}

void LED_SignalRecording(void)
{
	GPIO_PortToggle(RECORD_LED_PORT, 1UL << RECORD_LED_PIN);
}

void LED_SignalRecordingStop(void)
{
	LED_SetHigh(RECORD_LED_PORT, RECORD_LED_PIN);
}

void LED_SignalBackUpPowerAvailable(void)
{
	LED_SetHigh(BACKUP_POWER_LED_PORT, BACKUP_POWER_LED_PIN);
}

void LED_SignalLowMemory(void)
{
	LED_SetHigh(MEMORY_LOW_LED_PORT, MEMORY_LOW_LED_PIN);
}

void LED_SignalError(void)
{
	LED_SetHigh(ERROR_LED_PORT, ERROR_LED_PIN_RECORD);
}

void LED_SignalFlush(void)
{
	LED_SetHigh(FLUSH_LED_PORT, RECORD_LED_PIN_FLUSH);
}

void LED_ClearSignalFlush(void)
{
	LED_SetLow(FLUSH_LED_PORT, RECORD_LED_PIN_FLUSH);
}
