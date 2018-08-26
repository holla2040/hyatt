
#include <project.h>
#include "string.h"

#ifndef FDB_LIQUID_CRYSTAL_I2C_H
#define FDB_LIQUID_CRYSTAL_I2C_H

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0x04  // Enable bit
#define Rw 0x02  // Read/Write bit
#define Rs 0x01  // Register select bit

/**
 * This is the driver for the Liquid Crystal LCD displays that use the I2C bus.
 *
 * After creating an instance of this class, first call begin() before anything else.
 * The backlight is on by default, since that is the most likely operating mode in
 * most cases.
 */

/*
 * Constructor
 *
 * @param lcd_addr	I2C slave address of the LCD display. Most likely printed on the
 *					LCD circuit board, or look in the supplied LCD documentation.
 * @param lcd_cols	Number of columns your LCD display has.
 * @param lcd_rows	Number of rows your LCD display has.
 * @param charsize	The size in dots that the display has, use LCD_5x10DOTS or LCD_5x8DOTS.
*/
void LCD_Start(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows, uint8_t charsize);
	
void LCD_Write_byte(uint8_t addr, uint8_t data);

/*
 * Set the LCD display in the correct begin state, must be called before anything else is done.
*/
void LCD_Begin(void);
	
/*
 * Remove all the characters currently shown. Next print/write operation will start
 * from the first position on LCD display.
*/
void LCD_Clear(void);
	  
/*
 * Next print/write operation will will start from the first position on the LCD display.
*/
void LCD_Home(void);

/*
 * Do not show any characters on the LCD display. Backlight state will remain unchanged.
 * Also all characters written on the display will return, when the display in enabled again.
*/
void LCD_NoDisplay(void);
	  
/*
 * Show the characters on the LCD display, this is the normal behaviour. This method should
 * only be used after noDisplay() has been used.
*/ 
void LCD_Display(void);

/*
 * Do not blink the cursor indicator.
*/
void LCD_NoBlink(void);
	 
/*
 * Start blinking the cursor indicator.
*/ 
void LCD_Blink(void);	 

/*
 * Do not show a cursor indicator.
*/
void LCD_NoCursor(void);

/*
 * Show a cursor indicator, cursor can blink on not blink. Use the
 * methods blink() and noBlink() for changing cursor blink.*/ 
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

// Compatibility API function aliases
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

#endif  //  FDB_LIQUID_CRYSTAL_I2C_H
