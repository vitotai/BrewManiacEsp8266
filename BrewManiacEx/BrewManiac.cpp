/**********************************************************************
 BrewManiac 
 created by Vito Tai
 Copyright (C) 2015 Vito Tai
 
 This soft ware is provided as-is. Use at your own risks.
 You are free to modify and distribute this software without removing 
 this statement.
 BrewManiac by Vito Tai is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
***********************************************************************/

#include <EEPROM.h>
#include <OneWire.h>
#include <PID_v1.h>

#include "config.h"
#include "pins.h"

#define ESP8266 1
#include "mystrlib.h"

#if SerialDebug == true
#define DebugOut(a) DebugPort.print(a)
#define DBG_PRINTF(...) DebugPort.printf(__VA_ARGS__)
#else
#define DebugOut(a) 
#define DBG_PRINTF(...)
#endif

#include "BrewLogger.h"
// *************************
//*  Configuration
// *************************

#define SensorCheckPeriod 5000

#define PowerSwitchTime 10 // 50Hz

#if EnablePidAutoTune == true
#include <PID_AutoTune_v0.h>
#endif

#define WirelessSupported true
#define EnableSensorResolution true

#define EnableButtonFeedback true


#if SerialDebug == true
#define DBG_PRINTF(...) DebugPort.printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...)
#endif

//}debug
// *************************
//*  global variables
// *************************
unsigned long gSystemStartTime; // in mili seconds
unsigned long gCurrentTimeInMS; // in mili seconds
unsigned long gCurrentTimeInSec; // in seconds

float gCurrentTemperature;
float gSettingTemperature;
float gBoilStageTemperature;
float gPidStart;


#if MaximumNumberOfSensors > 1
float gSensorCalibrations[MaximumNumberOfSensors];
#else
float gSensorCalibration;
#endif

byte gBoilHeatOutput;

// the pump/heat on/off is requested by user
//  real pump/heat on/off depends on the temperature
//   and parameter setting

boolean gIsHeatOn;
boolean gIsPumpOn;
boolean gIsUseFahrenheit;

boolean gIsHeatProgramOff;

boolean gIsEnterPwm;
boolean gIsTemperatureReached;
boolean gIsPaused;

#if MANUAL_PUMP_MASH == true
boolean gManualPump;
#endif

#if MaximumNumberOfSensors > 1
byte gSensorAddresses[MaximumNumberOfSensors][8];
byte gSensorNumber;
byte gPrimarySensorIndex;
byte gAuxSensorIndex;
float gAuxTemperature;
float gTemperatureReading[MaximumNumberOfSensors];
#endif

#if SpargeHeaterSupport

#if MaximumNumberOfSensors > 1
// only when multi sensors available, temperatuer control of sparge water is possible.
#define  gSpargeWaterTemperature gAuxTemperature
float gSpargeWaterSetPoint;
float gSpargeTemperatureDifference;
byte  gSpargeWaterSensorIndex;
bool   gEnableSpargeWaterTemperatureControl;
#endif //#if MaximumNumberOfSensors > 1

bool   gHeatSpargeWater;
bool   gIsPhysicalSpargeWaterHeating;
bool   gEnableSpargeWaterHeatingControl;

#endif

// *************************
//*  function declaration
// *************************
typedef	void (*SetupFunc)(void);
typedef	void (*EventHandlerFunc)(byte);

void switchApplication(byte screenId);

void backToMain(void);

void setEventMask(byte);

// main screen
void mainSetup(void);
void mainEventHandler(byte);

// setup menu
void menuSetup(void);
void menuEventHandler(byte);

void settingPidSetup(void);
void settingPidEventHandler(byte);

void settingUnitSetup(void);
void settingUnitEventHandler(byte);

void settingAutoSetup(void);
void settingAutoEventHandler(byte);

// manual mode
void manualModeSetup(void);
void manualModeEventHandler(byte);
//auto mode
void autoModeSetup(void);
void autoModeEventHandler(byte);


#if EnablePidAutoTune == true
void autoTuneMenuSetup(void);
void autoTuneMenuEventHandler(byte);
#endif

#if MaximumNumberOfSensors > 1
void sensorMenuSetup(void);
void sensorMenuEventHandler(byte);
#endif

#if SpargeHeaterSupport == true
void spargeMenuSetup(void);
void spargeMenuEventHandler(byte);
#endif


#define ConvertF2C(d) (((d)-32)/1.8)
#define ConvertC2F(d) (((d)*1.8)+32)


#if WirelessSupported == true

//Stage
#define StageDoughIn 	0
// 1 -6 rest, 
// 7 mashout
#define StageBoil 		8
#define StageCooling 		9
#define StageWhirlpool  10
#define StageDelayStart 11
#define StageManualMode 100
#define StageIdleScreen 101
#define StageSetting 	102
#define StagePIDAutoTune 	103

//Event
// Timeup usually means another stage or notification, ignore it

#define RemoteEventTemperatureReached 1
#define RemoteEventAddMalt 			  2
#define RemoteEventRemoveMalt 		 3
#define RemoteEventIodineTest 		4
#define RemoteEventPause 			5
#define RemoteEventResume 			6
#define RemoteEventAddHop 			7
#define RemoteEventPwmOn 			8
#define RemoteEventPwmOff 			9
#define RemoteEventBoilFinished 	10

#define RemoteEventPumpRest         11
#define RemoteEventPumpRestEnd      12

#define RemoteEventBrewFinished 	99

void btReportCurrentStage(byte stage);
void btReportEvent(byte event);
void btReportSettingTemperature(void);
void btReportPwm(void);

#endif
// *************************
//*  Screens
// *************************

#define ButtonPressedEventMask  0x1
#define TemperatureEventMask (0x1 <<1)
#define TimeoutEventMask  (0x1 <<2)
#define PumpRestEventMask  (0x1 <<3)


typedef struct _CScreen{
	SetupFunc setup;
	EventHandlerFunc eventHandler;
}CScreen;

typedef enum _ScreenIdType{
MAIN_SCREEN= 0,
SETUP_SCREEN =1,
PID_SETTING_SCREEN= 2,
UNIT_SETTING_SCREEN= 3,
AUTO_SETTING_SCREEN = 4,

MANUAL_MODE_SCREEN = 5,
AUTO_MODE_SCREEN = 6,

#if MaximumNumberOfSensors > 1
SENSOR_SCREEN,
#endif
#if SpargeHeaterSupport == true
SPARGE_SETTING_SCREEN,
#endif
END_OF_SCREEN
} ScreenIdType;

const CScreen allScreens[]  =
{{
	&mainSetup,
	&mainEventHandler,
},
{
	&menuSetup,
	&menuEventHandler,
},
{
	&settingPidSetup,
	&settingPidEventHandler,
},
{
	&settingUnitSetup,
	&settingUnitEventHandler,
},
{
	&settingAutoSetup,
	&settingAutoEventHandler,
},
{	
	&manualModeSetup,
	&manualModeEventHandler,
},
{	
	&autoModeSetup,
	&autoModeEventHandler,
}
#if MaximumNumberOfSensors > 1
,{	
	&sensorMenuSetup,
	&sensorMenuEventHandler,
}
#endif
#if SpargeHeaterSupport == true
,{	
	&spargeMenuSetup,
	&spargeMenuEventHandler,
}
#endif
};


byte _currentEventMask;
//#define setEventMask(a) _currentEventMask=(a)

void setEventMask(byte mask)
{
	_currentEventMask=mask;
}


// *************************
//*  includes, follow Arduino conveniention
// *************************
#include "buzz.h"

#include "string.h"
#include "ui.h"

#include "ps.h"
#include "wi.h"

// *************************
//*  Time related function
// *************************

unsigned long _gTimeout;
unsigned long _gAuxTimeout;
boolean  _isAuxTimeout;

#define IsAuxTimeout _isAuxTimeout
/*
unsigned long getTimeLeft(void)
{
	if(_gTimeout ==0) return 0;
	return(_gTimeout - gCurrentTimeInMS);
}
*/
unsigned long tmGetRemainingTime(void)
{
	if(_gTimeout ==0) return 0;
	//else
	unsigned long ret=(_gTimeout - gCurrentTimeInMS);
	return ret;
}

unsigned long tmPauseTimer(void)
{
	//[TODO:] there are chance that _gTimeout is ZERO
	// after 50 days...
	
	if(_gTimeout ==0) return 0;
	//else
	
	unsigned long ret=(_gTimeout - gCurrentTimeInMS);
	_gTimeout=0;
	_gAuxTimeout=0;
	
	return ret;
}

void tmSetAuxTimeoutAfter(unsigned long duration)
{
	_gAuxTimeout = gCurrentTimeInMS + duration;
}

void tmSetTimeoutAfter(unsigned long duration)
{
	_gTimeout = gCurrentTimeInMS + duration;
#if 0
	Serial.print(F("setTimeoutAfter:"));
	Serial.print(duration);
	Serial.print(F(",current="));
	Serial.print(gCurrentTimeInMS);
	Serial.print(F(",expires="));
	Serial.println(_gTimeout);
#endif
}

void tmInitialize(void)
{
	gSystemStartTime = millis();
}

boolean tmTiming(void)
{
	gCurrentTimeInMS=millis();
	
	gCurrentTimeInSec = (gCurrentTimeInMS - gSystemStartTime) / 1000;
	
	if(_gTimeout
	 &&  _gTimeout <= gCurrentTimeInMS)
	{
		_gTimeout = 0;
		_isAuxTimeout=false;
		return true;
	}

	if(_gAuxTimeout
	 &&  _gAuxTimeout <= gCurrentTimeInMS)
	{
		_gAuxTimeout = 0;
		_isAuxTimeout=true;
		return true;
	}
	
	return false;
}

// *************************
//*  button related function
// *************************

#define ButtonUpMask    0x01
#define ButtonDownMask  (0x01 << 1)
#define ButtonStartMask (0x01 << 2)
#define ButtonEnterMask (0x01 << 3)

unsigned char _testButtunStatus;
unsigned long _buttonChangeTime;
unsigned long _continuousPressedDectedTime;
boolean _continuousPressedDetected;
boolean gLongPressed;
byte gButtonPressed=0;

unsigned long _oneFigerUp;

void btnInitialize(void)
{
 	_testButtunStatus=0;
	_buttonChangeTime=0;
	_continuousPressedDectedTime=0;
	_continuousPressedDetected=false;
	gLongPressed=0;
	gButtonPressed=0;
	_oneFigerUp=0;
}


#define btnIsUpPressed (gButtonPressed == ButtonUpMask)
#define btnIsDownPressed (gButtonPressed == ButtonDownMask)
#define btnIsEnterPressed (gButtonPressed == ButtonEnterMask)
#define btnIsStartPressed (gButtonPressed == ButtonStartMask)

#define btnIsStartLongPressed ((gButtonPressed == ButtonStartMask) && gLongPressed)
#define btnIsEnterLongPressed ((gButtonPressed == ButtonEnterMask) && gLongPressed)

#define btnIsUpContinuousPressed (gButtonPressed == (ButtonUpMask<<4))
#define btnIsDownContinuousPressed (gButtonPressed == (ButtonDownMask<<4))

#define isExactButtonsPressed(mask) ((mask) == gButtonPressed)
#define isButtonsPressed(mask) ((mask) & gButtonPressed)

#if SerialDebug != true
#define BUTTON_DEBUG false
#endif

#define BUTTON_DEBUG false

#if WirelessSupported == true

boolean _virtualButtonPressed=false;

void virtualButtonPress(byte mask,boolean longPressed)
{
       gButtonPressed = mask;
       gLongPressed = longPressed;
       _virtualButtonPressed=true;
      #if BUTTON_DEBUG == true
      Serial.println("virutal key");
      #endif
}

#endif



boolean btnReadButtons(void)
{
#if WirelessSupported == true
      if(_virtualButtonPressed)
      {
      	_virtualButtonPressed = false;
        return true;
       }
#endif

  	unsigned char buttons=0;
	btnPrepareRead();
  	if (btnReadPin(ButtonUpPin) == 0)
  	{
  		buttons |= ButtonUpMask;
  	}
  	if (btnReadPin(ButtonDownPin) == 0)
  	{
  		buttons |= ButtonDownMask;
  	}
  	if (btnReadPin(ButtonEnterPin) == 0)
  	{
  		buttons |= ButtonEnterMask;
  	}
  	if (btnReadPin(ButtonStartPin) == 0)
  	{
  		buttons |= ButtonStartMask;
  	}

	if(buttons==0)
	{
		if(_testButtunStatus ==0) return false;
		
#if		EnableButtonFeedback == true
		buzzMute();
#endif
		unsigned long duration=gCurrentTimeInMS - _buttonChangeTime;
 
  		#if BUTTON_DEBUG == true
    	Serial.print(F("pressed:"));
    	Serial.print(_testButtunStatus);
    	Serial.print(F(","));
    	Serial.print(buttons);
    	Serial.print(F("for:"));
    	Serial.println(duration);
   		#endif
		
		if(duration > ButtonPressedDetectMinTime)
		{
			if(duration > ButtonLongPressedDetectMinTime) gLongPressed=true;
			else gLongPressed =false;
			gButtonPressed = _testButtunStatus;
			
			_testButtunStatus =0;
			_continuousPressedDetected = false;
  		
  		#if BUTTON_DEBUG == true
  		Serial.print(gButtonPressed);
  		if (gLongPressed) Serial.println(F(" -Long Pressed"));
    	else Serial.println(F(" -Pressed"));
   		#endif


			return true; 
		}
		
  		#if BUTTON_DEBUG == true
    	Serial.println(F("Not Pressed"));
   		#endif

		_testButtunStatus =0;
		_continuousPressedDetected = false;
		
		return false; 
	}
	
	// buttons is not ZERO afterward
	if(buttons == _testButtunStatus) // pressing persists
	{
		if(_continuousPressedDetected )
		{
			//if duration exceeds a trigger point
			if( (gCurrentTimeInMS - _continuousPressedDectedTime) > ButtonContinuousPressedTrigerTime)
			{
				_continuousPressedDectedTime=gCurrentTimeInMS;

			  	#if BUTTON_DEBUG == true
			  	Serial.print(gButtonPressed);
    			Serial.print(F(" -Continues 2 pressed:"));
    			Serial.println(gCurrentTimeInMS);
   				#endif

				return true;
			}
		}
		else
		{
			unsigned long duration=gCurrentTimeInMS - _buttonChangeTime;

#if		EnableButtonFeedback == true
			if(duration > ButtonPressedDetectMinTime) buzzOn();
			if(duration > ButtonLongPressedDetectMinTime) buzzMute();
#endif
			
			if(duration > ButtonContinuousPressedDetectMinTime)
			{
				_continuousPressedDetected=true;
				_continuousPressedDectedTime=gCurrentTimeInMS;
				// fir the first event
				gButtonPressed = buttons << 4; // user upper 4bits for long pressed

			  	#if BUTTON_DEBUG == true
			  	Serial.print(gButtonPressed);
    			Serial.print(F(" -Continues detected pressed:"));
    			Serial.println(gCurrentTimeInMS);
   				#endif

				return true;
			}
		}
	}
	else // if(buttons == _testButtunStatus)
	{
		// for TWO buttons event, it is very hard to press and depress
		// two buttons at exactly the same time.
		// so if new status is contains in OLD status.
		// it might be the short period when two fingers are leaving, but one is detected
		// first before the other
		// the case might be like  01/10 -> 11 -> 01/10
		//  just handle the depressing case: 11-> 01/10
		if((_testButtunStatus & buttons)
			&&  (_testButtunStatus > buttons))
		{
			if(_oneFigerUp ==0)
			{
				_oneFigerUp = gCurrentTimeInMS;
				// skip this time
				return false;
			}
			else
			{
				// one fat finger is dected
				if( (gCurrentTimeInMS -_oneFigerUp) < ButtonFatFingerTolerance) 
				{
					return false;
				}
			}

		#if BUTTON_DEBUG == true
    	Serial.println(F("Failed fatfinger"));
   		#endif

		}
		// first detect, note time to check if presist for a duration.
		_testButtunStatus = buttons;
		_buttonChangeTime = gCurrentTimeInMS;
		_oneFigerUp = 0;
		
		#if BUTTON_DEBUG == true
		Serial.print(buttons);
    	Serial.println(F(" -Attempted"));
   		#endif

	}
	
	return false;
}

// *************************
//*  tempture related function
// ****************************
// temperature event
#define C2F(d)  ((d) * 1.8 + 32)
#define F2C(d)  (((d)-32)/1.8)

void temperatureUnitChange(bool useF)
{
	if(gIsUseFahrenheit == useF) return;
	gIsUseFahrenheit = useF;
	uiChangeTemperatureUnit(useF);
	// update EEPROM content
	// PS_BoilTemp, PS_TempPumpRest

	//	gBoilStageTemperature loaded in heatLoadParameter
	//	_pumpStopTemp loaded in pumpLoadParameter

	if(useF){
	 	float value=readSetting(PS_BoilTemp);
	 	updateSetting(PS_BoilTemp,(byte)C2F(value));

	 	value=readSetting(PS_TempPumpRest);
	 	updateSetting(PS_TempPumpRest,(byte)C2F(value));
	 
		gSettingTemperature=C2F(gSettingTemperature);
		
	 	value=readSetting(PS_SpargeWaterTemperatureControlAddress);
	 	updateSetting(PS_SpargeWaterTemperatureControlAddress,(byte)C2F(value));		
		
	}else{
	 	float value=readSetting(PS_BoilTemp);
	 	updateSetting(PS_BoilTemp,(byte)F2C(value));

	 	value=readSetting(PS_TempPumpRest);
	 	updateSetting(PS_TempPumpRest,(byte)F2C(value));

	 	value=readSetting(PS_SpargeWaterTemperatureControlAddress);
	 	updateSetting(PS_SpargeWaterTemperatureControlAddress,(byte)F2C(value));		
	
		gSettingTemperature=F2C(gSettingTemperature);
	}
}

OneWire ds(SensorPin);

#if MaximumNumberOfSensors > 1
bool _gIsSensorConverting[MaximumNumberOfSensors];

#else
boolean _isConverting;
#endif

byte _sensorData[9];


#if	MaximumNumberOfSensors	> 1
#define HEX(a) ((a) > 9)? ('A' + (a) -10):('0' + (a))

void printSensorAddress(char *buf, byte *addr)
{
	for(byte i=0;i<8;i++)
	{
		buf[i*2]= HEX(addr[i] >> 4);
		buf[i*2 +1]=HEX(addr[i] & 0xF);
	}
	buf[16]=0;
}


byte scanSensors(byte max,byte addresses[][8]) {
	byte i;
  	byte m=0;
#if SerialDebug == true	
  	Serial.print("Looking for 1-Wire devices...\n\r");// "\n\r" is NewLine 
#endif

  	while(m< max &&  ds.search(addresses[m])) {
    	if ( OneWire::crc8( addresses[m], 7) != addresses[m][7]) {
#if SerialDebug == true	
			Serial.print("CRC is not valid!\n\r");
#endif
      		return m;
    	}
    	if (addresses[m][0] != 0x28)
    	{
    		break;
		}
#if SerialDebug == true
		char pbuf[20];
		printSensorAddress(pbuf,addresses[m]);
		Serial.print("Found 0x");
		Serial.println(pbuf);
#endif
    	m++;
	}
	//  Serial.println();
#if SerialDebug == true    	
	Serial.println("Done");
#endif
	ds.reset_search();

	return m;
}

