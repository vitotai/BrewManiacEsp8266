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

#define UseLcdBuffer true

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



typedef enum _SymbolId{
SymbolCelsius=0,
SymbolSetpoint=1,
SymbolPump=2,
SymbolRevPump=3,
SymbolHeating=4,
SymbolRevHeating=5,
SymbolFahrenheit=6,
SymbolWireless=7,
SymbolWirelessAP=8,
SymbolRevSpargeHeating =9
#if SecondaryHeaterSupport == true
,
SymbolPrimaryHeater=10,
SymbolRevPrimaryHeater=11,
SymbolSecondaryHeater=12,
SymbolRevSecondaryHeater=13
#endif
} SymbolId;



#define LCD_COLUMN_NUM 20

#define MashOutStage 7
#define BoilingStage 8
#define CoolingStage 9
#define WhirlpoolStage 10
#define HopStandChillingStage 11
#define HopStandStage 12

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


void uiLcdLine(byte col,byte row,byte num);

void uiLcdInitialize(void);

void uiLcdAssignSymbol(byte sid,SymbolId symbol);
void uiLcdDrawSymbol(byte col,byte row,byte sid);
void uiLcdDrawSymbolBmp(byte col,byte row,SymbolId symbol);

#include "uiTempTime.h"
/**********************************************************************
variables
***********************************************************************/

byte _uibuffer[21];
byte ipAddress[4];
bool ipSet;

#if UseLcdBuffer
char _lcdBuffer[4][21];
void clearLcdBuffer(void){
	for(int i=0;i<4;i++){
		int j;
		for(j=0;j<20;j++) _lcdBuffer[i][j]=' ';
		_lcdBuffer[i][j]='\0';
	}
}

#endif

#if LCD_USE_SSD1306 == true

/**********************************************************************
LCD "driver"
***********************************************************************/

const byte _WirelessSymbol[] PROGMEM ={0x00,0x0E,0x11,0x04,0x12,0x00,0x04,0x00,0x00,0x00,0x00,0x00};
const byte _WirelessAPSymbol[] PROGMEM ={0x00,0x0E,0x11,0x04,0x12,0x00,0x04,0x00,0x00,0x00,0x00,0x00};

const byte _CelsiusSymbol[] PROGMEM = { 0x00, 0x02, 0x05, 0x02, 0x00, 0x3c, 0x26, 0x02, 0x26, 0x3c, 0x00, 0x00};
const byte _FahrenheitSymbol[] PROGMEM = {0x00, 0x02, 0x05, 0x02, 0x00, 0x1c, 0x04, 0x1c, 0x04, 0x04, 0x00, 0x00};
const byte _RevHeatingSymbol[] PROGMEM = { 0x3f, 0x3f, 0x3f, 0x3f, 0x2d, 0x2d, 0x21, 0x2d, 0x2d, 0x2d, 0x3f, 0x3f};
const byte _HeatingSymbol[] PROGMEM = { 0x00, 0x00, 0x00, 0x00, 0x12, 0x12, 0x1e, 0x12, 0x12, 0x12, 0x00, 0x00};
const byte _SetpointSymbol[] PROGMEM = {0x07, 0x01, 0x07, 0x04, 0x07, 0x00, 0x3c, 0x24, 0x24, 0x3c, 0x04, 0x04};
const byte _RevPumpSymbol[] PROGMEM = {0x3f, 0x3f, 0x3f, 0x21, 0x2d, 0x21, 0x3d, 0x3d, 0x3d, 0x3f, 0x3f, 0x3f};
const byte _PumpSymbol[] PROGMEM = {0x00, 0x00, 0x00, 0x1e, 0x12, 0x1e, 0x02, 0x02, 0x02, 0x00, 0x00, 0x00};

const byte _RevSpargeHeatingSymbol[] PROGMEM = { 0x3f, 0x3f, 0x33, 0x2d, 0x3d, 0x3b, 0x37, 0x2f, 0x2d, 0x33, 0x3f, 0x3f};
//const byte SpargeHeatingSymbol[] PROGMEM =    {0x00, 0x00, 0x0c, 0x12, 0x02, 0x04, 0x08, 0x10, 0x12, 0x0c, 0x00, 0x00};



