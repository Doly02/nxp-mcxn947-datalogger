/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      usb_vbus_detection.h
 *  Author:         Tomas Dolak
 *  Date:           19.10.2024
 *  Description:    Implements USB VBUS Detection.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           usb_vbus_detection.h
 *  @author         Tomas Dolak
 *  @date           19.10.2024
 *  @brief          Implements USB VBUS Detection.
 * ****************************/

#ifndef USB_VBUS_DETECTION_H_
#define USB_VBUS_DETECTION_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "defs.h"
#include "fsl_adapter_gpio.h"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
#if (true == USB0_DET_PIN_ENABLED)

void InitUSBVbusInterrupt(void);

#endif /* (true == USB0_DET_PIN_ENABLED) */

#endif /* USB_VBUS_DETECTION_H_ */
