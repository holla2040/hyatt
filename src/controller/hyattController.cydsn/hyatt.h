#ifndef hyatt_h
#define hyatt_h

#include <stdint.h>
#include "grbl.h"
    
    
void grblBlockSend(char *block);
char selectedAxisLetter();

#define AXISSELECTED_X      0x0001
#define AXISSELECTED_Y      0x0002
#define AXISSELECTED_Z      0x0004
#define AXISSELECTED_A      0x0008

#define WHEELSTEPSIZE_SMALL  0x0010
#define WHEELSTEPSIZE_MEDIUM 0x0020
#define WHEELSTEPSIZE_LARGE  0x0040

#define KEYPAD_ROW12_ADDR      0x20
#define KEYPAD_ROW34_ADDR      0x21

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

#define KEY_X       0x0001
#define KEY_Y       0x0002
#define KEY_Z       0x0004
#define KEY_SLOW    0x0010
#define KEY_MEDIUM  0x0020
#define KEY_FAST    0x0040
#define KEY_UNIT    0x0400
#define KEY_COORDSELECT 0x0200
#define KEY_AXISZERO 0x1000
#define KEY_SPINDLE 0x2000
#define KEY_MIST    0x4000

uint32_t hyattTicks;
uint8_t hyattAxisSelected;
uint8_t hyattFeedOverride;
uint8_t hyattFeedOverrideButton;
uint8_t hyattFeedOverrideOff;
uint8_t hyattWheelStepSize;
    
void hyattInit();
void hyattLoop();

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
void    i2cBufWrite(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len);

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


uint32_t hyattTimeoutDisplaySlowUpdate;
uint32_t hyattTimeoutDisplayFastUpdate;

void hyattControlPanelDisplayInit();
void hyattControlPanelDisplayLoop();

// feed override
void hyattControlPanelFeedOverrideInit();
void hyattControlPanelFeedOverrideLoop();

// keypad
uint32_t hyattTimeoutKeypadUpdate;

// wheel
void hyattControlPanelWheelInit();
void hyattControlPanelWheelLoop();

#endif