void loadSensorSetting(void)
{
	gPrimarySensorIndex=0;

#if SerialDebug == true    	
			Serial.print("loadSensorSetting\n\r");
#endif
	
	// read Temperature sensor addresses.
	byte i;
  	for(i=0;i< MaximumNumberOfSensors; i++) {
		// read sensor address from storage.

		int base=PS_SensorAddressOf(i);
		
		for(byte j=0;j<8;j++){
			gSensorAddresses[i][j]=readSetting(base +j);
		}

#if SerialDebug == true
		char pbuf[20];
		printSensorAddress(pbuf,gSensorAddresses[i]);
		Serial.printf("Sensor: addr:%d, 0x%s\n",base,pbuf);
#endif
		
    	if (gSensorAddresses[i][0] != 0x28
 	   	 		|| OneWire::crc8( gSensorAddresses[i], 7) != gSensorAddresses[i][7]) {
			break;
    	}
    	
		gSensorCalibrations[i]=((float)(readSetting(CalibrationAddressOf(i)) - 50) / 10.0);


	}
	gSensorNumber=i;
// if more than one sensor available, use the second as aux by default	
	gAuxSensorIndex =(gSensorNumber>1)? 1:0; 
	
#if SerialDebug == true    	
		Serial.printf("Number of sensors:%d",gSensorNumber);
#endif

}

#endif //#if	MaximumNumberOfSensors	> 1


#if EnableSensorResolution	== true
#define SensorResolution_12 3
#define SensorResolution_11 2
#define SensorResolution_10 1
#define SensorResolution_9  0

#define ResolutionDecode(d) (((d)>>5)&0x3)

byte gSensorResolution;

void tpSetSensorResolution(byte *addr, byte res)
{
    ds.reset();             // rest 1-Wire
    if(addr) ds.select(addr);
    else ds.skip();        // select DS18B20
    ds.write(0x4E);         // write on scratchPad
    ds.write(0x00);         // User byte 0 - Unused
    ds.write(0x00);         // User byte 1 - Unused
    ds.write((res << 5)|0x1F);         // set up en 12 bits (0x7F)
    ds.reset();             // reset 1-Wire
}
#endif //#if EnableSensorResolution	== true


void tpInitialize(void)
{

	gCurrentTemperature = INVALID_TEMP_C;
	gBoilStageTemperature=readSetting(PS_BoilTemp);

#if	MaximumNumberOfSensors	> 1
	
	gAuxTemperature = INVALID_TEMP_C;

	loadSensorSetting();

	for(byte i=0;i< MaximumNumberOfSensors;i++){
		_gIsSensorConverting[i]=false;
		gTemperatureReading[i]= INVALID_TEMP_C;
	}
#else
	_isConverting=false;
	gSensorCalibration= ((float)(readSetting(PS_Offset) - 50) / 10.0);
#endif

#if FakeHeating
	gCurrentTemperature =gIsUseFahrenheit? 67.8:19.9;

#if	MaximumNumberOfSensors	> 1
	
	loadSensorSetting();

	for(byte i=0;i< MaximumNumberOfSensors;i++){
		_gIsSensorConverting[i]=false;
		gTemperatureReading[i]= gIsUseFahrenheit? 67.8:19.9;
	}
#endif

#endif //#if FakeHeating

}

// the following code basically comes from Open ArdBir

#define DSCMD_CONVERT_T 0x44
#define DSCMD_READ_SCRATCHPAD 0xBE 

#if  MaximumNumberOfSensors > 1

float  _readTemperature(byte *addr)
{
	byte sensorData[9];

 	ds.reset();
 	ds.select(addr);
    
    // request data
    ds.write(DSCMD_READ_SCRATCHPAD);  
    for ( byte i = 0; i < 9; i++) {           // with crc we need 9 bytes
      		sensorData[i] = ds.read();
    } 
    	/* add this routine for crc version */
    if ( OneWire::crc8(sensorData, 8) != sensorData[8]) {  //if checksum fails start a new conversion right away
      	return INVALID_TEMP_C;
    }
	// data got!
    unsigned int raw = (sensorData[1] << 8) + sensorData[0];
    
#if EnableSensorResolution	== true
	gSensorResolution=sensorData[4] & 0x60;
#endif
    // at lower res, the low bits are undefined, so let's zero them
    if (gSensorResolution == 0x00) raw = raw & ~7;  // 0.5C 9 bit resolution, 93.75 ms
    else if (gSensorResolution == 0x20) raw = raw & ~3; // 0.25C 10 bit res, 187.5 ms
    else if (gSensorResolution == 0x40) raw = raw & ~1; // 0.125C 11 bit res, 375 ms
	// 0x60  0.0625C 12bits, 750ms
	
    float temperature = (float)raw * 0.0625;
    if(gIsUseFahrenheit) temperature = C2F(temperature);
    return temperature;
}
#define SensorForIdle 0
#define SensorForManual 1
#define SensorForPreMash 2
#define SensorForMash 3
#define SensorForBoil 4
#define SensorForCooling 5
#define NumberOfSensorStage 6


void setSensorForStage(byte s)
{
	byte pidx=readSetting(PS_SensorUseAddressOf(s));
	byte paux;

#if SpargeHeaterSupport == true && MaximumNumberOfSensors > 1
	if(gHeatSpargeWater && gEnableSpargeWaterTemperatureControl)
		paux = gSpargeWaterSensorIndex;
	else
#endif
	paux=readSetting(PS_AuxSensorAddressOf(s));

	gPrimarySensorIndex=(pidx >= gSensorNumber)? 0:pidx;
	gAuxSensorIndex=(paux >=gSensorNumber)? 0: paux;

#if SerialDebug == true
	Serial.printf("Set sensor for %d, P:%d, Aux:%d\n",s,gPrimarySensorIndex,gAuxSensorIndex);
#endif
}
#endif

uint32_t _lastTempRead;

#if MaximumNumberOfSensors > 1

void tpReadTemperature(void)
{
#if FakeHeating
	return;
#endif
	
	for(byte si=0;si < gSensorNumber;si++)
	{
  		if (_gIsSensorConverting[si] == false) 
  		{
  			if(gCurrentTimeInMS - _lastTempRead > MinimumTemperatureReadGap){
		  		// start conversion and return
				ds.reset();
				ds.select(gSensorAddresses[si]);

    			ds.write(DSCMD_CONVERT_T, 0);
    			_gIsSensorConverting[si] = true;
    			
    			if (si == (gSensorNumber -1)){
    				_lastTempRead = gCurrentTimeInMS;
    			}
    		}
  		}
  		else
  		{
  			// converting, check ready or not
			ds.reset();
  			ds.select(gSensorAddresses[si]);
			// check for conversion if it isn't complete return if it is then convert to decimal
    		byte busy = ds.read_bit();
    		if (busy != 0)
    		{
    			float reading=_readTemperature(gSensorAddresses[si]);

				if(IS_TEMP_INVALID(reading))
				{
					// invalid sensor data.
					if(! IS_TEMP_INVALID(gTemperatureReading[si])){
					//!error case. invalidate the data
						#if SerialDebug
						DebugPort.println("Sensor disconneced!");
						#endif
						buzzPlaySound(SoundIdWarnning);
					}				
				}
				else
					reading += gSensorCalibrations[si];

				gTemperatureReading[si] = reading;

				_gIsSensorConverting[si] = false;
				
				if(gPrimarySensorIndex == si) gCurrentTemperature = reading;
				if(gAuxSensorIndex == si)  gAuxTemperature = reading;
    		}
		}
	} // for every sensor
} // tpReadTemperature


#else // #if MaximumNumberOfSensors > 1

void tpReadTemperature(void)
{
#if FakeHeating
	return;
#endif

  	if (_isConverting == false) 
  	{
	  	if(gCurrentTimeInMS - _lastTempRead > MinimumTemperatureReadGap){
		  	// start conversion and return
			ds.reset();
			ds.skip();
    		ds.write(DSCMD_CONVERT_T, 0);
    		_isConverting = true;
			
			_lastTempRead = gCurrentTimeInMS;    		
		}
    	return;
  	}
  	// else if convert start 
  	//if (_isConverting) 
  	//
  
  	// check for conversion if it isn't complete return if it is then convert to decimal
  	ds.reset();
	ds.skip();
    byte busy = ds.read_bit();
    if (busy == 0) return;
	// reset & "select" again
    ds.reset();
  	ds.skip();    
    // request data
    ds.write(DSCMD_READ_SCRATCHPAD);  
    for ( byte i = 0; i < 9; i++) {           // with crc we need 9 bytes
      		_sensorData[i] = ds.read();
    } 
    	/* add this routine for crc version */
    if ( OneWire::crc8(_sensorData, 8) != _sensorData[8]) {  //if checksum fails start a new conversion right away
		// re-issue convert command
    	ds.reset();
		ds.skip();
      	ds.write(DSCMD_CONVERT_T, 0);
      	_isConverting = true;
      	return;
    }
	// data got!
    unsigned int raw = (_sensorData[1] << 8) + _sensorData[0];

#if EnableSensorResolution	== true
	gSensorResolution=_sensorData[4] & 0x60;
#endif
    // at lower res, the low bits are undefined, so let's zero them
    if (gSensorResolution == 0x00) raw = raw & ~7;  // 0.5C 9 bit resolution, 93.75 ms
    else if (gSensorResolution == 0x20) raw = raw & ~3; // 0.25C 10 bit res, 187.5 ms
    else if (gSensorResolution == 0x40) raw = raw & ~1; // 0.125C 11 bit res, 375 ms
	// 0x60  0.0625C 12bits, 750ms

    gCurrentTemperature = (float)raw  * 0.0625;
	if(gIsUseFahrenheit) gCurrentTemperature = C2F(gCurrentTemperature);
    //apply calibration 
    gCurrentTemperature +=  gSensorCalibration; //((float)(readSetting(PS_Offset) - 50) / 10.0);
    _isConverting = false;
}

#endif // #if MaximumNumberOfSensors > 1

void pumpLoadParameters(void);
void heatLoadParameters(void);

void loadBrewParameters(void)
{
	heatLoadParameters();
	pumpLoadParameters();
}
// *************************
//*  heating related function
// *************************


boolean _physicalHeattingOn;
byte _heatWindowSize;
unsigned long _windowStartTime;

double pidInput;
double pidSetpoint;
//pidInput=gCurrentTemperature;
//pidSetpoint=gSettingTemperature;

//#define pidInput gCurrentTemperature
//#define pidSetpoint gSettingTemperature

double pidOutput;

PID thePID(&pidInput,&pidOutput,&pidSetpoint,100,40,0,DIRECT);

#if EnablePidAutoTune == true 

PID_ATune autoTune(&pidInput, &pidOutput);

bool _isRunningAutoTune=false;

void endAutoTune(void)
{
	_isRunningAutoTune=false;
	thePID.SetMode(AUTOMATIC);
}

void cancelAutoTune(void)
{
	autoTune.Cancel();
	endAutoTune();
}

void startAutoTune(void)
{
	pidOutput=PidAutoTuneStartValue;
    autoTune.SetNoiseBand(PidAutoTuneNoise);
    autoTune.SetOutputStep(PidAutoTuneStep);
    autoTune.SetLookbackSec((int)PidAutoTuneLookBack);
    autoTune.SetControlType(1);
	_isRunningAutoTune = true;
}

void saveTunning(void)
{
	// update constant
	double  kp = autoTune.GetKp();
    double  ki = autoTune.GetKi();
    double  kd = autoTune.GetKd();
    thePID.SetTunings(kp,ki,kd);

    updateSetting(PS_kP,(byte)( kp + 100.0)); 
	updateSetting(PS_kI,(byte)( ki * 250.0 +100));
	//updateSetting(PS_kD,(byte)( kd + 100.0));
	wiSettingChanged(PS_kD,(byte)( kd + 100.0));// notify setting change.
	commitSetting();
}

#endif //#if EnablePidAutoTune == true 


#if FakeHeating == true
unsigned long lastTime;
#endif

void heatPhysicalOn(void)
{
	if(!_physicalHeattingOn)
	{
		setHeaterOut(HIGH);
		_physicalHeattingOn=true;
#if FakeHeating == true
	lastTime = gCurrentTimeInMS;
#endif
		uiHeatingStatus(HeatingStatus_On);
		wiReportHeater(HeatingStatus_On);

	}
}

void heatPhysicalOff(void)
{
	if(_physicalHeattingOn)
	{
		setHeaterOut(LOW);
		_physicalHeattingOn=false;
	}
	if(gIsHeatOn){
		uiHeatingStatus(HeatingStatus_On_PROGRAM_OFF);
		wiReportHeater(HeatingStatus_On_PROGRAM_OFF);
	}else{
		uiHeatingStatus(HeatingStatus_Off);
		wiReportHeater(HeatingStatus_Off);
	}
}


#if SpargeHeaterSupport == true

void heaterSpargeOn(void)
{
	DBG_PRINTF("Sparge Heater On\n");
	gIsPhysicalSpargeWaterHeating=true;
	setAuxHeaterOut(HIGH);
	uiAuxHeatingStatus(HeatingStatus_On);
	wiReportAuxHeater(HeatingStatus_On);
}

void heaterSpargeOff(void)
{
	DBG_PRINTF("Sparge Heater OFF\n");
	gIsPhysicalSpargeWaterHeating=false;
	setAuxHeaterOut(LOW);

	if(gHeatSpargeWater){
		uiAuxHeatingStatus(HeatingStatus_On_PROGRAM_OFF);
		wiReportAuxHeater(HeatingStatus_On_PROGRAM_OFF);
	}
	else{
		uiAuxHeatingStatus(HeatingStatus_Off);
		wiReportAuxHeater(HeatingStatus_Off);
	}
}


typedef enum _PowerState{
	PowerStateIdle,
	PowerStateHeating,
	PowerStateSpargeHeating,
	PowerStateHeatPending,
	PowerStateSpargePending
} PowerState;

PowerState _gPowerState;
PowerState _gReportedPowerState;

unsigned long _gTimeEnterIdle;
// need to delay report:
// M(O) S(X), M(X) S(O) usually at the same time but at 10ms delay
// sequence:
//  M req On,( S off ), M On
//  M req On, S off, M req Off, S on.
//  M req Off, M Off, (S On)
//  
void requestHeaterOn(bool on)
{
	if(on){
		if(_gPowerState == PowerStateIdle || _gPowerState == PowerStateHeatPending 
				 || _gPowerState == PowerStateSpargePending){
			if(( millis() - _gTimeEnterIdle) > PowerSwitchTime){
			  	// turn physical on
			  	_gPowerState =PowerStateHeating;
   		 		heatPhysicalOn();
			}else{
				_gPowerState = PowerStateHeatPending;
//				DBG_PRINTF("Heating pended\n");
			}
		}else if(_gPowerState == PowerStateSpargeHeating){
			 // turn sparge heater, and turn on main heating later
			 // another way to do it is using delay(10); and turn on
			 // directly.
			 // 
			heaterSpargeOff();
			_gPowerState = PowerStateHeatPending;
			_gTimeEnterIdle= millis();
		}
	}else{
		// turn off Main Heating
		heatPhysicalOff();
		
		if(_gPowerState == PowerStateHeating){
			// turn off. Sparge heater might be On later
			_gPowerState = PowerStateIdle;
			_gTimeEnterIdle= millis();
		}else if( _gPowerState == PowerStateHeatPending){
			 _gPowerState = PowerStateIdle;
		}
	}
	//DBG_PRINTF("requestHeaterOn:%d, %d, %d\n",on,_gPowerState,_physicalHeattingOn);
}

void requestSpargeHeaterOn(bool on)
{
	if(on){
		if(_gPowerState == PowerStateIdle || _gPowerState == PowerStateSpargePending){
			if(( millis() - _gTimeEnterIdle) > PowerSwitchTime){
			   // turn sparge heater on
				_gPowerState =PowerStateSpargeHeating;
			   heaterSpargeOn();
			}else{
				// wait for a moment.
				_gPowerState =PowerStateSpargePending;
			}
		}
	}else{
		heaterSpargeOff();
		if(_gPowerState == PowerStateSpargeHeating){
			// turn off Sparge Heater.
			_gPowerState = PowerStateIdle;
			_gTimeEnterIdle= millis();
		}else if (_gPowerState == PowerStateSpargePending){
			_gPowerState = PowerStateIdle;
		}
	}
	//DBG_PRINTF("requestSpargeHeaterOn:%d\n",_gPowerState);
}


void spargeHeaterControl(void)
{
	if(!gHeatSpargeWater) return;
#if MaximumNumberOfSensors > 1
	if(gEnableSpargeWaterTemperatureControl){
	 	if(gSpargeWaterTemperature <= (gSpargeWaterSetPoint - gSpargeTemperatureDifference)){
 			if(!gIsPhysicalSpargeWaterHeating)
	 			requestSpargeHeaterOn(true);
 		}else if(gSpargeWaterTemperature >= (gSpargeWaterSetPoint + gSpargeTemperatureDifference)){
	 		if(gIsPhysicalSpargeWaterHeating)
		 		requestSpargeHeaterOn(false);
 		}
 	}
 	else
#endif
 	{
 		// no temp. control, just output
 		if(!gIsPhysicalSpargeWaterHeating)
	 		requestSpargeHeaterOn(true);
 	}
}

#define SettingValueSpargeDifference(a) ((float)(a)/10.0)

void startHeatingSpargeWater(void)
{
	// load parameter
#if MaximumNumberOfSensors > 1
	gEnableSpargeWaterTemperatureControl= readSetting(PS_SpargeWaterTemperatureControlAddress);
	gSpargeWaterSensorIndex = readSetting(PS_SpargeWaterSensorIndexAddress);
	gSpargeWaterSetPoint=(float)readSetting(PS_SpargeWaterTemperatureAddress);
	gSpargeTemperatureDifference=SettingValueSpargeDifference(readSetting(PS_SpargeWaterTemperatureDifferenceAddress));
#endif
	gHeatSpargeWater=true;
	requestSpargeHeaterOn(false); // to draw the icon
}

void stopHeatingSpargeWater(void)
{
	gHeatSpargeWater=false;
	requestSpargeHeaterOn(false);
}

#endif //#if SpargeHeaterSupport == true


void heatInitialize(void)
{
	thePID.SetMode(AUTOMATIC);
	
	_physicalHeattingOn=false;
	gIsHeatOn=false;
	gIsHeatProgramOff=false;
	
#if SpargeHeaterSupport
	gHeatSpargeWater=false;
	gIsPhysicalSpargeWaterHeating=false;
	_gPowerState = PowerStateIdle;
	_gReportedPowerState= PowerStateIdle;
	_gTimeEnterIdle= millis();
#endif
	
}
// the should be call before REAL action instead of system startup
void heatLoadParameters(void)
{
	thePID.SetTunings((double)readSetting(PS_kP)-100.0, 
					  (double)((readSetting(PS_kI)-100.0) / 250.0),
					  (double)readSetting(PS_kD)-100.0);
					
	_heatWindowSize = readSetting(PS_WindowSize);    
 	thePID.SetSampleTime((int)readSetting(PS_SampleTime) * 250);

 	gBoilStageTemperature=(float)readSetting(PS_BoilTemp);
 	gBoilHeatOutput=readSetting(PS_BoilHeat);

	gPidStart = (float)readSetting(PS_PID_Start) /10.0;
	
	if(gPidStart < 1.0) gPidStart=1.0;
	else if(gPidStart > 3.5) gPidStart=3.5;
	
#if 0 // SerialDebug == true
	
	Serial.print("gBoilStageTemperature=");
	Serial.println(gBoilStageTemperature);

#endif

}


