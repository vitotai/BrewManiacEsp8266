/************************************************************
 * LiquidCrystal_I2C
 *
 * The LiquidCrystal_I2C library is a modified version of the standard 
 * LiquidCrystal library as found on the Arduino website.
 * This library is intended to be used when a parallel HD44780 compatible 
 * LCD is controlled over I2C using a PCF8574 extender.
 *
 * url: http://www.xs4all.nl/~hmario/arduino/LiquidCrystal_I2C/LiquidCrystal_I2C.zip
 * licence: unkown (same as Arduino LiquidCrystal library)
 *
 * Dec 2011	V2.0 by Mario H.
 * Jan 2012	adapted to AVR-Net-IO by M.Maassen <mic.maassen@gmail.com>
 * Aug 2013 adapted to many other Adapters by Sebastian Löser <i2clcd@dino0815.de>
 ************************************************************/
#ifndef LiquidCrystal_I2C_h
#define LiquidCrystal_I2C_h

#include <inttypes.h>
#include "Print.h" 
//#include <Wire.h>

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

#define HIGHNIB 1 // LCD-D4 -> PCF8574-P4, LCD-D5 -> PCF8574-P5, LCD-D6 -> PCF8574-P6, LCD-D7 -> PCF8574-P7
#define LOWNIB  2 // LCD-D4 -> PCF8574-P0, LCD-D5 -> PCF8574-P1, LCD-D6 -> PCF8574-P2, LCD-D7 -> PCF8574-P3
#define BUGNIB  3 // LCD-D4 -> PCF8574-P7, LCD-D5 -> PCF8574-P6, LCD-D6 -> PCF8574-P5, LCD-D7 -> PCF8574-P4

#define POLLIN   4 // www.pollin.de
#define RASP     4 // own adapter
#define BUGRASP  5 // own adapter
#define SYDZ     6 // ??? ebay
#define YWROBOT  6 // ??? ebay
#define MJKDZ    7 // www.mjkdz.com

class LiquidCrystal_I2C : public Print {
public:
  LiquidCrystal_I2C(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows, uint8_t displayadapter=POLLIN);
//  LiquidCrystal_I2C(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows);
//  LiquidCrystal_I2C(uint8_t lcd_Addr, uint8_t E,        uint8_t RS,       uint8_t RW,             uint8_t BL=0);
  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS );
//  void begin2(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS );
//  void begin3(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS );
  void clear();
  void home();
  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void printLeft();
  void printRight();
  void leftToRight();
  void rightToLeft();
  void shiftIncrement();
  void shiftDecrement();
  void noBacklight();
  void backlight();
  void autoscroll();
  void noAutoscroll(); 
  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t); 
  virtual size_t write(uint8_t);
  void command(uint8_t);
  void init();

////compatibility API function aliases
void blink_on();					      	// alias for blink()
void blink_off();       					// alias for noBlink()
void cursor_on();      	 					// alias for cursor()
void cursor_off();      					// alias for noCursor()
void setBacklight(uint8_t new_val);				// alias for backlight() and nobacklight()
void load_custom_character(uint8_t char_num, uint8_t *rows);	// alias for createChar()
void printstr(const char[]);

////Unsupported API functions (not implemented in this library)
uint8_t status();
void setContrast(uint8_t new_val);
uint8_t keypad();
void setDelay(int,int);
void on();
void off();
uint8_t init_bargraph(uint8_t graphtype);
void draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end);
void draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len,  uint8_t pixel_col_end);
void updateAddress(uint8_t lcd_addr){_Addr = lcd_addr; }	 

private:
  void init_priv();
  void send(uint8_t, uint8_t);
  void write4bits(uint8_t);
  void expanderWrite(uint8_t);
  void pulseEnable(uint8_t);
  uint8_t _Addr;
  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;
  uint8_t _numlines;
  uint8_t _cols;
  //uint8_t _rows;
  uint8_t _data_nib;
  uint8_t _backlightval;
  uint8_t _En;  // Enable bit
  uint8_t _Rw;  // Read/Write bit
  uint8_t _Rs;  // Register select bit
  uint8_t _Bl;  // Backlight control
  uint8_t _noBl;  // Backlight control
};

#endif
