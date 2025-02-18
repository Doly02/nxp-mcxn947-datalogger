/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      temperature.c
 *  Author:         Tomas Dolak
 *  Date:           11.02.2025
 *  Description:    Implements The Logic For Temperature Measurement.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           temperature.c
 *  @author         Tomas Dolak
 *  @date           11.02.2025
 *  @brief          Implements The Logic For Temperature Measurement.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "temperature.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define REGISTER_TEMPERATURE	0x00

#define REGISTER_CONFIG			0x01

/*
 * @brief Temperature Low.
 */
#define REGISTER_THVST			0x02
/*
 * @brief Temperature High.
 */
#define REGISTER_TOS			0x03


/*******************************************************************************
 * Structures
 ******************************************************************************/


/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * Callback Functions
 ******************************************************************************/

/*******************************************************************************
 * Functions
 ******************************************************************************/

error_t TMP_Init(void)
{
	error_t status = ERROR_UNKNOWN;
	if (ERROR_NONE != (error_t)I3C_Initialize())
	{
		return ERROR_UNKNOWN;
	}

	I3C_SendBroadcastCmd(CMD_RSTDAA_BROADCAST, NULL, 0);
	I3C_SendCommonCommandCode(SENSOR_STATIC_ADDR, CMD_SETDASA_BROADCAST, SENSOR_DYNAMIC_ADDR << 1);

}

float TMP_GetTemperature(void)
{
	float tmp = 0.0;
	uint8_t data_w[1] = { 0U };
	uint8_t data_r[2] = { 0U };

	I3C_Write(SENSOR_DYNAMIC_ADDR, data_w, sizeof(data_w), false);
	I3C_Read(SENSOR_DYNAMIC_ADDR, data_r, sizeof(data_r), false);

	tmp = (((uint32_t)data_r[ 0 ]) << 8 | data_r[ 1 ]) / 256.0;

	return tmp;
}


