/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      i3c.c
 *  Author:         Tomas Dolak
 *  Date:           02.02.2025
 *  Description:    Implements The Logic For Control of I3C Peripheral Unit.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           i3c.c
 *  @author         Tomas Dolak
 *  @date           02.02.2025
 *  @brief          Implements The Logic For Control of I3C Peripheral Unit.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "i3c.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BROADCAST_ADDR	 		0x7E

/**
 * @brief 	Default Baud Rate For Communication in I2C Mode.
 */
#define DEFAULT_I2C_FREQ		400000UL	// 400kHz

/**
 * @brief 	Default Baud Rate For Communication in I3C Open-Drain Mode.
 */
#define DEFAULT_I3C_OD_FREQ 	4000000UL 	// 4MHz

/**
 * @brief 	Default Baud Rate For Communication in I3C Push-Pull Mode.
 */
#define DEFAULT_I3C_PP_FREQ 	12500000UL	// 12.5MHz

/*******************************************************************************
 * Structures
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
/**
 * @brief 	Handle of I3C Master.
 */
i3c_master_handle_t		g_i3c_handle;

/**
 * @brief 	Stores Current Bus Over Communication Takes Over.
 */
i3c_bus_type_t			g_i3c_bus_type;

/**
 * @brief	Pointer To IBI Callback Function.
 */
func_ptr				g_ibi_callback	= NULL;

/**
 * @brief 	Status And Control Variables.
 */

/**
 * @brief 	State of Master Transfer Completion.
 */
volatile bool g_CompletionFlag			= false;

/**
 * @brief 	Status of Completion/Incompletion Transfer.
 */
volatile status_t g_CompletionStatus	= kStatus_Success;

volatile bool g_IBI_WonFlag				= false;

/**
 * @brief 	Global Master IBI Buffer.
 */
uint8_t g_IBI_MasterBuffer[IBI_BUFFER_SIZE] 	= { 0U };

/**
 * @brief 	First Broadcast Flag.
 * @details During First Broadcast The Lower Baud Rate Is Set.
 */
bool g_FirstBroadcast 					= true;

/**
 * @brief Global User IBI Buffer.
 */
static uint8_t g_IBI_UserBuff[IBI_BUFFER_SIZE] 	= { 0U };
/*******************************************************************************
 * Callback Functions
 ******************************************************************************/
/**
 * @brief 	I3C Master Callback.
 * @details	Transfer Completed Callback.
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
	}
}

/*******************************************************************************
 * Functions
 ******************************************************************************/
error_t I3C_Initialize(void)
{
    i3c_master_config_t config = { 0U };
    i3c_master_transfer_t xfer = { 0U };

    const i3c_master_transfer_callback_t masterCallback = {
        .slave2Master 		= NULL,
		.ibiCallback 		= I3C_IBICallback,
		.transferComplete 	= I3C_Callback};

    I3C_MasterGetDefaultConfig(&config);
    config.baudRate_Hz.i2cBaud          = I2C_BAUDRATE;
    config.baudRate_Hz.i3cPushPullBaud  = I3C_PP_BAUDRATE;
    config.baudRate_Hz.i3cOpenDrainBaud = I3C_OD_BAUDRATE;
    config.enableOpenDrainStop          = false;
    config.disableTimeout				= true;

    I3C_MasterInit(I3C1, &config, CLOCK_GetI3cClkFreq(1U));
    I3C_MasterTransferCreateHandle(I3C1, &g_i3c_handle, &masterCallback, NULL);

    return ERROR_NONE;
}

/**
 * @brief	De-Initializes I3C Peripheral Unit.
 */
error_t I3C_Deinitialize(void)
{
	I3C_MasterDeinit(I3C1);
	return ERROR_NONE;
}

/**
 * @brief 		Functions For Setup of Baud Rates of I3C Instance.
 * @details		Sets Classic I2C Frequency, Also Push-Pull and Open-Drain Frequencies
 * 				For I3C.
 *
 * @param[in] 	i2c_freq Desired I2C Baud Rate [Hz].
 * @param[in] 	i3c_pp_freq Desired I3C Push-Pull Baud Rate [Hz].
 * @param[in] 	i3cOpenDrainBaud Desired I3C Open-Drain Baud Rate [Hz].
 *
 * @retva;		Return 0 If Success.
 */
