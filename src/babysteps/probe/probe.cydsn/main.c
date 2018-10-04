#include <stdio.h>
#include "project.h"

uint16_t probeCount;

CY_ISR(probeHandler) {
    probeCount++;
}

int main(void) {
    uint16_t probeCountLast = 0;
    CyGlobalIntEnable;
    probePGA_Start();
    probeDAC_Start();
    probeComp_Start();
    
    isrProbe_StartEx(probeHandler);
    
    Serial_Start();
    Serial_PutString("\nprobe main\n");
    
    for(;;) {
        char buf[20];
        if (probeCountLast != probeCount) {
            sprintf(buf,"%u\n",probeCount);
            Serial_PutString(buf);
            probeCountLast = probeCount;
        }
    }
}
