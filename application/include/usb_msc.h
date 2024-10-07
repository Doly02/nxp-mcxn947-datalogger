/*
 * usb_msc.h
 *
 *  Created on: Sep 14, 2024
 *      Author: tomas
 */

#ifndef USB_MSC_H_
#define USB_MSC_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
/*${standard_header_anchor}*/
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_msc.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "disk.h"

#include "usb_disk_adapter.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif

#if (USB_DEVICE_CONFIG_USE_TASK < 1)
#error This application requires USB_DEVICE_CONFIG_USE_TASK value defined > 0 in usb_device_config.h. Please recompile with this option.
#endif
#include "sdmmc_config.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Data structure of msc device, store the information ,such as class handle */
extern usb_msc_struct_t g_msc;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitHardware(void);

void USB_DeviceClockInit(void);

void USB_DeviceIsrEnable(void);

#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

void USB_DeviceMscApp(void);

usb_status_t USB_DeviceMscCallback(class_handle_t handle, uint32_t event, void *param);

usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);

void USB_DeviceMscAppTask(void);

/*!
 * @brief 	Initializes Mass Storage Class In Device Mode.
 * @details This Function Initialize The USB Stack and SDHC Driver.
 * 			After Successful Initialization USB MSG Is Launched.
 *
 * @return None.
 */
void USB_MscInit(void);

void APP_task(void *handle);

#endif /* USB_MSC_H_ */
