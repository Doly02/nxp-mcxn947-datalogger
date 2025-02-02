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

/**
 * @brief Size of IBI Master Buffer.
 */
#define IBI_BUFFER_SIZE					8U
/*******************************************************************************
 * Structures
 ******************************************************************************/
typedef void (*func_ptr)(void);

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
/**
 * @brief Handle of I3C Master.
 */
i3c_master_handle_t		g_i3c_handle;

i3c_bus_type_t			g_i3c_bus_type;

func_ptr				g_ibi_callback	= NULL;

/**
 * @brief Status And Control Variables.
 */

/**
 * @brief State of Master Transfer Completion.
 */
volatile bool g_CompletionFlag			= false;
volatile status_t g_CompletionStatus	= kStatus_Success;
volatile bool g_IBI_WonFlag				= false;
/**
 * @brief Global Master IBI Buffer.
 */
uint8_t g_IBI_MasterBuffer[IBI_BUFFER_SIZE] 	= { 0U };

/**
 * @brief Global User IBI Buffer.
 */
static uint8_t g_IBI_UserBuff[IBI_BUFFER_SIZE] 	= { 0U };
/*******************************************************************************
 * Callbacks
 ******************************************************************************/
/**
 * @brief 	I3C Master Callback.
 * @details
 */
void I3C_Callback(I3C_Type *base, i3c_master_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_I3C_IBIWon == status)
    {
    	g_IBI_WonFlag = true;
    }
    else
    {
        /* Signal transfer complete when received complete status. */
    	g_CompletionFlag = true;
    }

    g_CompletionStatus = status;
}

/**
 * @brief 	I3C IBI Callback.
 * @details IBI (In-Band Interrupt) Is a Mechanism In The I3C Bus That Allows a Slave Device
 * 			To Send an Interrupt To a Master Device Without Requiring a Special Signal As In I2C
 * 			(e.g. Without Requiring an Extra Dedicated Interrupt Line).
 * @param 	base Base of I3C Instance.
 * @param 	handle I3C Handle.
 * @param	type IBI Type.
 * @param	state IBI State.
 */
void I3C_IBICallback(I3C_Type *base,
        			i3c_master_handle_t *handle,
					i3c_ibi_type_t type,
					i3c_ibi_state_t state)
{
	if (kI3C_IbiNormal == type)
	{
		if (kI3C_IbiDataBuffNeed == state)
		{
			handle->ibiBuff = g_IBI_MasterBuffer;
		}
		else
		{
			memcpy(g_IBI_UserBuff, (void *)handle->ibiBuff, handle->ibiPayloadSize);
		}
	}
	else
	{
		assert(false);
		break;
	}
}

/*******************************************************************************
 * Functions
 ******************************************************************************/
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

void I3C_SetBusType(i3c_bus_type_t type)
{
	g_i3c_bus_type = type;
}

void I3C_SetIBICallback( func_ptr callback)
{
	g_ibi_callback = callback;
}

int I3C_Transfer(i3c_bus_type_t bus_type,
					i3c_direction_t direction,
					uint8_t target_addr,
					uint8_t *data,
					bool stop_flag)
{
	i3c_master_transfer_t xfer = { 0U };
	_i3c_master_transfer_flags xfer_flags = 0;
	status_t status =
	/* Setup Of Transfer Parameters */

	xfer.busType 		= bus_type;
	xfer.direction 		= direction;
	xfer.slaveAddress 	= target_addr;
	xfer.data 			= data;
	xfer.dataSize 		= sizeof(data);
	if (true == stop)
	{
		xfer_flags = kI3C_TransferDefaultFlag;
	}
	else
	{
		xfer_flag = kI3C_TransferNoStopFlag;
	}
	xfer.flags = xfer_flags;
	status = I3C_MasterTransferBlocking(I3C1, &xfer);
	if (kStatus_Success != status)
	{
		return ERROR_READ;
	}
	return ERROR_NONE;
}

error_t I3C_Write(uint8_t target_addr, uint8_t *data, bool stop_flag)
{
	error_t status = ERROR_UNKNOWN;
	status = (error_t)I3C_Transfer( g_i3c_bus_type, kI3C_Write, target_addr, data, stop_flag);
	return status;
}

error_t I3C_Read(uint8_t target_addr, uint8_t *data, bool stop_flag)
{
	error_t status = ERROR_UNKNOWN;
	status = (error_t)I3C_Transfer( g_i3c_bus_type, kI3C_Read, target_addr, data, stop_flag);
	return status;
}

int TMP_Init(void)
{

}
