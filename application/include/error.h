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
/**
 * @brief Return Code When a Successful Operation is Performed.
 */
#define ERROR_NONE				0x0000u

/**
 * @brief Error Related To Internal RTC Integrated In MCXN947 MCU.
 */
#define ERROR_IRTC				0x0001u

/**
 * @brief Error Related To Data Logging.
 */
#define ERROR_RECORD			0x0002u

/**
 * @brief Error Related To Opening File.
 */
#define ERROR_OPEN				0x0005u

/**
 * @brief Error Related To Reading File.
 */
#define ERROR_READ				0x0006u
/**
 * @brief Error Related To Closing File.
 */
#define ERROR_CLOSE				0x0004u

/**
 * @brief Error Related To File System (e.g. Mount of File System, Setting Up File Meta-Data,...).
 */
#define ERROR_FILESYSTEM		0x0007u

/**
 * @brief Error Related To Reading and Parsing Configuration File Stored On SDHC Card (config.txt).
 */
#define ERROR_CONFIG			0x0008u

/**
 * @brief Error Related To Storing Data on SDHC Card Using ADMA (Advance DMA).
 */
#define ERROR_ADMA				0x0003u

/**
 * @brief Out Of The Main Cycle Error.
 * */
#define ERROR_OUT_OF_CYCLE		0xFFFEu

/**
 * @brief Occurrence Of an Unclassified Error.
 */
#define ERROR_UNKNOWN			0xFFFFu
/*******************************************************************************
 * Structures
 ******************************************************************************/
typedef uint16_t error_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/**
 * @brief Base Function For Error Handling.
 */
void ERR_HandleError(void);

#endif /* ERROR_H_ */
