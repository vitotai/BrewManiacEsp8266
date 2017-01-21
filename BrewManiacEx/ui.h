/**********************************************************************
 BrewManiacEx
 created by Vito Tai
 Copyright (C) 2015 Vito Tai
 
 This soft ware is provided as-is. Use at your own risks.
 You are free to modify and distribute this software without removing 
 this statement.
 BrewManiac by Vito Tai is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
***********************************************************************/

#ifndef UI_H
#define UI_H
#define INVALID_TEMP_C -127
#define IS_TEMP_INVALID(t) ((t)<-50.0)

#include "pins.h"
#include <Wire.h> 

#if LCD_USE_SSD1306 == true

#include "SSD1306.h"
#include "font_cousine_10.h"
SSD1306  display(0x3C,I2C_SDA,I2C_SCL);

#else

#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20,4,YWROBOT);

#endif

/**********************************************************************
Constant definition
***********************************************************************/

#define LcdCharDegree ((byte)0)
#define LcdCharSetpoint 1
#define LcdCharPump 2
#define LcdCharRevPump 3
#define LcdCharHeating 4
#define LcdCharRevHeating 5
#define LcdCharRevSpargeHeating 6

#define LcdCharReserved 7


#define LCD_COLUMN_NUM 20

#define MashOutStage 7
#define BoilingStage 8
#define CoolingStage 9
#define WhirlpoolStage 10

#define LeftAligned 0
#define RightAligned 1
#define CenterAligned 2

#define WiStateNotConnected 0
#define WiStateConnected 1
#define WiStateAccessPoint 2

#define PumpStatus_On 1
#define PumpStatus_On_PROGRAM_OFF 2
#define PumpStatus_Off 0

#define HeatingStatus_On 1
#define HeatingStatus_On_PROGRAM_OFF 2
#define HeatingStatus_Off 0

#define HeatingSymbolRow 1
#define HeatingSymbolCol 0

#define PumpSymbolRow 1
#define PumpSymbolCol 19

#define AuxHeatingSymbolRow 2
#define AuxHeatingSymbolCol 0

void uiLcdPrint(byte col,byte row,char* str);

void uiLcdPrint_P(byte col,byte row,const char* str);

void uiLcdClear(byte col,byte row,byte space);

void uiLcdClearAll(void);

void uiLcdDrawSymbol(byte col,byte row,byte sid);

void uiLcdDrawSymbolBmp(byte col,byte row,const byte* symbol);

void uiLcdLine(byte col,byte row,byte num);

void uiLcdInitialize(void);
void uiLcdAssignSymbol(byte sid,const byte* symbol);

#include "uiTempTime.h"
/**********************************************************************
variables
***********************************************************************/

byte _uibuffer[21];
byte ipAddress[4];
bool ipSet;


#if LCD_USE_SSD1306 == true

/**********************************************************************
LCD "driver"
***********************************************************************/

const byte WirelessSymbol[] PROGMEM ={0x00,0x0E,0x11,0x04,0x12,0x00,0x04,0x00,0x00,0x00,0x00,0x00};

const byte CelsiusSymbol[] PROGMEM = { 0x00, 0x02, 0x05, 0x02, 0x00, 0x3c, 0x26, 0x02, 0x26, 0x3c, 0x00, 0x00};
const byte FahrenheitSymbol[] PROGMEM = {0x00, 0x02, 0x05, 0x02, 0x00, 0x1c, 0x04, 0x1c, 0x04, 0x04, 0x00, 0x00};
const byte RevHeatingSymbol[] PROGMEM = { 0x3f, 0x3f, 0x3f, 0x3f, 0x2d, 0x2d, 0x21, 0x2d, 0x2d, 0x2d, 0x3f, 0x3f};
const byte HeatingSymbol[] PROGMEM = { 0x00, 0x00, 0x00, 0x00, 0x12, 0x12, 0x1e, 0x12, 0x12, 0x12, 0x00, 0x00};
const byte SetpointSymbol[] PROGMEM = {0x07, 0x01, 0x07, 0x04, 0x07, 0x00, 0x3c, 0x24, 0x24, 0x3c, 0x04, 0x04};
const byte RevPumpSymbol[] PROGMEM = {0x3f, 0x3f, 0x3f, 0x21, 0x2d, 0x21, 0x3d, 0x3d, 0x3d, 0x3f, 0x3f, 0x3f};
const byte PumpSymbol[] PROGMEM = {0x00, 0x00, 0x00, 0x1e, 0x12, 0x1e, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00};

