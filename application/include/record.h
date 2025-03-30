/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      fatfs.h
 *  Author:         Tomas Dolak
 *  Date:           18.11.2024
 *  Description:    File Includes Operation For Recoding Mode.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           fatfs.h
 *  @author         Tomas Dolak
 *  @date           18.11.2024
 *  @brief          File Includes Operation For Recoding Mode.
 * ****************************/

#ifndef RECORD_H_
#define RECORD_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <string.h>
#include "fsl_sd.h"
#include "ff.h"						/*<! File System */
#include "ffconf.h"					/*<! File System Configuration */
#include <stdio.h>

#include "fsl_debug_console.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "sdmmc_config.h"
#include "fsl_common.h"
#include "rtc_ds3231.h"

#include "error.h"
#include "uart.h"
#include <gpio.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FLUSH_TIMEOUT_TICKS pdMS_TO_TICKS(3000)

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
	REC_version_t 	version;	/**< Board That Will Be Recorded		*/
	uint32_t 		baudrate;	/**< Desired Baudrate					*/
	uint32_t		size;		/**< Maximum File Size 					*/
								/**< Maximal Log. Time In Per File		*/
	uint32_t		max_bytes	/**< Number of Bytes Between LED Signal	*/

} REC_config_t;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/** TODO:
 * @brief 		Creates Directory Based Actual Date.
 *
 * @return		Return Pointer to Created File Descriptor.
 */
error_t CONSOLELOG_CreateDirectory(void);


error_t CONSOLELOG_CreateFile(void);
/**
 * @brief 		Returns Active Configuration.
 *
 * @details 	This Function Returns The Global Configuration Structure That Contains
 * 				The Settings For The Recording, Such as Baudrate, Version,
 * 				And Other Relevant Parameters.
 *
 * @return		REC_config_t The Current Recording Configuration.
 */
REC_config_t CONSOLELOG_GetConfig(void);

/**
 * @brief 		Returns the Version of The Device Being Recorded.
 *
 * @return		REC_version_t Current Version of Recorded Device
 * 				(WCT_UNKOWN, WCT_AUTOS1 or WCT_AUTOS2).
 */
REC_version_t CONSOLELOG_GetVersion(void);

/**
 * @brief 		Returns the Baudrate of The Device Being Recorded.
 *
 * @return		uint32_t Baud Rate of Recorded Device
 *
 */
uint32_t CONSOLELOG_GetBaudrate(void);

uint32_t CONSOLELOG_GetTransferedBytes(void);

void CONSOLELOG_ClearTransferedBytes(void);

uint32_t CONSOLELOG_GetMaxBytes(void);

/**
 * @brief		Checks If The File System Is Initialized
 *
 * return		F_OK If File System Initialized.
 */
FRESULT CONSOLELOG_CheckFileSystem(void);

/**
 * @brief 		Initializes The Recording System and Mounts The File System.
 *
 * @details 	This Function Performs the Initialization of The Recording System,
 * 				Which includes:
 * 				- Setting Default Configuration Parameters.
 * 				- Mounting the File System on The Logical Disk.
 * 				- Optionally Setting Up The Current Working Drive.
 * 				- Formatting The File System If It is Not Found (If
 * 				  Formatting is Enabled).
 *
 * @return 		error_t Returns ERROR_NONE on Success, Otherwise ERROR_FILESYSTEM.
 */
error_t CONSOLELOG_Init(void);

/**
 * @brief 		Starts The Recording Process by Initializing the File
 * 				System, Creating a Directory, and Writing to a File.
 *
 * @details 	Function Uses `CONSOLELOG_Init` To Initialize The Recording
 * 				System.
 *
 * @return 		error_t Returns 0 on Success, Otherwise Returns a Non-Zero Value.
 */
error_t CONSOLELOG_Recording(uint32_t file_size);

/**
 * @brief 		Flushes Collected Data To The File If No Other Data Have Been Received
 * 				By The Time Specified By TIMEOUT Macro.
 * @details		If The Data Does Not Arrive By The Time Specified By The TIMEOUT Macro,
 * 				Then This Function Flushes All The Data So Far Stored In The DMA Buffer,
 * 				Saves It To a File on The Physical Media and Closes The File
 *
 * @return		error_t Returns 0 on Success, Otherwise Returns a Non-Zero Value.
 */
error_t CONSOLELOG_Flush(void);

/**
 * @brief 		De-Initializes The Recording System and Un-Mounts The File System.
 *
 * @return 		error_t Returns 0 on Success, Otherwise E_FAULT.
 */
error_t CONSOLELOG_Deinit(void);

/**
 * @brief		Reads and Processes The Configuration File From The Root directory.
 *
 * @details		This Function Scans The Root Directory For a Configuration File, If The File is Found
 * 				Reads its Contents Into g_config Buffer.
 *
 * @return 		error_t Returns 0 If Configuration File Is Correctly Processed,
 * 				Otherwise Returns E_FAULT.
 */
error_t CONSOLELOG_ReadConfig(void);

/**
 * @brief 		Processes The Content of The Configuration File To Extract
 * 				and Validate The Baudrate.
 *
 * @param[in] 	Content The Content of The Configuration File as a Null-Terminated
 * 				String.
 *
 * @return 		error_t Returns 0 If Configuration File Is Correctly Processed,
 * 				Otherwise Returns E_FAULT.
 */
error_t CONSOLELOG_ProccessConfigFile(const char *content);


#endif /* RECORD_H_ */