void heatOff(void)
{
	gIsHeatOn = false;
	#if SpargeHeaterSupport
	requestHeaterOn(false);
	#else
	heatPhysicalOff();
	#endif
}

boolean _isPIDMode;

void heatOn(bool pidmode)
{
	gIsHeatOn = true;
	gIsHeatProgramOff=false;
	_isPIDMode=pidmode;
	
	// should run through heating algorithm first
	// so that the correct symbol can be shown
	_windowStartTime=millis();

	#if SpargeHeaterSupport
	requestHeaterOn(false);
	#else
	heatPhysicalOff();
	#endif
}

void heatProgramOff(void)
{
	gIsHeatProgramOff=true;
	
	#if SpargeHeaterSupport
	requestHeaterOn(false);
	#else
	heatPhysicalOff();
	#endif
}

float round025(float num)
{
	int intPart=(int)num;
	return (float)intPart + ((int)((num - (float)intPart)*100.0)/25)*0.25;
}



void heaterControl(void)
{
	if(! gIsHeatOn) return;
	
	if(IS_TEMP_INVALID(gCurrentTemperature)) {
		if(_physicalHeattingOn) {
#if SpargeHeaterSupport
			requestHeaterOn(false);
#else
			heatPhysicalOff();
#endif
		}
		return;
	}
	
#if FakeHeating == true
#if MaximumNumberOfSensors > 1
	if(_physicalHeattingOn){
		for(byte i=0;i<gSensorNumber;i++)
			gTemperatureReading[i] += (gCurrentTimeInMS - lastTime) * (0.00000002 + i*0.000000001);
	}else{
		for(byte i=0;i<gSensorNumber;i++)
			gTemperatureReading[i] -= (gCurrentTimeInMS - lastTime) * (0.000000002+ i*0.0000000005);
	}
	gCurrentTemperature = gTemperatureReading[gPrimarySensorIndex];
	gAuxTemperature = gTemperatureReading[gAuxSensorIndex];
#else
	if(_physicalHeattingOn){
		gCurrentTemperature += (gCurrentTimeInMS - lastTime) * 0.0002;
		lastTime = gCurrentTimeInMS;
	}else{
		gCurrentTemperature -= (gCurrentTimeInMS - lastTime) * 0.000005;
		lastTime = gCurrentTimeInMS;
	}
#endif
#endif	

 	pidInput = gCurrentTemperature;
	pidSetpoint= gSettingTemperature;
   
  	if (_isPIDMode) //PID mode
  	{
  	 #if EnablePidAutoTune == true 
		if(_isRunningAutoTune)
		{
			if(autoTune.Runtime())
			{
				// complete
				saveTunning();
				endAutoTune();
				buzzPlaySound(SoundIdAddHop);
			}
		}
		else
  	 #endif
  		{
    		if ((pidSetpoint - pidInput) < gPidStart) // Delta < DeltaPID(3.5), 
    		{
        		thePID.Compute();   // was 6, getting close, start feeding the PID -mdw
    		} 
    		else // if (Rapporto < 1.00)
    		{
      			pidOutput = 255;      // was 5, ignore PID and go full speed -mdw  // set the output to full on
    		}  
    	}
  	}// end of _isPIDMode
  	else
  	{
	  	pidOutput = 255;  // NOT PID mode. just turn on full power. 
  	}

  	// In boiling stage, the output value is reassigned.
  	
	if (pidInput >= pidSetpoint && pidInput >= gBoilStageTemperature) 
		pidOutput = gBoilHeatOutput * 255.0 / 100.0;
  	
  	if(gIsHeatProgramOff) pidOutput=0;
  	
#if 0 // SerialDebug == true
    	DebugPort.print("PID.Compute");
        DebugPort.print(pidInput);
        DebugPort.print(",");
        DebugPort.print(pidSetpoint);
        DebugPort.print(",");
        DebugPort.print(gBoilStageTemperature);
        DebugPort.print(",");
        DebugPort.println(pidOutput);
#endif

	// PWM
  	unsigned long now = millis();
  	if (now - _windowStartTime > (unsigned int) _heatWindowSize * 250) 
  	{
    	_windowStartTime += (unsigned int)_heatWindowSize * 250; 
    	//time to shift the Relay Window
  	}
  	
  	if ((pidOutput / 255) * ((unsigned int)_heatWindowSize * 250) > now - _windowStartTime) 
  	{
  		if(!_physicalHeattingOn)
  		{
  			#if SpargeHeaterSupport
			requestHeaterOn(true);
	  		#else
   	 		heatPhysicalOn();
   	 		#endif
   	 	}
  	}
  	else 
  	{
  		// turn off heat 
  		if(_physicalHeattingOn)
  		{
  			#if SpargeHeaterSupport
			requestHeaterOn(false);
	  		#else
  			heatPhysicalOff();
  			#endif
  		}
  	}
} // end of heaterControl

void heatThread(void)
{
	heaterControl();
#if SpargeHeaterSupport
	spargeHeaterControl();
	if(_gReportedPowerState != _gPowerState){
		if(_gPowerState != PowerStateHeatPending
			&& _gPowerState != PowerStateSpargePending){
			wiUpdateHeaterStatus();
			_gReportedPowerState=_gPowerState;
		}
	}
#endif
}

// *************************
//*  pump related function
// *************************

boolean _pumpPhysicalOn;
unsigned long _pumpLastSwitchOnTime;
float _pumpStopTemp;
//byte _sensorType;

unsigned long _pumpRestTime;
unsigned long _pumpCycleTime;
boolean _isStageTempReached;
boolean _isPumpRestChanged;
boolean _pumpRestEnabled;

#define SensorInside 0
#define SensorOutside 1

void pumpPhysicalOn(void)
{
	if(_pumpPhysicalOn) return;
	setPumpOut(HIGH);
	_pumpPhysicalOn=true;
	_pumpLastSwitchOnTime = gCurrentTimeInMS;

	uiPumpStatus(PumpStatus_On);
	wiReportPump(PumpStatus_On);
}

void pumpPhysicalOff(void)
{
	if(_pumpPhysicalOn)
	{
		setPumpOut(LOW);
		_pumpPhysicalOn=false;
	}
	
	if(gIsPumpOn){
		uiPumpStatus(PumpStatus_On_PROGRAM_OFF);
		wiReportPump(PumpStatus_On_PROGRAM_OFF);
	}else{
		uiPumpStatus(PumpStatus_Off);
		wiReportPump(PumpStatus_Off);
	}
}

void pumpInitialize(void)
{
	_pumpPhysicalOn=false;
	gIsPumpOn=false;
	_isPumpRestChanged = false;
	_pumpRestEnabled=false;
}

void pumpLoadParameters(void)
{
	_pumpRestEnabled=false;
	_pumpStopTemp = (float) readSetting(PS_TempPumpRest);
//	_sensorType = readSetting(PS_SensorType);
#if UsePaddleInsteadOfPump
	_pumpRestTime =(unsigned long) readSetting(PS_PumpRest) *1000;
	_pumpCycleTime=(unsigned long) readSetting(PS_PumpCycle) *1000;
#else
	_pumpRestTime =(unsigned long) readSetting(PS_PumpRest) *60 *1000;
	_pumpCycleTime=(unsigned long) readSetting(PS_PumpCycle) *60 *1000;
#endif
	_isPumpRestChanged = false;
#if 0	
	Serial.print("pumpLoadParameters,cycletime=");
	Serial.print(_pumpCycleTime);
	Serial.print("resttime=");
	Serial.println(_pumpRestTime);
#endif
}

void pumpOff(void)
{
	if(!gIsPumpOn) return;
	gIsPumpOn=false;
	pumpPhysicalOff();
}

void pumpOn(void)
{
	if(gIsPumpOn) return;
	gIsPumpOn=true;

	pumpPhysicalOn();
}

#if MANUAL_PUMP_MASH == true
void togglePump(void)
{
	if(gIsPumpOn)
		pumpOff();
	else
		pumpOn();
}
#endif

bool pumpRestIsEnabled(void)
{
	return _pumpRestEnabled;
}

void pumpRestSetEnabled(boolean enable)
{
	_pumpRestEnabled=enable;
	
	if(_pumpRestEnabled && _pumpPhysicalOn) // restart counting time
		_pumpLastSwitchOnTime = gCurrentTimeInMS;
		
	if(!_pumpRestEnabled && gIsPumpOn)
	{
        if(!_pumpPhysicalOn)
        {
			pumpPhysicalOn();
		}
	}
	
//	if (_pumpRestEnabled) Serial.println(F("Enable pump rest"));
//	else Serial.println(F("disable pump rest"));
}

void pumpControl(boolean tempReached)
{
	_isStageTempReached =tempReached;
}

boolean isPumpRest(void)
{
	return ! _pumpPhysicalOn;
}

boolean pumpRestEvent(void)
{
	if(_isPumpRestChanged)
	{
		_isPumpRestChanged = false;
		return true;
	}
	return false;
}

void pumpThread(void)
{
	if(!gIsPumpOn) return;
	// temperature control	
	
	if(gCurrentTemperature >= _pumpStopTemp)
	{
      	if(_pumpPhysicalOn)
      	{
			pumpPhysicalOff();
        	_isPumpRestChanged = true;
		}
		return;
  	} 
  	else // of if(gCurrentTemperature >= _pumpStopTemp)
  	{
  		// if under pump stop temperature
		if(!_pumpRestEnabled)
		{
      		if(!_pumpPhysicalOn) pumpPhysicalOn();
			return;
		}
      	if((gCurrentTimeInMS - _pumpLastSwitchOnTime) < (unsigned long)_pumpCycleTime) 
      	{
      		if(!_pumpPhysicalOn)
      		{
	      		DBG_PRINTF("-End Pump Rest-");
      			pumpPhysicalOn();
        		_isPumpRestChanged = true;
        	}
      	} 
      	else 
      	{
	  		if(_pumpRestTime>0){
      		// pump rest state, heat will be off!
      			if(_pumpPhysicalOn)
      			{
      				DBG_PRINTF("-Pump Rest-");
      				pumpPhysicalOff();
        			_isPumpRestChanged = true;
        		}
        	}
        	if ((gCurrentTimeInMS - _pumpLastSwitchOnTime) >= (_pumpCycleTime + _pumpRestTime))
        	{ 
        		_pumpLastSwitchOnTime = gCurrentTimeInMS;
        		//Serial.println("on time start");
        	}
      	}	 
    } // end of else //if if(gCurrentTemperature >= _pumpStopTemp)
	
}//pumpThread()

// *************************
//*  buzzer related function
// *************************

// move to buzz.h
//#include "buzz.h"

// *************************
//*  Event handling function
// *************************

// *************************
//*  Applications
//*  application should have two
//*  functions, setup & eventHandler
// *************************

//**************************************************************
//* For setting editing
//**************************************************************
// I hope I could do it in C++ objective way.
// maybe someday

void changeSettingValue(int address,byte value)
{
	if(updateSetting(address,value))
		wiSettingChanged(address,value);
}
//TODO: what if the editing is interrupted? 
// user abort or power shortage during editing?
// the best way may be implemented as a "transaction"
// It is simple, but maybe overshooting?
void changeAutomationTime(byte stage, byte time)
{
	updateSetting(PS_StageTimeAddr(stage),time);
}

void changeAutomationTemperature(byte stage, int value)
{
	updateSettingWord(PS_StageTemperatureAddr(stage),value);
}

void finishAutomationEdit(void)
{
#if WirelessSupported == true
	wiRecipeChange();
#endif
}


//typedef void (*FuncValueDisplay)(int);

int _editingValue;
int  _maxValue;
int  _minValue;
void (*_displayFunc)(int);

int editItemValue(void)
{	
	return _editingValue;
}


void editItem(const char * label, int value, int max, int min,void (*displayFunc)(int))
{
	_editingValue=value;
	_maxValue=max;
	_minValue=min;

	if( _editingValue > _maxValue) _editingValue=_maxValue;
	if( _editingValue < _minValue) _editingValue=_minValue;
	_displayFunc=displayFunc;
	
	// uiClearSettingRow();
	// uiSettingTitle will know the length of
	// label, and fill spaces to clear the line
	uiSettingTitle(label);
	(*_displayFunc)(_editingValue);
}

void editItemTitleAppendNumber(byte num) // specially for HOP#
{
	uiSettingTitleAppendNumber(num);
}

void editItemChange(int change)
{
	int potential=_editingValue+change;

	if(potential > _maxValue) potential= _maxValue;
	if(potential < _minValue) potential= _minValue;
	if(potential!=_editingValue)
	{
		_editingValue =potential;	
		uiSettingFieldClear();	
		(*_displayFunc)(_editingValue);
	}	
}


// *************************
//*  PID & PWD settings
// *************************



void displayOffset100(int data)
{
	float fvalue=(float)data -100.0;
	uiSettingDisplayNumber(fvalue,0);
}

void displayMultiply250(int data)
{
	float fvalue=(float)data *250.0;
	uiSettingDisplayNumber(fvalue,0);
}

void displaySimpleInteger(int data)
{
	uiSettingDisplayNumber((float)data,0);
}

void displayTempShift50Divide10(int data)
{
	float fvalue=((float)data -50.0) /10.0;
	
	uiSettingShowTemperature(fvalue,1);
}

void displayTempDivide10(int data)
{
	float fvalue=((float)data) /10.0;
	
	uiSettingShowTemperature(fvalue,1);
}



void displayPercentage(int data)
{
	uiDisplaySettingPercentage(data);
}

void displayResolution(int value)
{
	uiSettingDisplayNumber((float)value,0);
}
void displayOnOff(int value)
{
	if (value==0) uiSettingDisplayText(STR(Off));
	else uiSettingDisplayText(STR(On));
}

//**************************************************************
//* PID PWM setting screen
//**************************************************************
byte _currentPidSetting;

#if MaximumNumberOfSensors > 1
byte _pidSettingAux;
#endif

// table implementation takes up a lot of memory.
// change to hard-coded.
#define PID_SETTING_KP 1
#define PID_SETTING_KI 2
#define PID_SETTING_KD 3
#define PID_SETTING_SampleTime 4
#define PID_SETTING_Window 5
#define PID_SETTING_PWM 6
#define PID_SETTING_Calibration 7
#define PID_SETTING_PIDStart 8
#define PID_SETTING_PIDDoughIn 9

#define PID_SETTING_Resolution 10


void settingPidEditSetting(void)
{	
	int value;
#if MaximumNumberOfSensors > 1

	if(_currentPidSetting==PID_SETTING_Calibration)
		value=(int)readSetting(CalibrationAddressOf(_pidSettingAux));	
	else
#endif
#if EnableSensorResolution	== true
	if(_currentPidSetting!=PID_SETTING_Resolution)
#endif
		value=(int)readSetting(PS_AddrOfPidSetting(_currentPidSetting));

	//Serial.printf("PID:%d val:%d\n",_currentPidSetting,value);
	
	//editItem(str_t label, byte value, byte max, byte min,CDisplayFunc displayFunc)
	if(_currentPidSetting== PID_SETTING_KP)
		editItem(STR(kP),value,200,0,& displayOffset100);
	else if(_currentPidSetting== PID_SETTING_KI)
		editItem(STR(kI),value, 255,0,& displayOffset100);
	else if(_currentPidSetting== PID_SETTING_KD)
		editItem(STR(kD),value,200,0,& displayOffset100);
	else if(_currentPidSetting== PID_SETTING_SampleTime)
		editItem(STR(SampleTime),value,3500/250,1500/250,& displayMultiply250);
	else if(_currentPidSetting== PID_SETTING_Window)
		editItem(STR(WindowSet_ms),value,7500/250,4000/250,& displayMultiply250);
	else if(_currentPidSetting== PID_SETTING_PWM)
		editItem(STR(Heat_in_Boil),value,100,0,& displayPercentage);
	else if(_currentPidSetting== PID_SETTING_Calibration){
		editItem(STR(Calibration),value,100,0,&displayTempShift50Divide10);
#if MaximumNumberOfSensors > 1		
		editItemTitleAppendNumber(_pidSettingAux+1);
#endif
	}else  if(_currentPidSetting== PID_SETTING_PIDStart){
		editItem(STR(Start_PID_In),value,35,10,&displayTempDivide10);
	}
#if EnableSensorResolution	== true
	else if(_currentPidSetting== PID_SETTING_PIDDoughIn){
		editItem(STR(PID_Dough_In),value,1,0,&displayOnOff);
	}else if(_currentPidSetting== PID_SETTING_Resolution){
		value=ResolutionDecode(gSensorResolution) + 9;
//		Serial.printf("res:%d,%d\n",value,gSensorResolution);
		editItem(STR(SensorResolution),value,12,9,&displayResolution);
	}
#endif
}

void settingPidSetup(void)
{
	uiButtonLabel(ButtonLabel(Up_Down_x_Ok));
	_currentPidSetting=1;
#if MaximumNumberOfSensors > 1
	_pidSettingAux =0;
#endif

	settingPidEditSetting();
}

void settingPidEventHandler(byte)
{
	if(btnIsEnterPressed)
	{
		byte value=(byte)editItemValue();
		
#if EnableSensorResolution	== true
		if(_currentPidSetting == PID_SETTING_Resolution)
		{
			byte res= value -9;
			
			Serial.printf("Setres to %d\n",res);
			
			#if	MaximumNumberOfSensors > 1
			for(byte i=0;i<gSensorNumber;i++)
				tpSetSensorResolution(gSensorAddresses[i],res);
			#else		
			tpSetSensorResolution(NULL,res);
			#endif
			//uiClearSettingRow();
			switchApplication(SETUP_SCREEN);
			return;
		}
		else
#endif
		if(_currentPidSetting == PID_SETTING_Calibration)
		{
#if	MaximumNumberOfSensors > 1
			changeSettingValue(CalibrationAddressOf(_pidSettingAux),value);				
			gSensorCalibrations[_pidSettingAux] =((float) value -50)/10.0;
			_pidSettingAux++;
				
			if (_pidSettingAux == gSensorNumber)
			{
				_currentPidSetting ++;
			}

#else
			changeSettingValue(PS_AddrOfPidSetting(_currentPidSetting),value);
			gSensorCalibration = ((float) value -50)/10.0;
			_currentPidSetting ++;
#endif
		}else{
			changeSettingValue(PS_AddrOfPidSetting(_currentPidSetting),value);
			
#if EnableSensorResolution	!= true
			if (_currentPidSetting == PID_SETTING_PIDStart){
				switchApplication(SETUP_SCREEN);
				return;
			}
#endif
#if MaximumNumberOfSensors > 1	
			if(_currentPidSetting == PID_SETTING_PWM && gSensorNumber==0) 
				_currentPidSetting ++;
#endif
			_currentPidSetting ++;

		}
		// not last item, (if it is last item, it won't run through here)
		settingPidEditSetting();
	}
	else if(btnIsUpPressed)
	{
		editItemChange(+1);
	}
	else if(btnIsDownPressed)
	{
		editItemChange(-1);
	}
	else if(btnIsUpContinuousPressed)
	{
		editItemChange(+4);
	}
	else if(btnIsDownContinuousPressed)
	{
		editItemChange(-4);
	}

}

// *************************
//*  Unit Parameters settings
// *************************
byte _currentUnitSetting;

void displayDegreeSymbol(int value)
{
	uiSettingDegreeSymbol(value);
	//if (value==0) uiSettingDisplayText(STR(Celius));
	//else uiSettingDisplayText(STR(Fahrenheit));
}

void displayInsideOutside(int value)
{
	if (value==0) uiSettingDisplayText(STR(Inside));
	else uiSettingDisplayText(STR(Outside));
}


