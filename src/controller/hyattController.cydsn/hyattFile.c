#include <stdio.h>
#include <stdlib.h>
#include "project.h"
#include "hyatt.h"
#include <FS.h>

uint16_t hyattFileBufferLen;
char hyattFileBuffer[FILEBUFFERLEN];
char *hyattFileBufferPtr;
FS_FILE *file;
uint32_t fileSize;

char filelist[CONTROLPANEL_SELECTIONCOUNTMAX][FILENAMEMAX];
uint8_t fileSelectIndex;
char fileoplist[CONTROLPANEL_SELECTIONCOUNTMAX][OPNAMEMAX];
float fileXMin,fileXMax,fileYMin,fileYMax;
uint32_t fileOpSeeks[CONTROLPANEL_SELECTIONCOUNTMAX+1];

extern char selections[CONTROLPANEL_SELECTIONCOUNTMAX][CONTROLPANEL_SELECTIONWIDTH];
uint32_t fileStart, fileEnd, fileIndex;

void fileSeeksSet(uint32_t s) {
    for (uint8_t i = 0; i < (CONTROLPANEL_SELECTIONCOUNTMAX+1); i++) {
        fileOpSeeks[i] = s;
    }
}

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

void hyattFileOperationsGet(char *fn) {
    FS_FILE *fp;
    char c,*lp,line[100];
    uint8_t selectionIndex;

    selectionIndex = 0;

    FS_Mount("");
    fp = FS_FOpen(fn, "r");
    findPattern(fp,'\n','\n'); // scan past header

    selectionsClear();

    // first mock in a 'header' op (fusion's cam can put multiple (???) in the header)
    strcpy(selections[selectionIndex],"header");
    fileOpSeeks[selectionIndex++] = 0; // using 0 will send the entire header, comments and all

    lp = line;
    while((hyattFileBufferLen = FS_Read(fp,&hyattFileBuffer,FILEBUFFERLEN))) {
        for (uint16_t i = 0; i < hyattFileBufferLen; i++) {
            c = hyattFileBuffer[i];
            if (c == '\r') continue;
            if (c > 96 && c < 123) {
                c |= 0x20; // uppercase
            }
           if (c == '\n') {
                *lp = 0;
                if (line[0] == '(' && line[strlen(line)-1] == ')') {
                    line[strlen(line)-1] = 0;
                    strncpy(selections[selectionIndex],&line[1],CONTROLPANEL_SELECTIONWIDTH-1);
                    if (selectionIndex >= CONTROLPANEL_SELECTIONCOUNTMAX) break;
                    fileOpSeeks[selectionIndex++] = FS_GetFilePos(fp) - hyattFileBufferLen + i;
                }
                lp = line;
            } else {
                *lp++ = c;
            }
        }
        if (selectionIndex >= CONTROLPANEL_SELECTIONCOUNTMAX) break;
    }

    FS_FClose(fp);
    FS_Unmount("");
}


void hyattFileScan(char *fn) {
    FS_FILE *fp;
    char c,word[30];
    char *wp;
    float v;

    fileXMin = 100000;
    fileYMin = 100000;
    fileXMax = -100000;
    fileYMax = -100000;

    FS_Mount("");
    fp = FS_FOpen(fn, "r");
    fileSize = FS_GetFileSize(fp);
    fileSeeksSet(fileSize);
    findPattern(fp,'\n','\n'); // scan past header

    wp = word;
    while((hyattFileBufferLen = FS_Read(fp,&hyattFileBuffer,FILEBUFFERLEN))) {
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
        sprintf(lastBlock,"%.3f,%.3f     %.3f,%.3f",fileXMin,fileYMin,fileXMax,fileYMax);
        hyattZDisplaySet("st",lastBlock);
    }

    FS_FClose(fp);
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
                if (serial_get_rx_buffer_available() < 5 ) break; // parser flow control
                hyattFileBufferLen--;
                c = *hyattFileBufferPtr++;
                if ((fileIndex >= fileStart ) && (fileIndex <= fileEnd)) {
                    rx_handler(c);
                }
                fileIndex++;
                
                if (c == '\n') {
                    break;
                 }
            }
            if (hyattFileBufferLen == 0) { // sent all buffer, read next file chunk
                hyattFileBufferLen = FS_Read(file,&hyattFileBuffer,FILEBUFFERLEN);
                if (hyattFileBufferLen == 0) { // no more data in file
                    FS_FClose(file);
                    FS_Unmount("");
                    hyattFileSenderState = FILESENDERSTATE_WAIT;
                    hyattControlPanelState = CONTROLPANEL_SELECT_FILE_OPERATION_SELECT;
                    return;
                }
                hyattFileBufferPtr = &hyattFileBuffer[0];
            }
            break;
        case FILESENDERSTATE_WAIT:
            // file completely sent, just waiting for planning buffer to complete
            if (plan_get_current_block() == NULL) {
                hyattFileSenderState = FILESENDERSTATE_IDLE;
                hyattControlPanelDisplayFileDisplay();
            }
            break;
                
   }
}

void hyattFileSend(char *filename, uint32_t s, uint32_t e) {
    fileStart = s;
    fileEnd   = e;
    fileIndex = 0;
    FS_Mount("");
    file = FS_FOpen(filename, "r");
    if (file) {
        hyattFileBufferLen = FS_Read(file,&hyattFileBuffer,FILEBUFFERLEN);
        hyattFileBufferPtr = hyattFileBuffer;
        hyattFileSenderState = FILESENDERSTATE_SEND;
    }
}


/*
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
*/