const byte* SymbolMaps[]={
_CelsiusSymbol, //0
_SetpointSymbol, //1
_PumpSymbol, //2
_RevPumpSymbol, //3
_HeatingSymbol, //4
_RevHeatingSymbol, //5
_FahrenheitSymbol, //6
_WirelessSymbol, //7
_WirelessAPSymbol, //8
_RevSpargeHeatingSymbol // 9
#if SecondaryHeaterSupport == true
,
_PrimaryHeaterSymbol, //10
_RevPrimaryHeaterSymbol, //11
_SecondaryHeaterSymbol, //12
_RevSecondaryHeaterSymbol //13
#endif
};

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

	#if UseLcdBuffer
	char *p=str;
	int j=col;
	while(*p && j < 20){
		_lcdBuffer[row][j]= *p;
		p++;
		j++;
	}
	#endif
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
	#if UseLcdBuffer
	for(int i=0;i<space;i++)
		_lcdBuffer[row][col + i]= ' ';
	#endif

}

void uiLcdClearAll(void)
{
    display.clear();
}


byte _reservedChar2SymbolId[8];

void uiLcdAssignSymbol(byte sid,const SymbolId symbol){
	_reservedChar2SymbolId[sid] = symbol;
}

void uiLcdDrawSymbol(byte col,byte row,byte sid)
{
	#if UseLcdBuffer
	_lcdBuffer[row][col]= _reservedChar2SymbolId[sid];
	#endif

	if(sid<sizeof(SymbolMaps)/sizeof(char*)){
		int x=XofCol(col);
		int y=YofRow(row);

	    display.setColor(BLACK);
    	display.fillRect(x,y,FontWidth,FontHeight);

		display.setColor(WHITE);
		display.drawXbm(x,y,FontWidth,FontHeight,(const char*)SymbolMaps[_reservedChar2SymbolId[sid]]);
	}else{
	}
	display.display();
}
void uiLcdDrawSymbolBmp(byte col,byte row,SymbolId symbol)
{
	#if UseLcdBuffer
	_lcdBuffer[row][col]= symbol;
	#endif

	int x=XofCol(col);
	int y=YofRow(row);

	display.setColor(BLACK);
    display.fillRect(x,y,FontWidth,FontHeight);

	display.setColor(WHITE);
	display.drawXbm(XofCol(col),YofRow(row),FontWidth,FontHeight,(const char*)SymbolMaps[symbol]);
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
}


#if UseLcdBuffer
void refreshLcdDisplay(void){}
#endif

#else //#if LCD_USE_SSD1306 == true
/**********************************************************************
 2004 LCD
***********************************************************************/

/**********************************************************************
"bitmap"s
***********************************************************************/


const byte _CelsiusSymbol[8]  PROGMEM  = {B01000, B10100, B01000, B00111, B01000, B01000, B01000, B00111};  // [0] degree c sybmol
const byte _FahrenheitSymbol[8] PROGMEM = {B01000, B10100, B01000, B00111, B00100, B00110, B00100, B00100};  // [0] degree f symbol

const byte _SetpointSymbol[8]  PROGMEM  = {B11100, B10000, B11100, B00111, B11101, B00111, B00100, B00100};  // [2] SP Symbol

const byte _PumpSymbol[8]  PROGMEM  = {B00000, B01110, B01010, B01110, B01000, B01000, B01000, B00000};  // [3] Pump Symbol
const byte _RevPumpSymbol[8] PROGMEM = {B11111, B10001, B10101, B10001, B10111, B10111, B10111, B11111};  // [4] Reverse PUMP Symbol
const byte _HeatingSymbol[8] PROGMEM   = {	B00000, B01010, B01010, B01110, B01110, B01010, B01010, B00000};  // [5] HEAT symbol
const byte _RevHeatingSymbol[8] PROGMEM = {B11111, B10101, B10101, B10001, B10001, B10101, B10101, B11111};  // [6] reverse HEAT symbol
const byte _RevSpargeHeatingSymbol[8] PROGMEM={B11111,B10001,B1011,B10001,B11110,B1110,B10001,B11111};

