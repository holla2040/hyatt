/* this project uses nextion display loaded with droSimple


*/
#include "project.h"
#include "stdlib.h"

#if defined (__GNUC__)
    /* Add an explicit reference to the floating point printf library */
    /* to allow usage of the floating point conversion specifiers. */
    /* This is not linked in by default with the newlib-nano library. */
    asm (".global _printf_float");
#endif

char displayBuffer[50];
char consoleBuffer[50];

void lineProcess(char *line) {
    char c = line[0];
    int  i = atoi(&line[1]);

    display_PutString("title.txt=\"");
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
    
    console_PutString("nextion main\n");
    display_PutString("title.txt=\"nextion main\"\xff\xff\xff");

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
    }
}
