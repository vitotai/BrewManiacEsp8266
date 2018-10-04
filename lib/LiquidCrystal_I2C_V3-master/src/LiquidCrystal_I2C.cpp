#include "LiquidCrystal_I2C.h"
#include <inttypes.h>
#include <Wire.h>
#include <Arduino.h> // Arduino =< 1.0

LiquidCrystal_I2C::LiquidCrystal_I2C(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows, uint8_t displayadapter){
	_Addr = lcd_Addr;
	_cols = lcd_cols;
	_numlines = lcd_rows;

  if(displayadapter == POLLIN){  // or RASP
		_data_nib = LOWNIB;
		_Rs   = B00010000; // Register select bit
		_Rw   = B00100000; // Read/Write bit
		_En   = B01000000; // Enable bit
		_Bl   = B00000000; // Backlight on 
		_noBl = B10000000; // Backlight off
		_backlightval = _Bl;
  }else if(displayadapter == MJKDZ ){
		_data_nib = LOWNIB;
		_Rs   = B01000000; // Register select bit
		_Rw   = B00100000; // Read/Write bit
		_En   = B00010000; // Enable bit
		_Bl   = B00000000; // Backlight on 
		_noBl = B10000000; // Backlight off
		_backlightval = _Bl;
  }else if(displayadapter == SYDZ){ // or YWROBOT
		_data_nib = HIGHNIB;
		_Rs   = B00000001; // Register select bit
		_Rw   = B00000010; // Read/Write bit
		_En   = B00000100; // Enable bit
		_Bl   = B00001000; // Backlight on
		_noBl = B00000000; // Backlight off
		_backlightval = _Bl;
  }else if(displayadapter == BUGRASP){
		_data_nib = BUGNIB;
		_Rs = B00000100; // Register select bit
		_Rw = B00000010; // Read/Write bit
		_En = B00000001; // Enable bit
		_Bl = B00001000; // Backlight on 
		_noBl = B00000000; // Backlight off
		_backlightval = _Bl;
  }//end if
}
 
void LiquidCrystal_I2C::init(){
	init_priv();
}

void LiquidCrystal_I2C::init_priv(){
	Wire.begin();
	_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
	if (_numlines)
	  begin(_cols, _numlines);  
	else
	  begin(16, 1);  
}

