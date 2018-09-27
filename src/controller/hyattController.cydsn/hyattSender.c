#include <stdio.h>
#include "project.h"
#include "hyatt.h"
#include <FS.h>

uint8_t senderState;
#define BUFFERLEN 512
uint16_t bufferLen;
char buffer[BUFFERLEN];
char *bufferPtr;
FS_FILE *file;


void hyattSenderInit() {
    senderState = SENDERSTATE_IDLE;
}

void hyattSenderLoop() {
    char c;
    switch (senderState) {
        case SENDERSTATE_READ:
            while(bufferLen) {
                bufferLen--;
                c = *bufferPtr++;
                if (c != '\r') {
                    rx_handler(c);
                    if (c == '\n') {
                       senderState = SENDERSTATE_WAIT;
                       return;
                    }
                }
            }
            if (bufferLen == 0) { // sent all buffer, read next file chunk
                bufferLen = FS_Read(file,&buffer,BUFFERLEN);
                if (bufferLen == 0) { // no more data in file
                    FS_FClose(file);
                    FS_Mount("");
                    senderState = SENDERSTATE_IDLE;
                    return;
                }
                bufferPtr = &buffer[0];
            }
            break;
        case SENDERSTATE_WAIT:
            break;
    }
}

void hyattSenderSend(char *filename) {
    FS_Mount("");
    file = FS_FOpen(filename, "r");
    if (file) {
        bufferLen = FS_Read(file,&buffer,BUFFERLEN);
        bufferPtr = &buffer[0];
        senderState = SENDERSTATE_READ;
    }
}

void hyattSenderCallback(uint8_t status_code) {
    switch (status_code) {
        case STATUS_OK:
            if (senderState == SENDERSTATE_WAIT) {
                senderState = SENDERSTATE_READ;
            }
            break;
        case STATUS_OVERFLOW:
            break;
    }
}