const byte _WirelessSymbol[8] PROGMEM =  {B00000,B01110,B10001,B00100,B01010,B00000,B00100,B00000};
const byte _WirelessAPSymbol[8] PROGMEM ={B00000,B10001,B01010,B10101,B00100,B00100,B00100,B01110};


#if SecondaryHeaterSupport == true

//1
const byte _PrimaryHeaterSymbol[8] PROGMEM= {0b00000, 0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b01110, 0b00000};
const byte _RevPrimaryHeaterSymbol[8] PROGMEM = { 0b11111, 0b11011, 0b10011, 0b11011, 0b11011, 0b11011, 0b10001, 0b11111};

//2
const byte _SecondaryHeaterSymbol[8] PROGMEM = { 0b00000, 0b00100, 0b01010, 0b00010, 0b00100, 0b01000, 0b01110, 0b00000 };
const byte _RevSecondaryHeaterSymbol[8]PROGMEM = {0b11111, 0b11011, 0b10101, 0b11101, 0b11011, 0b10111, 0b10001, 0b11111};

#endif


const byte* SymbolMaps[]={
_CelsiusSymbol, //0
_SetpointSymbol, //1
_PumpSymbol, //2
_RevPumpSymbol, //3
_HeatingSymbol, //4
_RevHeatingSymbol, //5
_FahrenheitSymbol, //6
_WirelessSymbol, //7
_WirelessAPSymbol, //8
_RevSpargeHeatingSymbol // 9
#if SecondaryHeaterSupport == true
,
_PrimaryHeaterSymbol, //10
_RevPrimaryHeaterSymbol, //11
_SecondaryHeaterSymbol, //12
_RevSecondaryHeaterSymbol //13
#endif
};




#define CreatecCustomChar(buff,idx,bm) uiGetBitmap((byte*)buff,bm); lcd.createChar(idx,(byte*)buff)

void uiGetBitmap(byte *dst,const byte *addr)
{
	for (int i=0; i<8; i++)
    {
      dst[i] =	pgm_read_byte_near(addr + i);
    }
}

byte _reservedChar2SymbolId[8];

void uiLcdAssignSymbol(byte sid,const SymbolId symbol){
	_reservedChar2SymbolId[sid] = symbol;

	char buffer[12];
	const byte* bitmap=SymbolMaps[symbol];
	CreatecCustomChar(buffer,sid,bitmap);
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
	#if UseLcdBuffer
	char *p=str;
	int j=col;
	while(*p && j < 20){
		lcd.write(*p);
		_lcdBuffer[row][j]= *p;
		p++;
		j++;
	}
	#else
	lcd.print(str);
	#endif
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
	for(byte i=0;i<space;i++){
		lcd.write(' ');
		#if UseLcdBuffer
		_lcdBuffer[row][col + i]= ' ';
		#endif
	}
}

void uiLcdClearAll(void)
{
	lcd.clear();

}

void uiLcdLine(byte col,byte row,byte num)
{
	lcd.setCursor(col,row);
	for(byte i=0;i<num;i++){
		lcd.write('-');
		#if UseLcdBuffer
		_lcdBuffer[row][col + i]= '-';
		#endif
	}
}

void uiLcdDrawSymbol(byte col,byte row,byte sid)
{
	lcd.setCursor(col,row);
	lcd.write((char)sid);
	#if UseLcdBuffer
	_lcdBuffer[row][col]= _reservedChar2SymbolId[sid];
	#endif
}

void uiLcdDrawSymbolBmp(byte col,byte row,SymbolId symbol)
{
	char buffer[12];
	const byte* bitmap= SymbolMaps[symbol];
   	CreatecCustomChar(buffer,LcdCharReserved,bitmap);
	lcd.setCursor(col,row);
	lcd.write(LcdCharReserved);
	
	#if UseLcdBuffer
	_lcdBuffer[row][col]= symbol;
	#endif

}


void uiLcdInitialize(void)
{
	uiScanLcdAddress();
	lcd.begin(20,4);
}

