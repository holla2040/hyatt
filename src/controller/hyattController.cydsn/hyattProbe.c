#include <stdio.h>
#include "project.h"
#include "hyatt.h"

CY_ISR(probeHandler) {
    probeCount++;
}

void hyattProbeInit(void) {
    probePGA_Start();
    probeDAC_Start();
    probeComp_Start();
    isrProbe_StartEx(probeHandler);
    probeCount = 0;
}

uint8_t hyattProbeRead() {
    return probeCount;
}

void hyattProbeReset() {
    probeCount = 0;
}