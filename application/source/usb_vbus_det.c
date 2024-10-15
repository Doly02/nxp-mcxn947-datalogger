/******************************
 *  Project:        NXP MCXN947 Datalogger
 *  File Name:      mainc.c
 *  Author:         Tomas Dolak
 *  Date:           07.08.2024
 *  Description:    Implements Datalogger Application.
 *
 * ****************************/

/******************************
 *  @package        NXP MCXN947 Datalogger
 *  @file           main.c
 *  @author         Tomas Dolak
 *  @date           07.08.2024
 *  @brief          Implements Datalogger Application.
 * ****************************/

#if (true == USB0_DET_PIN_ENABLED)

/* Global Variables */
volatile bool bVbusUsb0Detected = false;
volatile bool bInterruptHandled = false;

void USB0_VbusInterruptInit(void)
{
    /* Setup of Interrupt on Raising Edge (Connection to USB0 VBUS) */
    PORT_SetPinInterruptConfig(USB0_VBUS_DET_PORT, USB0_VBUS_DET_PIN, kPORT_InterruptRisingEdge);

    /* Enablement of Interrupt for Port in NVIC */
    EnableIRQ(USB0_VBUS_IRQ);
    return;
}

void PORT4_IRQHandler(void)
{
    /* Check if Interrupt Was Raised on USB0_VBUS_DET Pin */
    if (!isInterruptHandled && (PORT_GetPinsInterruptFlags(USB0_VBUS_DET_PORT) & (1U << USB0_VBUS_DET_PIN)))
    {
        /* Set That VBUS on USB0 Is Active  */
        bVbusUsb0Detected = true;
        /* Interrupt Was Processed          */
        bInterruptHandled = true;

        /* Clear Interrupt Flag -> For Repetition of Interrupt */
        PORT_ClearPinsInterruptFlags(USB0_VBUS_DET_PORT, (1U << USB0_VBUS_DET_PIN));
    }
}

#endif
