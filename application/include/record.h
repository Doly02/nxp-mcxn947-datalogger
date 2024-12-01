/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      fatfs.h
 *  Author:         Tomas Dolak
 *  Date:           18.11.2024
 *  Description:    --
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           fatfs.h
 *  @author         Tomas Dolak
 *  @date           18.11.2024
 *  @brief          --
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

#include "fsl_debug_console.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "sdmmc_config.h"
#include "fsl_common.h"
#include "rtc_ds3231.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Structures
 ******************************************************************************/
typedef enum
{
	WCT_UNKOWN = 0,
	WCT_AUTOS1,
	WCT_AUTOS2

} REC_version_t;

typedef struct
{
	REC_version_t 	version;	//<! Board That Will Be Recorded
	uint32_t 		baudrate;	//<! Desired Baudrate

} REC_config_t;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*
 * @brief 		Creates Files Based On Time And Date.
 *
 * @param[in]	Time That Will Be Part of Filename.
 * @param[in]	Date That Will Be Part of Filename.
 *
 * @return		Return Pointer to Created File Descriptor.
 */
FIL* RECORD_CreateFile(RTC_date_t date, RTC_time_t time);

/*
 * @brief 		Returns Active Configuration.
 *
 * @details 	This Function Returns The Global Configuration Structure That Contains
 * 				The Settings For The Recording, Such as Baudrate, Version,
 * 				And Other Relevant Parameters.
 *
 * @return		REC_config_t The Current Recording Configuration.
 */
REC_config_t RECORD_GetConfig(void);

/*
 * @brief 		Returns the Version of The Device Being Recorded.
 *
 * @return		REC_version_t Current Version of Recorded Device
 * 				(WCT_UNKOWN, WCT_AUTOS1 or WCT_AUTOS2).
 */
REC_version_t RECORD_GetVersion(void);

/*
 * @brief		Checks If The File System Is Initialized
 *
 * return		F_OK If File System Initialized.
 */
FRESULT RECORD_CheckFileSystem(void);

/*
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
 * @return 		uint8_t Returns 0 on Success, Otherwise E_FAULT.
 */
uint8_t RECORD_Init(void);

/*
 * @brief 		Starts The Recording Process by Initializing the File
 * 				System, Creating a Directory, and Writing to a File.
 *
 * @details 	Function Uses `RECORD_Init` To Initialize The Recording
 * 				System.
 *
 * @return 		uint8_t Returns 0 on Success, Otherwise E_FAULT.
 */
uint8_t RECORD_Start(void);
/*
 * @brief 		De-Initializes The Recording System and Un-Mounts The File System.
 *
 * @return 		uint8_t Returns 0 on Success, Otherwise E_FAULT.
 */
uint8_t RECORD_Deinit(void);

/*
 * @brief		Reads and Processes The Configuration File From The Root directory.
 *
 * @details		This Function Scans The Root Directory For a Configuration File, If The File is Found
 * 				Reads its Contents Into g_config Buffer.
 *
 * @return 		uint8_t Returns 0 If Configuration File Is Correctly Processed,
 * 				Otherwise Returns E_FAULT.
 */
uint8_t RECORD_ReadConfig(void);

/*
 * @brief 		Processes The Content of The Configuration File To Extract
 * 				and Validate The Baudrate.
 *
 * @param[in] 	Content The Content of The Configuration File as a Null-Terminated
 * 				String.
 *
 * @return 		uint8_t Returns 0 If Configuration File Is Correctly Processed,
 * 				Otherwise Returns E_FAULT.
 */
uint8_t RECORD_ProccessConfigFile(const char *content);


#endif /* RECORD_H_ */
