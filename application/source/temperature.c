/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      mainc.c
 *  Author:         Tomas Dolak
 *  Date:           07.08.2024
 *  Description:    Implements The Logic Of Time-Keeping.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           main.c
 *  @author         Tomas Dolak
 *  @date           07.08.2024
 *  @brief          Implements The Logic Of Time-Keeping.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
// #include "i3c.h"
#include "error.h"
#include "i3c.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define I2C_BAUDRATE            		100000
#define I3C_OD_BAUDRATE         		625000
#define I3C_PP_BAUDRATE         		1250000
#define I3C_MASTER_CLOCK_FREQUENCY      CLOCK_GetI3cClkFreq(1U)
#define USE_SETDASA_ASSIGN_ADDR 		1
/**
 * @brief 	Address of Temperature Sensor.
 * @details Address=1001000
 */
#define I3C_SLAVE_ADDR_7BIT 			0x48U
#define I3C_DATA_LENGTH 				33U

/**
 * @brief Size of IBI Master Buffer.
 */
#define IBI_BUFFER_SIZE					8U
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

int TMP_Init(void)
{

	error_t status = ERROR_UNKNOWN;
	if (ERROR_NONE != (error_t)I3C_Initialize())
	{
		return ERROR_UNKNOWN;
	}
	return ERROR_UNKNOWN;

}
