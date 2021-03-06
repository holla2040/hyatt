#ifndef hyatt_h
#define hyatt_h

#include <stdint.h>
#include "grbl.h"
    
// #define NEXTION

float x,y,z;

void grblBlockSend(char *block);
char selectedAxisLetter();

#define BLOCKLEN 128
char lastBlock[BLOCKLEN];

#define INSERTFN "14-20in.nc"

#define AXISSELECTED_X      0x0001
#define AXISSELECTED_Y      0x0002
#define AXISSELECTED_Z      0x0004
#define AXISSELECTED_A      0x0008

#define WHEELSTEPSIZE_SMALL  0x0010
#define WHEELSTEPSIZE_MEDIUM 0x0020
#define WHEELSTEPSIZE_LARGE  0x0040

#define KEYPAD_ROW12_ADDR      0x20
#define KEYPAD_ROW34_ADDR      0x21
#define DISPLAY2004_ADDR       0x27

#define IOA_DIR     0x00
#define IOA_POL     0x02
#define IOA_GPINTEN 0x04
#define IOA_DEFVAL  0x06
#define IOA_INTCON  0x08
#define IOA_CON     0x0A
#define IOA_GPPU    0x0C
#define IOA_INTF    0x0E
#define IOA_INTCAP  0x10
#define IOA_GPIO    0x12
#define IOA_OLAT    0x14

#define IOB_DIR     0x01
#define IOB_POL     0x03
#define IOB_GPINTEN 0x05
#define IOB_DEFVAL  0x07
#define IOB_INTCON  0x09
#define IOB_CON     0x0B
#define IOB_GPPU    0x0D
#define IOB_INTF    0x0F
#define IOB_INTCAP  0x11
#define IOB_GPIO    0x13
#define IOB_OLAT    0x15

#define KEY_X           0x0001
#define KEY_Y           0x0002
#define KEY_Z           0x0004

#define KEY_SLOW        0x0010
#define KEY_MEDIUM      0x0020
#define KEY_FAST        0x0040

#define KEY_SELECT      0x0100
// #define KEY_COORDSELECT 0x0200
#define KEY_ENTER       0x0200
#define KEY_UNIT        0x0400

#define KEY_AXISZERO    0x1000
#define KEY_SPINDLE     0x2000
#define KEY_MIST        0x4000

uint32_t hyattTicks;
uint8_t hyattAxisSelected;
uint8_t hyattFeedOverride;
uint8_t hyattWheelStepSize;
uint8_t feedOverride;
uint8_t enterCount;

void hyattInit();
void hyattLoop();

void axisZero();
void unitToggle();
    
void hyattControlPanelInit();
void hyattControlPanelLoop();

void hyattControlPanelKeypadInit();
void hyattControlPanelKeypadLoop();
void hyattControlPanelKeypadTick();
void hyattControlPanelKeypadindicatorUpdate();

#define AMUX_MOTOR_CURRENT_IN 0x00
#define AMUX_FEED_OVERRIDE_IN 0x01
#define AMUX_VIN_SCALE_IN 0x02

void    i2cInit();
void    i2cRegWrite(uint8_t addr, uint8_t reg, uint8_t value);
uint8_t i2cRegRead(uint8_t addr, uint8_t reg);

// hyattControlPanelLoop
uint8_t hyattControlPanelState;
enum name {
  CONTROLPANEL_IDLE_SETUP,
  CONTROLPANEL_IDLE,
  CONTROLPANEL_SELECT_MACRO_SETUP,
  CONTROLPANEL_SELECT_MACRO,
  CONTROLPANEL_SELECT_FILE_SETUP,
  CONTROLPANEL_SELECT_FILE,
  CONTROLPANEL_SELECT_FILE_ACTION,
  CONTROLPANEL_SELECT_FILE_OPERATION_SETUP,
  CONTROLPANEL_SELECT_FILE_OPERATION_SELECT,
  CONTROLPANEL_SELECT_INSPECT_SETUP,
  CONTROLPANEL_SELECT_INSPECT,
  CONTROLPANEL_SELECT_INSPECT_RESULT,
  CONTROLPANEL_SELECT_FUNCTION_SETUP,
  CONTROLPANEL_SELECT_FUNCTION
};
#define CONTROLPANEL_SELECTIONCOUNTMAX 9
#define CONTROLPANEL_SELECTIONWIDTH 7
#define CONTROLPANEL_LABELWIDTH 7    
int16_t wheel0;
int16_t wheel0Select;

