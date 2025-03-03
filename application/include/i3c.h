/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      i3c.h
 *  Author:         Tomas Dolak
 *  Date:           02.02.2025
 *  Description:    Implements The Logic For Control of I3C Peripheral Unit.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           i3c.h
 *  @author         Tomas Dolak
 *  @date           02.02.2025
 *  @brief          Implements The Logic For Control of I3C Peripheral Unit.
 * ****************************/

#ifndef I3C_H_
#define I3C_H_

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
 * Global Definitions
 ******************************************************************************/
#define I2C_BAUDRATE2            		100000
#define I3C_OD_BAUDRATE         		625000
#define I3C_PP_BAUDRATE         		1250000
#define I3C_MASTER_CLOCK_FREQUENCY      CLOCK_GetI3cClkFreq(1U)

/**
 * I2C Address: 1001000
 */
#define I3C_SLAVE_ADDR_7BIT 			0x48U
#define I3C_DATA_LENGTH 				33U

/**
 * @brief Size of IBI Master Buffer.
 */
#define IBI_BUFFER_SIZE					8U

#define	RW_BUFFER_SIZE					10

/**
 * @brief 	Common Command Code For RSTDAA (Reset Dynamic Address Assignment).
 */
#define CMD_RSTDAA_BROADCAST	0x06

/*
 * @brief 	Common Command Code For SETDASA (Set Dynamic Address From Static Address).
 */
#define CMD_SETDASA_BROADCAST	0x87
/*******************************************************************************
 * Global Structures
 ******************************************************************************/
typedef void (*func_ptr)(void);

/*******************************************************************************
 * Callback Prototypes
 ******************************************************************************/
/**
 * @brief 	I3C Master Callback.
 * @details	Transfer Completed Callback.
 */
void I3C_Callback(I3C_Type *base, i3c_master_handle_t *handle, status_t status, void *userData);

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
					i3c_ibi_state_t state);

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
error_t I3C_Initialize(void);

error_t I3C_Deinitialize(void);

error_t I3C_SetBaudrate(uint32_t i2c_freq, uint32_t i3c_pp_freq, uint32_t i3c_od_freq);

void I3C_SetBusType(i3c_bus_type_t type);

void I3C_SetIBICallback( func_ptr callback);

error_t I3C_Transfer(i3c_bus_type_t bus_type,
					i3c_direction_t direction,
					uint8_t target_addr,
					uint8_t *data,
					uint8_t data_len,
					bool stop_flag);


// Data Lenght Is Probably Missing!
error_t I3C_Write(uint8_t target_addr, uint8_t *data, uint8_t data_len, bool stop_flag);

error_t I3C_Read(uint8_t target_addr, uint8_t *data, uint8_t data_len, bool stop_flag);

error_t I3C_SetDynamicAddrFromStatic(uint32_t static_addr, uint32_t dynamic_addr);

error_t I3C_SendBroadcastCmd(uint8_t cmd, uint8_t *data, uint8_t len);

error_t I3C_SendCommonCommandCode(uint8_t addr, uint8_t code, uint8_t data);

error_t I3C_ReceiveCommonCommandCode(uint8_t addr, uint8_t cmd, uint8_t *data, uint8_t len);


#endif /* I3C_H_ */
