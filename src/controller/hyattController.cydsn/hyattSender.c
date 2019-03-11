#include <stdio.h>
#include "project.h"
#include "hyatt.h"
#include <FS.h>


#define SENDERBUFFERLEN 512
uint16_t senderBufferLen;
char senderBuffer[SENDERBUFFERLEN];
char *senderBufferPtr;
FS_FILE *file;

void hyattSenderInit() {
    senderState = SENDERSTATE_IDLE;
}

void hyattSenderLoop() {
    char c;
    int l;
    switch (senderState) {
        case SENDERSTATE_SEND:
            while(senderBufferLen) {
                l = plan_get_block_buffer_available();
                if (serial_get_rx_buffer_available() < 10 || plan_get_block_buffer_available() < 5) break; // parser flow control
                senderBufferLen--;
                c = *senderBufferPtr++;
                rx_handler(c);

                usb_uart_write(c);
                
                if ((c == '\n') || (c != '\r')) {
                    break; // need to break here so planner will plan and avail will updated
                 }

            }
            if (senderBufferLen == 0) { // sent all buffer, read next file chunk
                senderBufferLen = FS_Read(file,&senderBuffer,SENDERBUFFERLEN);
                if (senderBufferLen == 0) { // no more data in file
                    FS_FClose(file);
                    FS_Unmount("");
                    senderState = SENDERSTATE_IDLE;
                    return;
                }
                senderBufferPtr = &senderBuffer[0];
            }
            break;
    }
}

void hyattSenderSend(char *filename) {
    FS_Mount("");
    file = FS_FOpen(filename, "r");
    if (file) {
        senderBufferLen = FS_Read(file,&senderBuffer,SENDERBUFFERLEN);
        senderBufferPtr = senderBuffer;
        senderState = SENDERSTATE_SEND;
    }
}

/*
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