void displayYesNo(int value)
{
	if (value==0) uiSettingDisplayText(STR(No));
	else uiSettingDisplayText(STR(Yes));
}
/*
void displayTemperature(int value)
{
	uiSettingDisplayNumber((float)value,0);
}
*/
void displayActivePassive(int value)
{
	if (value==0) uiSettingDisplayText(STR(Passive));
	else uiSettingDisplayText(STR(Active));
}
#if UsePaddleInsteadOfPump
void displayTimeSec(int value)
{
	uiSettingTimeInSeconds((byte)value);
}
#endif

void displayTime(int value)
{
	uiSettingTimeInMinutes((byte)value);
}

void displayTimeOff(int value)
{
	if(value==0)
	{
		uiSettingDisplayText(STR(Off));
	} 
	else
	{
		displayTime(value);
	}
}

void displaySimpleTemperature(int value)
{
	uiSettingShowTemperature((float)value,0);
}

#if NoWhirlpool != true
#define WhirlpoolHot 2
#define WhirlpoolCold 1
#define WhirlpoolOff 0

void displayHotColdOff(int value)
{
	if (value==0) uiSettingDisplayText(STR(Off));
	else if (value==WhirlpoolCold) uiSettingDisplayText(STR(Cold));
	else uiSettingDisplayText(STR(Hot));
}
#endif

void settingUnitDisplayItem(void)
{
	int value=(int)readSetting(PS_AddrOfUnitSetting(_currentUnitSetting));
	
	//editItem(str_t label, byte value, byte max, byte min,CDisplayFunc displayFunc)
	if(_currentUnitSetting==0)
		editItem(STR(Set_Degree),value,1,0,&displayDegreeSymbol);
	else if(_currentUnitSetting==1)
		editItem(STR(No_Delay_Start),value,1,0,&displayYesNo);
	else if(_currentUnitSetting==2){
		int max= gIsUseFahrenheit? 221:105;
	#if DEVELOP_SETTING_VALUE == true	
		int min= gIsUseFahrenheit? 194:90;
	#else
		int min= gIsUseFahrenheit? 50:10;
	#endif
	
		editItem(STR(Temp_Boil),value,max,min,&displaySimpleTemperature);
	/*else if(_currentUnitSetting==3) 
	// ********* skip, for internal usage always use C
 		editItem(STR(Temp_Boil),value,105,90,&displaySimpleInteger);*/
	}else if(_currentUnitSetting==4){
		#if UsePaddleInsteadOfPump
		editItem(STR(Pump_Cycle),value,30,5,&displayTimeSec);
		#else
		editItem(STR(Pump_Cycle),value,15,5,&displayTime);
		#endif
	}else if(_currentUnitSetting==5){
		#if UsePaddleInsteadOfPump
		editItem(STR(Pump_Rest),value,60,0,&displayTimeSec);
		#else
		editItem(STR(Pump_Rest),value,5,0,&displayTime);
		#endif
	}else if(_currentUnitSetting==6)
		editItem(STR(Pump_PreMash),value,1,0,&displayOnOff);
	else if(_currentUnitSetting==7)
		editItem(STR(Pump_On_Mash),value,1,0,&displayOnOff);
	else if(_currentUnitSetting==8)
		editItem(STR(Pump_Mashout),value,1,0,&displayOnOff);
	else if(_currentUnitSetting==9)
		editItem(STR(Pump_On_Boil),value,1,0,&displayOnOff);
	else if(_currentUnitSetting==10){
		int max= gIsUseFahrenheit? 221:105;
		int min= gIsUseFahrenheit? 176:80;
		editItem(STR(Pump_Stop),value,max,min,&displaySimpleTemperature);
	/*else if(_currentUnitSetting==11) // ignore
	// ********* skip, for internal usage always use C
		editItem(STR(Pump_Stop),value,105,80,&displaySimpleTemperature);*/
	}else if(_currentUnitSetting==12)
		editItem(STR(PID_Pipe),value,1,0,&displayActivePassive);
	else if(_currentUnitSetting==13)
		editItem(STR(Skip_Add),value,1,0,&displayYesNo);
	else if(_currentUnitSetting==14)
		editItem(STR(Skip_Remove),value,1,0,&displayYesNo);
	else if(_currentUnitSetting==15)
		editItem(STR(Skip_Iodine),value,1,0,&displayYesNo);
	else if(_currentUnitSetting==16)
		editItem(STR(IodineTime),value,90,0,&displayTimeOff);
#if NoWhirlpool != true	
	else if(_currentUnitSetting==17)
		editItem(STR(Whirlpool),value,2,0,&displayHotColdOff);
#endif
}

// Initialization of the screen
void settingUnitSetup(void)
{
	uiButtonLabel(ButtonLabel(Up_Down_x_Ok));
	_currentUnitSetting=0;
	settingUnitDisplayItem();
}

void settingUnitEventHandler(byte)
{
	if(btnIsEnterPressed)
	{
		byte value=(byte)editItemValue();
		changeSettingValue(PS_AddrOfUnitSetting(_currentUnitSetting),value);
		
		if(_currentUnitSetting == 0 ) // degree setting
		{
			temperatureUnitChange((bool) value);
		}
		
		//goto next item
		_currentUnitSetting++;
		
		// use C only internally.
		if(_currentUnitSetting== 11 || _currentUnitSetting== 3)
			_currentUnitSetting ++;
#if NoWhirlpool == true	
		if(_currentUnitSetting== 17)
#else
		if(_currentUnitSetting== 18)
#endif
		{
			//uiClearSettingRow();
			switchApplication(SETUP_SCREEN);
			return;
		}
		settingUnitDisplayItem();
	}
	else if(btnIsUpPressed)
	{
		editItemChange(+1);
	}
	else if(btnIsDownPressed)
	{
		editItemChange(-1);
	}
	else if(btnIsUpContinuousPressed)
	{
		editItemChange(+4);
	}
	else if(btnIsDownContinuousPressed)
	{
		editItemChange(-4);
	}
}

// *************************
//*  Automation settings
// *************************


void displayStageTemperature(int value)
{
	float temperature=TempFromStorage(value);
	uiSettingShowTemperature(temperature,2);
}

byte _editingStage;
// useing stage to input number of hops & boild time
byte _editingStageAux;
// for Mash stage
//		_editingStageAux ==0 means temp
// 		_editingStageAux == 1 means time
// for hop time
//    it is number of hop
//    

int _maxHopTime; // to make sure hop time is in order
				  // will be set at BOIL time setting
				  // and every hoptime
byte _hopNumber;
#define MAX_STAGE_TIME 140
#define MIN_STAGE_TIME 1

void settingAutomationDisplayItem(void)
{
	int value;
	
	if(_editingStage <=7) // from MashIn,Phytase,Glucanase,Protease,bAmylase,aAmylase1,aAmylase2,MashOut
	{
		if(_editingStageAux==0){
			value = readSettingWord(PS_StageTemperatureAddr(_editingStage));
			// round to .25
			value = ToTempInStorage(round025(TempFromStorage(value)));
		}else{
			value =readSetting(PS_StageTimeAddr(_editingStage));
			if (value==0) value=1;
		}
	}
	// else read value later
		//8. number of hops
		//9. boil time
		//10. time hop number #
	
	if( _editingStageAux == 1   // 1:time editing of stage 1 to 5, step 6 has no End, nor More optoin
		&& _editingStage>0 && _editingStage < 6) // except MashIn/MashOut, and in Temperature editing
		uiButtonLabel(ButtonLabel(Up_Down_End_More));
	else
		uiButtonLabel(ButtonLabel(Up_Down_x_Ok));
	if(_editingStage ==0)
	{
		int max=(gIsUseFahrenheit)? ToTempInStorage(167):ToTempInStorage(75);
		int min=(gIsUseFahrenheit)? ToTempInStorage(68):ToTempInStorage(20);
		// Mash In:temp only
		editItem(STR(Mash_In),value,max,min,&displayStageTemperature);
	}

	else if(_editingStage >0 && _editingStage < 7)
	{
		int max=(gIsUseFahrenheit)? ToTempInStorage(169):ToTempInStorage(76);
		int minTemp;
		
		if(_editingStage==1){
			minTemp=(gIsUseFahrenheit)? ToTempInStorage(77):ToTempInStorage(25);
		}else{
			minTemp=readSettingWord(PS_StageTemperatureAddr(_editingStage-1));
		}
		
		if (_editingStageAux == 0){
			editItem(STR(Mash_x),value,max,minTemp,&displayStageTemperature);
		}else{
			editItem(STR(Mash_x),value,MAX_STAGE_TIME,MIN_STAGE_TIME,&displayTime);
		}		
		editItemTitleAppendNumber(_editingStage); 
	}

	else if(_editingStage ==7)
	{
		// MashOut
		if (_editingStageAux == 0){
			int max=(gIsUseFahrenheit)? ToTempInStorage(176):ToTempInStorage(80);
			int min=(gIsUseFahrenheit)? ToTempInStorage(167):ToTempInStorage(75);
		
			editItem(STR(Mash_out),value,max,min,&displayStageTemperature);
		}
		else
			editItem(STR(Mash_out),value,MAX_STAGE_TIME,MIN_STAGE_TIME,&displayTime);	
	}
	else if(_editingStage ==8)
	{
		// 8. number of hops
		value =readSetting(PS_NumberOfHops);
		// boiling, need to input 
		editItem(STR(Number_Of_Hops),value,10,0,&displaySimpleInteger);
	}
	else if(_editingStage ==9)
	{
		// 9. boil time
		value =readSetting(PS_BoilTime);
		// boiling, need to input 
		editItem(STR(Boil),value,MAX_STAGE_TIME,MIN_STAGE_TIME,&displayTime);
	}
	else //if(_editingStage ==10)
	{
		//10. hops
		value=readSetting(PS_TimeOfHop(_editingStageAux));
		
		if(value>_maxHopTime) value=_maxHopTime;
		//create a number
		// hop number starts from 1
		
		editItem(STR(Hops_Number_x),value,_maxHopTime,0,&displayTime);
		editItemTitleAppendNumber(_editingStageAux+1); // 
	}
}

void settingAutoSetup(void)
{
	_editingStage=0;
	_editingStageAux=0;
	settingAutomationDisplayItem();
}

void settingAutoEventHandler(byte)
{
	if(btnIsEnterPressed)
	{
		int value=editItemValue();
		
		if(_editingStage ==0)
		{			
			
			_editingStageAux=0;
		}
		if(_editingStage <= 7)
		{
			if(_editingStageAux ==0)
			{
				changeAutomationTemperature(_editingStage,value);
			
				if(_editingStage==0) // no time needed for Mash In
					_editingStage++;
				else
					_editingStageAux=1;
			}
			else
			{
				changeAutomationTime(_editingStage,(byte)value);
				//next stage	
				_editingStageAux=0;
				_editingStage++;
			}
		}
		else if(_editingStage == 8)
		{
			updateSetting(PS_NumberOfHops,(byte)value);
			//number of hops
			_editingStage++;
		}
		else if(_editingStage == 9)
		{
			updateSetting(PS_BoilTime,(byte)value);
			
			// set the maxHopTime for the first hop
			_maxHopTime=value;
			// boiling time;
			
			_editingStageAux=0;
			int hopsNum=readSetting(PS_NumberOfHops);
			if(hopsNum){
				_editingStage++;
			
			}else{
				finishAutomationEdit();
				
				//uiClearSettingRow();
				switchApplication(SETUP_SCREEN);
				return;
			}
		}
		else // if(_editingStage == 10)
		{
			updateSetting(PS_TimeOfHop(_editingStageAux),(byte)value);
			
			// update masxHoptime for next hop
			_maxHopTime = value-1;
			
			int hopsNum=readSetting(PS_NumberOfHops);
			
			if(_editingStageAux == (hopsNum-1))
			{
				//finish
				finishAutomationEdit();
				
				//uiClearSettingRow();
				switchApplication(SETUP_SCREEN);
				return;
			}
			else
			{
				_editingStageAux++;

			}
		}
		//next item
		settingAutomationDisplayItem();
	}
	else if(btnIsStartPressed)
	{
		// only handle in stage 1 to 5
		if((_editingStage >=1 && _editingStage <6)
			&& _editingStageAux == 1 )
		{
			int value=editItemValue();
			changeAutomationTime(_editingStage,(byte)value);
			// End Mash step and go to MashOut
			changeAutomationTime(_editingStage +1,(byte)0);
			_editingStage = 7;
			_editingStageAux=0;
			settingAutomationDisplayItem();
		}
	}
	else if(btnIsUpPressed)
	{
		if(_editingStage <8 && _editingStageAux == 0)
			editItemChange(+4);
		else
			editItemChange(+1);
	}
	else if(btnIsDownPressed)
	{
		if(_editingStage <8 && _editingStageAux == 0)
			editItemChange(-4);
		else
			editItemChange(-1);
	}
	else if(btnIsUpContinuousPressed)
	{
		if(_editingStage <8 && _editingStageAux == 0)
			editItemChange(+12);
		else
			editItemChange(+4);
	}
	else if(btnIsDownContinuousPressed)
	{
		if(_editingStage <8 && _editingStageAux == 0)
			editItemChange(-12);
		else
			editItemChange(-4);
	}
}// end of void settingAutoEventHandler(byte)


// *************************
//*  Sensor setup
// *************************
#if MaximumNumberOfSensors > 1
#define  _sensorSettingIndex _editingStage
#define _sensorSettingAux _editingStageAux

bool _sensorSelected[MaximumNumberOfSensors];
byte _sensorSelection;
/* helper functions */
void resetSelection(byte sensorNumber)
{
	for(byte i=0;i<MaximumNumberOfSensors;i++){
		_sensorSelected[i] = (i<sensorNumber)? false:true;
	}
}
int nextUp(byte i)
{
	int idx=i;
	idx --;
	for(;idx>=0;idx--){
		if(_sensorSelected[idx] == false) return idx;
	}
	return -1;
}

int firstAvailable(void)
{
	int idx = 0;
	for(;idx<gSensorNumber;idx++){
		if(_sensorSelected[idx] == false) return idx;
	}
	return -1;
}

int nextDown(byte i)
{
	int idx = i;
	idx ++;
	
	for(;idx<gSensorNumber;idx++){
		if(_sensorSelected[idx] == false) return idx;
	}
	return -1;
}

/* helper functions*/

void sensorMenuSelectSensorButtonChange(void)
{
	bool up=(nextUp(_sensorSelection) >=0);
	bool down= (nextDown(_sensorSelection) >=0);

	if(up && down) uiButtonLabel(ButtonLabel(Up_Down_x_Ok));
	else if( down) uiButtonLabel(ButtonLabel(x_Down_x_Ok));
	else if( up) uiButtonLabel(ButtonLabel(Up_x_x_Ok));
	else uiButtonLabel(ButtonLabel(x_x_x_Ok));
}

void sensorMenuItem(void)
{
	if(_sensorSettingIndex ==0)
	{
		if(gSensorNumber==0){
			uiSettingTitle(STR(No_Sensor_Found));
			uiButtonLabel(ButtonLabel(x_x_x_Ok));
			return;
		}
		//else
		_sensorSelection=(byte)firstAvailable();
		
		uiSettingSensorIndex(_sensorSettingAux); // print #1
		uiSettingSensorAddress(gSensorAddresses[_sensorSelection],gTemperatureReading[_sensorSelection]);
		sensorMenuSelectSensorButtonChange();
	}
	else 
	{
		if(_sensorSettingIndex ==(SensorForIdle+1)){
			if(_sensorSettingAux==0)
				uiSettingTitle(STR(Sensor_Idle));
			else
				uiSettingTitle(STR(AuxSensor_Idle));

		}else if(_sensorSettingIndex ==(SensorForPreMash+1)){
			if(_sensorSettingAux==0)
				uiSettingTitle(STR(Sensor_PreMash));
			else
				uiSettingTitle(STR(AuxSensor_PreMash));
		}else if(_sensorSettingIndex ==(SensorForMash+1)){
			if(_sensorSettingAux==0)
				uiSettingTitle(STR(Sensor_Mash));
			else
				uiSettingTitle(STR(AuxSensor_Mash));
		}else if(_sensorSettingIndex ==(SensorForBoil+1)){
			if(_sensorSettingAux==0)
				uiSettingTitle(STR(Sensor_Boil));
			else
				uiSettingTitle(STR(AuxSensor_Boil));

		}else  if(_sensorSettingIndex ==(SensorForCooling+1)){  
			if(_sensorSettingAux==0)
				uiSettingTitle(STR(Sensor_Cooling));
			else
				uiSettingTitle(STR(AuxSensor_Cooling));
		}else if(_sensorSettingIndex ==(SensorForManual+1)){
			if(_sensorSettingAux==0)
				uiSettingTitle(STR(Sensor_Manual));
			else
				uiSettingTitle(STR(AuxSensor_Manual));
		}
		int s=firstAvailable();
		_sensorSelection=(s>=0)? s:0;
		uiSettingDisplayNumber((float)_sensorSelection+1,0);
		sensorMenuSelectSensorButtonChange();
	}
}

void sensorMenuSetup(void)
{
	_sensorSettingIndex=0;
	_sensorSettingAux=0;
	gSensorNumber=scanSensors(MaximumNumberOfSensors,gSensorAddresses);
	for(byte i=0;i<MaximumNumberOfSensors;i++)
		gSensorCalibrations[i]=0;

	resetSelection(gSensorNumber);
	sensorMenuItem();
}

void saveSensor(byte idx,byte address[])
{
	int addr=PS_SensorAddressOf(idx);
	
	for(byte i=0;i<8;i++){
		updateSetting(addr+i,address[i]);
//		byte readback=readSetting(addr+i);
//		if(readback != address[i]) Serial.printf("Error: write failed?");
	}
	char pbuf[20];
	printSensorAddress(pbuf,address);
	Serial.printf("Save sensor %d 0x%s to %d\n",idx,pbuf,addr);	
}


void sensorMenuEventHandler(byte)
{
	if(_sensorSettingIndex==0  && gSensorNumber==0)
	{
		if(btnIsEnterPressed){			
			//uiClearSettingRow();
			switchApplication(SETUP_SCREEN);
		}
		return;
	}
		
	if(_sensorSettingIndex==0)
	{
		if(btnIsUpPressed || btnIsDownPressed){
			int next = (btnIsUpPressed)? nextUp(_sensorSelection):nextDown(_sensorSelection);
			if(next>= 0)
			{
				_sensorSelection=(byte) next;
				uiSettingSensorAddress(gSensorAddresses[_sensorSelection],gTemperatureReading[_sensorSelection]);
				sensorMenuSelectSensorButtonChange();

			}
		}else if(btnIsEnterPressed){
			saveSensor(_sensorSettingAux,gSensorAddresses[_sensorSelection]);
			updateSetting(CalibrationAddressOf(_sensorSettingAux),50); // zero
			gSensorCalibrations[_sensorSettingAux] = 0;
			_sensorSettingAux++;
			
			_sensorSelected[_sensorSelection]=true;
			
#if SerialDebug == true
		Serial.print("_sensorSettingAux:");
		Serial.print(_sensorSettingAux);
		Serial.print(" gSensorNumber=");
		Serial.println(gSensorNumber);
#endif

			if(_sensorSettingAux >= gSensorNumber){
				// next item
				_sensorSettingIndex ++;
				_sensorSettingAux=0;
				resetSelection(gSensorNumber);
			}
			sensorMenuItem();
		}
	}
	else // all after
	{
		if(btnIsUpPressed || btnIsDownPressed){
			int next = (btnIsUpPressed)? nextUp(_sensorSelection):nextDown(_sensorSelection);
			if(next >= 0)
			{
				_sensorSelection  =(byte) next;
				uiSettingDisplayNumber((float)_sensorSelection+1,0);
				sensorMenuSelectSensorButtonChange();
			}
		}else if(btnIsEnterPressed){
			// save
			if(_sensorSettingAux ==0){
				changeSettingValue(PS_SensorUseAddressOf(_sensorSettingIndex -1),_sensorSelection);
				_sensorSettingAux=1;
				_sensorSelected[_sensorSelection]=true;
			}else{
				changeSettingValue(PS_AuxSensorAddressOf(_sensorSettingIndex -1),_sensorSelection);
				_sensorSettingIndex++;
				_sensorSettingAux=0;
				resetSelection(gSensorNumber);
			}
			
			if(_sensorSettingIndex > NumberOfSensorStage)
			{
				//uiClearSettingRow();
				switchApplication(SETUP_SCREEN);
				commitSetting();
				// reaload sensor
				loadSensorSetting();
				return;
			}
			sensorMenuItem();
		}
	}
}
#endif //MaximumNumberOfSensors