#if SpargeHeaterSupport == true
const byte RevSpargeHeatingSymbol[] PROGMEM = { 0x3f, 0x3f, 0x33, 0x2d, 0x3d, 0x3b, 0x37, 0x2f, 0x2d, 0x33, 0x3f, 0x3f};
const byte SpargeHeatingSymbol[] PROGMEM =    {0x00, 0x00, 0x0c, 0x12, 0x02, 0x04, 0x08, 0x10, 0x12, 0x0c, 0x00, 0x00};

#endif

#define TOP_MARGIN (4)
#define LEFT_MARGIN 4
#define FontWidth 6
#define FontHeight 12
#define LineHeight 14
#define XofCol(col) (LEFT_MARGIN + FontWidth * col)
#define YofRow(row) (TOP_MARGIN + FontHeight * row)

void uiLcdPrint(byte col,byte row,char* str)
{
	int x=XofCol(col);
	int y=YofRow(row);
	byte len=strlen(str);
    display.setColor(BLACK);
    display.fillRect(x,y,FontWidth * len,FontHeight);

	display.setColor(WHITE);
	display.drawString(x,y,str);
	display.display();
}

void uiLcdPrint_P(byte col,byte row,const char* str)
{
	char buffer[21];
	strcpy_P(buffer,str);
	uiLcdPrint(col,row,buffer);
}

void uiLcdClear(byte col,byte row,byte space)
{
    display.setColor(BLACK);
    display.fillRect(XofCol(col),YofRow(row),FontWidth * space,FontHeight);
    display.display();
}

void uiLcdClearAll(void)
{
    display.clear();
}

const byte* symbols[]={CelsiusSymbol,SetpointSymbol,PumpSymbol,RevPumpSymbol,HeatingSymbol,RevHeatingSymbol,RevSpargeHeatingSymbol};

#define LcdCharSetpoint 1
#define LcdCharPump 2
#define LcdCharRevPump 3
#define LcdCharHeating 4
#define LcdCharRevHeating 5
//#define LcdCharWireless 6
//#define LcdCharReserved 7

void uiLcdAssignSymbol(byte sid,const byte* symbol)
{
	symbols[sid]=symbol;
}
void uiLcdDrawSymbol(byte col,byte row,byte sid)
{
	if(sid<sizeof(symbols)/sizeof(char*)){
		int x=XofCol(col);
		int y=YofRow(row);
		
	    display.setColor(BLACK);
    	display.fillRect(x,y,FontWidth,FontHeight);

		display.setColor(WHITE);
		display.drawXbm(x,y,FontWidth,FontHeight,(const char*)symbols[sid]);
	}else{
	}
	display.display();
}

void uiLcdDrawSymbolBmp(byte col,byte row,const byte* symbol)
{
	int x=XofCol(col);
	int y=YofRow(row);
		
	display.setColor(BLACK);
    display.fillRect(x,y,FontWidth,FontHeight);

	display.setColor(WHITE);
	display.drawXbm(XofCol(col),YofRow(row),FontWidth,FontHeight,(const char*)symbol);
	display.display();
}


void uiLcdLine(byte col,byte row,byte num)
{
	
}

