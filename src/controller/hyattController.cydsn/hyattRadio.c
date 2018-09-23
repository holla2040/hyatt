#include "project.h"
#include "hyatt.h"
#include <stdio.h>

uint8 errorStatus = 0u;

CY_ISR(hyattRadioRx){
    uint8 rxStatus;         
    
    do {
        rxStatus = uartRadio_RXSTATUS_REG;

        if((rxStatus & (uartRadio_RX_STS_BREAK      | uartRadio_RX_STS_PAR_ERROR |
                        uartRadio_RX_STS_STOP_ERROR | uartRadio_RX_STS_OVERRUN)) != 0u) {
           
            errorStatus |= rxStatus & ( uartRadio_RX_STS_BREAK      | uartRadio_RX_STS_PAR_ERROR | 
                                        uartRadio_RX_STS_STOP_ERROR | uartRadio_RX_STS_OVERRUN);
        }
        
        if((rxStatus & uartRadio_RX_STS_FIFO_NOTEMPTY) != 0u){
            rx_handler(uartRadio_RXDATA_REG);
        }
    }while((rxStatus & uartRadio_RX_STS_FIFO_NOTEMPTY) != 0u);
}

void hyattRadioInit() {
    uartRadio_Start();
    isrUartRadioRx_StartEx(hyattRadioRx);
}