/*
 * parser.h
 *
 *  Created on: Apr 16, 2025
 *      Author: tomas
 */

#ifndef PARSER_H_
#define PARSER_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>
#include <string.h>

#include "fsl_lpuart.h"
#include "fsl_debug_console.h"
#include "error.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Structures
 ******************************************************************************/
/**
 * @brief 	Enumeration of recording board versions.
 *
 * @details This enum defines the possible versions of the board for which the recording
 * 			system is configured.
 *
 * @note	Difference Between AUTOS1 and AUTOS2 Is In Baudrate.
 *
 */
typedef enum
{
	WCT_UNKOWN = 0,		/**< Unknown board version. 	*/
	WCT_AUTOS1,			/**< AUTOS1 Reference Board. 	*/
	WCT_AUTOS2			/**< AUTOS2 Reference Board. 	*/

} REC_version_t;

/**
 * @brief 	Configuration structure for the recording system.
 *
 * @details	Structure Holds The Configuration Parameters Required For Initializing
 * 			The Recording System, Including The Board Version and Baudrate.
 */
typedef struct
{
	/* Recorded NXP Device  */
	REC_version_t 	version;	/**< Board That Will Be Recorded		*/

	/* UART Setup */
	uint32_t 				baudrate;	/**< Desired Baudrate			*/
	lpuart_stop_bit_count_t	stop_bits;
    lpuart_data_bits_t		data_bits;
    lpuart_parity_mode_t	parity;


	uint32_t		size;		/**< Maximum File Size 					*/
								/**< Maximal Log. Time In Per File		*/
	uint32_t		max_bytes;	/**< Number of Bytes Between LED Signal	*/

} REC_config_t;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
REC_config_t PARSER_GetConfig(void);

REC_version_t PARSER_GetVersion(void);

uint32_t PARSER_GetBaudrate(void);

uint32_t PARSER_GetFileSize(void);

lpuart_data_bits_t PARSER_GetDataBits(void);

lpuart_parity_mode_t PARSER_GetParity(void);

lpuart_stop_bit_count_t PARSER_GetStopBits(void);

uint32_t PARSER_GetMaxBytes(void);

void PARSER_ClearConfig(void);

error_t PARSER_ParseBaudrate(const char *content);

error_t PARSER_ParseFileSize(const char *content);

error_t PARSER_ParseParity(const char *content);

error_t PARSER_ParseStopBits(const char *content);

error_t PARSER_ParseDataBits(const char *content);


#endif /* PARSER_H_ */
