#include "project.h"
#include "hyatt.h"

#define STATE_CHANGE_INTERVAL 500
uint32_t timeoutStateUpdate;

bool hyattControlDisplayExists;
bool hyattControlKeypadExists;


void hyattControlPanelInit() {
    uint8_t error;

    error = I2C_MasterSendStart(DISPLAY2004_ADDR,0);
    hyattControlDisplayExists = (error == I2C_MSTR_NO_ERROR);
    I2C_MasterSendStop();
    
    if (hyattControlDisplayExists) {   
        hyattControlPanelDisplayInit();
        LCD_SetCursor(0,0);     LCD_PutString("Display");
        CyDelay(100);
    }

    error = I2C_MasterSendStart(KEYPAD_ROW12_ADDR,0);
    hyattControlKeypadExists = (error == I2C_MSTR_NO_ERROR);
    I2C_MasterSendStop();
    error = I2C_MasterSendStart(KEYPAD_ROW34_ADDR,0);
    hyattControlKeypadExists &= (error == I2C_MSTR_NO_ERROR);
    I2C_MasterSendStop();
    
    if (hyattControlKeypadExists) {   
        hyattControlPanelKeypadInit();
    }

    if (hyattControlDisplayExists) {   
        LCD_SetCursor(0,1);     LCD_PutString("Keypad");
        CyDelay(100);
    }
    
    hyattControlPanelFeedOverrideInit();
    if (hyattControlDisplayExists) {   
        LCD_SetCursor(0,2);     LCD_PutString("FeedOverride");
        CyDelay(100);
    }
    
    hyattControlPanelWheelInit();
    if (hyattControlDisplayExists) {   
        LCD_SetCursor(0,3);     LCD_PutString("Jog Wheel");
        CyDelay(100);
    }
    hyattControlPanelIRInit();
    if (hyattControlDisplayExists) {   
        LCD_SetCursor(20,0);     LCD_PutString("Infrared");
    }
    CyDelay(1000);

    timeoutStateUpdate = 0;
    hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
}

void hyattControlPanelLoop() {
    if (hyattControlKeypadExists) {   
        hyattControlPanelKeypadLoop();
    }
    
    if (hyattControlDisplayExists) {   
        hyattControlPanelDisplayLoop();
    }
    
    hyattControlPanelFeedOverrideLoop();
    hyattControlPanelIRLoop();

    if ((sys.state == STATE_IDLE) | (sys.state == STATE_JOG)) {  // can't just OR these STATE_IDLE = 0
        if (hyattControlPanelState == CONTROLPANEL_IDLE) hyattControlPanelWheelLoop();
    }

    if (hyattTicks > timeoutStateUpdate) {
        timeoutStateUpdate = hyattTicks + STATE_CHANGE_INTERVAL;
    }
}
