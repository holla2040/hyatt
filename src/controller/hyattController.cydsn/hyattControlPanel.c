#include "project.h"
#include "hyatt.h"

#define STATE_CHANGE_INTERVAL 500
uint32_t timeoutStateUpdate;

bool hyattControlPanelDisplayExists;
bool hyattControlPanelKeypadExists;


void hyattControlPanelInit() {
    uint8_t error;

    error = I2C_MasterSendStart(DISPLAY2004_ADDR,0);
    hyattControlPanelDisplayExists = (error == I2C_MSTR_NO_ERROR);
    I2C_MasterSendStop();
    
    error = I2C_MasterSendStart(KEYPAD_ROW12_ADDR,0);
    hyattControlPanelKeypadExists = (error == I2C_MSTR_NO_ERROR);
    I2C_MasterSendStop();
    error = I2C_MasterSendStart(KEYPAD_ROW34_ADDR,0);
    hyattControlPanelKeypadExists &= (error == I2C_MSTR_NO_ERROR);
    I2C_MasterSendStop();

    // hyattControlPanelKeypadExists  =0;
    //hyattControlPanelDisplayExists = 0;
    
    I2C_Stop();
    I2C_Start();
    
    if (hyattControlPanelDisplayExists) {   
        hyattControlPanelDisplayInit();
        LCD_SetCursor(0,0);     LCD_PutString("Display");
        CyDelay(200);
    }

    if (hyattControlPanelKeypadExists) {   
        hyattControlPanelKeypadInit();
        LCD_SetCursor(0,1);     LCD_PutString("Keypad");
        CyDelay(200);
    }
    
    hyattControlPanelFeedOverrideInit();
    if (hyattControlPanelDisplayExists) {   
        LCD_SetCursor(0,2);     LCD_PutString("FeedOverride");
        CyDelay(200);
    }
    
    hyattControlPanelWheelInit();
    if (hyattControlPanelDisplayExists) {   
        LCD_SetCursor(0,3);     LCD_PutString("Jog Wheel");
        CyDelay(200);
    }
    hyattControlPanelIRInit();
    if (hyattControlPanelDisplayExists) {   
        LCD_SetCursor(10,0);     LCD_PutString("Infrared");
    }
    CyDelay(1000);

    timeoutStateUpdate = 0;
    hyattControlPanelState = CONTROLPANEL_IDLE_SETUP;
    
    enterCount = 0;
}

void hyattControlPanelLoop() {
    if (hyattControlPanelKeypadExists) {
        hyattControlPanelKeypadLoop();
    }
    
    if (hyattControlPanelDisplayExists) {   
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
