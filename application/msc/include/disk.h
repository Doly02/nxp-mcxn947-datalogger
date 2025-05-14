/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DISK_H__
#define __USB_DISK_H__ 1

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <disk.h>
#include <stdio.h>
#include <stdlib.h>

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_msc.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "usb_disk_adapter.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "semphr.h"

#include "usb_device_dci.h"
#include "usb_device_ehci.h"

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif

#include "sdmmc_config.h"

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
	#ifndef CONTROLLER_ID
	#define CONTROLLER_ID kUSB_ControllerEhci0
	#endif
#endif

#define USB_DEVICE_INTERRUPT_PRIORITY (3U)

/*! @brief enable the write task. 1U supported, 0U not supported . if this macro is enable ,the \
USB_DEVICE_CONFIG_USE_TASK macro should also be enable.*/
#define USB_DEVICE_MSC_USE_WRITE_TASK (0U)
#define USB_DEVICE_MSC_BUFFER_NUMBER  (3U)

#define LOGICAL_UNIT_SUPPORTED (1U)

/* USB MSC config*/
/*buffer size for sd card example. the larger the buffer size ,the faster the data transfer speed is ,*/
/*the block size should be multiple of 512, the least value is 512*/

#define USB_DEVICE_MSC_WRITE_BUFF_SIZE (512 * 64U)
#define USB_DEVICE_MSC_READ_BUFF_SIZE  (512 * 64U)

#define USB_DEVICE_SDCARD_BLOCK_SIZE_POWER (9U)
#define USB_DEVICE_MSC_ADMA_TABLE_WORDS    (8U)

/*******************************************************************************
 * Structures
 ******************************************************************************/

typedef struct _usb_msc_buffer_struct
{
    uint32_t offset; 						/*!< Offset of the block need to access*/
    uint32_t size;   						/*!< Size of the transfered data*/
    struct _usb_msc_buffer_struct *next;
    uint8_t *buffer; 						/*!< Buffer address of the transferred data*/
} usb_msc_buffer_struct_t;

typedef struct _usb_msc_struct
{
    usb_device_handle deviceHandle;
    usb_device_msc_struct_t mscStruct;

    uint8_t diskLock;
    uint8_t read_write_error;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_MSC_INTERFACE_COUNT];
    uint8_t speed;
    uint8_t attach;
    uint8_t stop; /* indicates this media keeps stop or not, 1: stop, 0: start */
    usb_msc_buffer_struct_t *headlist;
    usb_msc_buffer_struct_t *taillist;
    usb_msc_buffer_struct_t *transferlist;
} usb_msc_struct_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void BOARD_InitHardware(void);

void USB_DeviceClockInit(void);

void USB_DeviceIsrEnable(void);

void USB_DeviceModeInit(void);

#endif /* __USB_DISK_H__ */
