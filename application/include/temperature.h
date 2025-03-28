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

#include "error.h"

#include "fsl_lpi2c.h"
#include "fsl_lpi2c_edma.h"
#include "fsl_edma.h"
/*******************************************************************************
 * Global Definitions
 ******************************************************************************/
#define SENSOR_STATIC_ADDR  0x48U  // Statická adresa senzoru (I2C mód)
#define SENSOR_DYNAMIC_ADDR 0x08U  // Dynamická adresa (přiřazená)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/**
 * @brief		I2C Write Function.
 *
 * @param[in]	regAddress Address of The Register To Be Written To.
 * @param[in]	val Array of Values To Be Written To The Register.
 *
 * @retval		If The Write Succeeds, Returns 0.
 */
uint8_t Write(uint8_t regAddress, uint8_t val[]);

/**
 * @brief		I2C Read Function.
 *
 * @param[in]	regAddress Address of The Register From Which The Reading Will Take Place.
 *
 * @retval		Returns The Read Value From The Registry.
 */
uint16_t Read(uint8_t regAddress);

/**
 * @brief		Gets Temperature From On-Board P3T1755 Temperature Sensor.
 *
 * @retval		Returns Temperature As Float Number.
 */
float TMP_GetTemperature(void);

/**
 * @brief		Initialize On-Board P3T1755 Temperature Sensor.
 */
uint8_t TMP_Init(void);

#endif /* TEMPERATURE_H_ */