/* LCD begin from babystep/LCD2004 */
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0x04  // Enable bit
#define Rw 0x02  // Read/Write bit
#define Rs 0x01  // Register select bit

void LCD_Start(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows, uint8_t charsize);
void LCD_Write_byte(uint8_t addr, uint8_t data);
void LCD_Begin(void);
void LCD_Clear(void);
void LCD_Home(void);
void LCD_NoDisplay(void);
void LCD_Display(void);
void LCD_NoBlink(void);
void LCD_Blink(void);
void LCD_NoCursor(void);
void LCD_Cursor(void);

void LCD_ScrollDisplayLeft(void);
void LCD_ScrollDisplayRight(void);
void LCD_PrintLeft(void);
void LCD_PrintRight(void);
void LCD_LeftToRight(void);
void LCD_RightToLeft(void);
void LCD_ShiftIncrement(void);
void LCD_ShiftDecrement(void);
void LCD_NoBacklight(void);
void LCD_Backlight(void);
void LCD_Autoscroll(void);
void LCD_NoAutoscroll(void);
void LCD_CreateChar(uint8_t, uint8_t[]);
void LCD_SetCursor(uint8_t, uint8_t);
void LCD_Write(uint8_t);
void LCD_Command(uint8_t);
void LCD_PutString(char word[]);

void LCD_SetBacklight(uint8_t new_val);				// alias for backlight() and nobacklight()
void LCD_Load_custom_character(uint8_t char_num, uint8_t *rows);	// alias for createChar()
void LCD_Send(uint8_t, uint8_t);
void LCD_Write4bits(uint8_t);
void LCD_ExpanderWrite(uint8_t);
void LCD_PulseEnable(uint8_t);

extern uint8_t _addr;
extern uint8_t _displayfunction;
extern uint8_t _displaycontrol;
extern uint8_t _displaymode;
extern uint8_t _cols;
extern uint8_t _rows;
extern uint8_t _charsize;
extern uint8_t _backlightval;

void hyattControlPanelDisplayInit();
void hyattControlPanelDisplayLoop();
void hyattControlPanelDisplayMDIKey(uint16_t key);
void hyattControlPanelDisplayFileDisplay();

// feed override
void hyattControlPanelFeedOverrideInit();
void hyattControlPanelFeedOverrideLoop();

#define FEED_OVERRIDE_BTN 0x01
#define FEED_OVERRIDE_OFF 0x02



// keypad
uint32_t hyattTimeoutKeypadUpdate;

// wheel
void hyattControlPanelWheelInit();
void hyattControlPanelWheelLoop();

// radio
void hyattRadioInit();
uint16_t crcCCITT(char *data, uint16_t length, uint16_t seed);

// display
void selectionsClear();


// select
void hyattSelectInit();
void hyattSelectLoop();

// file
void hyattFileSenderInit();
void hyattFileSenderLoop();
void hyattFileSend(char *filename, uint32_t s, uint32_t e);
void hyattFileCallback(uint8_t status_code);
void hyattFilelistGet();
void hyattFileOperationsGet();
void hyattFileScan(char *fn);
#define FILESENDERSTATE_IDLE 'I'
#define FILESENDERSTATE_SEND 'S'
#define FILESENDERSTATE_WAIT 'W'
uint8_t hyattFileSenderState;
#define FILEBUFFERLEN 512
#define FILENAMEMAX 32
#define OPNAMEMAX   20

// IR
void hyattControlPanelIRInit();
void hyattControlPanelIRLoop();

// probe
uint8_t probeCount;
void hyattProbeInit();
uint8_t hyattProbeRead();
void hyattProbeReset();

// zDisplay
void hyattZDisplayInit();
void hyattZDisplayLoop();
void hyattZDisplaySet(char *attr,char *value);
void hyattZDisplayCommand(char *command);
uint32_t hyattZDisplayUpdate;

// laser

void laserZeroAxisXY();
void laserZeroOffsetSet();


#endif
