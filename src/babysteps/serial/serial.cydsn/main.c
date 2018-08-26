/*  This project duplicates works from 
    Barton Dring's Grbl_USB_Native project
    https://github.com/bdring/Grbl_USB_Native */
   
#include "project.h"

#define USBFS_DEVICE        (0u)
#define USBUART_BUFFER_SIZE (64u)
#define LINE_STR_LENGTH     (20u)
uint8 buffer[USBUART_BUFFER_SIZE]; // bytes received    

void usb_uart_write(uint8_t c) {
    uint16 ctr = 0;
    if (uartUsb_GetConfiguration() == 1) {    
        while (uartUsb_CDCIsReady() == 0u)
        {
            ctr++;  if (ctr > 400) 
                return; // prevent getting stuck
        }
        
        /* uppercasing alphabet */
        if (c >= 'a' && c <= 'z') c -= 0x20;
        
        uartUsb_PutChar(c);
    }
}

void usb_uart_check(){
    uint16 count; // number of bytes received   
           
    if (uartUsb_IsConfigurationChanged() != 0u) {
        /* Initialize IN endpoints when device is configured. */
        if (uartUsb_GetConfiguration() != 0u) {
            /* Enumeration is done, enable OUT endpoint to receive data 
             * from host. */
            uartUsb_CDC_Init();
        }
    }
    
    /* Service USB CDC when device is configured. */
    if (uartUsb_GetConfiguration() != 0u) {
        /* Check for input data from host. */
        if (uartUsb_DataIsReady() != 0u) {
            /* Read received data and re-enable OUT endpoint. */
            count = uartUsb_GetAll(buffer);

            if (count != 0u) {
                for (int i = 0; i < count; i++) {
                    usb_uart_write(buffer[i]);
                }
            }
        }
    }
}

void usb_uart_PutString(const char *s) {    
    uint16 ctr = 0;
    if (uartUsb_GetConfiguration() == 1) {    
        while (uartUsb_CDCIsReady() == 0u) {
            ctr++;  if (ctr > 400) 
                return; // prevent getting stuck
        }
        uartUsb_PutString(s);
    }        
}

#if defined (__GNUC__)
    /* Add an explicit reference to the floating point printf library */
    /* to allow usage of the floating point conversion specifiers. */
    /* This is not linked in by default with the newlib-nano library. */
    asm (".global _printf_float");
#endif

int main(void) {
    CyGlobalIntEnable;
    
    uartUsb_Start(USBFS_DEVICE, uartUsb_5V_OPERATION);
    
    for(;;) {
        usb_uart_check();
    }
}
