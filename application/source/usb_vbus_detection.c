/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      usb_vbus_detection.c
 *  Author:         Tomas Dolak
 *  Date:           19.10.2024
 *  Description:    Implements USB VBUS Detection.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           usb_vbus_detection.c
 *  @author         Tomas Dolak
 *  @date           19.10.2024
 *  @brief          Implements USB VBUS Detection.
 * ****************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "usb_vbus_detection.h"
#include "fsl_debug_console.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Definition of Pins */
#define USB0_VBUS_DET_PIN 		12U
#define USB0_VBUS_DET_PORT 		4
#define USB0_VBUS_DET_GPIO 		GPIO4
#define USB0_VBUS_IRQ 			GPIO40_IRQn

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
#if (true == USB0_DET_PIN_ENABLED)

volatile bool bVbusUsb0Detected = false;
volatile bool bInterruptHandled = false;

GPIO_HANDLE_DEFINE(s_Usb0DetGpioHandle);

#endif /* (true == USB0_DET_PIN_ENABLED) */
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
#if (true == USB0_DET_PIN_ENABLED)

void InitUSBVbusInterrupt(void)
{
    uint8_t pinState;
	hal_gpio_pin_config_t sw_config = {
	            kHAL_GpioDirectionIn,
	            1,
				USB0_VBUS_DET_PORT,
				USB0_VBUS_DET_PIN,
	        };
	HAL_GpioInit(s_Usb0DetGpioHandle, &sw_config);
    HAL_GpioSetTriggerMode(s_Usb0DetGpioHandle, kHAL_GpioInterruptLogicOne);

    /*
     *     	GPIO_SetPinInterruptConfig(s_GpioPort[gpioState->port], gpioState->pin, pinInt);
     *		NVIC_SetPriority(irqNo[gpioState->port], HAL_GPIO_ISR_PRIORITY);
     *		NVIC_EnableIRQ(irqNo[gpioState->port]);
     * */

    if (HAL_GpioGetInput(s_Usb0DetGpioHandle, &pinState) == kStatus_HAL_GpioSuccess)
    {
        if (pinState == 1U)
        {
        	PRINTF("USB Detected\r\n");
        }
        else
        {
        	PRINTF("USB Not Detected\r\n");
        }
    }
}



#endif /* (true == USB0_DET_PIN_ENABLED) */