#if UseLcdBuffer
void refreshLcdDisplay(void){
	lcd.begin(20,4);
	lcd.clear();

	for(byte i=0;i<4;i++){
		lcd.setCursor(0,i);
		for(byte j=0;j<20;j++){
			if(_lcdBuffer[i][j] & 0xF0)
				lcd.write(_lcdBuffer[i][j]);
			else{
				//symbol.
				//uiLcdDrawSymbolBmp(j,i,(SymbolId)_lcdBuffer[i][j]);

					char buffer[12];
					const byte* bitmap= SymbolMaps[(int)_lcdBuffer[i][j]];
   					CreatecCustomChar(buffer,LcdCharReserved,bitmap);
					lcd.write(LcdCharReserved);
			}
		}	
	}
}
#endif

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
	//byte sid;
	if(_wiStatus == WiStateNotConnected)
	{
	   	uiLcdClear(19,0,1);
	}
	else if(_wiStatus == WiStateConnected)
	{
		uiLcdDrawSymbolBmp(19,0,SymbolWireless);
	}
	else
	{
		uiLcdDrawSymbolBmp(19,0,SymbolWirelessAP);
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


void uiChangeTemperatureUnit(bool useF)
{
    if(useF)
    {
    	uiLcdAssignSymbol(LcdCharDegree,SymbolFahrenheit);
    }
   	else
   	{
    	uiLcdAssignSymbol(LcdCharDegree,SymbolCelsius);
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

#if EnableExtendedMashStep
#define MashExtensionNone 0
#define MashExtensionEnabled 1
#define MashExtensionRunning 2

void uiSetMashExtensionStatus(uint8_t status)
{
	char buf[2];
	if(status == MashExtensionNone) buf[0]=' ';
	else if(status == MashExtensionEnabled) buf[0]='+';
	else if(status == MashExtensionRunning) buf[0]='*';
	buf[1]='\0';
	uiLcdPrint(18,0,buf);
}
#endif
//******************************************************
// General interface
//******************************************************
#if SecondaryHeaterSupport == true
//   possiblie combination
//    1     2
//   ON     ON   On: Both on
//   ON     OFF  On: Primary
//   OFF    ON   On: Secondary
//   OFF    OFF  OFF
//   SUS    SUS  Suspended, both on
//   SUS    OFF  suspended,
//   OFF    SUS  Suspended
//   other combinations are invalid
void loadHeatingIcons(byte mask)
{
    if(mask == 1){ // primary
   	    uiLcdAssignSymbol(LcdCharHeating,SymbolPrimaryHeater);
   	    uiLcdAssignSymbol(LcdCharRevHeating,SymbolRevPrimaryHeater);
    }else if(mask == 2){ // secondary
   	    uiLcdAssignSymbol(LcdCharHeating,SymbolSecondaryHeater);
   	    uiLcdAssignSymbol(LcdCharRevHeating,SymbolRevSecondaryHeater);
    }else{ // both
   	    uiLcdAssignSymbol(LcdCharHeating,SymbolHeating);
       	uiLcdAssignSymbol(LcdCharRevHeating,SymbolRevHeating);
    }
}

void uiHeatingStatus(byte primary, byte secondary)
{
    // suspended state must be sync to each other

	if(primary==HeatingStatus_Off && secondary == HeatingStatus_Off){
    	uiLcdClear(HeatingSymbolCol,HeatingSymbolRow,1);
	}else{
	    // check if the state is the same as previous.
	    static byte loadedIconMask = 0;
	    byte mask = ((primary != HeatingStatus_Off)? 1:0) | ((secondary != HeatingStatus_Off)? 2:0);

	    if(mask != loadedIconMask){
	        loadHeatingIcons(mask);
	        loadedIconMask=mask;
	    }

	    if(primary==HeatingStatus_On || secondary == HeatingStatus_On){
		    uiLcdDrawSymbol(HeatingSymbolCol,HeatingSymbolRow,LcdCharRevHeating);
		}else{
    		uiLcdDrawSymbol(HeatingSymbolCol,HeatingSymbolRow,LcdCharHeating);
    	}
	}
}

#else
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
#endif

#if SpargeHeaterSupport == true
char const S_STR[] PROGMEM ="S";

void uiAuxHeatingStatus(byte status)
{
//	wiReportHeater(status);

	if(status==HeatingStatus_On){
		uiLcdDrawSymbol(AuxHeatingSymbolCol,AuxHeatingSymbolRow,LcdCharRevSpargeHeating);
	}else if(status==HeatingStatus_On_PROGRAM_OFF){
		//uiLcdDrawSymbol(AuxHeatingSymbolCol,AuxHeatingSymbolRow,SpargeHeatingSymbol);
		uiLcdPrint_P(AuxHeatingSymbolCol,AuxHeatingSymbolRow,S_STR);

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
	SymbolId bmp;
	if(value ==0)
	{
		bmp=SymbolCelsius;
	}
	else
	{
		bmp=SymbolFahrenheit;
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

#if UsePaddleInsteadOfPump || EnableLevelSensor
void uiSettingTimeInSeconds(byte sec)
{
	uiSettingDisplayField((float)sec,0,'s');
}
#endif
void uiSettingTimeInMinutes(byte minutes)
{
	uiSettingDisplayField((float)minutes,0,'m');
}

void uiSettingShowTemperature(float temp,byte precision)
{
	float displayTemp=temp;

	if(IS_TEMP_INVALID(displayTemp)){
		displayTemp = 0;
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
	#if UseLcdBuffer
	clearLcdBuffer();
	#endif

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

#if SupportDistilling
void uiPrintSettingTemperature(float settemp);
float _currentSettingTemperature;

void uiSetSettingTemperature(float settemp)
{
	_currentSettingTemperature=settemp;
	uiPrintSettingTemperature(settemp);
}

void uiHideSettingTemperature(void)
{
	uiLcdClear(12,1,6);
}

void uiPrintSettingTemperature(float settemp)
#else
void uiSetSettingTemperature(float settemp)
#endif
{
	
	char buffer[20];
	float displayTemp;
	displayTemp = settemp;

    int digitNum=sprintFloat((char*)buffer,displayTemp,2);
    buffer[digitNum]='\0';

   	uiLcdClear(12,1,6);
    uiLcdPrint(12+6 - digitNum,1,buffer);
    uiLcdDrawSymbol(18,1,LcdCharSetpoint);
}

void uiClearPwmDisplay(void)
{
#if	MaximumNumberOfSensors > 1
	uiLcdClear(8,2,3);
#else
	uiLcdClear(1,2,8);
#endif
}

void uiShowPwmLabel(void)
{
#if	MaximumNumberOfSensors > 1
	char buf[2];
	buf[0]='%';
	buf[1]='\0';
	uiLcdPrint(10,2,buf);
#else
	uiLcdPrint_P(1,2,STR(PWM_Is));
#endif
}

void uiShowPwmValue(byte pwm)
{
	char buffer[8];
	// make it simple, should optimize later
#if	MaximumNumberOfSensors > 1
	if(pwm>=100){
		buffer[0]='H';
		buffer[1]='H';
	}else{
		buffer[0]=(pwm/10)? ('0' +(pwm/10)):' ';
		buffer[1]=	'0' + (pwm%10);
	}
	buffer[2]='%';

	buffer[3]='\0';
	uiLcdPrint(8,2,buffer);
#else
	if(pwm>=100){
		buffer[0]='1';
		buffer[1]='0';
	}else{
		buffer[0]=' ';
		buffer[1]=(pwm/10)? ('0' +(pwm/10)):' ';
	}
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
	const char* str="";
	if(idx ==0) str = STR( Mash_In);
	else if(idx ==MashOutStage)  str=STR( Mash_out);
	else if(idx ==BoilingStage) str=STR( Boil);
	else if(idx ==CoolingStage) str=STR( Cooling);
	else if(idx ==WhirlpoolStage) str=STR( Whirlpool);
	else if(idx ==HopStandChillingStage) str=STR( HopStandChilling);
	else /*if(idx ==HopStandStage) */str=STR( HopStand);
	
	uiLcdClear(10,0,9);
	uiLcdPrint_P(10,0,str);
}

void uiAutoModeShowGeneralHopNumber(const char* hop,byte number)
{
	char buffer[20];
	strcpy_P(buffer,hop);

	byte len=strlen_P(hop);
	buffer[len]='0' + (number/10);
	buffer[len+1]='0' + (number%10);
	buffer[len+2]='\0';
	uiLcdClear(10,0,9);
	uiLcdPrint(10,0,buffer);
}

void uiAutoModeShowHopNumber(byte number)
{
    uiAutoModeShowGeneralHopNumber(STR(Hops_Number_x),number);
}

void uiAutoModeShowPostBoilHopNumber(byte number)
{
    uiAutoModeShowGeneralHopNumber(STR(Pbh_Number_x),number);
}


void uiPreparePasueScreen(str_t message)
{
	uiClearScreen();

//	byte i;
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

#if SupportDistilling
bool _uiSettingTemperatureBlinking;
bool _uiSettingTemperatureHidden;
unsigned long _settingTempBlinkTime;
#define SettingTempteratureBlinkHideCycle 350
#define SettingTempteratureBlinkShowCycle 650
void uiSettingTemperatureBlinking(bool blink)
{
	_uiSettingTemperatureBlinking = blink;
	if(_uiSettingTemperatureHidden && !_uiSettingTemperatureBlinking){
		_uiSettingTemperatureHidden = false;
		uiPrintSettingTemperature(_currentSettingTemperature);
	}
}
#endif

void uiInitialize(void)
{
	// scan addresses before lcd.begin.
	ipSet = false;
	uiRunningTimeStop();
	uiLcdInitialize();
	#if UseLcdBuffer
	clearLcdBuffer();
	#endif

    if(gIsUseFahrenheit)
    {
       	uiLcdAssignSymbol(LcdCharDegree,SymbolFahrenheit);
    }
   	else
   	{
   		uiLcdAssignSymbol(LcdCharDegree,SymbolCelsius);
   	}

   	uiLcdAssignSymbol(LcdCharSetpoint,SymbolSetpoint);

   	uiLcdAssignSymbol(LcdCharPump,SymbolPump);
   	uiLcdAssignSymbol(LcdCharRevPump,SymbolRevPump);
   	uiLcdAssignSymbol(LcdCharHeating,SymbolHeating);
   	uiLcdAssignSymbol(LcdCharRevHeating,SymbolRevHeating);
   	uiLcdAssignSymbol(LcdCharRevSpargeHeating,SymbolRevSpargeHeating);


	#if SupportDistilling
	_uiSettingTemperatureBlinking=false;
	_uiSettingTemperatureHidden=false;
	#endif
}


void uiLoop(void)
{
	#if SupportDistilling
	if(_uiSettingTemperatureBlinking)
	{
		if(_uiSettingTemperatureHidden)
		{
			if((gCurrentTimeInMS - _settingTempBlinkTime) > SettingTempteratureBlinkHideCycle)
			{
				_settingTempBlinkTime=gCurrentTimeInMS;
				_uiSettingTemperatureHidden = false;
				//show
				uiPrintSettingTemperature(_currentSettingTemperature);
			}
		}
		else
		{
			if((gCurrentTimeInMS - _settingTempBlinkTime) > SettingTempteratureBlinkShowCycle)
			{
				_settingTempBlinkTime=gCurrentTimeInMS;
				_uiSettingTemperatureHidden = true;
				// shown
				uiHideSettingTemperature();
			}
		}

	}

	#endif
	uiDisplayTemperatureAndRunningTime();
}

#if SupportDistilling
void uiDistillingModeTitle(void)
{
	uiLcdPrint_P(1,0,STR(DistillTitle));
}
#define DistillStageStart 0
#define DistillStageHead 1
#define DistillStageHeart 2
#define DistillStageTail 3

void uiDistillingModeStage(byte idx)
{
	const char* str;
	if(idx ==DistillStageStart) str = STR( Start);
	else if(idx ==DistillStageHead)  str=STR( Head);
	else if(idx ==DistillStageHeart)  str=STR( Heart);
	else /*if(idx ==DistillStageTail)*/  str=STR( Tail);

	uiLcdClear(10,0,9);
	uiLcdPrint_P(10,0,str);
}
#endif //#if SupportDistilling

#endif