#if SpargeHeaterSupport == true
#define  _spargeSettingIndex _editingStage

#define SpargeEnableIndex 0
#define SpargeEnableTemperatureControlIndex 1
#define SpargeSensorSelectIndex 2
#define SpargeTemperatureIndex 3
#define SpargeTempDiffIndex 4


void spargeMenuItem(void)
{
	byte value;
	if(_spargeSettingIndex == SpargeEnableIndex){
		value =readSetting(PS_SpargeWaterEnableAddress);
		editItem(STR(Enable),value,1,0,&displayYesNo);
		uiButtonLabel(ButtonLabel(Up_Down_x_Ok));
	}
	#if MaximumNumberOfSensors >1
	else if(_spargeSettingIndex == SpargeEnableTemperatureControlIndex){
		value =readSetting(PS_SpargeWaterTemperatureControlAddress);
		// if only one sensor available. no temperature control is possible.
		editItem(STR(Temp_Ctrl),value,(gSensorNumber>1)? 1:0,0,&displayYesNo);
	}else  if(_spargeSettingIndex == SpargeSensorSelectIndex){

		value =readSetting(PS_SpargeWaterSensorIndexAddress) +1;
		editItem(STR(Sparge_Sensor),value,gSensorNumber,1,&displaySimpleInteger);

	}else  if(_spargeSettingIndex == SpargeTemperatureIndex){

		value =readSetting(PS_SpargeWaterTemperatureAddress);
		
		int max=(gIsUseFahrenheit)? ToTempInStorage(176):ToTempInStorage(80);
		int min=(gIsUseFahrenheit)? ToTempInStorage(167):ToTempInStorage(75);

		editItem(STR(Sparge_Temp),value,max,min,&displaySimpleTemperature);

	}else  if(_spargeSettingIndex == SpargeTempDiffIndex){

		value =readSetting(PS_SpargeWaterTemperatureDifferenceAddress);
		
		editItem(STR(Temp_Diff),value,20,5,&displayTempDivide10);

	}
	#endif
}

void spargeMenuSetup(void)
{
	_spargeSettingIndex=0;
	spargeMenuItem();
}

void spargeMenuEventHandler(byte)
{
	byte value=(byte)editItemValue();
	
	if(btnIsEnterPressed)
	{	
		if(_spargeSettingIndex == SpargeEnableIndex){
			changeSettingValue(PS_SpargeWaterEnableAddress,value);
			
			#if MaximumNumberOfSensors >1
			if(!value){ // no enable
			#endif
				switchApplication(SETUP_SCREEN);
				commitSetting();
				return;
			#if MaximumNumberOfSensors >1
			}
			#endif
		}	
#if MaximumNumberOfSensors >1
		else if(_spargeSettingIndex == SpargeEnableTemperatureControlIndex){
		
			changeSettingValue(PS_SpargeWaterTemperatureControlAddress,value);
			if(!value){
				// no temp control. finished
				switchApplication(SETUP_SCREEN);
				commitSetting();
				return;
			}
		}else  if(_spargeSettingIndex == SpargeSensorSelectIndex){

			changeSettingValue(PS_SpargeWaterSensorIndexAddress,value -1);

		}else  if(_spargeSettingIndex == SpargeTemperatureIndex){

			changeSettingValue(PS_SpargeWaterTemperatureAddress,value);

		}else  if(_spargeSettingIndex == SpargeTempDiffIndex){

			changeSettingValue(PS_SpargeWaterTemperatureDifferenceAddress,value);
			switchApplication(SETUP_SCREEN);
			commitSetting();
		}

		_spargeSettingIndex++;
		spargeMenuItem();
#endif //#if MaximumNumberOfSensors >1
	}
	else if(btnIsUpPressed)
	{
		editItemChange(+1);
	}
	else if(btnIsDownPressed)
	{
		editItemChange(-1);
	}
}
#endif //#if SpargeHeaterSupport == true
// *************************
//*  Level 1 Menu (settings)
// *************************

str_t const level1Menu[]={STR(PID_PWM),STR(Unit_Parameters),STR(Set_Automation)
#if MaximumNumberOfSensors > 1
,STR(Sensor_Setting)
#endif
#if SpargeHeaterSupport == true
,STR(Sparge_Heater)
#endif

};
const ScreenIdType level1Screens[]={
	PID_SETTING_SCREEN,
	UNIT_SETTING_SCREEN,
	AUTO_SETTING_SCREEN

#if MaximumNumberOfSensors > 1
	,SENSOR_SCREEN
#endif

#if SpargeHeaterSupport == true
	,SPARGE_SETTING_SCREEN
#endif
};

void menuDisplayList(byte index)
{
	byte menuNO=(sizeof(level1Menu) / sizeof(char const*)) -1;

	uiSubTitle(level1Menu[index]);
	if(index ==0)
		uiButtonLabel(ButtonLabel(x_Down_Quit_Ok));
	else if(index == menuNO)
		uiButtonLabel(ButtonLabel(Up_x_Quit_Ok));
	else 
		uiButtonLabel(ButtonLabel(Up_Down_Quit_Ok));
}

byte _currentLevelOne=0;
void menuSetup(void)
{
	uiTitle(STR(Setup));	
//	_currentLevelOne=0; "remember" last menu position
	menuDisplayList(_currentLevelOne);
	
	#if WirelessSupported == true
	wiReportCurrentStage(StageSetting);
	#endif
}

void menuEventHandler(byte event)
{
	if(btnIsEnterPressed)
	{
		//go to level 2
		switchApplication(level1Screens[_currentLevelOne]);
	}
	else if(btnIsStartPressed)
	{
		// got to main
		backToMain();	
	}
	else if(btnIsUpPressed)
	{
		if(_currentLevelOne>0)
		{
			_currentLevelOne--;
			menuDisplayList(_currentLevelOne);
		}
	}
	else if(btnIsDownPressed)
	{
		
		byte menuNO=(sizeof(level1Menu) / sizeof(char const*)) -1;
				
		if(_currentLevelOne < menuNO)
		{
			_currentLevelOne++;
			menuDisplayList(_currentLevelOne);
		}
	}
}

// ***************************************************************************
//*  Common function to handle Setting temperature
//*
// ***************************************************************************

float _maxAdjustTemp;
float _minAdjustTemp;

void togglePwmInput(void)
{
			//turn on/off PWM
	if(gCurrentTemperature >= gSettingTemperature
				&& gCurrentTemperature >= gBoilStageTemperature)
	{
		if(!gIsEnterPwm)
		{
//			DBG_PRINTF("togglePwmInput\n");
			uiShowPwmLabel();
			uiShowPwmValue(gBoilHeatOutput);
			gIsEnterPwm=true;
			#if WirelessSupported
			wiReportPwm();
			wiTogglePwm();
			#endif
		}
	}
	else
	{
		if(gIsEnterPwm)		
		{
//			DBG_PRINTF("Hide PWM\n");
			// turn off
			uiClearPwmDisplay();
			gIsEnterPwm = false;
			#if WirelessSupported
			wiTogglePwm();
			#endif
		}
	}
}

void setAdjustTemperature(float max, float min)
{
	_maxAdjustTemp=max;
	_minAdjustTemp=min;
}

void adjustSp(float adjust)
{
	gSettingTemperature += adjust;
	
	if(gSettingTemperature > _maxAdjustTemp) gSettingTemperature=_maxAdjustTemp;
	if(gSettingTemperature < _minAdjustTemp) gSettingTemperature=_minAdjustTemp;
	
	uiDisplaySettingTemperature(gSettingTemperature);
	brewLogger.setPoint(gSettingTemperature);

	// if adjust above delta
		
	#if WirelessSupported == true
	wiReportSettingTemperature();
	#endif

}
void adjustPwm(int adjust)
{
	// gBoilHeatOutput is byte, which is, uhhh,, unsigned
	
	// excludes the case adjust <0 and gBoilheatOutput ==0
	if(adjust > 0 || gBoilHeatOutput !=0)
		gBoilHeatOutput += adjust;
	
	if(gBoilHeatOutput > 100) gBoilHeatOutput=100;
	
	uiShowPwmValue(gBoilHeatOutput);
	DBG_PRINTF("adjustPwm\n");	
	#if WirelessSupported == true
	wiReportPwm();
	#endif
}

boolean processAdjustButtons(void)
{
	if(btnIsUpPressed)
	{
		if(gIsEnterPwm)
			adjustPwm(+1);
		else
			adjustSp(+0.25);
	}
	else if(btnIsDownPressed)
	{
		if(gIsEnterPwm)
			adjustPwm(-1);
		else
			adjustSp(-0.25);
	}
	else if(btnIsUpContinuousPressed)
	{
		if(gIsEnterPwm)
			adjustPwm(+2);
		else
			adjustSp(+0.75);
	}
	else if(btnIsDownContinuousPressed)
	{
		if(gIsEnterPwm)
			adjustPwm(-2);
		else
			adjustSp(-0.75);
	}
	else
	{
		return false;
	}
	return true; // handled
}

// ***************************************************************************
//*  Manual Mode Screen
//*
// ***************************************************************************

#define DEFAULT_MANUL_MODE_TEMPERATURE (gIsUseFahrenheit)? 95:35
//states variables

#define MSAskWater 0
#define MSManualMode 1
#define MSAskAutoTune 2
#define MSRunningAutoTune 3
#define MSAskExitingAutoTune 4

// display "Water Added?"

byte _state;

#if SupportManualModeCountDown == true
unsigned long manualModeChangeCountDownTime; 
bool isCountDownTimeBlinking;
bool isManualModeCountDownMode;
#endif
//
void manualModeSetup(void)
{
	uiTitle(STR(Manual_Mode));
	uiSubTitle(STR(Water_Added));
	uiButtonLabel(ButtonLabel(Continue_Yes_No));
	_state = MSAskWater;
	gIsEnterPwm=false;
	
	gSettingTemperature = DEFAULT_MANUL_MODE_TEMPERATURE;
	#if WirelessSupported == true
	wiReportSettingTemperature();
	#endif

	#if MaximumNumberOfSensors > 1
	setSensorForStage(SensorForManual);
	#endif
}

#define TEMPERATURE_ADJUST_THRESHOLD 2.0

//function after confirmation of adding water 
void manualModeEnterManualMode(void)
{
	uiClearSubTitleRow();
	uiButtonLabel(ButtonLabel(Up_Down_Heat_Pmp));
	
	// Setpoint temperature
	uiDisplaySettingTemperature(gSettingTemperature);
	// displace current temperature
	uiTempDisplaySetPosition(TemperatureManualModePosition);
	// display counting time
	uiRunningTimeSetPosition(RunningTimeNormalPosition);
	uiRunningTimeShowInitial(0);	
	
	if(gCurrentTemperature >=gSettingTemperature)
	{
		//temp reached
		gIsTemperatureReached=true;
		uiRunningTimeStart();
	}
	else
	{
		gIsTemperatureReached=false;
		// wait to reach temperature setting
	}
	
	#if SupportManualModeCountDown == true
	setEventMask(TemperatureEventMask | ButtonPressedEventMask | TimeoutEventMask);
	#endif
	if(gIsUseFahrenheit)
		setAdjustTemperature(230.0,68.0);
	else
		setAdjustTemperature(110.0,20.0);
	
	#if WirelessSupported == true
	wiReportCurrentStage(StageManualMode);
	wiReportSettingTemperature();
	#endif
	
}

void finishAutoTuneBackToManual(void)
{
	_state = MSManualMode;
	
	gSettingTemperature =gIsUseFahrenheit? 95:35;
	uiDisplaySettingTemperature(gSettingTemperature);
	
	uiRunningTimeShowInitial(0);
	
	uiTitle(STR(Manual_Mode));
	uiButtonLabel(ButtonLabel(Up_Down_Heat_Pmp));
	
	if(gIsHeatOn) heatOff();
	if(gIsPumpOn) pumpOff();
	
	brewLogger.stage(StageManualMode);
	brewLogger.setPoint(gSettingTemperature);

	#if WirelessSupported == true
	wiReportCurrentStage(StageManualMode);
	wiReportSettingTemperature();
	#endif
}

void manualModeEventHandler(byte event)
{

	if(_state == MSAskWater)
	{
		//only button event will come before
		// we change it
		//if(event != ButtonPressedEventMask) return;
		
		// cares only button start & enter
		if(btnIsStartPressed)
		{
			// yes.
			//load heating parameters
			loadBrewParameters();
			manualModeEnterManualMode();
			_state = MSManualMode;
			// setup to be called for Timer & temperature event			
			setEventMask(ButtonPressedEventMask|TemperatureEventMask|TimeoutEventMask);

			#if MaximumNumberOfSensors > 1
			brewLogger.startSession(gSensorNumber,TemperatureChartPeriod,gIsUseFahrenheit,false);
			#else
			brewLogger.startSession(1,TemperatureChartPeriod,gIsUseFahrenheit,false);
			#endif
			
			brewLogger.stage(StageManualMode);
			brewLogger.setPoint(gSettingTemperature);

			#if MaximumNumberOfSensors > 1
			brewLogger.temperatures(gTemperatureReading);
			#else
			brewLogger.temperature(gCurrentTemperature);
			#endif
		}
		else if(btnIsEnterPressed)
		{
			// NO. back to main
			switchApplication(MAIN_SCREEN);
		}
	}
#if	EnablePidAutoTune == true	
	else if(_state == MSAskAutoTune)
	{
		if(event != ButtonPressedEventMask) return;

		if(btnIsStartPressed)
		{
			// NO, back to manual mode
			_state = MSManualMode;
			uiButtonLabel(ButtonLabel(Up_Down_Heat_Pmp));	
		}
		else if(btnIsEnterPressed)
		{
			// YES, enter Auto tune mode
			_state = MSRunningAutoTune;

			uiTitle(STR(PID_AUTOTUNE));
			uiButtonLabel(ButtonLabel(x_x_Exit_Pmp));
			gSettingTemperature = gCurrentTemperature;
			uiDisplaySettingTemperature(gSettingTemperature);

			uiRunningTimeShowInitial(0);
			uiRunningTimeStart();

			startAutoTune();
			heatOn(true);

			brewLogger.stage(StagePIDAutoTune);
			brewLogger.setPoint(gSettingTemperature);
			
			#if WirelessSupported == true
			wiReportCurrentStage(StagePIDAutoTune);
			wiReportSettingTemperature();
			#endif
		}
	}
	else if(_state == MSRunningAutoTune)
	{
		if(event == ButtonPressedEventMask)
		{
			if(btnIsStartPressed)
			{
				//Exit
				_state = MSAskExitingAutoTune;
				uiButtonLabel(ButtonLabel(Stop_No_Yes));
			}
			else if(btnIsEnterPressed)
			{
				// Pump
				if(gIsPumpOn) pumpOff();
				else pumpOn();
			}
		}
		if(!_isRunningAutoTune)
		{
			// auto tuning finished
			buzzPlaySound(SoundIdAutoTuneFinished);
			finishAutoTuneBackToManual();
		}
	}
	else if(_state == MSAskExitingAutoTune)
	{
		if(event != ButtonPressedEventMask) return;

		if(btnIsStartPressed)
		{
			//No
			_state = MSRunningAutoTune;
			uiButtonLabel(ButtonLabel(x_x_Exit_Pmp));
		}
		else if(btnIsEnterPressed)
		{
			// YES
			// back to Manual Mode.
			cancelAutoTune();
			finishAutoTuneBackToManual();			
		}		
	}
#endif //#if	EnablePidAutoTune == true
	else if(_state == MSManualMode) // NOT Wait for Water State
	{
		// states other than MSAskWater, handle Heat & Pump button
		if(event == ButtonPressedEventMask)
		{
			if(btnIsStartPressed)
			{
#if	EnablePidAutoTune == true
				if(btnIsStartLongPressed)
				{
					uiButtonLabel(ButtonLabel(Tune_PID_No_Yes));
					_state = MSAskAutoTune;
					if(gIsHeatOn) heatOff();
					if(gIsPumpOn) pumpOff();
				}
				else
#endif
				{
					//turn heating on/off
					if(gIsHeatOn) heatOff();
					else heatOn(true);
				}
			}
			else if(btnIsEnterPressed)
			{
				if(btnIsEnterLongPressed)
				{
					pumpRestSetEnabled(!pumpRestIsEnabled());
				}
				else
				{
					// turn pump on/off
					if(gIsPumpOn) pumpOff();
					else pumpOn();
				}
			}
			#if SpargeHeaterSupport
			else if(isExactButtonsPressed(ButtonDownMask | ButtonStartMask))
			{
				if(gHeatSpargeWater) stopHeatingSpargeWater();
				else startHeatingSpargeWater();
			}
			#endif 
			#if SupportManualModeCountDown == true
			else if(isExactButtonsPressed(ButtonEnterMask | ButtonStartMask))
			{
				isCountDownTimeBlinking = ! isCountDownTimeBlinking;
				if(isCountDownTimeBlinking)
				{
					uiRunningTimeShowInitial(0);
					manualModeChangeCountDownTime=0;
					tmPauseTimer();
					gIsTemperatureReached=true; // force to ignore temperature 
				}
				else
				{
					// end of input. if zero, make it normal mode
					// else make it count down mode
					isManualModeCountDownMode=(manualModeChangeCountDownTime > 0);
					gIsTemperatureReached=false;
				}
				uiRunningTimeBlink(isCountDownTimeBlinking);

			}
			else if(isCountDownTimeBlinking)
			{
				if(btnIsUpPressed)
				{
					if(manualModeChangeCountDownTime< 140)
					{
						manualModeChangeCountDownTime ++;
						uiRunningTimeShowInitial(manualModeChangeCountDownTime * 60);
					}
				}
				else if(btnIsDownPressed)
				{
					if(manualModeChangeCountDownTime> 0)
					{
					 	manualModeChangeCountDownTime --;
					 	uiRunningTimeShowInitial(manualModeChangeCountDownTime * 60);
					}
				}
			}
			#endif
			else
			{
				if(processAdjustButtons())
				{
					if( gIsTemperatureReached
							&&	gSettingTemperature >= (TEMPERATURE_ADJUST_THRESHOLD + gCurrentTemperature))
					{
						#if SupportManualModeCountDown == true

						// pause counting, reset it

						if(isManualModeCountDownMode)
						{
							uiRunningTimeShowInitial(manualModeChangeCountDownTime * 60);
						}
						else
						#endif
							uiRunningTimeShowInitial(0);
						gIsTemperatureReached = false;
					}
				}
			}
		} 
		else if(event == TemperatureEventMask)
		{
			// Handle temperature change or other states 
			//
			if (! gIsTemperatureReached)
			{
				if(gCurrentTemperature >= gSettingTemperature)
				{	
					// beep & start counting time
					
					buzzPlaySound(SoundIdTemperatureReached);
					
					gIsTemperatureReached=true;
					
					#if SupportManualModeCountDown == true
					if(isManualModeCountDownMode)
					{
						uiRunningTimeStartCountDown(manualModeChangeCountDownTime*60);
						tmSetTimeoutAfter(manualModeChangeCountDownTime*60 * 1000);
					}
					else
					#endif
					uiRunningTimeStart();

					brewLogger.event(RemoteEventTemperatureReached);
					#if WirelessSupported == true
					wiReportEvent(RemoteEventTemperatureReached);
					#endif
				}
			}
			// Temperate Reached state				
			
			togglePwmInput();

		} // end of temperature handling
		#if SupportManualModeCountDown == true
		else if(event == TimeoutEventMask)
		{
			buzzPlaySound(SoundIdCountDown);
			isManualModeCountDownMode=false;
			uiRunningTimeStart();
		}
		#endif
	} // else of if(_state == MSAskWater)
}//void manualModeEventHandler(byte event)

