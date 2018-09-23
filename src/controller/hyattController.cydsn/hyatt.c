#include "project.h"
#include "hyatt.h"


uint32_t timeoutStatusUpdate;
#define STATUSUPDATEINTERVAL 50

/* 24HZ interrupt */
CY_ISR(isrHyattMainHandler) {
    hyattTicks++;
}

char selectedAxisLetter() {
    switch (hyattAxisSelected) {
        case AXISSELECTED_X:
            return 'X';
            break;
        case AXISSELECTED_Y:
            return 'Y';
            break;
        case AXISSELECTED_Z:
            return 'Z';
            break;
        case AXISSELECTED_A:
            return 'A';
            break;
    }
    return 0;
 }   
        


void hyattInit() {
    isrHyatt_StartEx(isrHyattMainHandler);
    
    I2C_Start();   
    CyDelay(50);
    
    IO_RESET_Write(0); // reset the IO Expanders
    CyDelay(1);
    IO_RESET_Write(1);
    
    AMUX_Start();
    hyattRadioInit();
    hyattControlPanelInit();
    
    hyattAxisSelected = AXISSELECTED_X;
    hyattWheelStepSize = WHEELSTEPSIZE_SMALL;
}

void hyattLoop() {
    hyattControlPanelLoop();
    
    if (hyattTicks > timeoutStatusUpdate) {
        system_set_exec_state_flag(EXEC_STATUS_REPORT);
        timeoutStatusUpdate = hyattTicks + STATUSUPDATEINTERVAL;
    }
}

void grblBlockSend(char *block) {
    for (uint8_t i = 0; i < strlen(block); i++) {
        rx_handler(block[i]);                                                                                                            
    }
    rx_handler('\n');
}
