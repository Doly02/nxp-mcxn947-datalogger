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
 * Prototypes
 ******************************************************************************/

FIL* RECORD_CreateFile(RTC_date_t date, RTC_time_t time);


uint8_t RECORD_Init(void);

uint8_t RECORD_Deinit(void);

uint8_t RECORD_Start(void);


#endif /* RECORD_H_ */
