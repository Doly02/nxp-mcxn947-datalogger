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
 * Functions Definitions
 ******************************************************************************/
void GPIO_ConfigureGpioPins(void);

static inline void GPIO_SetHigh(GPIO_Type *port_base, uint32_t pin);

static inline void GPIO_SetLow(GPIO_Type *port_base, uint32_t pin);

#endif /* GPIO_H_ */
