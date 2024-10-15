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

#define RTC_ENABLED				(false) 	//!< Enables Functionality of DS3231 Real-Time Circuit. (Just For Development)
#define MSC_ENABLED				(true)		//!< Enables Functionality of USB Mass Storage. (Just For Development)
#define USB0_DET_PIN_ENABLED 	(true) 	//!< Enables VBUS Detection on USB-C0

#if (true == USB0_DET_PIN_ENABLED)

/* Definition of Pins */
#define USB0_VBUS_DET_PIN 12U
#define USB0_VBUS_DET_PORT PORT4
#define USB0_VBUS_DET_GPIO GPIO4
#define USB0_VBUS_IRQ PORT4_IRQn

#endif /* (true == USB0_DET_PIN_ENABLED */

#endif /* DEFS_H_ */
