#include "project.h"
#include "hyatt.h"
#include <FS.h>

extern parser_block_t gc_block;

uint32_t timeoutStatusUpdate;
#define STATUSUPDATEINTERVAL 100

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
    hyattFileSenderInit();
    hyattControlPanelInit();
    hyattProbeInit();
    hyattZDisplayInit();
    FS_Init();
    
    hyattAxisSelected = AXISSELECTED_X;
    hyattWheelStepSize = WHEELSTEPSIZE_SMALL;
}

void hyattLoop() {
#ifdef NEXTION
    char c;
    if ((c = uartZDisplay_GetChar()) != 0) {
        usb_uart_write(c);
    }
#else
    hyattControlPanelLoop();
    hyattZDisplayLoop();
    hyattFileSenderLoop();
    
    if (hyattTicks > timeoutStatusUpdate) {
        // system_set_exec_state_flag(EXEC_STATUS_REPORT);
        timeoutStatusUpdate = hyattTicks + STATUSUPDATEINTERVAL;
    }
#endif
   
}

void grblBlockSend(char *block) {
    for (uint8_t i = 0; i < strlen(block); i++) {
        rx_handler(block[i]);                                                                                                            
    }
    rx_handler('\n');
}

void axisZero() {
    char buf[50];
    float offset = 0.00;

/*
    switch (hyattAxisSelected) {
        case AXISSELECTED_X:
            offset = settings.hyatt_zero_offset_x;
            break;
        case AXISSELECTED_Y:
            offset = settings.hyatt_zero_offset_y;
            break;
    }
*/

    sprintf(buf,"G10L20P%d%c%f",gc_state.modal.coord_select+1,selectedAxisLetter(),offset);
    grblBlockSend(buf);
}

void unitToggle() {
    (gc_block.modal.units == UNITS_MODE_INCHES) ? grblBlockSend("G21"):grblBlockSend("G20");
}
