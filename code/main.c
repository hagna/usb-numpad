//standard libs
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

//avr libs
#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

//LUFA stuff
#include <LUFA/Version.h>
#include <LUFA/Drivers/USB/USB.h>
#include "Descriptors.h"

//own stuff
#include "defines.h"
#include "keypad.h"

/**********************************************************
 * Function prototypes
 **********************************************************/

void initialize(void);

//LUFA callbacks
void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);

/**********************************************************
 * External variables
 **********************************************************/

/**********************************************************
 * Global variables
 **********************************************************/

/** LUFA CDC Class driver interface configuration and state 
 *  information. This structure is passed to all CDC Class 
 *  driver functions, so that multiple instances of the same 
 *  class within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
{
    .Config =
    {
        .ControlInterfaceNumber         = 0,

        .DataINEndpointNumber           = CDC_TX_EPNUM,
        .DataINEndpointSize             = CDC_TXRX_EPSIZE,
        .DataINEndpointDoubleBank       = false,

        .DataOUTEndpointNumber          = CDC_RX_EPNUM,
        .DataOUTEndpointSize            = CDC_TXRX_EPSIZE,
        .DataOUTEndpointDoubleBank      = false,

        .NotificationEndpointNumber     = CDC_NOTIFICATION_EPNUM,
        .NotificationEndpointSize       = CDC_NOTIFICATION_EPSIZE,
        .NotificationEndpointDoubleBank = false,
    },
};

/** Standard file stream for the CDC interface when set up,
 *  so that the virtual CDC COM port can be used like any
 *  regular character stream in the C APIs
 */
FILE USBSerialStream;

/**********************************************************
 * Main function
 **********************************************************/
int main(void){

    //variables
    //uint8_t i;
    char sBuffer[STRBUFLEN]; //!< character buffer for usb serial
    uint8_t data;


    //initialize
    initialize();

    //main loop
    while (1)
    {
        _delay_ms(50);
        while ((data = keypad_get())){
            sprintf(sBuffer,"%.2X\r\n",data);
            fputs(sBuffer,&USBSerialStream);
        }

        /**LUFA usb related tasks*/
        CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
        USB_USBTask();

    }
    return 0;
}

/**********************************************************
 * Interrupt vectors
 **********************************************************/


/**********************************************************
 * Other functions
 **********************************************************/

/** Initializes all of the hardware. */
void initialize(void){
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

    /** LUFA USB related inits */
	USB_Init();
	CDC_Device_CreateBlockingStream
        (&VirtualSerial_CDC_Interface, &USBSerialStream);

    /* Enable Keypad */
    keypad_init();

    /** enable interrupts*/
    sei();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void){
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void){
}

void EVENT_USB_Device_ConfigurationChanged(void){
    CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void){
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}