error_t I3C_SetBaudrate(uint32_t i2c_freq, uint32_t i3c_pp_freq, uint32_t i3c_od_freq)
{
	assert(i2c_freq > 0);
	assert(i3c_pp_freq > 0);
	assert(i3c_od_freq > 0);

	i3c_baudrate_hz_t Baudrate; 	/* Baud Rate in Hz */
	Baudrate.i2cBaud 			= i2c_freq;
	Baudrate.i3cOpenDrainBaud 	= i3c_od_freq;
	Baudrate.i3cPushPullBaud	= i3c_pp_freq;

	I3C_MasterSetBaudRate(I3C1, &Baudrate, CLOCK_GetI3cClkFreq(1U));
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

error_t I3C_Transfer(i3c_bus_type_t bus_type,
					i3c_direction_t direction,
					uint8_t target_addr,
					uint8_t *data,
					uint8_t data_len,
					bool stop_flag)
{
	assert(bus_type < 4);	// TODO: Update
	assert(direction == kI3C_Write || direction == kI3C_Read);

	i3c_master_transfer_t xfer 	= { 0U };
	uint32_t xfer_flags 		= 0;
	status_t status				= kStatus_I3C_MsgError;
	/* Setup Of Transfer Parameters */

	xfer.busType 		= bus_type;
	xfer.direction 		= direction;
	xfer.slaveAddress 	= target_addr;
	xfer.data 			= data;
	xfer.dataSize 		= data_len;
	if (true == stop_flag)
	{
		xfer_flags = kI3C_TransferDefaultFlag;
	}
	else
	{
		xfer_flags = kI3C_TransferNoStopFlag;
	}
	xfer.flags = xfer_flags;
	//status = I3C_MasterTransferNonBlocking(I3C1, &g_i3c_handle, &xfer);
	status = I3C_MasterTransferBlocking(I3C1, &xfer);
	if (kStatus_Success != status)
	{
		return ERROR_READ;
	}
	return ERROR_NONE;
}

// Data Lenght Is Probably Missing!
error_t I3C_Write(uint8_t target_addr, uint8_t *data, uint8_t data_len, bool stop_flag)
{
	error_t status = ERROR_UNKNOWN;
	status = (error_t)I3C_Transfer( g_i3c_bus_type, kI3C_Write, target_addr, data, data_len, stop_flag);
	return status;
}

error_t I3C_Read(uint8_t target_addr, uint8_t *data, uint8_t data_len, bool stop_flag)
{
	error_t status = ERROR_UNKNOWN;
	status = (error_t)I3C_Transfer( g_i3c_bus_type, kI3C_Read, target_addr, data, data_len, stop_flag);
	return status;
}

error_t I3C_SetDynamicAddrFromStatic(uint32_t static_addr, uint32_t dynamic_addr)
{
	status_t status = kStatus_I3C_MsgError;
	status = I3C_SendBroadcastCmd(CMD_RSTDAA_BROADCAST, NULL, 0U);
	if (kStatus_Success == status)
	{
		return (error_t)status;
	}
	status = I3C_SendCommonCommandCode(static_addr, CMD_SETDASA_BROADCAST, (dynamic_addr << 1));
	return (error_t)status;
}

error_t I3C_SendBroadcastCmd(uint8_t cmd, uint8_t *data, uint8_t len)
{
	uint8_t buffer[RW_BUFFER_SIZE] = { 0U };
	status_t status = kStatus_I3C_MsgError;

	/* Note:
	 * - Broadcast Commands Are Send To Special Address 0x7F
	 * - Broadcast Commands Can By Named As CCC (Common Command Code) Are Used
	 * 	 To Configure Entire I3C Bus or To Communicate With Multiple Devices Simultaneously.
	 *
	 * - structure:
	 *
	 * 	------------------------------
	 * 	|	Address	|	CCC	|	Data |
	 * 	------------------------------
	 *
	 * 	- RSTDAA (Reset Dynamic Address Assignment):
	 * 	---------------------
	 * 	|	0x7F	|  0x06 |
	 * 	---------------------
	 */
	if (len >= RW_BUFFER_SIZE - 1)
	{
		return 8; /* Overflow Error */
	}

	buffer[0] = cmd;
	memcpy(&buffer[1], data, len);

	/* Set Baud Rates To Lower On First Broadcast */
	if (g_i3c_bus_type == kI3C_TypeI2C)	// TODO: This Is Probably Not Correct
	{
		I3C_SetBaudrate(0, 2000000, 2000000);
		status = I3C_Write(BROADCAST_ADDR, buffer, (len + 1), false);
		if (kStatus_Success != status)
		{
			return status;
		}
		/* Restore Initial (Default) Frequency */
		I3C_SetBaudrate(DEFAULT_I2C_FREQ, DEFAULT_I3C_PP_FREQ, DEFAULT_I3C_OD_FREQ);
	}
	status = I3C_Write(BROADCAST_ADDR, buffer, (len + 1), false);
	return (error_t)status;
}

error_t I3C_SendCommonCommandCode(uint8_t addr, uint8_t code, uint8_t data)
{
	status_t status	= kStatus_I3C_Timeout;
	// status = I3C_Write(BROADCAST_ADDR, &code, 1U, false);
	status = I3C_Write(BROADCAST_ADDR, &code, 1U, false);
	if (kStatus_Success != status)
	{
		return status;
	}
	status = I3C_Write(addr, &data, 1U, false);
	return status;
}

error_t I3C_ReceiveCommonCommandCode(uint8_t addr, uint8_t cmd, uint8_t *data, uint8_t len)
{
	status_t status = kStatus_I3C_Timeout;
	status = I3C_Write(addr, data, 1U, false);
	if (kStatus_Success != status)
	{
		return (error_t)status;
	}

	status = I3C_Read(addr, data, len, false);
	if (kStatus_Success != status)
	{
		return (error_t)status;
	}
	return ERROR_NONE;
}
