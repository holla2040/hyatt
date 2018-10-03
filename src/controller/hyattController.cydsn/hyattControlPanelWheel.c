#include <stdio.h>
#include "project.h"
#include "hyatt.h"

int16_t countLast;
uint8_t diff0Count;

extern parser_block_t gc_block;

uint32_t timeoutWheelUpdate;
#define WHEELUPDATEINTERVAL 10


void hyattControlPanelWheelInit() {
    wheelDecoder_Start();
    countLast = wheelDecoder_GetCounter();
}

double wheelClickDistance() {
    if (gc_block.modal.units == UNITS_MODE_MM) {
        switch (hyattWheelStepSize) {
            case WHEELSTEPSIZE_SMALL:
                return 0.01; // 1 wheel turn 1mm
            case WHEELSTEPSIZE_MEDIUM:
                return 0.1; // 1 wheel turn 10mm
            case WHEELSTEPSIZE_LARGE:
                return 1.0; // 1 wheel turn 100mm
        }
    } else {
        switch (hyattWheelStepSize) {
            case WHEELSTEPSIZE_SMALL:
                return 0.001; // 1 wheel turn 10mil
            case WHEELSTEPSIZE_MEDIUM:
                return 0.01; // 1 wheel turn 1inch
            case WHEELSTEPSIZE_LARGE:
                return 0.05; // 1 wheel turn ?inch
        }
    }
    return 0;
}

void hyattControlPanelWheelLoop() {  // this is kinda crappy, wheel shouldn't actually send the block
    char buf[50];
    if (hyattTicks > timeoutWheelUpdate) {
        int16_t count = wheelDecoder_GetCounter();
        int16_t diff  = count - countLast;
        if (diff != 0) {
            if (plan_get_block_buffer_available() > 3) {  // don't over run the planning buffer
                sprintf(buf,"$J=%c%-.4fG2%dG91F7500",selectedAxisLetter(),diff*wheelClickDistance(),1-gc_block.modal.units);
                grblBlockSend(buf);
                diff0Count = 0;
                countLast = count;
            }
        } else {
            diff0Count++;
            if (diff0Count == 3) { // this is third time there's no count difference, ie wheel not turning
                if (sys.state & STATE_JOG) { // Block all other states from invoking motion cancel.
                 //   usb_uart_PutString("------------ Cancel\n");
                  system_set_exec_state_flag(EXEC_MOTION_CANCEL);
                }
            }
        }
        timeoutWheelUpdate = hyattTicks + WHEELUPDATEINTERVAL;
    }
}