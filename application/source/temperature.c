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

/*  Standard C Included Files */

/*  SDK Header Files */
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i3c.h"

#include "error.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_MASTER                  I3C1
#define I2C_BAUDRATE            		100000
#define I3C_OD_BAUDRATE         		625000
#define I3C_PP_BAUDRATE         		1250000
#define I3C_MASTER_CLOCK_FREQUENCY      CLOCK_GetI3cClkFreq(1U)
#define WAIT_TIME                       1000
#define I3C_HDR_SUPPORT         		1
#define I3C_IBI_SUPPORT         		1
#define USE_SETDASA_ASSIGN_ADDR 		1
/**
 * I2C Address: 1001000
 */
#define I3C_SLAVE_ADDR_7BIT 			0x48U
#define I3C_DATA_LENGTH 				33U

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
/**
 * @brief Handle of I3C Master.
 */
i3c_master_handle_t		g_i3c_handle;
/*******************************************************************************
 * Variables
 ******************************************************************************/
void I3C_Callback(void)
{

}

int I3C_Init(void)
{
    i3c_master_config_t config = { 0U };
    i3c_master_transfer_t xfer = { 0U };

    I3C_MasterGetDefaultConfig(&config);
    config.baudRate_Hz.i2cBaud          = I2C_BAUDRATE;
    config.baudRate_Hz.i3cPushPullBaud  = I3C_PP_BAUDRATE;
    config.baudRate_Hz.i3cOpenDrainBaud = I3C_OD_BAUDRATE;
    config.enableOpenDrainStop          = false;
    config.disableTimeout				= true;

    I3C_MasterInit(I3C1, &config, CLOCK_GetI3cClkFreq(1U));
    I3C_MasterTransferCreateHandle(I3C1, &g_i3c_handle, &I3C_Callback, NULL);

    return ERROR_NONE;
}

int I3C_Deinit(void)
{
	I3C_MasterDeinit(I3C1);
	return ERROR_NONE;
}

int I3C_SetBaudrate(uint32_t i2c_freq)
{
	i3c_baudrate_hz_t Baudrate; 	/* Baud Rate in Hz */
	Baudrate.i2cBaud = i2c_freq;

	I3C_MasterSetBaudRate(I3C1, Baudrate, CLOCK_GetI3cClkFreq(1U));
	return ERROR_NONE;
}
int TMP_Init(void)
{

}
