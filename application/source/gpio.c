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

static inline void GPIO_SetHigh(GPIO_Type *port_base, uint32_t pin)
{
	port_base->PSOR = (1U << pin);
}

static inline void GPIO_SetLow(GPIO_Type *port_base, uint32_t pin)
{
	port_base->PCOR = (1U << pin);
}