void uiLcdInitialize(void)
{
	display.init();
	display.flipScreenVertically();
    display.clear();
    display.display();

    display.setFont(Cousine_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setContrast(255);
    if(gIsUseFahrenheit) uiLcdAssignSymbol(LcdCharDegree,FahrenheitSymbol);
}

#else //#if LCD_USE_SSD1306 == true
/**********************************************************************
 2004 LCD
***********************************************************************/

/**********************************************************************
"bitmap"s
***********************************************************************/


const byte WirelessSymbol[8] PROGMEM =  {B00000,B01110,B10001,B00100,B01010,B00000,B00100,B00000};
const byte WirelessAPSymbol[8] PROGMEM ={B00000,B10001,B01010,B10101,B00100,B00100,B00100,B01110};

	

const byte CelsiusSymbol[8]  PROGMEM  = {B01000, B10100, B01000, B00111, B01000, B01000, B01000, B00111};  // [0] degree c sybmol 
const byte FahrenheitSymbol[8] PROGMEM = {B01000, B10100, B01000, B00111, B00100, B00110, B00100, B00100};  // [0] degree f symbol

const byte SetpointSymbol[8]  PROGMEM  = {B11100, B10000, B11100, B00111, B11101, B00111, B00100, B00100};  // [2] SP Symbol

const byte PumpSymbol[8]  PROGMEM  = {B00000, B01110, B01010, B01110, B01000, B01000, B01000, B00000};  // [3] Pump Symbol 
const byte RevPumpSymbol[8] PROGMEM = {B11111, B10001, B10101, B10001, B10111, B10111, B10111, B11111};  // [4] Reverse PUMP Symbol
const byte HeatingSymbol[8] PROGMEM   = {	B00000, B01010, B01010, B01110, B01110, B01010, B01010, B00000};  // [5] HEAT symbol
const byte RevHeatingSymbol[8] PROGMEM = {B11111, B10101, B10101, B10001, B10001, B10101, B10101, B11111};  // [6] reverse HEAT symbol
const byte RevSpargeHeatingSymbol[8] PROGMEM={B11111,B10001,B1111,B10001,B11110,B1110,B10001,B11111};


#define CreatecCustomChar(buff,idx,bm) uiGetBitmap((byte*)buff,bm); lcd.createChar(idx,(byte*)buff)

void uiGetBitmap(byte *dst,const byte *addr)
{	
	for (int i=0; i<8; i++)
    {
      dst[i] =	pgm_read_byte_near(addr + i);
    }
}


void uiLcdAssignSymbol(byte sid,const byte* symbol)
{
	char buffer[10];
	CreatecCustomChar(buffer,sid,symbol);
}

void uiScanLcdAddress(void)
{
	// wire.begin() should be called at PCF8574F constructor.
	Wire.begin();

	for(byte address = 0; address <= 127; address++ )
 	{
 		if(address == PCF8574_ADDRESS) continue;
 		Wire.beginTransmission(address);
		byte error = Wire.endTransmission();
 		if (error == 0)
 		{
 			lcd.updateAddress(address);
 			break;
		}
	}
}


void uiLcdPrint(byte col,byte row,char* str)
{
	lcd.setCursor(col,row);
	lcd.print(str);
}

void uiLcdPrint_P(byte col,byte row,const char* str)
{
	char buffer[21];
	strcpy_P(buffer,str);
	uiLcdPrint(col,row,buffer);
}

void uiLcdClear(byte col,byte row,byte space)
{
	lcd.setCursor(col,row);
	for(byte i=0;i<space;i++)
		lcd.write(' ');
}

void uiLcdClearAll(void)
{
	lcd.clear();
}

void uiLcdDrawSymbol(byte col,byte row,byte sid)
{
	lcd.setCursor(col,row);
	lcd.write((char)sid);
}

void uiLcdDrawSymbolBmp(byte col,byte row,const byte* symbol)
{
	char buffer[12];
   	CreatecCustomChar(buffer,LcdCharReserved,symbol);
	lcd.setCursor(col,row);
	lcd.write(LcdCharReserved);
}

void uiLcdLine(byte col,byte row,byte num)
{
	lcd.setCursor(col,row);
	for(byte i=0;i<num;i++)
		lcd.write('-');
}

void uiLcdInitialize(void)
{
	char buffer[12];
	uiScanLcdAddress();
	lcd.begin(20,4);
    if(gIsUseFahrenheit)
    {
       	CreatecCustomChar(buffer,LcdCharDegree,FahrenheitSymbol);
    }
   	else 
   	{
   		CreatecCustomChar(buffer,LcdCharDegree,CelsiusSymbol);
   	}
   	
   	CreatecCustomChar(buffer,LcdCharSetpoint,SetpointSymbol);

   	CreatecCustomChar(buffer,LcdCharPump,PumpSymbol);
   	CreatecCustomChar(buffer,LcdCharRevPump,RevPumpSymbol);
   	CreatecCustomChar(buffer,LcdCharHeating,HeatingSymbol);
   	CreatecCustomChar(buffer,LcdCharRevHeating,RevHeatingSymbol);
   	CreatecCustomChar(buffer,LcdCharRevSpargeHeating,RevSpargeHeatingSymbol);
}
#endif //#if LCD_USE_SSD1306 == true

//********************************************************
//* helper functions
//********************************************************

void uiClearRow(byte row)
{	
	byte i=0;
	byte num=20;
	if(row ==0) num=19;
	else if(row ==2) {i=1; num=19;}
	uiLcdClear(i,row,num);
}

void uiShowTextAtRow(byte row,char *text,byte alignment,int indent)
{
	byte length=strlen(text);
	byte col;
	if(alignment ==LeftAligned)
	{
		col=indent;
	}
	else
	{
		
		if(alignment ==RightAligned)
		{
			col=LCD_COLUMN_NUM - indent - length;
		}
		else
		{
			// center, indent is useless
			col=(LCD_COLUMN_NUM - length)/2;
		}
	}
	uiLcdPrint(col,row,text);
}


void uiShowTextAtRow_P(byte row,str_t text,byte alignment,int indent)
{
	char buffer[21];
	strcpy_P(buffer,text);
	uiShowTextAtRow(row,buffer,alignment,indent);
}
//********************************************************
//* LCD interface
//********************************************************

byte _wiStatus;

void uiDisplayWirelessIcon(void)
{
	byte sid;
	if(_wiStatus == WiStateNotConnected)
	{
	   	uiLcdClear(19,0,1);
	}
	else if(_wiStatus == WiStateConnected)
	{
		uiLcdDrawSymbolBmp(19,0,WirelessSymbol);
	}
	else
	{
		uiLcdDrawSymbolBmp(19,0,WirelessAPSymbol);	
	}
}


void uiSetWirelessStatus(byte status)
{
	_wiStatus=status;
	uiDisplayWirelessIcon();

	if(_wiStatus == WiStateNotConnected){
		ipSet = false;
	}

}

void uiClearIpAddress(void)
{
	uiLcdClear(1,2,17);
}

void uiPrintIpAddress(void)
{
	char buffer[21];
	
	if(ipSet){
		strcpy_P(buffer,STR(IpAddress));
		byte idx=strlen(buffer);
		
		for(byte i=0;i<4;i++){
			byte len=sprintInt(buffer + idx,(int)ipAddress[i]);
			idx += len;
			if(i<3) buffer[idx++]='.';
		}
		buffer[idx]='\0';
		uiLcdPrint(1,2,buffer);
		uiLcdClear(idx+1,2,17-idx);
	}
}

void uiSetIp(byte ip[])
{
	if(ip==0 || (ip[0] ==0 && ip[1] ==0 && ip[2] ==0 && ip[3] ==0)){
		ipSet = false;
		return;
	}
	for(byte i=0;i<4;i++)
		ipAddress[i]=ip[i];
	ipSet=true;
}

void uiPrintInitialScreen(void)
{
	uiShowTextAtRow_P(2,STR(Initialization),CenterAligned,1);
	
}

void uiChangeTemperatureUnit(bool useF)
{
    if(useF)
    {
    	uiLcdAssignSymbol(LcdCharDegree,FahrenheitSymbol);
    }
   	else 
   	{
    	uiLcdAssignSymbol(LcdCharDegree,CelsiusSymbol);
   	}
}

#define  uiClearTitle() uiClearRow(0)
#define  uiClearSettingRow() uiClearRow(2)
#define  uiClearSubTitleRow() uiClearRow(1)
#define  uiNoMenu() uiClearRow(3)

#if MaximumNumberOfSensors > 1
#define uiClearPrompt()	uiLcdClear(8,2,11);
#else
#define uiClearPrompt() uiClearRow(2)
#endif

//******************************************************
// General interface
//******************************************************

void uiHeatingStatus(byte status)
{
	if(status==HeatingStatus_On){
		uiLcdDrawSymbol(HeatingSymbolCol,HeatingSymbolRow,LcdCharRevHeating);
	}else if(status==HeatingStatus_On_PROGRAM_OFF){
		uiLcdDrawSymbol(HeatingSymbolCol,HeatingSymbolRow,LcdCharHeating);
	}else{ 
		uiLcdClear(HeatingSymbolCol,HeatingSymbolRow,1);
	}
}

#if SpargeHeaterSupport == true

void uiAuxHeatingStatus(byte status)
{
//	wiReportHeater(status);

	if(status==HeatingStatus_On){
		uiLcdDrawSymbol(AuxHeatingSymbolCol,AuxHeatingSymbolRow,LcdCharRevSpargeHeating);
	}else if(status==HeatingStatus_On_PROGRAM_OFF){
		//uiLcdDrawSymbol(AuxHeatingSymbolCol,AuxHeatingSymbolRow,SpargeHeatingSymbol);
		uiLcdPrint(AuxHeatingSymbolCol,AuxHeatingSymbolRow,"S");
		
	}else{ 
		uiLcdClear(AuxHeatingSymbolCol,AuxHeatingSymbolRow,1);
	}
}

#endif

void uiPumpStatus(byte status)
{
//	wiReportPump(status);

	if(status==PumpStatus_On)
		uiLcdDrawSymbol(PumpSymbolCol,PumpSymbolRow,LcdCharRevPump);
	else if(status==PumpStatus_On_PROGRAM_OFF)
		uiLcdDrawSymbol(PumpSymbolCol,PumpSymbolRow,LcdCharPump);
	else 
		uiLcdClear(PumpSymbolCol,PumpSymbolRow,1);
}

void uiTitle(str_t text)
{
	uiClearTitle();
	uiShowTextAtRow_P(0,text,CenterAligned,0);
}

void uiSubTitle(str_t text)
{
	uiClearRow(1);
  	uiShowTextAtRow_P(1,text,CenterAligned,0);
}

//****************************************
// For setting
// we have to remember the length of

byte _labelLegth;

void uiSettingFieldClear(void)
{
	uiLcdClear(_labelLegth+1,2,LCD_COLUMN_NUM-1 -_labelLegth);
}

void uiSettingTitle(str_t text)
{
	// need to remember the length of text
	// so that when showing parameters, 
	// we know how to clear the LCD line
	_labelLegth = strlen_P(text);
	uiShowTextAtRow_P(2,text,LeftAligned,1);
	uiSettingFieldClear();
}


void uiSettingTitleAppendNumber(byte number) // specially for HOP
{
	char buffer[4];
	buffer[0]='0' + (number/10);
	buffer[1]='0' + (number%10);
	buffer[2]='\0';
	uiLcdPrint(_labelLegth+1,2,buffer);
	
	_labelLegth += 2;
}

// the following code is for show Setting Value
void uiSettingDisplayText(str_t text)
{
//	uiSettingFieldClear();
	uiShowTextAtRow_P(2,text,RightAligned,1);
}

void uiSettingDisplayTextDynamic(char* text)
{
//	uiSettingFieldClear();
	uiShowTextAtRow(2,text,RightAligned,1);
}

// for degree symbol

void uiSettingDegreeSymbol(byte value)
{
//	uiSettingFieldClear();
	const byte *bmp;
	if(value ==0)
	{
		bmp=CelsiusSymbol;
	}
	else
	{
		bmp=FahrenheitSymbol;
	}
	uiLcdDrawSymbolBmp(18,2,bmp);
}


void uiSettingDisplayField(float number,byte precision,char unit)
{
//	uiSettingFieldClear();
	char buffer[20];
	
	int digitNum=sprintFloat((char*)buffer,number,precision);
	buffer[digitNum]=unit;
	buffer[digitNum+1]='\0';
	uiLcdPrint(LCD_COLUMN_NUM -1 -1 - digitNum ,2,buffer);
}


void uiSettingTimeInMinutes(byte minutes)
{
	uiSettingDisplayField((float)minutes,0,'m');
}

void uiSettingShowTemperature(float temp,byte precision)
{
	float displayTemp=temp;
	
	if(IS_TEMP_INVALID(displayTemp)){
		displayTemp = 0;
	}else{
		if(gIsUseFahrenheit){ displayTemp=ConvertC2F(temp); }
   	}
   	
   	uiSettingDisplayField((float)displayTemp,2,0);
    uiLcdDrawSymbol(LCD_COLUMN_NUM -2,2,LcdCharDegree);
}

void uiDisplaySettingPercentage(int number)
{
	uiSettingDisplayField((float)number,0,'%');
}

void uiSettingDisplayNumber(float number,byte precision)
{
	uiSettingDisplayField((float)number,0,' ');
}


void uiClearScreen(void)
{
	uiLcdClearAll();
	uiDisplayWirelessIcon();	
}


#if MaximumNumberOfSensors > 1
void uiSettingSensorIndex(byte num)
{
	char buffer[4];
	buffer[0]='#';
	buffer[1]='1' + num;
	buffer[2]='\0';
	uiLcdPrint(1,2,buffer);
}

#define HexCode(a)  ((a)<10)? ('0'+(a)):('A'+(a)-10)

void uiSettingSensorAddress(byte address[],float temp)
{
	char buffer[20];
 // only print 4 bytes(8 digit hex)
 	byte idx=0;
	for(byte i=4;i<8;i++)
	{
		buffer[idx++]=HexCode(address[i] >> 4);
		buffer[idx++]=HexCode(address[i] & 0xF);
	}
	buffer[idx]='\0';
	uiLcdPrint(4,2,buffer);
	uiLcdClear(12,2,2);
	uiSettingShowTemperature(temp,0);
}

#endif

void wiSendButtonLabel(const byte labelId);

void uiButtonLabel(const byte labelId,const char* text)
{
	uiClearRow(3);
	uiShowTextAtRow_P(3,text,LeftAligned,1);
	wiSendButtonLabel(labelId);
}

void uiDisplaySettingTemperature(float settemp)
{
	char buffer[20];
	float displayTemp;
	if(gIsUseFahrenheit){ displayTemp=ConvertC2F(settemp); }
	else displayTemp = settemp;

    int digitNum=sprintFloat((char*)buffer,displayTemp,2);
    buffer[digitNum]='\0';
    
   	uiLcdClear(12,1,6);
    uiLcdPrint(12+6 - digitNum,1,buffer);
    uiLcdDrawSymbol(18,1,LcdCharSetpoint);
}

void uiClearPwmDisplay(void)
{
	uiLcdClear(1,2,8);
}

void uiShowPwmLabel(void)
{
#if	MaximumNumberOfSensors > 1
	char buf[2];
	buf[0]='%';
	buf[1]='\0';
	uiLcdPrint(1,10,buf);
#else
	uiLcdPrint_P(1,2,STR(PWM_Is));
#endif
}

void uiShowPwmValue(byte pwm)
{
	char buffer[8];
	// make it simple, should optimize later
#if	MaximumNumberOfSensors > 1
	if(pwm==100){
		buffer[0]='H';
		buffer[1]='H';
		buffer[2]='H';
	}else{
		buffer[0]=(pwm/10)? ('0' +(pwm/10)):' ';
		buffer[1]=	'0' + (pwm%10);
		buffer[2]='%';
	}
	buffer[3]='\0';
	uiLcdPrint(8,2,buffer);
#else
	buffer[0]=(pwm==100)? '1':' ';
	buffer[1]=(pwm/10)? ('0' +(pwm/10)):' ';
	buffer[2]=	'0' + (pwm%10);
	buffer[3]='\0';

	uiLcdPrint(5,2,buffer);
#endif	
	
}

 
void  uiPrompt(str_t prompt)
{

	uiClearPrompt();
#if MaximumNumberOfSensors > 1
	uiShowTextAtRow_P(2,prompt,RightAligned,1);
#else
	uiShowTextAtRow_P(2,prompt,CenterAligned,0);
#endif
}

void uiAutoModeTitle(void)
{
	uiLcdPrint_P(1,0,STR(Auto_Label));
}

void uiAutoModeMashTitle(byte idx,byte num)
{
	char buffer[20];

	strcpy_P(buffer,STR(Mash_x));
	byte len=strlen_P(STR(Mash_x));
	buffer[len]='0' +idx;
	buffer[len+1]='/';
	buffer[len+2]='0' +num;
	buffer[len+3]='\0';
	
	uiLcdClear(10,0,9);		
	uiLcdPrint(10,0,buffer);
}


void uiAutoModeStage(byte idx)
{
	const char* str;
	if(idx ==0) str = STR( Mash_In);
	else if(idx ==MashOutStage)  str=STR( Mash_out);
	else if(idx ==BoilingStage) str=STR( Boil);
	else if(idx ==CoolingStage) str=STR( Cooling);
	else if(idx ==WhirlpoolStage) str=STR( Whirlpool);

	uiLcdClear(10,0,9);	
	uiLcdPrint_P(10,0,str);
}

void uiAutoModeShowHopNumber(byte number)
{
	char buffer[20];
	strcpy_P(buffer,STR(Hops_Number_x));
	
	byte len=strlen_P(STR(Hops_Number_x));
	buffer[len]='0' + (number/10);
	buffer[len+1]='0' + (number%10);
	buffer[len+2]='\0';
	uiLcdPrint(10,0,buffer);
}


void uiPreparePasueScreen(str_t message)
{
	uiClearScreen();	

	byte i;
#if MaximumNumberOfSensors > 1
	uiLcdLine(0,0,3);
	uiLcdLine(17,0,3);
#else //#if MaximumNumberOfSensors > 1
	uiLcdLine(0,0,5);
	uiLcdLine(15,0,5);
#endif //#if MaximumNumberOfSensors > 1

	uiLcdLine(1,2,4);
	uiLcdLine(15,2,4);

	uiShowTextAtRow_P(1,message,CenterAligned,0);
	uiTempDisplaySetPosition(TemperaturePausePosition);
	uiRunningTimeSetPosition(RunningTimePausePosition);
}

void uiAutoModeFinishScreen(void)
{
	uiShowTextAtRow_P(1,STR(Brewing_Process),CenterAligned,0);
	uiShowTextAtRow_P(2,STR(Finished),CenterAligned,0);
}

void uiInitialize(void)
{
	// scan addresses before lcd.begin.
	ipSet = false;
	uiRunningTimeStop();
	uiLcdInitialize();
}

#endif
































