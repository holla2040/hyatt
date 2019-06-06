#include <stdio.h>
#include "project.h"
#include "hyatt.h"
#include <FS.h>

uint16_t hyattFileBufferLen;
char hyattFileBuffer[FILEBUFFERLEN];
char *hyattFileBufferPtr;
FS_FILE *file;


float fileXMin,fileXMax,fileYMin,fileYMax;
char filelist[CONTROLPANEL_SELECTIONCOUNTMAX][FILENAMEMAX];
uint8_t fileSelectIndex;
char fileoplist[CONTROLPANEL_SELECTIONCOUNTMAX][OPNAMEMAX];

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

void hyattFilePerimeter(char *fn) {
    // fileXMin,fileXMax,fileYMin,fileYMax;

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

void hyattFileSenderSend(char *filename) {
    FS_Mount("");
    file = FS_FOpen(filename, "r");
    if (file) {
        hyattFileBufferLen = FS_Read(file,&hyattFileBuffer,FILEBUFFERLEN);
        hyattFileBufferPtr = hyattFileBuffer;
        hyattFileSenderState = FILESENDERSTATE_SEND;
    }
}

/*

old stuff
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
        case STATUS_GCODE_UNSUPPORTED_COMMAND:
            if (senderState == SENDERSTATE_WAIT) {
                senderState = SENDERSTATE_READ;
            }
            break;
        case STATUS_OVERFLOW:
        case STATUS_SOFT_LIMIT_ERROR:
            break;
    }
}



status_codes from report.h
        case STATUS_OK:
        case STATUS_EXPECTED_COMMAND_LETTER:
        case STATUS_BAD_NUMBER_FORMAT:
        case STATUS_INVALID_STATEMENT:
        case STATUS_NEGATIVE_VALUE:
        case STATUS_SETTING_DISABLED:
        case STATUS_SETTING_STEP_PULSE_MIN:
        case STATUS_SETTING_READ_FAIL:
        case STATUS_IDLE_ERROR:
        case STATUS_SYSTEM_GC_LOCK:
        case STATUS_SOFT_LIMIT_ERROR:
        case STATUS_OVERFLOW:
        case STATUS_MAX_STEP_RATE_EXCEEDED:
        case STATUS_CHECK_DOOR:
        case STATUS_LINE_LENGTH_EXCEEDED:
        case STATUS_TRAVEL_EXCEEDED:
        case STATUS_INVALID_JOG_COMMAND:
        case STATUS_GCODE_UNSUPPORTED_COMMAND:
        case STATUS_GCODE_MODAL_GROUP_VIOLATION:
        case STATUS_GCODE_UNDEFINED_FEED_RATE:
        case STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER:
        case STATUS_GCODE_AXIS_COMMAND_CONFLICT:
        case STATUS_GCODE_WORD_REPEATED:
        case STATUS_GCODE_NO_AXIS_WORDS:
        case STATUS_GCODE_INVALID_LINE_NUMBER:
        case STATUS_GCODE_VALUE_WORD_MISSING:
        case STATUS_GCODE_UNSUPPORTED_COORD_SYS:
        case STATUS_GCODE_G53_INVALID_MOTION_MODE:
        case STATUS_GCODE_AXIS_WORDS_EXIST:
        case STATUS_GCODE_NO_AXIS_WORDS_IN_PLANE:
        case STATUS_GCODE_INVALID_TARGET:
        case STATUS_GCODE_ARC_RADIUS_ERROR:
        case STATUS_GCODE_NO_OFFSETS_IN_PLANE:
        case STATUS_GCODE_UNUSED_WORDS:
        case STATUS_GCODE_G43_DYNAMIC_AXIS_ERROR:
        case STATUS_GCODE_MAX_VALUE_EXCEEDED:
*/
