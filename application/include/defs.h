/*
 *		Author: 		Tomas Dolak
 * 		File Name:		rtc_ds1307.c
 *      Description:	RTC DS1307 Driver.
 *		Created on: 	Aug 7, 2024
 *
 *		@author			Tomas Dolak
 * 		@brief			RTC DS1307 Driver.
 * 		@filename		rtc_ds1307.c
 */

#ifndef DEFS_H_
#define DEFS_H_

#define SUCCESS					(0U)
#define E_FAULT					(1U)
#define NOT_IMPLEMENTED			#error "This Feature Is Not Implemented!"

/*
 * @brief Enables Mass Storage Functionality.
 * */
#define MSC_ENABLED				(true)
#define MSC_STACK_SIZE			(9000L / sizeof(portSTACK_TYPE))

#endif /* DEFS_H_ */
