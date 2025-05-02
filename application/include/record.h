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
#include <led.h>
#include <string.h>
#include "fsl_sd.h"
#include "ff.h"						/*<! File System */
#include "ffconf.h"					/*<! File System Configuration */
#include <stdio.h>

#include "fsl_sd_disk.h"
#include "fsl_common.h"
#include "diskio.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "sdmmc_config.h"
#include "rtc_ds3231.h"
#include "fsl_common_arm.h"

#include "task.h"

#include "error.h"
#include "uart.h"
#include "parser.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/**
 * @brief Timeout Interval Before Flush If No New Data Were Received [In Mili-Seconds].
 */
#define FLUSH_TIMEOUT_TICKS (int)pdMS_TO_TICKS(3000)

#define REINIT_TIMEOUT_TICKS (int)pdMS_TO_TICKS(1000)
/*******************************************************************************
 * Structures
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/**
 * @brief 		Return Absolute Value.
 *
 * @return		Returns Free Space on SD Card.
 */
int CONSOLELOG_Abs(int x);

/**
 * @brief 		Gets Free Space on SD Card.
 *
 * @return		Returns Free Space on SD Card.
 */
uint32_t CONSOLELOG_GetFreeSpaceMB(void);

/**
 * @brief 		Creates Directory Based Actual Date.
 *
 * @return		Returns Zero If Directory Creation Succeeded.
 */
error_t CONSOLELOG_CreateDirectory(void);

/**
 * @brief 		Creates File Based Actual Date and Counter Value.
 *
 * @return		Returns Zero If File Creation Succeeded.
 */
error_t CONSOLELOG_CreateFile(void);

/**
 * @brief 		Returns Currently Received Bytes Between LED Blinking.
 *
 * @return		uint32_t Received Bytes Between LED Blinking.
 */
uint32_t CONSOLELOG_GetTransferedBytes(void);

/**
 * @brief 		Returns If Flush Was Completed or Not.
 *
 * @return		If Recording Is Ongoing That Return False.
 */
bool CONSOLELOG_GetFlushCompleted(void);
/**
 * @brief 		Clears Currently Received Bytes After LED Blinking.
 */
void CONSOLELOG_ClearTransferedBytes(void);

/**
 * @brief 		Maximal Received Bytes Between LED Blinking.
 */
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
 * 				Saves It To a File on The Physical Media and Closes The File.
 *
 * @return		error_t Returns 0 on Success, Otherwise Returns a Non-Zero Value.
 */
error_t CONSOLELOG_Flush(void);

/**
 * @brief 		Flushes Collected Data To The File If Power Loss Was Detected.
 * @details		If Power Loss Was Detected, Then This Function Flushes All The Data
 * 				So Far Stored In The DMA Buffer, Saves It To a File on The Physical Media and Closes The File.
 *
 * @return		error_t Returns 0 on Success, Otherwise Returns a Non-Zero Value.
 */
error_t CONSOLELOG_PowerLossFlush(void);

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
 * @param[in] 	content Content The Content of The Configuration File as a Null-Terminated
 * 				String.
 *
 * @return 		error_t Returns 0 If Configuration File Is Correctly Processed,
 * 				Otherwise Returns E_FAULT.
 */
error_t CONSOLELOG_ProccessConfigFile(const char *content);


#endif /* RECORD_H_ */