void LiquidCrystal_I2C::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
	if(lines > 1){_displayfunction |= LCD_2LINE;}
	_numlines = lines;
	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	delay(50); 
	expanderWrite(_backlightval);
	delay(1000);
	if(_data_nib == LOWNIB){
		write4bits(0x03);
		delay(5);         // wait min 4.1ms
	   	write4bits(0x03); // second try
		delay(5);         // wait min 4.1ms
		write4bits(0x03); // third go!
		delay(1);
	   	write4bits(0x02); // finally, set to 4-bit interface
	}else{
		write4bits(0x30);
		delay(5);         // wait min 4.1ms
	   	write4bits(0x30); // second try
		delay(5);         // wait min 4.1ms
		write4bits(0x30); // third go!
		delay(1);
	   	write4bits(0x20); // finally, set to 4-bit interface
	}
	// set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);  	
	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF; 	
	display();
	// clear it off
	clear();	
	// Initialize to default text direction (for roman languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT; 	
	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);	
	home();  
}
/*
void LiquidCrystal_I2C::begin3(uint8_t cols, uint8_t lines, uint8_t dotsize){
	if(lines > 1) {
		_displayfunction |= LCD_2LINE;
	}
	_numlines = lines;

	// for some 1 line displays you can select a 10 pixel high font
	if ((dotsize != 0) && (lines == 1)) {
		_displayfunction |= LCD_5x10DOTS;
	}

	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	delay(50); 
  
	// Now we pull both RS and R/W low to begin commands
	expanderWrite(_backlightval);	// reset expanderand turn backlight off (Bit 8 =1)
	delay(1000);

  	//put the LCD into 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46
	
	  // we start in 8bit mode, try to set 4 bit mode
   write4bits(0x03 << 4);
   delayMicroseconds(4500); // wait min 4.1ms
   
   // second try
   write4bits(0x03 << 4);
   delayMicroseconds(4500); // wait min 4.1ms
   
   // third go!
   write4bits(0x03 << 4); 
   delayMicroseconds(150);
   
   // finally, set to 4-bit interface
   write4bits(0x02 << 4); 


	// set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);  
	
	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();
	
	// clear it off
	clear();
	
	// Initialize to default text direction (for roman languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	
	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);
	
	home();
  
}

void LiquidCrystal_I2C::begin2(uint8_t cols, uint8_t lines, uint8_t dotsize) {
	if (lines > 1) {
		_displayfunction |= LCD_2LINE;
	}
	_numlines = lines;

	delayMicroseconds(50000); 
  
	// we start in 8bit mode, try to set 4 bit mode
	write4bits(0x03);
	delayMicroseconds(4500); // wait min 4.1ms
	
	// second try
	write4bits(0x03);
	delayMicroseconds(4500); // wait min 4.1ms
	
	// third go!
	write4bits(0x03); 
	delayMicroseconds(150);
	
	// finally, set to 4-bit interface
	write4bits(0x02); 


	// set # lines, font size, etc.
	command(LCD_FUNCTIONSET | _displayfunction);  
	
	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();
	
	// clear it off
	clear();
	
	// Initialize to default text direction (for roman languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	
	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);
	home();
}
*/
/*******************************************************/
/********** high level commands, for the user! */
void LiquidCrystal_I2C::clear(){
	command(LCD_CLEARDISPLAY);// clear display, set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

/*******************************************************/
void LiquidCrystal_I2C::home(){
	command(LCD_RETURNHOME);  // set cursor position to zero
	delayMicroseconds(2000);  // this command takes a long time!
}

/*******************************************************/
void LiquidCrystal_I2C::setCursor(uint8_t col, uint8_t row){
	int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
	if ( row > _numlines ) {
		row = _numlines-1;    // we count rows starting w/0
	}
	command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

/*******************************************************/
// Turn the display on/off (quickly)
void LiquidCrystal_I2C::noDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
/*******************************************************/
void LiquidCrystal_I2C::display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

/*******************************************************/
// Turns the underline cursor on/off
void LiquidCrystal_I2C::noCursor() {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
/*******************************************************/
void LiquidCrystal_I2C::cursor() {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
/*******************************************************/
void LiquidCrystal_I2C::noBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
/*******************************************************/
void LiquidCrystal_I2C::blink() {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

/*******************************************************/
// These commands scroll the display without changing the RAM
void LiquidCrystal_I2C::scrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
/*******************************************************/
void LiquidCrystal_I2C::scrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

/*******************************************************/
// This is for text that flows Left to Right
void LiquidCrystal_I2C::leftToRight(void) {
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

/*******************************************************/
// This is for text that flows Right to Left
void LiquidCrystal_I2C::rightToLeft(void) {
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

/*******************************************************/
// This will 'right justify' text from the cursor
void LiquidCrystal_I2C::autoscroll(void) {
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

/*******************************************************/
// This will 'left justify' text from the cursor
void LiquidCrystal_I2C::noAutoscroll(void) {
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

/*******************************************************/
// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal_I2C::createChar(uint8_t location, uint8_t charmap[]) {
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (int i=0; i<8; i++) {
		write(charmap[i]);
	}
}

/*******************************************************/
// Turn the (optional) backlight off/on
void LiquidCrystal_I2C::noBacklight(void) {
	_backlightval = _noBl;
	expanderWrite(0);
}

/*******************************************************/
void LiquidCrystal_I2C::backlight(void) {
	_backlightval = _Bl;
	expanderWrite(0);
}

/*******************************************************/
/*********** mid level commands, for sending data/cmds */
inline void LiquidCrystal_I2C::command(uint8_t value) {
	send(value, 0);
}

/*******************************************************/
inline size_t LiquidCrystal_I2C::write(uint8_t value) {
	send(value, _Rs);
	return 1;
}

/*******************************************************/
/************ low level data pushing commands **********/
// write either command or data
void LiquidCrystal_I2C::send(uint8_t value, uint8_t mode) {
  if((_data_nib == HIGHNIB)||(_data_nib == BUGNIB)){
	uint8_t highnib=value&0xf0;
	uint8_t lownib=(value<<4)&0xf0;
        write4bits((highnib)|mode);
	write4bits((lownib)|mode); 

  }else if(_data_nib == LOWNIB){
		uint8_t highnib=value>>4;
		uint8_t lownib=value & 0x0F;
		write4bits((highnib)|mode);
		write4bits((lownib)|mode);
  }//end if _data_nib
}

/*******************************************************/
void LiquidCrystal_I2C::write4bits(uint8_t value) {
	if(_data_nib == BUGNIB){
		uint8_t temp = 0;
		if(value&0x80){temp=(temp)|(0x10);}
		if(value&0x40){temp=(temp)|(0x20);}
		if(value&0x20){temp=(temp)|(0x40);}
		if(value&0x10){temp=(temp)|(0x80);}
		temp=(temp)|(value&0x0F);
		value = temp;
	}
	expanderWrite(value);
	pulseEnable(value);
}

/*******************************************************/
void LiquidCrystal_I2C::expanderWrite(uint8_t _data){                                        
	Wire.beginTransmission(_Addr);
	Wire.write((int)(_data) | _backlightval);
	Wire.endTransmission();   
}

/*******************************************************/
void LiquidCrystal_I2C::pulseEnable(uint8_t _data){
	expanderWrite(_data | _En);	// En high
	delayMicroseconds(1);		// enable pulse must be >450ns
	delayMicroseconds(10);		// enable pulse must be >450ns

	expanderWrite(_data & ~_En);	// En low
	delayMicroseconds(50);		// commands need > 37us to settle
	delayMicroseconds(150);		// commands need > 37us to settle
} 

/*******************************************************/
// Alias functions
void LiquidCrystal_I2C::cursor_on(){
	cursor();
}

/*******************************************************/
void LiquidCrystal_I2C::cursor_off(){
	noCursor();
}

/*******************************************************/
void LiquidCrystal_I2C::blink_on(){
	blink();
}

/*******************************************************/
void LiquidCrystal_I2C::blink_off(){
	noBlink();
}

/*******************************************************/
void LiquidCrystal_I2C::load_custom_character(uint8_t char_num, uint8_t *rows){
		createChar(char_num, rows);
}

/*******************************************************/
void LiquidCrystal_I2C::setBacklight(uint8_t new_val){
	if(new_val){
		backlight();		// turn backlight on
	}else{
		noBacklight();		// turn backlight off
	}
}

/*******************************************************/
void LiquidCrystal_I2C::printstr(const char c[]){
	//This function is not identical to the function used for "real" I2C displays
	//it's here so the user sketch doesn't have to be changed 
	print(c);
}


/*******************************************************/
// unsupported API functions
void LiquidCrystal_I2C::off(){}
void LiquidCrystal_I2C::on(){}
void LiquidCrystal_I2C::setDelay (int cmdDelay,int charDelay) {}
uint8_t LiquidCrystal_I2C::status(){return 0;}
uint8_t LiquidCrystal_I2C::keypad (){return 0;}
uint8_t LiquidCrystal_I2C::init_bargraph(uint8_t graphtype){return 0;}
void LiquidCrystal_I2C::draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end){}
void LiquidCrystal_I2C::draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_row_end){}
void LiquidCrystal_I2C::setContrast(uint8_t new_val){}

	
