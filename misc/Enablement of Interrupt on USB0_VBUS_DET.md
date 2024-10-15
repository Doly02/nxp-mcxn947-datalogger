#### Implementace preruseni pro detekci aktivnoho USB-C0

##### Nastaveni preruseni na Pin P4_12
```
/* Definition of Pins */
#define USB0_VBUS_DET_PIN 12U
#define USB0_VBUS_DET_PORT PORT4
#define USB0_VBUS_DET_GPIO GPIO4
#define USB0_VBUS_IRQ PORT4_IRQn

void InitUSBVbusInterrupt(void)
{
    /* Setup of Interrupt on Raising Edge (Connection to USB0 VBUS) */
    PORT_SetPinInterruptConfig(USB0_VBUS_DET_PORT, USB0_VBUS_DET_PIN, kPORT_InterruptRisingEdge);

    /* Enablement of Interrupt for Port in NVIC */
    EnableIRQ(USB0_VBUS_IRQ);
    return;
}
```

##### Implementace obsluzne rutiny

```
/* Global Variables */
volatile bool bVbusUsb0Detected = false;
volatile bool bInterruptHandled = false;

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
```

##### Zkontroluj zda jsou preruseni globalne povolena!
```
__enable_irq();
```