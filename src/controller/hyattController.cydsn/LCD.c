#include "LCD.h"

//Portado para PSoC por Šarūnas Straigis
//http://www.element14.com/community/people/sarunaszx/blog
//Portado y modificado para PSoC 4 y PSoC 5LP por Brayan Bermudez y Carlos Diaz
//http://digitalprojectsudistrital.blogspot.com.co/


// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LiquidCrystal constructor is called).

uint8_t _addr;
uint8_t _displayfunction;
uint8_t _displaycontrol;
uint8_t _displaymode;
uint8_t _cols;
uint8_t _rows;
uint8_t _charsize;
uint8_t _backlightval;

void LCD_Start(uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows, uint8_t charsize) {
	_addr = lcd_addr;
	_cols = lcd_cols;
	_rows = lcd_rows;
	_charsize = charsize;
	_backlightval = LCD_BACKLIGHT;
    
    LCD_Begin();
}

void LCD_Begin(void){
	_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;

	if (_rows > 1) {
		_displayfunction |= LCD_2LINE;
	}

	// for some 1 line displays you can select a 10 pixel high font
	if ((_charsize != 0) && (_rows == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	CyDelay(50); 

	// Now we pull both RS and R/W low to begin commands
	LCD_ExpanderWrite(_backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	CyDelay(1000);

	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46

	// we start in 8bit mode, try to set 4 bit mode
	LCD_Write4bits(0x03 << 4);
	CyDelayUs(4500); // wait min 4.1ms

	// second try
	LCD_Write4bits(0x03 << 4);
	CyDelayUs(4500); // wait min 4.1ms

	// third go!
	LCD_Write4bits(0x03 << 4); 
	CyDelayUs(150);

	// finally, set to 4-bit interface
	LCD_Write4bits(0x02 << 4); 

	// set # lines, font size, etc.
	LCD_Command(LCD_FUNCTIONSET | _displayfunction);  
	
	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	LCD_Display();
	
	// clear it off
	LCD_Clear();
	
	// Initialize to default text direction (for roman languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	
	// set the entry mode
	LCD_Command(LCD_ENTRYMODESET | _displaymode);
	
	LCD_Home();
}

/********** high level commands, for the user! */
void LCD_Clear(void){
	LCD_Command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	CyDelay(2);  // this command takes a long time!
}

void LCD_Home(void){
    LCD_Command(LCD_RETURNHOME);  // set cursor position to zero
	CyDelay(2);  // this command takes a long time!
}

void LCD_SetCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if (row > _rows) {
		row = _rows-1;    // we count rows starting w/0
	}
	LCD_Command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
void LCD_NoDisplay(void){
    _displaycontrol &= ~LCD_DISPLAYON;
	LCD_Command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_Display(void){
	_displaycontrol |= LCD_DISPLAYON;
	LCD_Command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LCD_NoCursor(void){
    _displaycontrol &= ~LCD_CURSORON;
	LCD_Command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LCD_Cursor(void){
    _displaycontrol |= LCD_CURSORON;
	LCD_Command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LCD_NoBlink(void){
    _displaycontrol &= ~LCD_BLINKON;
	LCD_Command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCD_Blink(void){
	_displaycontrol |= LCD_BLINKON;
	LCD_Command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LCD_ScrollDisplayLeft(void){
	LCD_Command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LCD_ScrollDisplayRight(void){
    LCD_Command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LCD_LeftToRight(void){
	_displaymode |= LCD_ENTRYLEFT;
	LCD_Command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LCD_RightToLeft(void){
	_displaymode &= ~LCD_ENTRYLEFT;
	LCD_Command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LCD_Autoscroll(void){
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
	LCD_Command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LCD_NoAutoscroll(void){
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	LCD_Command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LCD_CreateChar(uint8_t location, uint8_t charmap[]){ uint32_t i = 0;
    location &= 0x7; // we only have 8 locations 0-7
	LCD_Command(LCD_SETCGRAMADDR | (location << 3));
	for (i=0; i<8; i++) {
		LCD_Write(charmap[i]);
	}
}

// Turn the (optional) backlight off/on
void LCD_NoBacklight(void) {
    _backlightval=LCD_NOBACKLIGHT;
	LCD_ExpanderWrite(0);
}

void LCD_Backlight(void){
	_backlightval=LCD_BACKLIGHT;
	LCD_ExpanderWrite(0);
}

/*********** mid level commands, for sending data/cmds */

void LCD_Command(uint8_t value){
    LCD_Send(value, 0);
}

 void LCD_Write(uint8_t value){ 
	LCD_Send(value, Rs);
}


/************ low level data pushing commands **********/

// write either command or data
void LCD_Send(uint8_t value, uint8_t mode){
	uint8_t highnib=value&0xf0;
	uint8_t lownib=(value<<4)&0xf0;
	LCD_Write4bits((highnib)|mode);
	LCD_Write4bits((lownib)|mode); 
}

void LCD_Write4bits(uint8_t value) {
	LCD_ExpanderWrite(value);
	LCD_PulseEnable(value);
}

void LCD_ExpanderWrite(uint8_t _data){     
	LCD_Write_byte(_addr,_data | _backlightval);	   
}

void LCD_PulseEnable(uint8_t _data){
	LCD_ExpanderWrite(_data | En);	// En high
	CyDelayUs(1);		// enable pulse must be >450ns
	LCD_ExpanderWrite(_data & ~En);	// En low
	CyDelayUs(1);		// was 50 commands need > 37us to settle
}

void LCD_Load_custom_character(uint8_t char_num, uint8_t *rows){
    LCD_CreateChar(char_num, rows);
}

void LCD_SetBacklight(uint8_t new_val){   
	if (new_val) {
		LCD_Backlight();		// turn backlight on
	} else {
		LCD_NoBacklight();		// turn backlight off
	}
}

void LCD_PutString(char word[]){ 
    uint32_t i = 0;
    uint8_t size = strlen(word);
    
    for (i = 0; i < size; i++){
        LCD_Write(word[i]);
    }
}

void LCD_Write_byte(uint8_t addr,uint8_t data){ 
    I2C_MasterWriteBuf(addr,&data,1,I2C_MODE_COMPLETE_XFER);
    while ((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT)==0) {};

//    I2C_MasterSendStart(addr, 0);
//    I2C_MasterWriteByte(data);
//    I2C_MasterSendStop();
}

