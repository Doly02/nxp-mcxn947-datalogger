/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      temperature.h
 *  Author:         Tomas Dolak
 *  Date:           11.02.2025
 *  Description:    Implements The Logic Of Power Loss Detection.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           temperature.h
 *  @author         Tomas Dolak
 *  @date           11.02.2025
 *  @brief          Implements The Logic Of Power Loss Detection.
 * ****************************/

#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "i3c.h"
#include "error.h"

/*******************************************************************************
 * Global Definitions
 ******************************************************************************/
#define SENSOR_STATIC_ADDR  0x48U  // Statická adresa senzoru (I2C mód)
#define SENSOR_DYNAMIC_ADDR 0x08U  // Dynamická adresa (přiřazená)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
error_t TMP_Init(void);

float TMP_GetTemperature(void);

#endif /* TEMPERATURE_H_ */
