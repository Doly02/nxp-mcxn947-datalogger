/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USB_DEVICE_CONFIG_H_
#define _USB_DEVICE_CONFIG_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*!
 * @addtogroup usb_device_configuration
 * @{
 */

/*!
 * @name Hardware instance define
 * @{
 */

/* @TEST_ANCHOR */

/*! @brief EHCI instance count */
#ifndef USB_DEVICE_CONFIG_EHCI

#define USB_DEVICE_CONFIG_EHCI 						(1U)

#endif

/*! @brief Device instance count, the sum of KHCI and EHCI instance counts*/
#define USB_DEVICE_CONFIG_NUM (USB_DEVICE_CONFIG_EHCI)

/* @} */

/*!
 * @name class instance define
 * @{
 */

/*! @brief MSC instance count */
#define USB_DEVICE_CONFIG_MSC 						(1U)

/* @} */

/*! @brief Whether device is self power. 1U supported, 0U not supported */
#define USB_DEVICE_CONFIG_SELF_POWER 				(1U)

/*! @brief How many endpoints are supported in the stack. */
#define USB_DEVICE_CONFIG_ENDPOINTS 				(4U)

/*! @brief Whether the device task is enabled. */
#define USB_DEVICE_CONFIG_USE_TASK 					(0U)

/*! @brief How many the notification message are supported when the device task is enabled. */
#define USB_DEVICE_CONFIG_MAX_MESSAGES 				(8U)

#if ((defined(USB_DEVICE_CONFIG_EHCI)) && (USB_DEVICE_CONFIG_EHCI > 0U))
/*! @brief How many the DTD are supported. */
#define USB_DEVICE_CONFIG_EHCI_MAX_DTD 				(16U)

/*! @brief Whether the EHCI ID pin detect feature enabled. */
#define USB_DEVICE_CONFIG_EHCI_ID_PIN_DETECT 		(0U)

#endif

/*! @brief Whether the keep alive feature enabled. */
#define USB_DEVICE_CONFIG_KEEP_ALIVE_MODE 			(0U)

/*! @brief Whether the transfer buffer is cache-enabled or not. */
#ifndef USB_DEVICE_CONFIG_BUFFER_PROPERTY_CACHEABLE

#define USB_DEVICE_CONFIG_BUFFER_PROPERTY_CACHEABLE (0U)

#endif
/*! @brief Whether the low power mode is enabled or not. */
#define USB_DEVICE_CONFIG_LOW_POWER_MODE 			(0U)

#if ((defined(USB_DEVICE_CONFIG_LOW_POWER_MODE)) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))

/*! @brief Whether device remote wakeup supported. 1U supported, 0U not supported */
#define USB_DEVICE_CONFIG_REMOTE_WAKEUP 			(0U)

/*! @brief Whether LPM is supported. 1U supported, 0U not supported */
#define USB_DEVICE_CONFIG_LPM_L1 					(0U)
#else
/*! @brief The device remote wakeup is unsupported. */
#define USB_DEVICE_CONFIG_REMOTE_WAKEUP 			(0U)
#endif

/*! @brief Whether the device detached feature is enabled or not. */
#define USB_DEVICE_CONFIG_DETACH_ENABLE 			(0U)

/*! @brief Whether handle the USB bus error. */
#define USB_DEVICE_CONFIG_ERROR_HANDLING 			(0U)

/*! @brief Whether checking return value is enabled. */
#define USB_DEVICE_CONFIG_RETURN_VALUE_CHECK 		(0U)

/*! @brief Whether the SOF interrupt is enabled or not. */
#define USB_DEVICE_CONFIG_SOF_NOTIFICATIONS 		(0U)

/* @} */

#endif /* _USB_DEVICE_CONFIG_H_ */
