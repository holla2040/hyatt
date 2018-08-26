/*  This project duplicates works from 
    Barton Dring's Grbl_USB_Native project
    https://github.com/bdring/Grbl_USB_Native */
    
/*  also uartDisplay is connected to a nextion display
    which has its own protocol, for example x.txt="111.222"\xFF\xFF\xFF
    sets the nextion text field named 'x'. widgets 'y','m' and 'u' also 
    exists. Load the nextion display with psocSerialTest.HMI  */
   
#include <stdio.h>
#include "project.h"

#define USBFS_DEVICE        (0u)
#define USBUART_BUFFER_SIZE (64u)
#define LINE_STR_LENGTH     (20u)
uint8 buffer[USBUART_BUFFER_SIZE]; // bytes received    

void displayProcess(char c);

void usb_uart_write(uint8_t c) {
    uint16 ctr = 0;
    if (uartUsb_GetConfiguration() == 1) {    
        while (uartUsb_CDCIsReady() == 0u) {
            ctr++;  if (ctr > 400) 
                return; // prevent getting stuck
        }      
        uartUsb_PutChar(c);
    }
}

/*
void broadcast(uint8_t c) {
    usb_uart_write(c);
    uartWifi_PutChar(c);
    uartKitprog_PutChar(c);
    uartDisplay_PutChar(c);
}
*/

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
                    uartWifi_PutChar(buffer[i]);
                    uartKitprog_PutChar(buffer[i]);
                    displayProcess(buffer[i]);
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

char displayBuffer[50];
void displayProcess(char c) {
    if (c == '\r') {
        uartDisplay_PutString(displayBuffer);
        uartDisplay_PutString("\xff\xff\xff");
        strcpy(displayBuffer,"");
    } else {
        char tmpstr[2];
        tmpstr[0] = c;
        tmpstr[1] = 0;

        strcat(displayBuffer,tmpstr);
    }
    if (strlen(displayBuffer) > 48) {
        strcpy(displayBuffer,"");
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
    uint8_t c;
    uartUsb_Start(USBFS_DEVICE, uartUsb_5V_OPERATION);
    uartKitprog_Start();
    uartDisplay_Start();
    uartWifi_Start();
    
    strcpy(displayBuffer,"");
    
    for(;;) {
        usb_uart_check();
        if ((c = uartKitprog_GetChar()) != 0) { 
            usb_uart_write(c);
            uartWifi_PutChar(c);
            displayProcess(c);
        }
        if ((c = uartDisplay_GetChar()) != 0) { 
            if ((c > 31 && c < 127) || c == '\r' || c == '\n') {
                uartWifi_PutChar(c);
                uartKitprog_PutChar(c);
                usb_uart_write(c);
            }
        }
        if ((c = uartWifi_GetChar()) != 0) { 
            uartKitprog_PutChar(c);
            displayProcess(c);
            usb_uart_write(c);
        }
    }
}