// *************************
//*  Auto Mode Screen
// *************************

// delay_start_q
// (resume ?)
// water_added_q
// pmmp Prime
// input delay

// use the same variable  with maunal mode 
//byte _state;

#define AS_AskDelayStart 	0
#define AS_AskResume   		1
#define AS_AskWaterAdded 	2
#define AS_PumpPrime 	 	3
#define AS_DelayTimeInput   4
#define AS_DelayTimeConfirm 5
#define AS_DelayWaiting     6

#define AS_DoughIn			7
#define AS_MashInAskContinue   8
#define AS_AskAddMalt       9

#define AS_Mashing          10
//#define AS_Mashout , the same as Mashing procedure
#define AS_AskMaltRemove   11
#define AS_Boiling		   12

#define AS_Whirlpool       13
#define AS_IodineTest      14
#define AS_Pause       15
#define AS_Cooling       16
#define AS_Finished       17

#define AS_AskSpargeWaterAdded 18

#define HOP_ALTERTING_TIME 10
#define ADVANCE_BEEP_TIME 5
#define AutoStateIs(s) (_state==(s))

byte _primePumpCount;


//************************************
// for recovery
//

//this is non sense for SPIFFS #define LEVEL_FACTOR 5



//**************************
// Delay start

#if NoDelayStart == false
unsigned long _delayTime;
bool _delayRequested;
#endif

void autoModeSetup(void)
{
	loadBrewParameters();

	if(brewLogger.checkRecovery())
	{
		_state=AS_AskResume;
		uiSubTitle(STR(Resume_Process));
		uiButtonLabel(ButtonLabel(Continue_Yes_No));
		return;
	}
	//else
#if NoDelayStart == false
	if(!readSetting(PS_NoDelayStart))
	{
		_state=AS_AskDelayStart;
		_delayTime=0;
		// output Delay State
		uiTitle(STR(AutomaticMode));
		uiSubTitle(STR(Delay_Start));
		uiButtonLabel(ButtonLabel(No_Yes));
	}else
#endif
	{
		_state = AS_AskWaterAdded;
		uiTitle(STR(AutomaticMode));
		uiSubTitle(STR(Water_Added));
		uiButtonLabel(ButtonLabel(Continue_Yes_No));
	}
}

void autoModeEnterDoughIn(void)
{	
	_state = AS_DoughIn;
	// setup temperature event mask request after this.
	setEventMask(TemperatureEventMask /*| ButtonPressedEventMask */);
	
	//load temperature value	
	gSettingTemperature = TempFromStorage(readSettingWord(PS_StageTemperatureAddr(0)));

	// setup screen
	uiClearTitle();
	uiAutoModeTitle();
	uiClearSubTitleRow();

	uiAutoModeStage(0); // 0 is Mash-In
	
	// displace temperature
	uiDisplaySettingTemperature(gSettingTemperature);

	uiTempDisplaySetPosition(TemperatureAutoModePosition);

	#if MANUAL_PUMP_MASH == true
	uiButtonLabel(ButtonLabel(Up_Down_Pause_Pmp));
	#else
	uiButtonLabel(ButtonLabel(Up_Down_Pause_x));
	#endif
	// start pump, if request,
	
	if(readSetting(PS_PumpPreMash)) pumpOn();
	else pumpOff();

#if MaximumNumberOfSensors > 1
	setSensorForStage(SensorForPreMash);
#endif
	
	// start heat
	heatOn(true);	
	if(gIsUseFahrenheit)
		setAdjustTemperature(167,77);
	else
		setAdjustTemperature(75.0,25.0);
	gIsEnterPwm=false;
	
#if SpargeHeaterSupport == true
	if(gEnableSpargeWaterHeatingControl)
	{
		startHeatingSpargeWater();
	}
#endif	

	//
	
	#if MaximumNumberOfSensors > 1
		brewLogger.startSession(gSensorNumber,TemperatureChartPeriod,gIsUseFahrenheit);
	#else
		brewLogger.startSession(1,TemperatureChartPeriod,gIsUseFahrenheit);
	#endif
	
	brewLogger.stage(StageDoughIn);
	brewLogger.setPoint(gSettingTemperature);
	
	#if MaximumNumberOfSensors > 1
		brewLogger.temperatures(gTemperatureReading);
	#else
		brewLogger.temperature(gCurrentTemperature);
	#endif

	#if WirelessSupported == true
	wiReportCurrentStage(StageDoughIn);
	#endif
}


//************************************
// Mashing state
//
#if EnableExtendedMashStep

bool _mashingStageExtendEnable;
bool _mashingStageExtending;

void autoModeToggleMashExtension(void)
{
	_mashingStageExtendEnable = ! _mashingStageExtendEnable;
	uiSetMashExtensionStatus(_mashingStageExtendEnable? MashExtensionEnabled:MashExtensionNone);
}

void autoModeResetMashExtension(void)
{
	_mashingStageExtendEnable = false;
	_mashingStageExtending = false;
	uiSetMashExtensionStatus(MashExtensionNone);
}

void autoModeEnterMashingExtension(void)
{
	uiSetMashExtensionStatus(MashExtensionRunning);
	_mashingStageExtending = true;
	uiRunningTimeStart();
}
#endif // #if EnableExtendedMashStep


byte _mashingStep;
byte _numberMashingStep;

//boolean _mashingTemperatureReached;
#define _mashingTemperatureReached gIsTemperatureReached

boolean _askingSkipMashingStage;


void autoModeNextMashingStep(bool resume)
{
	
	// in autoModeEnterMashing, this value is set to zero
	// so it really starts from 1.
	_mashingStep++;

	
	byte time;
	

	if (_mashingStep > _numberMashingStep){
		// go direct to mashout
		_mashingStep = 7;
	}
	time = readSetting(PS_StageTimeAddr(_mashingStep));
	if(time==0) time=1;

	// 	if(_mashingStep > 7), mashout time will always more than 1
	
	if(_mashingStep > 0 && _mashingStep <7)
		uiAutoModeMashTitle(_mashingStep,_numberMashingStep);
	else
		uiAutoModeStage(_mashingStep);
	
	uiRunningTimeSetPosition(RunningTimeNormalPosition);
	uiRunningTimeShowInitial(time * 60);
			
	gSettingTemperature = TempFromStorage(readSettingWord(PS_StageTemperatureAddr(_mashingStep)));	
	uiDisplaySettingTemperature(gSettingTemperature);
	
	#if	MANUAL_PUMP_MASH == true
	uiButtonLabel(ButtonLabel(Up_Down_PmPus_STP));
	#else
	uiButtonLabel(ButtonLabel(Up_Down_Pause_STP));
	#endif
	_mashingTemperatureReached=false;

	if(isPumpRest())
	{
		heatOn(true);
	}
	pumpRestSetEnabled(false);
	
#if	MANUAL_PUMP_MASH == true
	if(!gManualPump)
	{
#endif
	if(_mashingStep <=6)
	{
		// pump is off at the time AddMalt
		if(readSetting(PS_PumpOnMash)) pumpOn();
		else pumpOff();
#if MaximumNumberOfSensors > 1
	setSensorForStage(SensorForMash);
#endif
		
	}
	else if(_mashingStep ==7)
	{
		if(readSetting(PS_PumpOnMashOut)) pumpOn();
		else pumpOff();
#if MaximumNumberOfSensors > 1
	setSensorForStage(SensorForMash);
#endif
	}
	
#if	MANUAL_PUMP_MASH == true
	}
#endif

	if(!resume) {
		brewLogger.stage(_mashingStep);
		brewLogger.setPoint(gSettingTemperature);
	}
	#if WirelessSupported == true
	wiReportCurrentStage(_mashingStep);
	#endif
	
#if EnableExtendedMashStep
	autoModeResetMashExtension();
#endif	
}

void autoModeGetMashStepNumber(void)
{	
	byte idx=1;
	byte time;
	while(idx < 7 && (time = readSetting(PS_StageTimeAddr(idx))) != 0)
	{
		idx++;
	}
	_numberMashingStep = idx -1; // total mash steps.
}

void autoModeEnterMashing(void)
{
	_state = AS_Mashing;
	setEventMask(TemperatureEventMask | ButtonPressedEventMask | TimeoutEventMask | PumpRestEventMask);

	_askingSkipMashingStage = false;
	_mashingStep = 0; // 0 is mash in , real mashing starts from 1, this number will be increased in 
					  // autoModeNextMashingStep() later.
	heatOn(true);

#if	MANUAL_PUMP_MASH == true
	gManualPump=false;
#endif

	autoModeGetMashStepNumber();

	autoModeNextMashingStep(false);
}

void autoModeEnterIodineTest(void)
{
	_state = AS_IodineTest;
	
	uiPreparePasueScreen(STR(IODINE_TEST));
	uiButtonLabel(ButtonLabel(x_x_Ok_x));

	byte iodineTime=readSetting(PS_IodineTime);
	if(iodineTime)
	{
		// timer, else wait until user input
		// dont' change event, just ignore the temperature event
		uiRunningTimeShowInitial(iodineTime * 60);
		// [IMPORTANT!] cast is needed
		tmSetTimeoutAfter((unsigned long)iodineTime * 60 *1000);
		buzzPlaySound(SoundIdIodineTest);
		uiRunningTimeStartCountDown(iodineTime * 60);
	}
	else
	{
		uiRunningTimeShowInitial(0);
		buzzPlaySoundRepeat(SoundIdUserInteractiveNeeded);
	}
	
	#if WirelessSupported == true
	wiReportEvent(RemoteEventIodineTest);
	#endif
	
}

void autoModeIodineTestToMashout(void)
{
	uiRunningTimeStop();
	buzzMute();
	// restore Screen

	uiClearScreen();
	
	uiAutoModeTitle();
	// temperateure position
	uiTempDisplaySetPosition(TemperatureAutoModePosition);
	//phase name, setting point, and counting time will be shown
	// in autoModeNextMashingStep()
	// restore Mashing 
	_state = AS_Mashing;
	autoModeNextMashingStep(false);
	
}
void autoModeEnterAskRemoveMalt(void)
{
	_state = AS_AskMaltRemove;
	pumpOff();
	
	uiRunningTimeStop();
	//uiClearPrompt();
	uiPrompt(STR(Remove_Malt));
	uiButtonLabel(ButtonLabel(Continue_Yes_No));
	// skip event mask, just filter it out in handling code
	
	buzzPlaySoundRepeat(SoundIdWaitUserInteraction);

//	if(!(readSetting(PS_PidPipe) == 1 && readSetting(PS_SensorType) == SensorInside))

	if(! readSetting(PS_PidPipe)) 
		heatProgramOff(); // heat off, programming

	#if WirelessSupported == true
	wiReportEvent(RemoteEventRemoveMalt);
	#endif
}

//******************************
// Pause

byte _stateBeforePause;
unsigned long _savedTime;
boolean _savedHeating;
boolean _savedPump;
void autoModePause(unsigned long time)
{	
	_stateBeforePause = _state;
	_state = AS_Pause;
	gIsPaused=true;
	// stop Heating & pump
	_savedHeating = gIsHeatOn;
	_savedPump = gIsPumpOn;
	heatOff();
	pumpOff();
	// just wait for user button
	
	uiPreparePasueScreen(STR(In_Pause));
	uiButtonLabel(ButtonLabel(x_x_Exit_x));
	
	_savedTime=time;
	
	uiRunningTimeShowInitial(_savedTime/1000);
	brewLogger.event(RemoteEventPause);
	#if WirelessSupported == true
	wiReportEvent(RemoteEventPause);
	#endif
}

void autoModeExitPause(void)
{
	// restore state
	_state = _stateBeforePause;
	gIsPaused=false;
	// restore timer, if any
	if(_savedTime > 0)
	{
		tmSetTimeoutAfter(_savedTime);
		if(_savedTime > ADVANCE_BEEP_TIME*1000)
			tmSetAuxTimeoutAfter(_savedTime -ADVANCE_BEEP_TIME*1000);
	}
	// restore screen
	uiClearScreen();
	
	uiAutoModeTitle();
	
	if(_mashingStep > 0 && _mashingStep <7)
		uiAutoModeMashTitle(_mashingStep,_numberMashingStep);
	else
		uiAutoModeStage(_mashingStep);


	// temperateure position
	uiTempDisplaySetPosition(TemperatureAutoModePosition);
	// set temperature point
	uiDisplaySettingTemperature(gSettingTemperature);

	// counting time
	uiRunningTimeSetPosition(RunningTimeNormalPosition);
	
	if(_savedTime==0)
	{
		if(_state != AS_DoughIn)
		{
			byte time = readSetting(PS_StageTimeAddr(_mashingStep));
			uiRunningTimeShowInitial(time * 60);
		}
	}
	else //if(_savedTime==0)
	{
		// temperature reached, timer already started
		// this should always NOT be MashIn
		if(_state == AS_Mashing)
		{
			uiRunningTimeShowInitial(_savedTime/1000);
			uiRunningTimeStartCountDown(_savedTime/1000);
		}
	}
		
	// menu is different for mashin & mashing
#if MANUAL_PUMP_MASH == true
	if(_state == AS_DoughIn)
		uiButtonLabel(ButtonLabel(Up_Down_Pause_Pmp));
	else
		uiButtonLabel(ButtonLabel(Up_Down_PmPus_STP));
#else
	if(_state == AS_DoughIn)
		uiButtonLabel(ButtonLabel(Up_Down_Pause_x));
	else
		uiButtonLabel(ButtonLabel(Up_Down_Pause_STP));
#endif		
	// restore heating and pump
	if(_savedHeating) heatOn(true);
	if(_savedPump) pumpOn();

	brewLogger.event(RemoteEventResume);	
	#if WirelessSupported == true
	wiReportEvent(RemoteEventResume);
	#endif
}
void autoModeEnterBoiling(void);
void autoModeMashingStageFinished(void)
{
	//[TODO:] make sure step 6 , beta 2 is non-skippable.
	if(_mashingStep < 7) // step 7 = mashout
	{
		if(_mashingStep == 6
			&& readSetting(PS_SkipIodineTest) ==0)
		{
			// before MashOut(7) and not Skip Iodine Test
			autoModeEnterIodineTest();
		}
		else
		{
			autoModeNextMashingStep(false);
		}
	}
	else
	{
		// change to boiling stage, or malt out waiting state
			if(readSetting(PS_SkipRemoveMalt))
				autoModeEnterBoiling();
			else
				autoModeEnterAskRemoveMalt();
	}
}

//*********************************
// boiling stage

//boolean _isBoilTempReached;
#define _isBoilTempReached gIsTemperatureReached

//boolean _isBoilTimerPaused;
#define _isBoilTimerPaused gIsPaused

void autoModeEnterBoiling(void)
{
#if SpargeHeaterSupport == true
	if(gEnableSpargeWaterHeatingControl)
	{
		stopHeatingSpargeWater();
	}
#endif	

	_state = AS_Boiling;
	_isBoilTempReached=false;
	_isBoilTimerPaused=false;
	gBoilStageTemperature=readSetting(PS_BoilTemp);
	//gSettingTemperature =110;//
	gSettingTemperature = gBoilStageTemperature;
	
	uiDisplaySettingTemperature(gSettingTemperature);
	// display time
	byte boilTime=readSetting(PS_BoilTime);

	brewLogger.stage(StageBoil);
	brewLogger.setPoint(gSettingTemperature);

	uiRunningTimeShowInitial(boilTime * 60);
	
	uiAutoModeStage(BoilingStage);
	uiButtonLabel(ButtonLabel(Up_Down_x_Pmp));
	
	if(readSetting(PS_PumpOnBoil)) pumpOn();
	else pumpOff();

#if MaximumNumberOfSensors > 1
	setSensorForStage(SensorForBoil);
#endif

	if(gIsUseFahrenheit)
		setAdjustTemperature(230.0,176.0);
	else
		setAdjustTemperature(110.0,80.0);
			
	gIsEnterPwm =false;
	heatOn(false); // NO need of PID, just full power until boiling
	#if WirelessSupported == true
	wiReportCurrentStage(StageBoil);	
	#endif
}



// reuse the variable 
//#define _restoreBoilingTimer _finishedTimer
#define _numHopToBeAdded _primePumpCount

void autoModeShowHopAdding(void)
{
	uiAutoModeShowHopNumber(readSetting(PS_NumberOfHops) - _numHopToBeAdded +1);
}

//#define AUX_TIMER_HOP 1

//#ifdef AUX_TIMER_HOP // try use Aux timer for HOP

bool recoveryTimer;

void autoModeAddHopNotice(void)
{
			
			// the first hop is added at the time boiling starts
			tmSetAuxTimeoutAfter(HOP_ALTERTING_TIME * 1000);
			recoveryTimer = true;
			
			autoModeShowHopAdding();
			_numHopToBeAdded --;
			buzzPlaySound(SoundIdAddHop);
			
			#if WirelessSupported == true
			wiReportEvent(RemoteEventAddHop);
			#endif			
}

unsigned long _remainingBoilTime;

void autoModeReStartBoilingTimer(void)
{
	#if SerialDebug == true
	Serial.print("Boil time:");
	Serial.println(_remainingBoilTime);
	Serial.print("_numHopToBeAdded:");
	Serial.println(_numHopToBeAdded);
	#endif


	// [IMPORTANT!] cast to (unsigned long) is needed

	tmSetTimeoutAfter(_remainingBoilTime);
			
	if(_numHopToBeAdded > 0)
	{
		byte idx=readSetting(PS_NumberOfHops) - _numHopToBeAdded;
	
		unsigned long nextHopTime=(unsigned long)readSetting(PS_TimeOfHop(idx))
									* 60 * 1000;
		unsigned long nextHopTimeout=_remainingBoilTime - nextHopTime;
		if(nextHopTimeout == 0)
		{
			// alert directly, start timer to restore
			autoModeAddHopNotice();			
		}
		else
		{
			recoveryTimer = false;
			tmSetAuxTimeoutAfter(nextHopTimeout);
		}
	}
}

void autoModeStartBoilingTimer(void)
{
	// [IMPORTANT!] cast to (unsigned long) is needed
	// NO hop adding. just start last before 

	byte boilTime=readSetting(PS_BoilTime);

	_remainingBoilTime= (unsigned long)boilTime * 60 *1000;
	
	_numHopToBeAdded =  readSetting(PS_NumberOfHops);

	autoModeReStartBoilingTimer();
}


