#include <stdio.h>
#include <stdlib.h>
#include "project.h"
#include "hyatt.h"
#include <FS.h>

uint16_t hyattFileBufferLen;
char hyattFileBuffer[FILEBUFFERLEN];
char *hyattFileBufferPtr;
FS_FILE *file;

char filelist[CONTROLPANEL_SELECTIONCOUNTMAX][FILENAMEMAX];
uint8_t fileSelectIndex;
char fileoplist[CONTROLPANEL_SELECTIONCOUNTMAX][OPNAMEMAX];
float fileXMin,fileXMax,fileYMin,fileYMax;

extern char selections[CONTROLPANEL_SELECTIONCOUNTMAX][CONTROLPANEL_SELECTIONWIDTH];

void hyattFilelistGet() {
    uint8_t i = 0;
    FS_FIND_DATA fd;
    char fn[32];

    FS_Mount("");
    selectionsClear();
    if (FS_FindFirstFile(&fd, "", fn, sizeof(fn)) == 0) {
        do {
            if (!(fd.Attributes & FS_ATTR_DIRECTORY)) {
                if (strlen(fn)){
                    strcpy(filelist[i],fn);
                    strncpy(selections[i],fn,CONTROLPANEL_SELECTIONWIDTH-1);
                    for (uint8_t j = 0; j < strlen(selections[i]); j++) {
                        if (selections[i][j] == '.') selections[i][j] = 0x00;
                    }
                } else {
                    strcpy(filelist[i],"");
                }
                i++;
                if (i == CONTROLPANEL_SELECTIONCOUNTMAX) break; // there's more than 9 files on sd
            }
        } while (FS_FindNextFile (&fd));
    }
    FS_FindClose(&fd);
    FS_Unmount("");
}

void findPattern(FS_FILE *fp, char c1, char c2) {
    char c,lastc;
    lastc = 0;
    while (FS_Read(fp,&c,1)) {
        if (c == '\r') continue;
        if (lastc == c1 && c == c2)  {
            break;
        }
        lastc = c;
    }
}

void hyattFilePerimeter(char *fn) {
    FS_FILE *fp;
    char c,word[30],line[50];
    char *wp;
    float v;

    fileXMin = 100000;
    fileYMin = 100000;
    fileXMax = -100000;
    fileYMax = -100000;

    FS_Mount("");
    fp = FS_FOpen(fn, "r");
    findPattern(fp,'\n','\n');

    wp = word;
    while((hyattFileBufferLen = FS_Read(file,&hyattFileBuffer,FILEBUFFERLEN))) {
        for (uint16_t i = 0; i < hyattFileBufferLen; i++) {
            c = hyattFileBuffer[i];
            if (c == '\r') continue;
            // printf("%c",(char)c);
            if (c > 96 && c < 123) {
                c |= 0x20; // uppercase
            }
            if (c == ' ' || c == '\n') {
                *wp = 0;
                if (strlen(word) == 0) {
                    continue;
                }

                v = atof(&word[1]);
                switch (word[0]) {
                    case 'X':
                        if (v > fileXMax) fileXMax = v;
                        if (v < fileXMin) fileXMin = v;
                        break;
                    case 'Y':
                        if (v > fileYMax) fileYMax = v;
                        if (v < fileYMin) fileYMin = v;
                        break;
                }

                wp = word;
            } else {
                *wp++ = c;
            }
        }
        sprintf(lastBlock,"%f,%f %f,%f",fileXMin,fileYMin,fileXMax,fileYMax);
        hyattZDisplayLoop();
    }

    FS_FClose(fp);
    fp = FS_FOpen("perim.nc", "w");
    if (fp != 0) {
        sprintf(line,"G1 F2500\n");
        FS_Write(fp,line,strlen(line));

        sprintf(line,"X%.4f Y%.4f\nM0\n",fileXMin,fileYMin);
        FS_Write(fp,line,strlen(line));

        sprintf(line,"X%.4f Y%.4f\nM0\n",fileXMax,fileYMin);
        FS_Write(fp,line,strlen(line));

        sprintf(line,"X%.4f Y%.4f\nM0\n",fileXMax,fileYMax);
        FS_Write(fp,line,strlen(line));

        sprintf(line,"X%.4f Y%.4f\nM0\n",fileXMin,fileYMax);
        FS_Write(fp,line,strlen(line));

        sprintf(line,"X%.4f Y%.4f\nM0\n",fileXMin,fileYMin);
        FS_Write(fp,line,strlen(line));

        FS_FClose(fp);
        // hyattFileSend("perim.nc");
    }

    FS_Unmount("");
}

void hyattFileSenderInit() {
    hyattFileSenderState = FILESENDERSTATE_IDLE;
}

void hyattFileSenderLoop() {
    char c;
    switch (hyattFileSenderState) {
        case FILESENDERSTATE_SEND:
            while(hyattFileBufferLen) {
                if (serial_get_rx_buffer_available() < 10 ) break; // parser flow control
                hyattFileBufferLen--;
                c = *hyattFileBufferPtr++;
                rx_handler(c);

               // usb_uart_write(c);
                
                if ((c == '\n') || (c == '\r')) {
                    break;
                 }
            }
            if (hyattFileBufferLen == 0) { // sent all buffer, read next file chunk
                hyattFileBufferLen = FS_Read(file,&hyattFileBuffer,FILEBUFFERLEN);
                if (hyattFileBufferLen == 0) { // no more data in file
                    FS_FClose(file);
                    FS_Unmount("");
                    hyattFileSenderState = FILESENDERSTATE_IDLE;
                    return;
                }
                hyattFileBufferPtr = &hyattFileBuffer[0];
            }
            break;
   }
}

void hyattFileSend(char *filename) {
    FS_Mount("");
    file = FS_FOpen(filename, "r");
    if (file) {
        hyattFileBufferLen = FS_Read(file,&hyattFileBuffer,FILEBUFFERLEN);
        hyattFileBufferPtr = hyattFileBuffer;
        hyattFileSenderState = FILESENDERSTATE_SEND;
    }
}

