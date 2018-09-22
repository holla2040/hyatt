#include <stdio.h>
#include "project.h"
#include "hyatt.h"

int16_t countLast;
extern parser_block_t gc_block;

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
                return 0.05; // 1 wheel turn 5inch
        }
    }
    return 0;
}

void hyattControlPanelWheelLoop() {
    char buf[50];
    
    int16_t count = wheelDecoder_GetCounter();
    int16_t diff  = count - countLast;
    if ((diff != 0) && ((sys.state == STATE_IDLE) || (sys.state == STATE_JOG))) {
        sprintf(buf,"$J=F1000G2%dG91%c%-.4f",1-gc_block.modal.units,selectedAxisLetter(),diff*wheelClickDistance());
        grblBlockSend(buf);
    }    
    countLast = count;
}