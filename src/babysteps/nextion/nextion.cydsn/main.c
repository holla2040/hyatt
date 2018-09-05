/* this project uses nextion display loaded with droSimple

   add m library gcc command line -lm
    change heap size to 0x200 in cydwr System
    add linker command line -u_printf_float

*/
#include "project.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

char displayBuffer[50];
char consoleBuffer[50];
double t,x,y,z;
bool displayPending;

CY_ISR(updateHandler) {
    t += 0.01;
    displayPending = true;
    isrDisplay_ClearPending();
}

void displayUpdate() {
    char line[100];
    sprintf(line,"x.txt=\"%9.4f\"\xff\xff\xffy.txt=\"%9.4f\"\xff\xff\xffz.txt=\"%9.4f\"\xff\xff\xff",x,y,z);
    display_PutString(line);
    displayPending = false;

/*
    char line[50];
    sprintf(line,"x.txt=\"%9.4f\"\xff\xff\xff",500.0*cos(t)+500.0);
    display_PutString(line);
    sprintf(line,"y.txt=\"%9.4f\"\xff\xff\xff",500.0*sin(t)+500.0);
    display_PutString(line);
    sprintf(line,"z.txt=\"%9.4f\"\xff\xff\xff",50.0*sin(t + 3.00)-50.0);
    display_PutString(line);
*/
}    
 
void lineProcess(char *line) {
    char c = line[0];
    int  i = atoi(&line[1]);

    display_PutString("line.txt=\"");
    display_PutString(line);
    display_PutString("\"\xff\xff\xff");
    
    if (c == 'G') {
        switch (i) {
            case 0:
                display_PutString("motion.txt=\"G0\"\xff\xff\xff");
                display_PutString("motionlabel.txt=\"rapid\"\xff\xff\xff");
                break;
            case 1:
                display_PutString("motion.txt=\"G1\"\xff\xff\xff");
                display_PutString("motionlabel.txt=\"linear\"\xff\xff\xff");
                break;
            case 20:
                display_PutString("unit.txt=\"G20\"\xff\xff\xff");
                display_PutString("unitlabel.txt=\"inch\"\xff\xff\xff");
                break;
            case 21:
                display_PutString("unit.txt=\"G21\"\xff\xff\xff");
                display_PutString("unitlabel.txt=\"mm\"\xff\xff\xff");
                break;
            case 90:
                display_PutString("distance.txt=\"G90\"\xff\xff\xff");
                display_PutString("distancelabel.txt=\"absolute\"\xff\xff\xff");
                break;
            case 91:
                display_PutString("distance.txt=\"G91\"\xff\xff\xff");
                display_PutString("distancelabel.txt=\"incremental\"\xff\xff\xff");
                break;
            case 54:
            case 55:
            case 56:
            case 57:
            case 58:
                display_PutString("coordinate.txt=\"");
                display_PutString(line);
                display_PutString("\"\xff\xff\xff");
                break;
        }
    }               
}
void displayProcessChar(uint8_t c) {
    if (c == '\n') {
        lineProcess(displayBuffer);
        strcpy(displayBuffer,"");
    } else {
        char tmpstr[2];
        tmpstr[0] = c;
        tmpstr[1] = 0;

        strcat(displayBuffer,tmpstr);
    }
    if (strlen(displayBuffer) > 48) {
        strcpy(displayBuffer,"");
    }
}
void consoleProcessChar(uint8_t c) {
    if (c == '\r') {
        lineProcess(consoleBuffer);
        strcpy(consoleBuffer,"");
    } else {
        char tmpstr[2];
        tmpstr[0] = c;
        tmpstr[1] = 0;

        strcat(consoleBuffer,tmpstr);
    }
    if (strlen(consoleBuffer) > 48) {
        strcpy(consoleBuffer,"");
    }
}

int main(void)
{
    uint8_t c;
    CyGlobalIntEnable;
    console_Start();
    display_Start();
//   clockDisplay_Start();
    isrDisplay_StartEx(updateHandler);
    
    console_PutString("nextion main\n");
    display_PutString("line.txt=\"nextion main\"\xff\xff\xff");

    for(;;)
    {
        if ((c = display_GetChar()) != 0) { 
            if ((c > 31 && c < 127) || c == '\r' || c == '\n') {        
                displayProcessChar(c);
                console_PutChar(c);
            }
        }
        if ((c = console_GetChar()) != 0) { 
            if ((c > 31 && c < 127) || c == '\r' || c == '\n') {        
                consoleProcessChar(c);
                console_PutChar(c);
            }
        }
        if (displayPending) displayUpdate();
        x = 500.0*cos(t)+500.0;
        y = 500.0*sin(t)+500.0;
        z = 50.0*sin(t+3.00)-50.0;
    }
}
