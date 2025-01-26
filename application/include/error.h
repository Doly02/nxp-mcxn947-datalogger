/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      error.h
 *  Author:         Tomas Dolak
 *  Date:           22.01.2025
 *  Description:    Implements Error Handling Logic.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           error.h
 *  @author         Tomas Dolak
 *  @date           22.01.2025
 *  @brief          Implements Error Handling Logic.
 * ****************************/

#ifndef ERROR_H_
#define ERROR_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ERROR_NONE				0x0000u
#define ERROR_IRTC				0x0001u

#define ERROR_RECORD			0x0002u
#define ERROR_OPEN				0x0005u
#define ERROR_READ				0x0006u
#define ERROR_CLOSE				0x0004u
#define ERROR_FILESYSTEM		0x0007u
#define ERROR_CONFIG			0x0008u

#define ERROR_ADMA				0x0003u
/**
 * @brief Out Of The Main Cycle Error.
 * */
#define ERROR_OUT_OF_CYCLE		0xFFFEu
#define ERROR_UNKNOWN			0xFFFFu
/*******************************************************************************
 * Structures
 ******************************************************************************/
typedef uint16_t error_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ERR_HandleError(void);

#endif /* ERROR_H_ */
