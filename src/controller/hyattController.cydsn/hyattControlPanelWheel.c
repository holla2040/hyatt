#include <stdio.h>
#include "project.h"
#include "hyatt.h"

int16_t countLast;
extern parser_block_t gc_block;

uint32_t timeoutWheelUpdate;
#define WHEELUPDATEINTERVAL 250


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
                return 5.0; // 1 wheel turn 500mm
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

void hyattControlPanelWheelLoop() {
    char buf[50];
    
    int16_t count = wheelDecoder_GetCounter();
    int16_t diff  = count - countLast;
    if (hyattTicks > timeoutWheelUpdate) {
        if (diff != 0) {
            sprintf(buf,"$J=%c%-.4fG2%dG91F5000",selectedAxisLetter(),diff*wheelClickDistance(),1-gc_block.modal.units);
            grblBlockSend(buf);
            countLast = count;
        }
        timeoutWheelUpdate = hyattTicks + WHEELUPDATEINTERVAL;
    }
}