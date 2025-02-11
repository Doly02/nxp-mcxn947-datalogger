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
 * Definitions
 ******************************************************************************/
#define I2C_BAUDRATE            		100000
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

/*******************************************************************************
 * Structures
 ******************************************************************************/
typedef void (*func_ptr)(void);

/*******************************************************************************
 * Callback Functions
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
 * Functions
 ******************************************************************************/
int I3C_Initialize(void);

int I3C_Deinitialize(void);

int I3C_SetBaudrate(uint32_t i2c_freq);

void I3C_SetBusType(i3c_bus_type_t type);

void I3C_SetIBICallback( func_ptr callback);

int I3C_Transfer(i3c_bus_type_t bus_type,
					i3c_direction_t direction,
					uint8_t target_addr,
					uint8_t *data,
					bool stop_flag);

error_t I3C_Write(uint8_t target_addr, uint8_t *data, bool stop_flag);

error_t I3C_Read(uint8_t target_addr, uint8_t *data, bool stop_flag);


#endif /* I3C_H_ */