void autoModeStartNextHopTimer(void)
{
	// it is done at hop timer expires :_numHopToBeAdded--;
	// this function is called after Screen is restored.(restore timer expires)
	
	byte lastHopIdx=readSetting(PS_NumberOfHops) - _numHopToBeAdded -1;

	
	if(_numHopToBeAdded > 0) // there are next timer
	{
		byte lastHopTime=readSetting(PS_TimeOfHop(lastHopIdx));
		
		byte nextHopTime= readSetting(PS_TimeOfHop(lastHopIdx+1));
		
		tmSetAuxTimeoutAfter(((unsigned long)(lastHopTime - nextHopTime) * 60 
								-HOP_ALTERTING_TIME)* 1000);
		recoveryTimer = false;
	}
}


void autoModeBoilingPauseHandler(void)
{
	if(_isBoilTimerPaused)
	{
		// resume
		uiButtonLabel(ButtonLabel(Up_Down_Pause_Pmp));
		uiRunningTimeStartCountDown(_remainingBoilTime/1000);
		autoModeReStartBoilingTimer();
		
		brewLogger.event(RemoteEventResume);
	}
	else
	{
		// to pause boiling timer only
		uiRunningTimeStop();
		_remainingBoilTime=tmPauseTimer();
		// in case hop reminder is running. restore the screen
		uiAutoModeStage(BoilingStage);
		uiButtonLabel(ButtonLabel(Up_Down_RUN_Pmp));
		
		brewLogger.event(RemoteEventPause);
	}
	_isBoilTimerPaused = ! _isBoilTimerPaused;
}

//#endif

//******************************
#if NoDelayStart == false
// unit is 15 min, use 12 hour, 
#define MAX_DELAY_TIME (12*60/15)

void autoModeEnterDelayTimeInput(void)
{
	_state = AS_DelayTimeInput;
	uiClearSubTitleRow();
	uiSubTitle(STR(Setting_Delay));
	uiButtonLabel(ButtonLabel(Up_Down_Quit_Ok));
	// use display time counting
	uiRunningTimeSetPosition(RunningTimeDelayInputPosition);
	uiRunningTimeShowInitial(15 *60);
	_delayTime=1; // one unit is 15min
}
#endif
//******************************
// Cooling & Whirlpool

bool _stageConfirm;

bool _coolingTempReached;

void autoModeCoolingAsk(const char* msg)
{
	
	#if WirelessSupported == true
	wiReportCurrentStage(StageCooling);	
	#endif	
	
	_stageConfirm=false;

	//dismiss Temperature & running time
	uiTempDisplayHide(); 
	uiRunningTimeStop();
	
	uiClearScreen();
	
	uiSubTitle(msg);
	
	uiButtonLabel(ButtonLabel(Continue_Yes_No));
}

void autoModeEnterCooling(unsigned long elapsed)
{
	
	uiClearPrompt();
	uiClearSubTitleRow();
	
	uiAutoModeTitle();
	uiAutoModeStage(CoolingStage);
	gSettingTemperature =gIsUseFahrenheit? 68.0:20.0;
	_coolingTempReached=false;
	// temperature at automode
	uiTempDisplaySetPosition(TemperatureAutoModePosition);
	uiDisplaySettingTemperature(gSettingTemperature);
	
	uiRunningTimeShowInitial(elapsed);
	uiRunningTimeStartFrom(elapsed);
	
	uiButtonLabel(ButtonLabel(Up_Down_END_Pmp));
	
	if(gIsUseFahrenheit)
		setAdjustTemperature(86.0,50.0);
	else
		setAdjustTemperature(30,10);
	
	gIsEnterPwm=false;

#if MaximumNumberOfSensors > 1
	setSensorForStage(SensorForCooling);
#endif	
}

#define BREW_END_STAY_DURATION 5


void autoModeBrewEnd(void)
{
	pumpOff();
	
	uiTempDisplayHide(); 
	uiRunningTimeStop();
	
	_state = AS_Finished;
	
	uiClearScreen();
	
	uiAutoModeFinishScreen();

	buzzPlaySoundRepeat(SoundIdBrewEnd);
	tmSetTimeoutAfter(BREW_END_STAY_DURATION * 1000);

	brewLogger.endSession();
	
	#if WirelessSupported == true
	wiReportEvent(RemoteEventBrewFinished);
	#endif
}


#if NoWhirlpool != true
#define MAX_WHIRLPOOL_TIME 10
#define MIN_WHIRLPOOL_TIME 1

unsigned long _whirlpoolTime;
boolean _whirlpoolInput;
boolean _pumpRunning;

void autoModeWhirlpoolInputTime(void)
{
	_whirlpoolTime=3;
	_whirlpoolInput=true;

	uiTempDisplayHide();
	uiClearScreen();
	
	uiSubTitle(STR(Timeing_Whirlpool));
	uiRunningTimeShowInitial(_whirlpoolTime * 60);
	uiButtonLabel(ButtonLabel(Up_Down_Quit_Ok));	
}

void autoModeWhirlpool(byte elapsed)
{
	uiClearSubTitleRow();
	uiClearPrompt();

	uiAutoModeTitle();
	uiAutoModeStage(WhirlpoolStage);
	if(readSetting(PS_Whirlpool)== WhirlpoolCold) gSettingTemperature =(gIsUseFahrenheit)? 86:30;
	else gSettingTemperature = gIsUseFahrenheit? 185:85;
	
	// temperature at automode
	uiTempDisplaySetPosition(TemperatureAutoModePosition);
	uiDisplaySettingTemperature(gSettingTemperature);
	unsigned long time= (unsigned long)(_whirlpoolTime - elapsed) * 60;
	uiRunningTimeShowInitial( time);
	uiButtonLabel(ButtonLabel(x_x_x_Pmp));
	
	_pumpRunning = true;
	pumpOn();
	tmSetTimeoutAfter((unsigned long)time *1000);
	uiRunningTimeStartCountDown(time);

	#if WirelessSupported == true
	wiReportCurrentStage(StageWhirlpool);	
	#endif
	
}

void autoModeWhirlpoolFinish(void)
{
	pumpOff();

	// if cool whirlpool, got to whirlpool, or go to end
	if(readSetting(PS_Whirlpool) == WhirlpoolHot)
	{
		_state = AS_Cooling;
		autoModeCoolingAsk(STR(START_COOLING));
	}
	else
	{
		autoModeBrewEnd();
	}

}
#endif

void autoModeCoolingFinish(void)
{
#if NoWhirlpool != true
	if(readSetting(PS_Whirlpool) == WhirlpoolCold)
	{
		_state = AS_Whirlpool;
		autoModeCoolingAsk(STR(WHIRLPOOL));
	}
	else
#endif
	{
		autoModeBrewEnd();
	}
}
#if UsePaddleInsteadOfPump

void autoModeStartWithoutPumpPrimming(void)
{
#if NoDelayStart == false
	if(_delayRequested)
	{
		autoModeEnterDelayTimeInput();
	}
	else
	{
		autoModeEnterDoughIn();
	}
#else
	autoModeEnterDoughIn();
#endif
}

#else
void autoModeEnterPumpPriming(void)
{
	_state = AS_PumpPrime;
	// request on off timer			
	setEventMask(TimeoutEventMask /*| ButtonPressedEventMask */);

	//[TODO:] pause or stop the action?
	uiNoMenu();
	uiSubTitle(STR(Pump_Prime));
	// start pump & timer
	pumpOn();
	_primePumpCount=0;
	tmSetTimeoutAfter(1000); // 1sec
}
#endif
//************************************
// for recovery
//


void autoModeResumeProcess(void)
{
	// get stage
	byte stage;
	byte elapsed;
	brewLogger.resumeSession(&stage,&elapsed);
	DBG_PRINTF("resume elapsed:%d\n",elapsed);

	setEventMask(TemperatureEventMask | ButtonPressedEventMask | TimeoutEventMask | PumpRestEventMask);

	uiClearScreen();

	uiAutoModeTitle();
	//uiDisplaySettingTemperature(gSettingTemperature); will be set later in each entering procedure
	uiTempDisplaySetPosition(TemperatureAutoModePosition);
	uiRunningTimeSetPosition(RunningTimeNormalPosition);
	// time may be 0xFF, invalid, => not just enter
	// less then stage time. temperature reached
	//
	loadBrewParameters();
	heatOn(true); // default to PID mode, if in boil stage, PID mode will be changed in
					// autoModeEnterBoiling()

	if (stage == 8) // boiling
	{
		autoModeEnterBoiling();
		// if 0xfFF, assume not 
		if(elapsed != INVALID_RECOVERY_TIME)
		{
			byte boilTime=readSetting(PS_BoilTime);
			byte time = boilTime - elapsed;

				// findout whihc hop is current 

				_isBoilTempReached=true;
				unsigned long sec=(unsigned long)time *60;
				uiRunningTimeStartCountDown(sec);
				tmSetTimeoutAfter(sec *1000);

				#if WirelessSupported == true
				//wiReportEvent(RemoteEventTemperatureReached);
				#endif
				
				// start hop & boiling out timer
				byte hopnum =  readSetting(PS_NumberOfHops);	
				_numHopToBeAdded=hopnum;
				if(hopnum > 0)
				{
					byte i;
					byte nextHopTime;
					for(i=0;i<hopnum;i++)
					{
						nextHopTime=readSetting(PS_TimeOfHop(i));
						if(nextHopTime > time) _numHopToBeAdded--;
						else break;
					}
					if(_numHopToBeAdded > 0)
					{
						unsigned long hopTimer =time - nextHopTime;
						recoveryTimer = false;
						tmSetAuxTimeoutAfter((unsigned long)hopTimer * 60 * 1000);
					}
				}				

		}
	}
	else if (stage == 9) // cooling
	{
	    heatOff();
	    if(elapsed != INVALID_RECOVERY_TIME)
        	autoModeEnterCooling(elapsed*60);
        else
            autoModeEnterCooling(0);
            
    	_state = AS_Cooling;
    	_stageConfirm= (elapsed !=0);
	}
	else if (stage == 10) // Whirlpool
	{
    	heatOff();
    	if(elapsed != INVALID_RECOVERY_TIME)
    	    autoModeWhirlpool(elapsed);
    	else
    	    autoModeWhirlpool(0);
	    _state = AS_Whirlpool;
	}
	else if (stage == 0) // Daugh-in
	{
		autoModeEnterDoughIn();
	}
	else
	{
		// everything else is in MASHING state
		// just enter mashing step ... 
		_state = AS_Mashing;
		_askingSkipMashingStage = false;

		autoModeGetMashStepNumber();

		_mashingStep = stage - 1; // next step will increase the step		
		autoModeNextMashingStep(true);
		
		// adjust timer if necessary
		if(elapsed != INVALID_RECOVERY_TIME)
		{
				byte stagetime = readSetting(PS_StageTimeAddr(_mashingStep));
				byte time = stagetime - elapsed;
				_mashingTemperatureReached = true;
				unsigned long seconds=(unsigned long)time * 60;
				
				tmSetTimeoutAfter( seconds *1000);
				tmSetAuxTimeoutAfter((seconds-ADVANCE_BEEP_TIME) *1000);
				uiRunningTimeStartCountDown(seconds);	
				pumpRestSetEnabled(true);
								
				#if WirelessSupported == true
				//wiReportEvent(RemoteEventTemperatureReached);
				#endif
		}

	}
}

