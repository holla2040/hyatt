#include <stdio.h>
#include "project.h"

int main(void)
{
    int16_t countLast,count;
    char report[100];

    CyGlobalIntEnable; /* Enable global interrupts. */
 
    Serial_Start();
    Serial_PutString("WheelDecoder main\n");

    WheelDecoder_Start();
    countLast = WheelDecoder_GetCounter();
    
    for(;;)
    {
        count = WheelDecoder_GetCounter();
        if (count != countLast) {
            sprintf(report,"%d %d\n",count,countLast);
            Serial_PutString(report);
            countLast = count;
        }
    }
}
