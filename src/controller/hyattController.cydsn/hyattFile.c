#include <stdio.h>
#include "project.h"
#include "hyatt.h"
#include <FS.h>


#define SENDERBUFFERLEN 512
uint16_t senderBufferLen;
char senderBuffer[SENDERBUFFERLEN];
char *senderBufferPtr;
FS_FILE *file;

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

/*
void hyattSenderLoop() {
    char c;
    switch (senderState) {
        case SENDERSTATE_SEND:
            while(senderBufferLen) {
                if (serial_get_rx_buffer_available() < 10 ) break; // parser flow control
                senderBufferLen--;
                c = *senderBufferPtr++;
                rx_handler(c);

               // usb_uart_write(c);
                
                if ((c == '\n') || (c == '\r')) {
                    break;
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
*/