//******************************
// Auto Mode Event Handling
void autoModeEventHandler(byte event)
{
	// switch-case uses more memory, though it looks better
	// 
	if(AutoStateIs( AS_AskResume))
	{
		if(btnIsStartPressed)
		{
			// YES
			autoModeResumeProcess();
		}
		else if(btnIsEnterPressed)
		{
			// clear the flag
			brewLogger.clearRecovery();

			autoModeSetup();
		}
	}
	else
	
#if NoDelayStart == false	
	if(AutoStateIs( AS_AskDelayStart))
	{
		// initially only Button Event will come
		if(btnIsStartPressed)
		{
			// NO
			_delayRequested=false;
			// next state
			_state = AS_AskWaterAdded;
		}
		else if(btnIsEnterPressed)
		{
			// YES
			_delayRequested=true;
			
			_state = AS_AskWaterAdded;
		}
		
		// if state changed.		
		if (_state == AS_AskWaterAdded)
		{
			// ask resume, just ignore this for now
			uiSubTitle(STR(Water_Added));
			uiButtonLabel(ButtonLabel(Continue_Yes_No));
		}
	}//if(_state == AS_AskDelayStart)
	else 
#endif	
	
	if(AutoStateIs(AS_AskWaterAdded))
	{
		if(btnIsStartPressed)
		{
			#if SpargeHeaterSupport == true
			// ask sparge water if sparge water is enable.
			if(readSetting(PS_SpargeWaterEnableAddress)){
				_state = AS_AskSpargeWaterAdded;
				uiSubTitle(STR(SpargeWater_Added));
				uiButtonLabel(ButtonLabel(No_Yes));
			}else{
				gEnableSpargeWaterHeatingControl = false;
				#if UsePaddleInsteadOfPump
				autoModeStartWithoutPumpPrimming();
				#else
				autoModeEnterPumpPriming();
				#endif
			}
			#else
				#if UsePaddleInsteadOfPump
				autoModeStartWithoutPumpPrimming();
				#else
				autoModeEnterPumpPriming();
				#endif
			#endif
		}
		else if(btnIsEnterPressed)
		{
			// NO; before heat & pump are used, it is safe to switch directly
			// instead of "backToMain"
			switchApplication(MAIN_SCREEN);
			return; // good bye
		}
	}  //end of state AS_AskWaterAdded
#if SpargeHeaterSupport == true
	else if(AutoStateIs(AS_AskSpargeWaterAdded))
	{
		if(btnIsStartPressed)
		{
			gEnableSpargeWaterHeatingControl = false;
			#if UsePaddleInsteadOfPump
			autoModeStartWithoutPumpPrimming();
			#else
			autoModeEnterPumpPriming();
			#endif
		}
		else if(btnIsEnterPressed)
		{
			// no sparge
			gEnableSpargeWaterHeatingControl = true;
			#if UsePaddleInsteadOfPump
			autoModeStartWithoutPumpPrimming();
			#else
			autoModeEnterPumpPriming();
			#endif
		}
	}  //end of state AS_AskWaterAdded

#endif
	else if(AutoStateIs(AS_PumpPrime))
	{
		if(event == TimeoutEventMask)
		{
			if(gIsPumpOn)
			{
				pumpOff();
				tmSetTimeoutAfter(350);
			}
			else
			{
				_primePumpCount++;

				if(_primePumpCount < 5)
				{
					pumpOn();
					tmSetTimeoutAfter(1000 + _primePumpCount * 250);
				}
				else
				{
					// next stage is setting delay or mash start
#if NoDelayStart == false
					if(_delayRequested)
					{
						autoModeEnterDelayTimeInput();
					}
					else
					{
						//_state = AS_DoughIn;
						autoModeEnterDoughIn();
					}
#else
						autoModeEnterDoughIn();
#endif
				} // else of prime pump < 5

			} // end of else if(gIsPumpOn)
		} // end of handling of TimeoutEventMask
	} // end of state AS_PumpPrime
#if NoDelayStart == false	
	else if(AutoStateIs(AS_DelayTimeInput))
	{
		// input delay timer
		if(event != ButtonPressedEventMask) return;
		
		if(btnIsUpPressed)
		{
			if( (_delayTime +1) < MAX_DELAY_TIME )
			{
				_delayTime ++;
				uiRunningTimeShowInitial(_delayTime * 15 * 60);
			}
		}
		else if(btnIsDownPressed)
		{
			if(_delayTime > 1)
			{
				_delayTime --;
				uiRunningTimeShowInitial(_delayTime * 15 * 60);
			}
		}
		else if(btnIsStartPressed)
		{
			// quit
			backToMain();
		}
		else if(btnIsEnterPressed)
		{
			_state = AS_DelayTimeConfirm;
			uiButtonLabel(ButtonLabel(Continue_Yes_No));
		}
	} // state AS_DelayTimeInput
	else if(AutoStateIs(AS_DelayTimeConfirm))
	{
		if(event != ButtonPressedEventMask) return;
		
		if(btnIsStartPressed)
		{
			// YES
			_state = AS_DelayWaiting;
			uiClearSubTitleRow();
			uiSubTitle(STR(To_be_started_in));
			uiButtonLabel(ButtonLabel(x_x_Quit_Go));
			
			tmSetTimeoutAfter(_delayTime * 15 * 60 * 1000);
			uiRunningTimeStartCountDown(_delayTime * 15 * 60);
			setEventMask(TimeoutEventMask | ButtonPressedEventMask );
			
			#if WirelessSupported == true
			wiReportCurrentStage(StageDelayStart);
			#endif
		}
		else if(btnIsEnterPressed)
		{
			//NO
			backToMain();
		}
		
	} //AS_DelayTimeConfirm
	else if(AutoStateIs(AS_DelayWaiting))
	{
		if(event == ButtonPressedEventMask)
		{
			if(btnIsStartPressed)
			{
				// Quit
				uiRunningTimeStop();
				tmPauseTimer();
				backToMain();
			}
			else if(btnIsEnterPressed)
			{
				//GO
				// cancel timer
				uiRunningTimeStop();
				tmPauseTimer();
				uiClearSettingRow();
				//_state = AS_DoughIn;
				autoModeEnterDoughIn();				
			}
		}
		else if(event == TimeoutEventMask)
		{
			buzzPlaySound(SoundIdDelayTimeout);
			uiRunningTimeStop();
			uiClearSettingRow();
			autoModeEnterDoughIn();
		}
	}//AS_DelayWaiting
#endif
	else if(AutoStateIs(AS_DoughIn))
	{
		if(event == TemperatureEventMask)
		{
			if(gCurrentTemperature >=gSettingTemperature)
			{
				// temp reached. ask continue & malt in
				_state = AS_MashInAskContinue;
				_mashingTemperatureReached = true;
				
				uiPrompt(STR(TemperatureReached));
				//{ADD_MALT_MOD
				#if 1
				uiButtonLabel(ButtonLabel(Continue_Yes_No));
				#else
				uiButtonLabel(ButtonLabel(Continue_Yes_x));
				#endif
				//}ADD_MALT_MOD

				setEventMask(ButtonPressedEventMask);
				
				buzzPlaySoundRepeat(SoundIdWaitUserInteraction);

				brewLogger.event(RemoteEventTemperatureReached);
				#if WirelessSupported == true
				wiReportEvent(RemoteEventTemperatureReached);
				#endif

			}
		}//TemperatureEventMask
		else if(event == ButtonPressedEventMask)
		{
			// up/down/pause
			if(btnIsStartPressed)
			{
				autoModePause(0);
			}
#if MANUAL_PUMP_MASH == true
			else if(btnIsEnterPressed)
			{
				togglePump();
			}
#endif
			else
			{
				//for up/down
				processAdjustButtons();
			}
		}//ButtonPressedEventMask
	} // endof state AS_DoughIn
	else if(AutoStateIs(AS_Pause))
	{
		if(event == ButtonPressedEventMask
			&& btnIsStartPressed)
		{	
			autoModeExitPause();
		} 
	} //AS_Pause
	else if(AutoStateIs(AS_MashInAskContinue))
	{
		if(btnIsStartPressed)
		{
			buzzMute();
			if(readSetting(PS_PID_DoughIn)){
				// change temperature to first rest
				/*
				gSettingTemperature = TempFromStorage(readSettingWord(PS_StageTemperatureAddr(1)));	
				uiDisplaySettingTemperature(gSettingTemperature);
			    */
			}else{
				heatOff(); // turn off heat. during "dough-in"
			}
			// goto next stage, Mashing or ask MaltADD
			if(readSetting(PS_SkipAddMalt))
			{
				uiClearPrompt();
				// skip Add Malt , enter mashing state
				autoModeEnterMashing();
			}
			else
			{
				pumpOff();
				// ask Add Malt
				//uiClearPrompt();
				uiPrompt(STR(Add_Malt));
				//{ADD_MALT_MOD
				//uiButtonLabel(ButtonLabel(Continue_Yes_No));
				uiButtonLabel(ButtonLabel(Continue_Yes_Pmp));
				//}ADD_MALT_MOD
				
				_state = AS_AskAddMalt;
				
				#if WirelessSupported == true
				wiReportEvent(RemoteEventAddMalt);
				#endif

			}
		}
		//{ADD_MALT_MOD
		else if(btnIsEnterPressed)
		{
			// NO
			// heater & pump might started, so use back to main
			backToMain();
		}
		//}ADD_MALT_MOD
	} /// AS_MashInAskContinue
	else if(AutoStateIs(AS_AskAddMalt))
	{
		if(btnIsStartPressed)
		{
			// YES
			uiClearPrompt();
			autoModeEnterMashing();
		}
		else if(btnIsEnterPressed)
		{
    		//{ADD_MALT_MOD
			// heater & pump might started, so use back to main
			//backToMain();
			togglePump();
			//}ADD_MALT_MOD
		}
	} // AS_AskAddMalt
	else if(AutoStateIs(AS_Mashing))
	{
		// handle key event together.
		// the same way reached or not.
		if(event == ButtonPressedEventMask)
		{	
			if(_askingSkipMashingStage)
			{
#if EnableExtendedMashStep
				if(btnIsUpPressed)
				{
					autoModeToggleMashExtension();
				} else
#endif //#if EnableExtendedMashStep		
				if(btnIsStartPressed)
				{
					// YES.
					// undone _askingSkipMashingStage
					uiClearPrompt();
					// not necessary , autoModeMashingStageFinished()
					// will print eht menu againuiButtonLabel(ButtonLabel(Up_Down_Pause_STP));
					// unwind the change
					uiRunningTimeHide(false);
					_askingSkipMashingStage = false;
					tmPauseTimer(); // cancel timer, if any
					// go to next stage
					autoModeMashingStageFinished();
				}
				else if(btnIsEnterPressed)
				{
					// NO
					uiClearPrompt();
					#if	MANUAL_PUMP_MASH == true
					uiButtonLabel(ButtonLabel(Up_Down_PmPus_STP));
					#else
					uiButtonLabel(ButtonLabel(Up_Down_Pause_STP));
					#endif
					// unwind the change
					uiRunningTimeHide(false);
					_askingSkipMashingStage = false;					
				}
				return;
			}
			// else
			if(btnIsStartPressed)
			{

			#if	MANUAL_PUMP_MASH == true
				if(btnIsStartLongPressed)
				{
			#endif

				// if in 
				if(_mashingTemperatureReached)
				{
					buzzMute();
					autoModePause(tmPauseTimer());
					
				}
				else
				{
					autoModePause(0);
				}
			#if	MANUAL_PUMP_MASH == true
				}
				else
				{
					togglePump();
					gManualPump = true;
				}
			#endif			
			}
			else if(btnIsEnterPressed)
			{
				#if EnableExtendedMashStep
				
				if(_mashingStageExtending)
				{
					// go to next step.
					autoModeMashingStageFinished();
				}
				else
				#endif // #if EnableExtendedMashStep
				
				// Skip, go to next stage
				if(btnIsEnterLongPressed)  // long pressed is "cover" in normal pressed
				{
					if(_mashingTemperatureReached)
						buzzMute();
					uiRunningTimeHide(true);
					_askingSkipMashingStage = true;
					//uiClearPrompt();
					#if EnableExtendedMashStep
					uiPrompt(STR(Skip_Or_Extend));
					uiButtonLabel(ButtonLabel(Extend_Skip_Back));
					#else //#if EnableExtendedMashStep
					uiPrompt(STR(Go_to_next_step));
					uiButtonLabel(ButtonLabel(Continue_Yes_No));
					#endif //#if EnableExtendedMashStep
				}
			}
			else
			{
				//up, down etc.
				if(_askingSkipMashingStage) return; // ignore

				processAdjustButtons();
			}

		}
		else if(event == PumpRestEventMask)
		{
			//
			if(isPumpRest())
			{
				// into rest
				//no special manual uiButtonLabel(ButtonLabel(_Pump_Rest_));
				// stop heat
				#if !UsePaddleInsteadOfPump
				heatProgramOff();		
				buzzPlaySound(PumpRestSoundId);
				#endif

				wiReportEvent(RemoteEventPumpRest);
			}
			else
			{
				#if 0
				// back from rest
				#if MANUAL_PUMP_MASH == true
				uiButtonLabel(ButtonLabel(Up_Down_PmPus_STP));
				#else
				uiButtonLabel(ButtonLabel(Up_Down_Pause_STP));
				#endif
				#endif
				
				#if !UsePaddleInsteadOfPump
				heatOn(true);				
				buzzPlaySound(PumpRestEndSoundId);
				#endif
				
				wiReportEvent(RemoteEventPumpRestEnd);
			}
		}
		else // else of PumpRestEvent & Button,
		{
			//DBG_PRINTF("reach:%d, setting:%d\n",_mashingTemperatureReached,(int)gSettingTemperature);
			if(_mashingTemperatureReached)
			{
				if(event == TimeoutEventMask)
				{
					// counting time
					// except button, we care also two timer
					// one for 10  or 5 seconds before time out
					// the other for end of phase timeout
					if(IsAuxTimeout)
					{
						buzzPlaySound(SoundIdCountDown);
					}
					else
					{
					
						// next stage
						if(_askingSkipMashingStage)
						{
							uiClearPrompt();
							uiRunningTimeHide(false);
							_askingSkipMashingStage = false;
						}
						
						#if EnableExtendedMashStep
						if(_mashingStageExtendEnable)
						{
							autoModeEnterMashingExtension();
						}
						else
						#endif //#if EnableExtendedMashStep
						{
							autoModeMashingStageFinished();
						}
					}
				}
			}
			else // of if(_mashingTemperatureReached)
			{
				if(event == TemperatureEventMask)
				{
					// rising temperature
					if(gCurrentTemperature >= gSettingTemperature)
					{
						brewLogger.event(RemoteEventTemperatureReached);

						_mashingTemperatureReached = true;
						unsigned long seconds=(unsigned long)readSetting(PS_StageTimeAddr(_mashingStep)) * 60;
				
						tmSetTimeoutAfter( seconds *1000);
						tmSetAuxTimeoutAfter((seconds-ADVANCE_BEEP_TIME) *1000);
						
						uiRunningTimeStartCountDown(seconds);
					
						buzzPlaySound(SoundIdTemperatureReached);
					
						pumpRestSetEnabled(true);
						
						#if WirelessSupported == true
						wiReportEvent(RemoteEventTemperatureReached);
						#endif

					}
				}
			} 	// end of else if(_mashingTemperatureReached)
		}		// end of temperature and timeout handling
	}//AS_Mashing
	else if(AutoStateIs(AS_IodineTest))
	{
		// timeout or user press ok
		if(event ==ButtonPressedEventMask
			&& btnIsStartPressed)
		{
			uiClearPrompt();
			// back to next mashing step: Mashout
			autoModeIodineTestToMashout();
		}
		else if(event ==TimeoutEventMask)
		{
			uiClearPrompt();
			//[TODO:] make sure not other timeout event
			autoModeIodineTestToMashout();			
		}
	}//AS_IodineTest
	else if(AutoStateIs(AS_AskMaltRemove))
	{
		if(event ==ButtonPressedEventMask)
		{
			if(btnIsStartPressed)
			{
				buzzMute();

				// yes
				uiClearPrompt();
				autoModeEnterBoiling();
			}
			else if(btnIsEnterPressed)
			{
				// back to main
				backToMain();
			}
		}
	}//AS_AskMaltRemove
	else if(AutoStateIs(AS_Boiling))
	{
		if(event ==ButtonPressedEventMask)
		{
				if (btnIsEnterPressed)
				{
					// pump control
					if(gIsPumpOn) pumpOff();
					else pumpOn();
				}
				else if(btnIsStartPressed)
				{
					if(_isBoilTempReached)
					{
						autoModeBoilingPauseHandler();
					}
				}
				else
				{
					processAdjustButtons();
				}
		}
		else if(event ==TimeoutEventMask)
		{
//#ifdef AUX_TIMER_HOP
			if(IsAuxTimeout)
			{
				// start next timer to end notice of hop adding	
				if(recoveryTimer)
				{
					uiAutoModeStage(BoilingStage);
					autoModeStartNextHopTimer();
				}
				else
				{
					// start next timer
					autoModeAddHopNotice();
				}
			}
			else
			{

					// next stage
					heatOff(); // heat OFF
					pumpOff();
					
					brewLogger.event(RemoteEventBoilFinished);
					
					#if WirelessSupported == true
					wiReportEvent(RemoteEventBoilFinished);
					#endif
					buzzPlaySoundRepeat(SoundIdWaitUserInteraction);
										
					#if NoWhirlpool != true
					if(readSetting(PS_Whirlpool) == WhirlpoolHot)
					{
						_state = AS_Whirlpool;
						autoModeCoolingAsk(STR(WHIRLPOOL));
					}
					else
					{
						_state = AS_Cooling;
						autoModeCoolingAsk(STR(START_COOLING));
					}
					#else
						_state = AS_Cooling;
						autoModeCoolingAsk(STR(START_COOLING));					
					#endif
			}

//#endif
		}
		else // if(event ==TemperatureMask)
		{
			togglePwmInput();

			if(gCurrentTemperature >= gBoilStageTemperature)
			{
				if(_isBoilTempReached == false)
				{
					brewLogger.event(RemoteEventTemperatureReached);
					_isBoilTempReached=true;
					
					//buzz temperature reach first
					// because later "add hop" buzz may interrupt
					// it
					buzzPlaySound(SoundIdBoil);
					// start counting down
					byte boilTime=readSetting(PS_BoilTime);
					uiRunningTimeStartCountDown((unsigned long)boilTime *60);
					// start hop & boiling out timer
					autoModeStartBoilingTimer();				

					#if WirelessSupported == true
					wiReportEvent(RemoteEventTemperatureReached);
					#endif

					uiButtonLabel(ButtonLabel(Up_Down_Pause_Pmp));
				}
			}
		}
	} //AS_Boiling
	else if(AutoStateIs(AS_Cooling))
	{
		if(_stageConfirm)
		{
			if(event == ButtonPressedEventMask) 
			{
				if (btnIsEnterPressed)
				{
					// pump control
					if(gIsPumpOn) pumpOff();
					else pumpOn();
				}
				else if (btnIsStartPressed)
				{
					// next stage
					autoModeCoolingFinish();					
				}
				else
				{
					processAdjustButtons();
				}
			}
			else if(event == TemperatureEventMask) 
			{
				// if temperature drop to desire temp
				// end this phase
				if(gCurrentTemperature <= gSettingTemperature )
				{
					// next stage
					//Manual END. no Auto end 
					// autoModeCoolingFinish();
					if(!_coolingTempReached){
						buzzPlaySound(SoundIdTemperatureReached);
						wiReportEvent(RemoteEventTemperatureReached);
						_coolingTempReached=true;
					}
				}
			}
		}
		else // of if(_stageConfirm), in state of asking Enter Cooling
		{
			// wait confirm
			if(event != ButtonPressedEventMask) return;
			buzzMute();
			if(btnIsStartPressed)
			{
				brewLogger.stage(StageCooling);
				// yes
				_stageConfirm=true;
				autoModeEnterCooling(0); 
				brewLogger.setPoint(gSettingTemperature);
			}
			else if (btnIsEnterPressed)
			{
				// no
				autoModeCoolingFinish();
			}
		} // end of else of if(_stageConfirm)
	} //AS_Cooling
#if NoWhirlpool != true
	else if(AutoStateIs(AS_Whirlpool))
	{
		if(_stageConfirm)
		{
			if(_whirlpoolInput) 
			{
				// input screen of Whirlpool time
				if(event != ButtonPressedEventMask) return;
				
				if(btnIsUpPressed)
				{
					//up
					if((_whirlpoolTime + 1) <= MAX_WHIRLPOOL_TIME)
					{
						_whirlpoolTime++;
						uiRunningTimeShowInitial(_whirlpoolTime * 60);
					}
				}
				else if (btnIsDownPressed)
				{
					// down
					if((_whirlpoolTime - 1) >= MIN_WHIRLPOOL_TIME)
					{
						_whirlpoolTime--;
						uiRunningTimeShowInitial(_whirlpoolTime * 60);
					}
				}
				else if (btnIsStartPressed)
				{
					//Quit
					autoModeWhirlpoolFinish();
				}
				else if (btnIsEnterPressed)
				{
					//OK
					_whirlpoolInput=false;
					autoModeWhirlpool(0);
				}
			}
			else // of _whirlpoolInput
			{
				// Whirlpool stage running
				// counting time & running pump
				if(event == ButtonPressedEventMask)
				{			
	
					if(btnIsStartPressed)
					{
						if(!_pumpRunning)
						{
							// time, back to time setting
							autoModeWhirlpoolInputTime();
						}
					}
					else if(btnIsEnterPressed)
					{
						if(_pumpRunning)
						{
							// stop pump ,and stop & reset time
							_pumpRunning = false;
							pumpOff();
							uiRunningTimeShowInitial(_whirlpoolTime * 60);
							tmPauseTimer();
							
							uiButtonLabel(ButtonLabel(x_x_Time_Pmp));
						}
						else
						{
							_pumpRunning = true;
							pumpOn();
							uiRunningTimeStartCountDown(_whirlpoolTime * 60);
							tmSetTimeoutAfter((unsigned long)_whirlpoolTime*60*1000);
							
							uiButtonLabel(ButtonLabel(x_x_x_Pmp));

						}
					}
				}else if(event == TemperatureEventMask)
				{
					//[TODO:] temperature control
				}
				else if(event == TimeoutEventMask)
				{
					autoModeWhirlpoolFinish();
				}
			}
		}
		else // if(_stageConfirm)
		{
			// wait confirm
			if(event != ButtonPressedEventMask) return;

			buzzMute();

			if(btnIsStartPressed)
			{
				brewLogger.stage(StageWhirlpool);
				// yes
				_stageConfirm=true;
				autoModeWhirlpoolInputTime();
			}
			else if (btnIsEnterPressed)
			{
				// no
				autoModeWhirlpoolFinish();
			}
		} // of else // if(_stageConfirm)

	}//AS_Whirlpool
#endif
	else if(AutoStateIs(AS_Finished))
	{
		if(event == TimeoutEventMask)
		{
			backToMain();
		}
	}//AS_Finished
} // end of autoModeEventHandler


// *************************
//*  Main Screen
// *************************

void mainSetup(void)
{
	uiClearScreen();

	uiTitle(STR(welcome));
	uiButtonLabel(ButtonLabel(Manual_Auto_Setup));
    uiTempDisplaySetPosition(TemperatureMainScreenPosition);

    #if WirelessSupported == true
    uiPrintIpAddress();
    wiReportCurrentStage(StageIdleScreen);
    #endif
    
    #if MaximumNumberOfSensors > 1
    setSensorForStage(SensorForIdle);
    #endif
}

// main screen
//  -down button-> manual
//  -start button->auto
//  -ener button ->setup 

void mainEventHandler(byte event)
{
	if(btnIsEnterPressed)
	{
		switchApplication(SETUP_SCREEN);

	}
  	else if(btnIsDownPressed)
  	{
      	switchApplication(MANUAL_MODE_SCREEN);
  	}
  	else if(btnIsStartPressed)
  	{
      	switchApplication(AUTO_MODE_SCREEN);
  	}
  
}

// *************************
//*  Screen switch
// *************************


const CScreen *currentScreen;
void switchApplication(byte screenId)
{
	currentScreen=allScreens+screenId;
	//turn off temperature update by default, let those who want turn it on

	uiClearSettingRow();

	uiRunningTimeStop();
	uiTempDisplayHide();
	
	//default event is keyboard
	setEventMask(ButtonPressedEventMask);

	(* currentScreen->setup)();
}

void backToMain(void)
{
	brewLogger.abortSession();
	// turn pump & heat off
	heatOff();
#if SpargeHeaterSupport == true	
	stopHeatingSpargeWater();
#endif
	pumpOff();
	buzzMute();

	uiRunningTimeBlink(false); // stop blink if any. additional time print will be done
								// however, it will be clear later, before enter "Main"
	
	if(_isRunningAutoTune) cancelAutoTune();
	//
	commitSetting(); //ESP8266
	switchApplication(MAIN_SCREEN);
}
// *************************
//*  Main procedure
// *************************
void uiPrintInitialScreen(void)
{
	uiClearScreen();
	uiTitle(STR(welcome));

	uiShowTextAtRow_P(1,STR(Initialization),CenterAligned,1);
}

void brewmaniac_ApPrompt(void)
{
	uiShowTextAtRow_P(1,STR(SetupNetwork),CenterAligned,1);
	uiButtonLabel(ButtonLabel(AccessPoint_Yes));
}

bool readSkipNetCfgButton(void)
{
	tmTiming();
	if(btnReadButtons()) 
	{
		if(btnIsEnterLongPressed)
			return true;	
	}
	return false;
}

void startBrewManiac()
{
	switchApplication(MAIN_SCREEN);
}

void brewmaniac_setup() {

	EepromInit();
	// ESP8266
  	Wire.begin(I2C_SDA,I2C_SCL);

	initIOPins();
	
	gIsUseFahrenheit = readSetting(PS_TempUnit);

	tmInitialize();
	btnInitialize();
	tpInitialize();
	
	uiInitialize();
	
	heatInitialize();
	pumpInitialize();
		
	
	uiPrintInitialScreen();
	
#if WirelessSupported == true
	wiInitialize();
#endif


}

//*********************************************************************
//*********************************************************************



void brewmaniac_loop() {

	// Process Events
	//    button, temperature, time
	//  Event: BUTTON,  TEMP Reach, TimeOut.

	tpReadTemperature();
	
	// let the handler compare the temperatures themselves if they request it.
	if(_currentEventMask & TemperatureEventMask)
		(*currentScreen->eventHandler)(TemperatureEventMask);

	if(tmTiming())
	{
		if(_currentEventMask & TimeoutEventMask)
			(*currentScreen->eventHandler)(TimeoutEventMask);	
	}
	
	if(btnReadButtons()) 
	{
		if(isExactButtonsPressed(ButtonUpMask | ButtonDownMask))
			backToMain();
		else
		{
			// if(_currentEventMask & ButtonPressedEventMask) button event is always handled in all 
			// screen!
			(*currentScreen->eventHandler)(ButtonPressedEventMask);
		}
	}
	
	if(pumpRestEvent())
	{
		if(_currentEventMask & PumpRestEventMask)
			(*currentScreen->eventHandler)(PumpRestEventMask);
	}

	#if MaximumNumberOfSensors > 1
	brewLogger.temperatures(gTemperatureReading);
	#else
	brewLogger.temperature(gCurrentTemperature);
	#endif

	//update Time & temperature 
	uiDisplayTemperatureAndRunningTime();
	
	//	
	// threads
	heatThread();
	pumpThread();
	buzzThread();

#if WirelessSupported == true	
	wiThread();
#endif	
	// handler state machine

}// end of loop();




