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
void GPIO_ConfigureGpioPins(void)
{
	/* Debug Pin */
	gpio_pin_config_t gpio_debug =
	{
			kGPIO_DigitalOutput,	/* Pin Direction 			*/
			0,						/* Default Value on Output 	*/
	};

    /* Enable CLK For GPIO */
    CLOCK_EnableClock(kCLOCK_Gpio0);

    GPIO_PinInit(GPIO0, 10U, &gpio_debug);
}

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

void GPIO_SignalConfigError(void)
{
	GPIO_SetHigh(ERROR_LED_PORT, ERROR_LED_PIN_CONFIG);
}

void GPIO_SignalRecordError(void)
{
	GPIO_SetHigh(ERROR_LED_PORT, ERROR_LED_PIN_RECORD);
}

void GPIO_SignalTemperatureError(void)
{
	GPIO_SetHigh(ERROR_LED_PORT, ERROR_LED_PIN_TEMPERATURE);
}
