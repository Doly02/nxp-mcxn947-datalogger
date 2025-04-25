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
#include <errno.h>
#include <stdlib.h>

#include "fsl_lpuart.h"
#include "fsl_debug_console.h"
#include "error.h"
#include "defs.h"
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


	uint32_t		size;				/**< Maximum File Size 					*/
										/**< Maximal Log. Time In Per File		*/
	uint32_t		max_bytes;			/**< Number of Bytes Between LED Signal	*/
	uint32_t 		free_space_limit_mb;

} REC_config_t;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/**
 * @brief 		Returns Active Configuration.
 *
 * @details 	This Function Returns The Global Configuration Structure That Contains
 * 				The Settings For The Recording, Such as Baudrate, Version,
 * 				And Other Relevant Parameters.
 *
 * @return		REC_config_t The Current Recording Configuration.
 */
REC_config_t PARSER_GetConfig(void);

/**
 * @brief 		Returns the Version of The Device Being Recorded.
 *
 * @return		REC_version_t Current Version of Recorded Device
 * 				(WCT_UNKOWN, WCT_AUTOS1 or WCT_AUTOS2).
 */
REC_version_t PARSER_GetVersion(void);

/**
 * @brief 		Returns the Baudrate of The Device Being Recorded.
 *
 * @return		uint32_t Baud Rate of Recorded Device.
 *
 */
uint32_t PARSER_GetBaudrate(void);

/**
 * @brief 		Returns the Maximal File Size.
 *
 * @return		uint32_t Maximal File Size.
 *
 */
uint32_t PARSER_GetFileSize(void);

/**
 * @brief 		Returns The Number of Data Bits.
 *
 * @return		lpuart_data_bits_t Number of Data Bits.
 *
 */
lpuart_data_bits_t PARSER_GetDataBits(void);

/**
 * @brief 		Returns The Parity.
 *
 * @return		lpuart_parity_mode_t kLPUART_ParityDisabled, kLPUART_ParityEven or kLPUART_ParityOdd.
 *
 */
lpuart_parity_mode_t PARSER_GetParity(void);

/**
 * @brief 		Returns The Number of Stop Bits.
 *
 * @return		lpuart_stop_bit_count_t Number of Stop Bits.
 *
 */
lpuart_stop_bit_count_t PARSER_GetStopBits(void);

/**
 * @brief 		Returns The Free Space Limit on SD Card For LED Signaling.
 *
 * @return		uint32_t Free Space Limit.
 *
 */
uint32_t PARSER_GetFreeSpaceLimitMB(void);

/**
 * @brief 		Returns The Number of Maximal Bytes Between LED Blinking.
 *
 * @return		uint32_t Number of Maximal Bytes Between LED Blinking.
 *
 */
uint32_t PARSER_GetMaxBytes(void);

/**
 * @brief 		Clears The Configuration To Default.
 */
void PARSER_ClearConfig(void);

/**
 * @brief 		Parse Baudrate From Configuration File.
 * @param[in]	chContent Pointer To Content of Configuration File.
 */
error_t PARSER_ParseBaudrate(const char *chContent);

/**
 * @brief 		Parse Record File Size From Configuration File.
 * @param[in]	chContent Pointer To Content of Configuration File.
 */
error_t PARSER_ParseFileSize(const char *chContent);

/**
 * @brief 		Parse Parity From Configuration File.
 * @param[in]	chContent Pointer To Content of Configuration File.
 */
error_t PARSER_ParseParity(const char *chContent);

/**
 * @brief 		Parse The Number of Stop Bits From Configuration File.
 * @param[in]	chContent Pointer To Content of Configuration File.
 */
error_t PARSER_ParseStopBits(const char *chContent);

/**
 * @brief 		Parse The Number of Data Bits From Configuration File.
 * @param[in]	chContent Pointer To Content of Configuration File.
 */
error_t PARSER_ParseDataBits(const char *chContent);


error_t PARSER_ParseFreeSpace(const char *chContent);

#endif /* PARSER_H_ */
