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

/*

void hyattRadioTest() {
    uint16_t crc;
    char buf[] = "G21\x0a\
G90\x0a\
M3\x0a\
G0 X-35.3338 Y0.6071 Z15.0\x0a\
G1 Z13.0217 F800\x0a\
G1 Y-0.6071\x0a\
G1 X-35.4547 Y-1.8152\x0a\
G1 X-35.6952 Y-3.0052\x0a\
G1 X-36.0531 Y-4.1654\x0a\
G1 X-36.5247 Y-5.2842\x0a\
G1 X-37.1054 Y-6.3505\x0a\
G1 X-37.7894 Y-7.3537\x0a\
G1 X-38.5698 Y-8.2838\x0a\
G1 X-39.4389 Y-9.1315\x0a\
G1 X-40.3882 Y-9.8885\x0a\
G1 X-41.4081 Y-10.5473\x0a\
G1 X-42.4885 Y-11.1012\x0a\
G1 X-43.6188 Y-11.5448\x0a\
G1 X-44.7875 Y-11.8736\x0a\
G1 X-45.9814 Y-12.0841\x0a\
G1 X-47.1615 Y-12.1739\x0a\
G1 X-48.4036 Y-12.1453\x0a\
G1 X-49.5907 Y-11.9974\x0a\
G1 X-50.7719 Y-11.7303\x0a\
G1 X-51.9224 Y-11.3464\x0a\
G1 X-53.029 Y-10.";
    
    crc = crcCCITT(buf,strlen(buf),0x0000);
}

*/