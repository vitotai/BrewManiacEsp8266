/**********************************************************************
 BrewManiac
 created by Vito Tai
 Copyright (C) 2015 Vito Tai

 This soft ware is provided as-is. Use at your own risks.
 You are free to modify and distribute this software without removing
 this statement.
 BrewManiac by Vito Tai is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
***********************************************************************/
#include <Arduino.h>
#include <pgmspace.h>
#include <EEPROM.h>
#include <OneWire.h>
//#include <PID_v1.h>
#include <PID_v2.h>

#include "config.h"
#include "ps.h"

#if USE_MAX6675
#include "max6675.h"
#endif

#if ESP32
#include "pins_esp32.h"
#else
#include "pins.h"
#endif

#include "automation.h"


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
#undef LIBRARY_VERSION
// to elminate warning
#include <PID_AutoTune_v0.h>
#endif

#define WirelessSupported true
#define EnableSensorResolution true


#if SerialDebug == true
#define DBG_PRINTF(...) DebugPort.printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...)
#endif

#define ValidSensorValueC(v)  ((v) <120 && (v) > -1)

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

float gCorrectionA0s[MaximumNumberOfSensors];
float gCorrectionA1s[MaximumNumberOfSensors];

#else

float gCorrectionA0;
float gCorrectionA1;

#endif

byte gBoilHeatOutput;

// the pump/heat on/off is requested by user
//  real pump/heat on/off depends on the temperature
//   and parameter setting

boolean gIsHeatOn;
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
bool  gSensorDisconnected[MaximumNumberOfSensors];
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
typedef	bool (*EventHandlerFunc)(byte);

void switchApplication(byte screenId);

void backToMain(void);

void setEventMask(byte);

// main screen
void mainSetup(void);
bool mainEventHandler(byte);

// setup menu
void menuSetup(void);
bool menuEventHandler(byte);

void settingPidSetup(void);
bool settingPidEventHandler(byte);

void settingUnitSetup(void);
bool settingUnitEventHandler(byte);

void settingAutoSetup(void);
bool settingAutoEventHandler(byte);

// manual mode
void manualModeSetup(void);
bool manualModeEventHandler(byte);
//auto mode
void autoModeSetup(void);
bool autoModeEventHandler(byte);

void miscSettingSetup(void);
bool miscSettingEventHandler(byte);


#if EnablePidAutoTune == true
void autoTuneMenuSetup(void);
bool autoTuneMenuEventHandler(byte);
#endif

#if MaximumNumberOfSensors > 1
void sensorMenuSetup(void);
bool sensorMenuEventHandler(byte);
#endif

#if SpargeHeaterSupport == true
void spargeMenuSetup(void);
bool spargeMenuEventHandler(byte);
#endif

#if SupportDistilling
void distillRecipeSetup(void);
bool distillRecipeEventHandler(byte);
void distillingSetup(void);
bool distillingEventHandler(byte);
#endif

#define ConvertF2C(d) (((d)-32)/1.8)
#define ConvertC2F(d) (((d)*1.8)+32)


//Stage
#define StageDoughIn 	    0
// 1 -6 rest,
// 7 mashout
#define MashStepMashOut 7
#define StageBoil 		    8
#define StageCooling 		9
#define StageWhirlpool     10

#define StageHopStandChill 11
#define StageHopStand      12
#define StageAutoResume   13

#define StageDelayStart   99
#define StageManualMode  100
#define StageIdleScreen  101
#define StageSetting 	 102
#define StagePIDAutoTune 103

#if SupportDistilling
#define StageDistillingPreHeat 110
#define StageDistillingHead 111
#define StageDistillingHeart 112
#define StageDistillingTail 113
#endif

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

#if SpargeHeaterSupport == true
	#define RemoteEventSpargeWaterAdded	98	// Added to keep track of active sparge for recovery purposes
#endif

#define RemoteEventBrewFinished 	99

void btReportCurrentStage(byte stage);
void btReportEvent(byte event);
void btReportSettingTemperature(void);
void btReportPwm(void);

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
MISC_SETTING_SCREEN = 7,

#if MaximumNumberOfSensors > 1
SENSOR_SCREEN,
#endif
#if SupportDistilling
	DISTILL_SETTING_SCREEN,
	DISTILLING_MODE_SCREEN,
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
},
{
	&miscSettingSetup,
	&miscSettingEventHandler,
}
#if MaximumNumberOfSensors > 1
,{
	&sensorMenuSetup,
	&sensorMenuEventHandler,
}
#endif
#if SupportDistilling
,{
	&distillRecipeSetup,
	&distillRecipeEventHandler,
},
{
	&distillingSetup,
	&distillingEventHandler,
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
#include "resources.h"
#include "ui.h"

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

//#define BUTTON_DEBUG true


boolean _virtualButtonPressed=false;

void virtualButtonPress(byte mask,boolean longPressed)
{
       gButtonPressed = mask;
       gLongPressed = longPressed;
       _virtualButtonPressed=true;
//      #if BUTTON_DEBUG == true
      DBG_PRINTF("virutal key:%d\n",mask);
//      #endif
}



boolean btnReadButtons(void)
{
      if(_virtualButtonPressed)
      {
      	_virtualButtonPressed = false;
        return true;
       }

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

		buzzMute(); // mute anything after button depressed.
		            // even if button feedback is off. the alert for
		            // temperature reached for doughing-in will be muted.

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

            if(readSetting(PS_ButtonFeedback)){
    			if(duration > ButtonPressedDetectMinTime) buzzOn();
	    		if(duration > ButtonLongPressedDetectMinTime) buzzMute();
	    	}

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

void calTemperatureCalibration(void){
	// correction foumula =
	// Correction Temp = ( Reading - Read1)*(Ref2 - Ref1)/(Read2 - Read1)  + Ref1
	//  =  Reading *(Ref2 - Ref1)/(Read2 - Read1) - Read1 *(Ref2 - Ref1)/(Read2 - Read1) +Ref1
	//  Correction Temp = A1 * Reading + A0
	// A1 =(Ref2 - Ref1)/(Read2 - Read1)
	// A0 = - Read1 *(Ref2 - Ref1)/(Read2 - Read1) +Ref1
#if	MaximumNumberOfSensors	> 1
	if(readSetting(PS_EnableTwoPointCalibration)){
		for(byte si=0;si < gSensorNumber;si++){
			float Raw1 = TempFromEeprom(readSettingWord(PS_CalibrationReadingP1Of(si)));
			float Raw2 = TempFromEeprom(readSettingWord(PS_CalibrationReadingP2Of(si)));
			float Ref1 = TempFromEeprom(readSettingWord(PS_CalibrationReferenceP1Of(si)));
			float Ref2 = TempFromEeprom(readSettingWord(PS_CalibrationReferenceP2Of(si)));
			if( (Raw2 - Raw1) != 0){
				gCorrectionA1s[si] = (Ref2 - Ref1)/(Raw2 - Raw1);
				gCorrectionA0s[si] = Ref1 - Raw1 * gCorrectionA1s[si];
			}else{
				gCorrectionA1s[si] = 1;
				gCorrectionA0s[si] =0;
			}
		}
	}else{
		for(byte si=0;si < gSensorNumber;si++){
			gCorrectionA0s[si] = ((float)(readSetting(PS_Offset) - 50) / 10.0);
			gCorrectionA1s[si] = 1.0;
		}
	}


#else
	if(readSetting(PS_EnableTwoPointCalibration)){
		float Raw1 = TempFromEeprom(readSettingWord(PS_CalibrationReadingP1));
		float Raw2 = TempFromEeprom(readSettingWord(PS_CalibrationReadingP2));
		float Ref1 = TempFromEeprom(readSettingWord(PS_CalibrationReferenceP1));
		float Ref2 = TempFromEeprom(readSettingWord(PS_CalibrationReferenceP2));
		if( (Raw2 - Raw1) != 0){
			gCorrectionA1 = (Ref2 - Ref1)/(Raw2 - Raw1);
			gCorrectionA0 = Ref1 - Raw1 * gCorrectionA1;
		}else{
			gCorrectionA0=0;	
			gCorrectionA1 = 1.0;
		}
	}else{
		gCorrectionA0 = ((float)(readSetting(PS_Offset) - 50) / 10.0);
		gCorrectionA1 = 1.0;
	}
#if SerialDebug == true

	Serial.print("calTemperatureCalibration, A1=");
	Serial.print(gCorrectionA1);
	Serial.print(" A0=");
	Serial.println(gCorrectionA0);
#endif
#endif
}

#if	MaximumNumberOfSensors	> 1

void resetSensorCorrection(int si){
	gCorrectionA0s[si] = 0;
	gCorrectionA1s[si] = 1.0;
}

inline float sensorCorrection(float reading,int index){
	return reading * gCorrectionA1s[index] + gCorrectionA0s[index];
}

#else
inline float sensorCorrection(float reading){
	return reading * gCorrectionA1 + gCorrectionA0;
}
#endif

void temperatureUnitChange(bool useF)
{
	if(gIsUseFahrenheit == useF) return;
	gIsUseFahrenheit = useF;
	uiChangeTemperatureUnit(useF);
	// update EEPROM content
	// PS_BoilTemp, PS_TempPumpRest

	//	gBoilStageTemperature loaded in heatLoadParameter
	//	_pumpStopTemp loaded in pumpLoadParameter
/*
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
*/
}
class LowPassFilter{
private:
	float _filteredValue;
public:
	void setInitialValue(float value){
		_filteredValue = value;
	}

	float addValue(float value){
		_filteredValue= _filteredValue + LowPassFilterParameter *(value - _filteredValue);
		return _filteredValue;
	}
};

#if USE_MAX6675 != true
OneWire ds(SensorPin);

#if MaximumNumberOfSensors > 1
bool _gIsSensorConverting[MaximumNumberOfSensors];

#else
boolean _isConverting;
#endif

byte _sensorData[9];



#if	MaximumNumberOfSensors	> 1
#define HEXChar(a) ((a) > 9)? ('A' + (a) -10):('0' + (a))

void printSensorAddress(char *buf, byte *addr)
{
	for(byte i=0;i<8;i++)
	{
		buf[i*2]= HEXChar(addr[i] >> 4);
		buf[i*2 +1]=HEXChar(addr[i] & 0xF);
	}
	buf[16]=0;
}

#if FakeHeating
byte FakeSensors[][8]={
{0x28,0xFF,0x2B,0x00,0x00,0x00,0x01,0x9e},
{0x28,0xFF,0x2B,0x00,0x00,0x00,0x02,0x7c},
{0x28,0xFF,0x2B,0x00,0x00,0x00,0x03,0x22},
{0x28,0xFF,0x2B,0x00,0x00,0x00,0x04,0xa1}
};
byte scanSensors(byte max,byte addresses[][8]) {
//	byte i;
	for(int i=0;i<4;i++){
		memcpy(addresses[i],FakeSensors[i],8);
		gTemperatureReading[i]= gIsUseFahrenheit? 67.8:19.9;
	}
	return 4;
}
#else
byte scanSensors(byte max,byte addresses[][8]) {
//	byte i;
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
#endif

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
			DBG_PRINTF("invalid sensor address! %x",OneWire::crc8( gSensorAddresses[i], 7));
			break;
    	}
		calTemperatureCalibration();
	}
	gSensorNumber=i;
// if more than one sensor available, use the second as aux by default
	gAuxSensorIndex =(gSensorNumber>1)? 1:0;

#if SerialDebug == true
		Serial.printf("Number of sensors:%d\n",gSensorNumber);
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
	// write EEPROM, by lekrom
	if(addr) ds.select(addr);
	else ds.skip();        
	ds.write(0x48);			// Save settings to non-volatile	
	delay(15);                            // Wait for EEPROM write to complete.
}
#endif //#if EnableSensorResolution	== true

void tpReadInitialTemperature(void);


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
		gSensorDisconnected[i]=true;
	}
#else
	_isConverting=false;

	calTemperatureCalibration();
#endif

#if FakeHeating
	gCurrentTemperature =gIsUseFahrenheit? 67.8:19.9;

#if	MaximumNumberOfSensors	> 1

	for(byte i=0;i< MaximumNumberOfSensors;i++){
		gTemperatureReading[i]= gIsUseFahrenheit? 67.8:19.9;
	}
#endif

#endif //#if FakeHeating
	tpReadInitialTemperature();
}
#endif
// the following code basically comes from Open ArdBir

#define DSCMD_CONVERT_T 0x44
#define DSCMD_READ_SCRATCHPAD 0xBE

#if  MaximumNumberOfSensors > 1

float _filteredValues[MaximumNumberOfSensors];

void lpfSetInitialValue(uint8_t idx,float value){
	_filteredValues[idx]= value;
}

float lpfAddValue(uint8_t idx,float value){
	_filteredValues[idx]= _filteredValues[idx] + LowPassFilterParameter *(value - _filteredValues[idx]);
	return _filteredValues[idx];
}
void tpSensorRequestConvert(uint8_t address[])
{
	ds.reset();
	ds.select(address);
    ds.write(DSCMD_CONVERT_T, 0);
}
bool tpSensorDataReady(uint8_t address[]){
	ds.reset();
  	ds.select(address);
	// check for conversion if it isn't complete return if it is then convert to decimal
    byte busy = ds.read_bit();
	return busy != 0;
}
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
	if(!ValidSensorValueC(temperature)) return INVALID_TEMP_C;
    if(gIsUseFahrenheit) temperature = C2F(temperature);
    return temperature;
}

void tpReadInitialTemperature(void)
{
	for(byte si=0;si < gSensorNumber;si++)
	{
		tpSensorRequestConvert(gSensorAddresses[si]);
  	}
	bool sensorResponse[MaximumNumberOfSensors];
	for(byte si=0;si < gSensorNumber;si++) sensorResponse[si]=false;

	int i=0;	
	while(i< 10){
		for(byte si=0;si < gSensorNumber;si++){
    		if (tpSensorDataReady(gSensorAddresses[si])){
    			float rawreading=_readTemperature(gSensorAddresses[si]);

				if(! IS_TEMP_INVALID(rawreading)){
					lpfSetInitialValue(si,rawreading);
					float reading = rawreading * gCorrectionA1s[si] + gCorrectionA0s[si];
					gTemperatureReading[si] = reading;
					if(gPrimarySensorIndex == si) gCurrentTemperature = reading;
					else if(gAuxSensorIndex == si)  gAuxTemperature = reading;
				}
				//else ,invalid. just ignore
				// if all read, return.
				sensorResponse[si]=true;
				byte s;
				for(s=0;s < gSensorNumber;s++){
					if(sensorResponse[s]) break;
				}
				if(s == gSensorNumber) return;
    		}
			// else , not ready
		} // for every sneosr
		delay(100);
		i++;
	} // while not timeout
}

#define SensorForIdle 0
#define SensorForManual 1
#define SensorForPreMash 2
#define SensorForMash 3
#define SensorForBoil 4
#define SensorForCooling 5

#if SupportDistilling
#define SensorForDistilling 6
#define NumberOfSensorStage 7
#else
#define NumberOfSensorStage 6
#endif


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

uint32_t _lastValidTempRead[MaximumNumberOfSensors];

void tpReadTemperature(void)
{
#if FakeHeating

#if  MaximumNumberOfSensors > 1
	for(int i=0;i<MaximumNumberOfSensors;i++){
		if(gTemperatureReading[i]<0) gTemperatureReading[i] =0;
	}
#endif
	return;
#endif

	for(byte si=0;si < gSensorNumber;si++)
	{
  		if (_gIsSensorConverting[si] == false)
  		{
  			if(gCurrentTimeInMS - _lastTempRead > MinimumTemperatureReadGap){				  
		  		// start conversion and return
				tpSensorRequestConvert(gSensorAddresses[si]);
    			_gIsSensorConverting[si] = true;
    			if (si == (gSensorNumber -1)){
    				_lastTempRead = gCurrentTimeInMS;
    			}
    		}
  		}
  		else
  		{
    		if (tpSensorDataReady(gSensorAddresses[si]))
    		{
    			float rawreading=_readTemperature(gSensorAddresses[si]);
				_gIsSensorConverting[si] = false;

				float reading;

				if(IS_TEMP_INVALID(rawreading))
				{
					// invalid sensor data.
					if(gSensorDisconnected[si]  &&
						(gCurrentTimeInMS - _lastValidTempRead[si] > SensorDiscGuardTime)){
						//!error case. invalidate the data
						#if SerialDebug
						DebugPort.println("Sensor disconneced!");
						#endif
						buzzPlaySound(SoundIdWarnning);
						gSensorDisconnected[si]=false; //reset to beep on next time
					}else{
						gSensorDisconnected[si]=true;
					}
				}else{
					gSensorDisconnected[si]=false;
					reading =sensorCorrection(lpfAddValue(si,rawreading),si);
					_lastValidTempRead[si] =  gCurrentTimeInMS;
					gTemperatureReading[si] = reading;
					if(gPrimarySensorIndex == si) gCurrentTemperature = reading;
					else if(gAuxSensorIndex == si)  gAuxTemperature = reading;
				}				
    		}
		}
	} // for every sensor
} // tpReadTemperature


#else //  of  MaximumNumberOfSensors > 1
#if USE_MAX6675

MAX6675 _max6675(SPI_SCK,SPI_CS,SPI_MISO);
// single sensor
#undef ResolutionDecode
#define ResolutionDecode(a) 0

void tpSetSensorResolution(byte *addr, byte res){}

void tpReadTemperature(void){
	if(gCurrentTimeInMS - _lastTempRead < MinimumTemperatureReadGap) return;

	_lastTempRead = gCurrentTimeInMS;

	float reading = gIsUseFahrenheit? _max6675.readFahrenheit(): _max6675.readCelsius();

	gCurrentTemperature = sensorCorrection() 
}

void tpInitialize(void){
	gCurrentTemperature = INVALID_TEMP_C;
	gBoilStageTemperature=readSetting(PS_BoilTemp);
	calTemperatureCalibration();
	tpReadTemperature();
}


#else // of #if USE_MAX6675

float _filteredValue;
void lpfSetInitialValue(float value){
	_filteredValue= value;
}

float lpfAddValue(float value){
	_filteredValue= _filteredValue + LowPassFilterParameter *(value - _filteredValue);
	return _filteredValue;
}


void tpSensorRequestConvert(void){
	ds.reset();
	ds.skip();
    ds.write(DSCMD_CONVERT_T, 0);
}

bool tpSensorDataReady(void){
 	ds.reset();
	ds.skip();
    byte busy = ds.read_bit();
    if (busy == 0) return false;
	return true;
}


bool tpSensorRead(float* pReading){
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
      	return false;
    }
	// data got!
#if EnableSensorResolution	== true
	gSensorResolution=_sensorData[4] & 0x60;
#endif

	// data got!
    unsigned int raw = (_sensorData[1] << 8) + _sensorData[0];

    // at lower res, the low bits are undefined, so let's zero them
    if (gSensorResolution == 0x00) raw = raw & ~7;  // 0.5C 9 bit resolution, 93.75 ms
    else if (gSensorResolution == 0x20) raw = raw & ~3; // 0.25C 10 bit res, 187.5 ms
    else if (gSensorResolution == 0x40) raw = raw & ~1; // 0.125C 11 bit res, 375 ms
	// 0x60  0.0625C 12bits, 750ms

	float reading = raw  * 0.0625;
	if(!ValidSensorValueC(reading)) return false;

	if(gIsUseFahrenheit) reading = C2F(reading);
	*pReading = reading;
	return true;
}

void tpReadInitialTemperature(void){
	tpSensorRequestConvert();
	int i=0;
	while(i< 10){
		if(tpSensorDataReady()) break;
		delay(100);
		i++;
	}
	float reading;
	if(tpSensorRead(&reading)){
		lpfSetInitialValue(reading);
		gCurrentTemperature= sensorCorrection(reading);
	}
}

void tpReadTemperature(void)
{
#if FakeHeating
	return;
#endif

  	if (_isConverting == false)
  	{
	  	if(gCurrentTimeInMS - _lastTempRead > MinimumTemperatureReadGap){
		  	// start conversion and return
			tpSensorRequestConvert();
    		_isConverting = true;

			_lastTempRead = gCurrentTimeInMS;
		}
    	return;
  	}
  	// else if convert start
  	//if (_isConverting)
  	//

  	// check for conversion if it isn't complete return if it is then convert to decimal
	if(! tpSensorDataReady()) return;

	float reading;
	if(! tpSensorRead(&reading)){
		// CRC Error;
		tpSensorRequestConvert();
      	_isConverting = true;
      	return;
    }
    //apply calibration
    gCurrentTemperature = sensorCorrection(lpfAddValue(reading));
    _isConverting = false;
}
#endif //(not) else of USE_MAX6675
#endif // #if MaximumNumberOfSensors > 1

// *************************
//*  pump related function
// *************************

class RestableDevice
{
    bool _isDeviceOn;
    bool _physicalOn;
    unsigned long _lastSwitchOnTime;
    float _stopTemp;

    unsigned long _restTime;
    unsigned long _cycleTime;
    bool _isRestStateChanged;
    bool _restEnabled;

#if EnableLevelSensor
	bool _forcedRest;
#endif

    void virtual deviceOn(void)=0;
    void virtual deviceOff(bool)=0;
public:
    RestableDevice(void)
    {
	    _physicalOn=false;
	    _isDeviceOn=false;
	    _isRestStateChanged = false;
	    _restEnabled=false;
#if EnableLevelSensor
		_forcedRest = false;
#endif

    }

    void turnPhysicalOn(void)
    {
	    if(_physicalOn) return;
    	deviceOn();
	    _physicalOn=true;
	    _lastSwitchOnTime = gCurrentTimeInMS;
    }

    void turnPhysicalOff(void)
    {
        deviceOff(_isDeviceOn);
		_physicalOn=false;
    }

    void setCycle(unsigned long cycle, unsigned long rest)
    {
    	_restTime =rest;
	    _cycleTime=cycle;
    }

    void resetRest(void)
    {
	    _restEnabled=false;
    	_isRestStateChanged = false;
    }

    void setStopTemp(float temp)
    {
        _stopTemp = temp;
    }

    void off(void)
    {
	    if(!_isDeviceOn) return;
	    _isDeviceOn=false;
	    turnPhysicalOff();
    }

    void on(void)
    {
	    if(_isDeviceOn) return;
	    _isDeviceOn=true;

	    turnPhysicalOn();
    }

    void toggle(void)
    {
	    if(_isDeviceOn)
		    off();
	    else
		    on();
    }

    bool isRestEnabled(void)
    {
	    return _restEnabled;
    }
    bool isOn(void)
    {
        return _isDeviceOn;
    }

    bool isRest(void)
    {
	    return ! _physicalOn;
    }
	bool isPhysicalOn(void)
	{
		return  _physicalOn;
	}
#if EnableLevelSensor
	void setForcedRest(bool rest){
		_forcedRest = rest;
		
		if(!_forcedRest){
			// end of forced Rest, 
			// reset the time. so that the pump will be turned on in run()
			_lastSwitchOnTime = gCurrentTimeInMS;
			if(_isDeviceOn) _isRestStateChanged = true;
		}
	}
#endif

    void setRestEnabled(boolean enable)
    {
	    _restEnabled=enable;

    	if(_restEnabled && _physicalOn) // restart counting time
	    	_lastSwitchOnTime = gCurrentTimeInMS;

    	if(!_restEnabled && _isDeviceOn)
	    {
            if(!_physicalOn)
            {
			    turnPhysicalOn();
		    }
	    }
    }

    boolean restEvent(void)
    {
	    if(_isRestStateChanged)
	    {
		    _isRestStateChanged = false;
		    return true;
	    }
	    return false;
    }

    void run(void)
    {
	    if(!_isDeviceOn) return;

#if EnableLevelSensor
		if(_forcedRest){
      	    if(_physicalOn)
      	    {
			    turnPhysicalOff();
        	    _isRestStateChanged = true;
		    }
			return;
		}
#endif

	    // overheat temperature protection
	    if(!IS_TEMP_INVALID(gCurrentTemperature) &&  (gCurrentTemperature >= _stopTemp))
	    {
      	    if(_physicalOn)
      	    {
			    turnPhysicalOff();

				// don't affect heating status if it stopTemp hit
				//if(!IS_TEMP_INVALID(gCurrentTemperature)) _isRestStateChanged = true;
		    }
		    return;
  	    }
  	    else // of if(gCurrentTemperature >= _pumpStopTemp)
  	    {
			// if under pump stop temperature

			// device is "ON" (or it returns at beginning of this function)
		    if(!_restEnabled)
		    {
				// not in REST state.
      		    if(!_physicalOn) turnPhysicalOn();
			    return;
		    }
         	if((gCurrentTimeInMS - _lastSwitchOnTime) < (unsigned long)_cycleTime)
          	{
      	    	if(!_physicalOn)
      		    {
	      		    DBG_PRINTF("-End Pump Rest-");
      			    turnPhysicalOn();
        		    _isRestStateChanged = true;
        	    }
      	    }
      	    else
      	    {
	  		    if(_restTime>0)
	  		    {
      		        // pump rest state, heat will be off!
      			    if(_physicalOn)
      			    {
      				    DBG_PRINTF("-Pump Rest-");
      				    turnPhysicalOff();
        			    _isRestStateChanged = true;
        		    }
        	    }
        	    if ((gCurrentTimeInMS - _lastSwitchOnTime) >= (_cycleTime + _restTime))
        	    {
        		    _lastSwitchOnTime = gCurrentTimeInMS;
					// pump will be truned-on on next "run()" call.
        		    //Serial.println("on time start");
        	    }
      	    }
        } // end of else //if if(gCurrentTemperature >= _pumpStopTemp)

    }//pumpThread()
};

class PumpControl: public RestableDevice
{
	bool _inverted;
    public:
    PumpControl(){}

    void virtual deviceOn(void)
    {
	    setPumpOut(_inverted? LOW:HIGH);
	    uiPumpStatus(PumpStatus_On);
	    wiReportPump(PumpStatus_On);
    }

    void virtual deviceOff(bool programOff)
    {
        setPumpOut(_inverted? HIGH:LOW);
	    if(programOff){
		    uiPumpStatus(PumpStatus_On_PROGRAM_OFF);
		    wiReportPump(PumpStatus_On_PROGRAM_OFF);
	    }else{
		    uiPumpStatus(PumpStatus_Off);
		    wiReportPump(PumpStatus_Off);
	    }
    }
    void loadParameters(void)
    {
	    setStopTemp( (float) readSetting(PS_TempPumpRest));
	    resetRest();
		_inverted = (bool) readSetting(PS_PumpActuatorInverted);

    #if UsePaddleInsteadOfPump
        setCycle((unsigned long) readSetting(PS_PumpCycle) *1000,(unsigned long) readSetting(PS_PumpRest) *1000);
    #else
        setCycle((unsigned long) readSetting(PS_PumpCycle) *60*1000,(unsigned long) readSetting(PS_PumpRest)*60 *1000);
    #endif
    }

	void updateUI() {
		if ( isOn() ) {
			uiPumpStatus(PumpStatus_On);
		    wiReportPump(PumpStatus_On);
		} else {
			uiPumpStatus(PumpStatus_Off);
			wiReportPump(PumpStatus_Off);
		}
	}
};

PumpControl pump;

// *************************
//*  heating related function
// *************************
bool _hopStanding;

bool _physicalHeattingOn;
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


#if SecondaryHeaterSupport == true
// two way we can do it
// individual status, => ON, Off, suspend x2
//   possiblie combination
//    1     2
//   ON     ON   Both on
//   ON     OFF  Primary
//   OFF    ON   Secondary
//   OFF    OFF  OFF
//   SUS    SUS  Suspended
//   other combinations are invalid
//   Q: in suspended, UI(display) needs to know the "ON" status
//   ON/OFF/SUSPEND flag with "elements" usage
//
#define PrimaryHeaterMask  1
#define SecondaryHeaterMask  2
#define BothHeaterMask  3

byte _gElementInUseMask;

void setHeatingElementsInUse(byte elements)
{
    if(_gElementInUseMask == elements) return;
    _gElementInUseMask = elements;
    // change PID
    byte kp, ki, kd;
    if(_gElementInUseMask == PrimaryHeaterMask){
        kp =readSetting(PS_kP);
        ki=readSetting(PS_kI);
        kd=readSetting(PS_kD);
	}else if(_gElementInUseMask == PrimaryHeaterMask){
        kp =readSetting(PS_kP_Secondary);
        ki=readSetting(PS_kI_Secondary);
        kd=readSetting(PS_kD_Secondary);
	}else{
        kp =readSetting(PS_kP_AllOn);
        ki=readSetting(PS_kI_AllOn);
        kd=readSetting(PS_kD_AllOn);
	}

    thePID.SetTunings(kPfromEeprom(kp),
					  kIfromEeprom(ki),
					  kDfromEeprom(kd),
					  readSetting(PS_POM)? P_ON_M:P_ON_E);
}

#define HeatingStagePreMash 0
#define HeatingStageMashing 1
#define HeatingStageBoiling 2
#define HeatingStagePostBoil 3


void setHeatingElementForStage(byte stage)
{
    byte elements=readSetting(PS_PreMashHeating + stage);
    setHeatingElementsInUse(elements);
}


void saveTunning(void)
{
	// update constant
	double  kp = autoTune.GetKp();
    double  ki = autoTune.GetKi();
    double  kd = autoTune.GetKd();
    thePID.SetTunings(kp,ki,kd);

    byte kpAddr,kiAddr,kdAddr;
    if(_gElementInUseMask == PrimaryHeaterMask){
        kpAddr = PS_kP;
        kiAddr = PS_kI;
        kdAddr = PS_kD;
	}else if(_gElementInUseMask == PrimaryHeaterMask){
        kpAddr= PS_kP_Secondary;
        kiAddr= PS_kI_Secondary;
        kdAddr= PS_kD_Secondary;
	}else{
        kpAddr = PS_kP_AllOn;
        kiAddr = PS_kI_AllOn;
        kdAddr = PS_kD_AllOn;
	}


    updateSetting(kpAddr,kPtoEeprom(kp));
	updateSetting(kiAddr,kItoEeprom(ki));
	updateSetting(kdAddr,kDtoEeprom(kd));
	wiSettingChanged(kdAddr,kDtoEeprom(kd));// notify setting change.
	commitSetting();
}

#else
void saveTunning(void)
{
	// update constant
	double  kp = autoTune.GetKp();
    double  ki = autoTune.GetKi();
    double  kd = autoTune.GetKd();
    thePID.SetTunings(kp,ki,kd);

    updateSetting(PS_kP,kPtoEeprom( kp));
	updateSetting(PS_kI,kItoEeprom( ki ));
	updateSetting(PS_kD,kDtoEeprom( kd ));
	wiSettingChanged(PS_kD,kDtoEeprom( kd ));// notify setting change.
	commitSetting();
}
#endif

#endif //#if EnablePidAutoTune == true


#if FakeHeating == true
unsigned long lastTime;
#endif

void heatPhysicalOn(void)
{
	if(!_physicalHeattingOn)
	{
		_physicalHeattingOn=true;

	    #if SecondaryHeaterSupport == true
	    bool primary = _gElementInUseMask & PrimaryHeaterMask;
	    bool secondary =_gElementInUseMask & SecondaryHeaterMask;
	    if(primary) setHeaterOut(HIGH);
	    if(secondary) setSecondaryHeaterOut(HIGH);

		uiHeatingStatus(primary? HeatingStatus_On:HeatingStatus_Off,secondary? HeatingStatus_On:HeatingStatus_Off);
		wiReportHeater(primary? HeatingStatus_On:HeatingStatus_Off,secondary? HeatingStatus_On:HeatingStatus_Off);

	    #else
		setHeaterOut(HIGH);
		uiHeatingStatus(HeatingStatus_On);
		wiReportHeater(HeatingStatus_On);
		#endif

#if FakeHeating == true
	lastTime = gCurrentTimeInMS;
#endif

	}
}

void heatPhysicalOff(void)
{
#if SecondaryHeaterSupport == true

	if(_physicalHeattingOn)
	{
		setHeaterOut(LOW);
		setSecondaryHeaterOut(LOW);
		_physicalHeattingOn=false;
	}
	if(gIsHeatOn){

	    bool primary = _gElementInUseMask & PrimaryHeaterMask;
	    bool secondary =_gElementInUseMask & SecondaryHeaterMask;

		uiHeatingStatus(primary? HeatingStatus_On_PROGRAM_OFF:HeatingStatus_Off,secondary? HeatingStatus_On_PROGRAM_OFF:HeatingStatus_Off);
		wiReportHeater(primary? HeatingStatus_On_PROGRAM_OFF:HeatingStatus_Off,secondary? HeatingStatus_On_PROGRAM_OFF:HeatingStatus_Off);
	}else{
		uiHeatingStatus(HeatingStatus_Off,HeatingStatus_Off);
		wiReportHeater(HeatingStatus_Off,HeatingStatus_Off);
	}


#else
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
#endif
}


#if SpargeHeaterSupport == true

void heaterSpargeOn(void)
{
	DBG_PRINTF("Sparge Heater On\n");
	gIsPhysicalSpargeWaterHeating=true;
	setSpargeHeaterOut(HIGH);
	uiAuxHeatingStatus(HeatingStatus_On);
	wiReportAuxHeater(HeatingStatus_On);
}

void heaterSpargeOff(void)
{
	DBG_PRINTF("Sparge Heater OFF\n");
	gIsPhysicalSpargeWaterHeating=false;
	setSpargeHeaterOut(LOW);

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
void requestHeaterOn(void)
{
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
}
void requestHeaterOff(void)
{
		// turn off Main Heating
		heatPhysicalOff();

		if(_gPowerState == PowerStateHeating){
			// turn off. Sparge heater might be On later
			_gPowerState = PowerStateIdle;
			_gTimeEnterIdle= millis();
		}else if( _gPowerState == PowerStateHeatPending){
			 _gPowerState = PowerStateIdle;
		}
	//DBG_PRINTF("requestHeaterOn:%d, %d, %d\n",on,_gPowerState,_physicalHeattingOn);
}

void requestSpargeHeaterOn(void)
{
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
}

void requestSpargeHeaterOff(void)
{
		heaterSpargeOff();
		if(_gPowerState == PowerStateSpargeHeating){
			// turn off Sparge Heater.
			_gPowerState = PowerStateIdle;
			_gTimeEnterIdle= millis();
		}else if (_gPowerState == PowerStateSpargePending){
			_gPowerState = PowerStateIdle;
		}
}


void spargeHeaterControl(void)
{
	if(!gHeatSpargeWater) return;
#if MaximumNumberOfSensors > 1
	if(gEnableSpargeWaterTemperatureControl){
	 	if(gSpargeWaterTemperature <= (gSpargeWaterSetPoint - gSpargeTemperatureDifference)){
 			if(!gIsPhysicalSpargeWaterHeating)
	 			requestSpargeHeaterOn();
 		}else if(gSpargeWaterTemperature >= (gSpargeWaterSetPoint + gSpargeTemperatureDifference)){
	 		if(gIsPhysicalSpargeWaterHeating)
		 		requestSpargeHeaterOff();
 		}
 	}
 	else
#endif
 	{
 		// no temp. control, just output
 		if(!gIsPhysicalSpargeWaterHeating)
	 		requestSpargeHeaterOn();
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
	requestSpargeHeaterOff(); // to draw the icon
}

void stopHeatingSpargeWater(void)
{
	gHeatSpargeWater=false;
	requestSpargeHeaterOff();
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
	thePID.SetTunings(kPfromEeprom(readSetting(PS_kP)),
					  kIfromEeprom(readSetting(PS_kI)),
					  kDfromEeprom(readSetting(PS_kD)),
					  readSetting(PS_POM)? P_ON_M:P_ON_E);
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
	requestHeaterOff();
	#else
	heatPhysicalOff();
	#endif
}

#if PwmHeatingSupport
typedef enum _HeatingMode{
	HeatingModePID,
	HeatingModeBoil,
	HeatingModePWM
} HeatingMode;

HeatingMode _heatingMode;

#else
boolean _isPIDMode;
#endif

#if PwmHeatingSupport
void heatOn(HeatingMode mode=HeatingModePID)
{
	_heatingMode=mode;

#else
void heatOn(bool pidmode=true)
{
	_isPIDMode=pidmode;
#endif
	gIsHeatOn = true;
	gIsHeatProgramOff=false;

	// should run through heating algorithm first
	// so that the correct symbol can be shown
	_windowStartTime=millis();

	#if SpargeHeaterSupport
	requestHeaterOff();
	#else
	heatPhysicalOff();
	#endif
	_hopStanding=false;
}

void hsHeatOn(void){
	heatOn();
	_hopStanding=true;	
}

void heatProgramOff(void)
{
	gIsHeatProgramOff=true;

	#if SpargeHeaterSupport
	requestHeaterOff();
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

	// heat is ON by requested following.
	if(IS_TEMP_INVALID(gCurrentTemperature)) {
		if(_physicalHeattingOn) {
#if SpargeHeaterSupport
			requestHeaterOff();
#else
			heatPhysicalOff();
#endif
		}
		return;
	}

#if PwmHeatingSupport
	if(_heatingMode == HeatingModePID
#else
	if(_isPIDMode 
#endif	
	&& readSetting(PS_HeatOnPump)){
		if(! pump.isPhysicalOn()){
			if(_physicalHeattingOn) {

				#if SpargeHeaterSupport
				requestHeaterOff();
				#else
				heatPhysicalOff();
				#endif
			}
			return;
		}
	}

 	pidInput = gCurrentTemperature;
	pidSetpoint= gSettingTemperature;

#if PwmHeatingSupport
	if(_heatingMode == HeatingModePID)
#else
  	if (_isPIDMode) //PID mode
#endif
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
    		if (((pidSetpoint - pidInput) < gPidStart ) && ( pidSetpoint < gBoilStageTemperature) ) // Delta < DeltaPID(3.5),
    		{
        		thePID.Compute();   // was 6, getting close, start feeding the PID -mdw
    		}
    		else // if (Rapporto < 1.00)
    		{
      			pidOutput = 255;      // was 5, ignore PID and go full speed -mdw  // set the output to full on
    		}
    	}
  	}// end of PID mode
  	else
#if PwmHeatingSupport
	if(_heatingMode == HeatingModeBoil)
#endif
  	{
	  	pidOutput = 255;  // NOT PID mode. just turn on full power.
  	}

  	// In boiling stage, the output value is reassigned.
#if PwmHeatingSupport
	if(_heatingMode == HeatingModePWM){
		pidOutput = gBoilHeatOutput * 255.0 / 100.0;
		//DebugPort.printf("gIsHeatProgramOff=%d, gBoilHeatOutput=%d pidOutput=",gIsHeatProgramOff,gBoilHeatOutput);
		//DebugPort.println(pidOutput);
	} else
#endif
	// the logic assumes always "heating", which is not true in hopstand
	// say, current temperature is 99, setpoint is 80, boil temp is 95
	// the PWM mode is always running.
	//if (pidInput >= pidSetpoint && pidInput >= gBoilStageTemperature)
	//	pidOutput = gBoilHeatOutput * 255.0 / 100.0;

	if(! _hopStanding &&
		(pidInput >= pidSetpoint && pidInput >= gBoilStageTemperature))
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
  	if (now - _windowStartTime > (unsigned long) _heatWindowSize * 250)
  	{
    	_windowStartTime += (unsigned long)_heatWindowSize * 250;
    	//time to shift the Relay Window
  	}

  	if ((pidOutput / 255) * ((unsigned long)_heatWindowSize * 250) > now - _windowStartTime)
  	{
  		if(!_physicalHeattingOn)
  		{
  			#if SpargeHeaterSupport
			requestHeaterOn();
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
			requestHeaterOff();
	  		#else
  			heatPhysicalOff();
  			#endif
  		}
  	}
} // end of heaterControl

void heatThread(void)
{

#if FakeHeating == true
#if MaximumNumberOfSensors > 1
	if(_physicalHeattingOn){
		for(byte i=0;i<gSensorNumber;i++)
			gTemperatureReading[i] += (gCurrentTimeInMS - lastTime) * (0.0004 + i*0.0001);
	}else{
		for(byte i=0;i<gSensorNumber;i++)
			gTemperatureReading[i] -= (gCurrentTimeInMS - lastTime) * (0.000025+ i*0.0007);
	}
	gCurrentTemperature = gTemperatureReading[gPrimarySensorIndex];
	gAuxTemperature = gTemperatureReading[gAuxSensorIndex];
#else
	if(_physicalHeattingOn){
    	if(gCurrentTemperature < gBoilStageTemperature+1)
    		gCurrentTemperature += (gCurrentTimeInMS - lastTime) * 0.0004;
		lastTime = gCurrentTimeInMS;
	}else{
	    if(gCurrentTemperature > 30)
		    gCurrentTemperature -= (gCurrentTimeInMS - lastTime) * 0.000025;
		lastTime = gCurrentTimeInMS;
	}
#endif
#endif

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

void setSettingTemperature(float temp)
{
    gSettingTemperature=temp;
    uiSetSettingTemperature(gSettingTemperature);
    brewLogger.setPoint(gSettingTemperature);
    wiReportSettingTemperature();
}



// *************************
//*  Wort/Water level sensor // for K-RIMS
// *************************
#if EnableLevelSensor

#define WL_IDLE 0
#define WL_REST 1
#define WL_REST_MINIMUM 2

class WaterLevelMonitor{
	bool _monitoring;
	bool _full;
	bool _mightFull;
	uint8_t _state;
	uint32_t _lastChanged;
	uint32_t _lastPumpSwitch;
	uint32_t _pumpExtendedTime;
	uint32_t _sensorMinTriggerTime;
public:
	WaterLevelMonitor(){
		_monitoring =false;
		_full=false;
		_mightFull=false;
		_state = WL_IDLE;
	}

	void startMonitor(){
		DBG_PRINTF("start monitoring level\n");
		_monitoring= true;
		_state = WL_IDLE;
	}

	void stopMonitor(){
		DBG_PRINTF("stop monitoring level\n");

		_monitoring =false;
		if(_state == WL_REST_MINIMUM  || _state == WL_REST){
			pump.setForcedRest(false);
		}
	}

	bool isFull(){
		return _full;
	}
	
	bool checkState(){
		bool fullness=isWaterLevelFull();

		if(fullness != _mightFull){
			_mightFull = fullness;
			_lastChanged = gCurrentTimeInMS;
			DBG_PRINTF("waterlevel attempt\n");
			return false;
		} else{
			// the same, check time
			if( _full != _mightFull
			    && (gCurrentTimeInMS - _lastChanged) >= _sensorMinTriggerTime){
					// meet minimum time requirement
					DBG_PRINTF("waterlevel full:%d\n",_mightFull);
					_full = _mightFull;
					return true;
				}
		}

		return false;
	}
	void run(){
		if(!_monitoring) return;
		if(checkState()){
			// state changed
			if(_full){
				pump.setForcedRest(true);
				_state = WL_REST;
			}else{
				// switch indicates NOT FULL
				if(_state == WL_REST){
					_lastPumpSwitch=gCurrentTimeInMS;
					_state = WL_REST_MINIMUM;
				}
			}
		}
		if(_state == WL_REST_MINIMUM){
			// pump to non=rest
			// check if minimum off time meets
			if((gCurrentTimeInMS - _lastPumpSwitch) >= _pumpExtendedTime){
				pump.setForcedRest(false);
				_state = WL_IDLE;
			}
		}
	}
	void loadParameters(){
		// load from EEPROM
		_sensorMinTriggerTime = readSetting(PS_LevelSensorMinimumTime) * 50; // 100ms unit
		_pumpExtendedTime = readSetting(PS_PumpRestExtendedTime) * 1000; // 1s

	}
	void reset(){
		stopMonitor();
	}

};

WaterLevelMonitor lvMonitor;
#endif

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
	automation.setStageTime(stage,time);
}

void changeAutomationTemperature(byte stage, float value)
{
	automation.setStageTemperature(stage, value);
}

void finishAutomationEdit(void)
{
    automation.save();
	wiRecipeChange();
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
//*  Generic Setting
// *************************

typedef struct _SettingItem{
    const char* title;
    void (*display)(int);
    byte address;
    int16_t max;
    int16_t min;
} SettingItem;

class SettingEditor{

protected:
    byte _currentSettingAddress;
    byte _currentSettingIndex;
    const SettingItem *_SettingItems;
 	
	SettingItem _item;
	int _itemValue;

    int (*_getValue)(int index);
    void (*_setValue)(int index, int value);

public:
    SettingEditor(void){}

    void setup(const SettingItem * items,int (*getter)(int index), void (*setter)(int index, int value))
    {
	    uiButtonLabel(ButtonLabel(Up_Down_x_Ok));
	    _currentSettingIndex=0;
	    _SettingItems=items;
	    _getValue = getter;
	    _setValue = setter;
    }
    void setup(const SettingItem * items)
    {
        setup(items,NULL,NULL);
    }

    void getItemDetail(void)
    {
 	    const char* addr=(const char*) & _SettingItems[_currentSettingIndex];

        char *dst=(char*) &_item;
 	    for (int i=0; i< (int)sizeof(SettingItem); i++)
        {
            dst[i] =	pgm_read_byte_near(addr + i);
        }
        if( _item.address ==0 && _getValue != NULL) _itemValue=_getValue(_currentSettingIndex);
	    else _itemValue=(int)readSetting(_item.address);
		
		_currentSettingAddress=_item.address;
    }

    void nextItem(void)
    {
        _currentSettingIndex++;
        displayItem();
    }

    void nextItem(byte max, byte min)
    {
        _currentSettingIndex++;
	    getItemDetail();
	    editItem(_item.title,_itemValue,max,min,_item.display);
    }

    void displayItem(void)
    {
	    getItemDetail();
	    //editItem(str_t label, byte value, byte max, byte min,CDisplayFunc displayFunc)
	    editItem(_item.title,_itemValue,_item.max,_item.min,_item.display);
    }

    void displayItem(byte max, byte min)
    {
	    getItemDetail();
	    //editItem(str_t label, byte value, byte max, byte min,CDisplayFunc displayFunc)
	    editItem(_item.title,_itemValue,max,min,_item.display);
    }

    byte index(void){ return _currentSettingIndex; }
    void setIndex(byte index) {_currentSettingIndex = index; }

    bool buttonHandler(void)
    {
	    if(btnIsEnterPressed)
	    {
		    byte value=(byte)editItemValue();

		    if( _currentSettingAddress ==0 && _setValue != NULL) _setValue(_currentSettingIndex,value);
		    changeSettingValue(_currentSettingAddress,value);

            return true;
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
	    return false;
    }
};
SettingEditor settingEditor;
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

void displayMultiply50(int data)
{
	float fvalue=(float)data *50.0;
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
/*
void displayActivePassive(int value)
{
	if (value==0) uiSettingDisplayText(STR(Passive));
	else uiSettingDisplayText(STR(Active));
}*/

#if UsePaddleInsteadOfPump  || EnableLevelSensor
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

//#if NoWhirlpool != true
#define WhirlpoolHot 2
#define WhirlpoolCold 1
#define WhirlpoolOff 0

void displayHotColdOff(int value)
{
	if (value==0) uiSettingDisplayText(STR(Off));
	else if (value==WhirlpoolCold) uiSettingDisplayText(STR(Cold));
	else uiSettingDisplayText(STR(Hot));
}
//#endif

//**************************************************************
//* Distill
//**************************************************************
#if SupportDistilling

#define DistillHeadPwm 0
#define DistillHeartPwm 1
#define DistillTailPwm 2
#define DistillHeadTemp 0
#define DistillHeartTemp 1
#define DistillTailTemp 2
#define DistillEndTemp 3

class DistillRecipe{
	uint8_t _pwms[3];
	uint8_t _temps[4];
	bool _loaded;
public:
	DistillRecipe(){ _loaded=false;}
	void load(){
		if(_loaded) return;
		int addr=PS_Distill_Base;
		for(int i=0; i< 3; i++){
			_pwms[i]=readSetting(addr);
			addr ++;
		}
		for(int i=0; i< 4; i++){
			_temps[i]=readSetting(addr);
			addr ++;
		}
		_loaded=true;
	}
	
	void save(){
		int addr=PS_Distill_Base;
		for(int i=0; i< 3; i++){
			updateSetting(addr,_pwms[i]);
			addr ++;
		}
		for(int i=0; i< 4; i++){
			updateSetting(addr,_temps[i]);
			addr ++;
		}		
	}

	uint8_t pwmOf(uint8_t index){ return _pwms[index]; }
	void   setPwmOf(uint8_t index, uint8_t value){ _pwms[index]=value; }
	uint8_t tempOf(uint8_t index){ return _temps[index]; }
	void   setTempOf(uint8_t index, uint8_t value){ _temps[index]=value; }

} distillRecipe;

const SettingItem distillRecipeItems[] PROGMEM=
{
/*0*/{STR(StartTemp),          & displaySimpleTemperature,  0,100,35},
/*1*/{STR(HeadPwm),          & displayPercentage,  0, 100,0},
/*2*/{STR(HeartTemp),          & displaySimpleTemperature,  0, 100,35},
/*3*/{STR(HeartPwm),          & displayPercentage,  0, 100,0},
/*4*/{STR(TailTemp),          & displaySimpleTemperature,  0, 100,35},
/*5*/{STR(TailPwm),          & displayPercentage,  0, 100,0},
/*6*/{STR(EndTemp),          & displaySimpleTemperature,  0, 100,35}
};

int distillGetValue(int index)
{
    if(index == 0) return distillRecipe.tempOf(DistillHeadTemp);
    else if(index == 1) return distillRecipe.pwmOf(DistillHeadPwm);
    else if(index == 2) return distillRecipe.tempOf(DistillHeartTemp);
    else if(index == 3) return distillRecipe.pwmOf(DistillHeartPwm);
    else if(index == 4) return distillRecipe.tempOf(DistillTailTemp);
    else if(index == 5) return distillRecipe.pwmOf(DistillTailPwm);
    else if(index == 6) return distillRecipe.tempOf(DistillEndTemp);
	return 0;
}

void distillSetValue(int index, int value)
{
    if(index == 0) return distillRecipe.setTempOf(DistillHeadTemp,(uint8_t)value);
    else if(index == 1) return distillRecipe.setPwmOf(DistillHeadPwm,(uint8_t)value);
    else if(index == 2) return distillRecipe.setTempOf(DistillHeartTemp,(uint8_t)value);
    else if(index == 3) return distillRecipe.setPwmOf(DistillHeartPwm,(uint8_t)value);
    else if(index == 4) return distillRecipe.setTempOf(DistillTailTemp,(uint8_t)value);
    else if(index == 5) return distillRecipe.setPwmOf(DistillTailPwm,(uint8_t)value);
    else if(index == 6) return distillRecipe.setTempOf(DistillEndTemp,(uint8_t)value);
}

void distillRecipeSetup(void)
{
	distillRecipe.load();
	settingEditor.setup(distillRecipeItems,& distillGetValue,& distillSetValue);
	if(gIsUseFahrenheit)
		settingEditor.displayItem(221,100);
	else
		settingEditor.displayItem();

}

bool distillRecipeEventHandler(byte)
{

	if(settingEditor.buttonHandler()){
	    size_t index =(size_t) settingEditor.index();
		if(index  >= (sizeof(distillRecipeItems)/sizeof(SettingItem) -1)){
			distillRecipe.save();
	        switchApplication(SETUP_SCREEN);
            return true;
	    }

		if(gIsUseFahrenheit && ((index +1) & 1)==0){ // %2 == 0, temperature
			settingEditor.nextItem(221,100);
        }else
            settingEditor.nextItem();

	}
	return true;
}
#endif //#if SupportDistilling
//**************************************************************
//* PID PWM setting screen
//**************************************************************
const SettingItem pidSettingItems[] PROGMEM=
{
 {STR(PoM),     &displayYesNo,              PS_POM,1,0},
 #if SecondaryHeaterSupport == true
/*1*/{STR(kP_1),          & displayOffset100,  PS_kP,200,0},
/*2*/{STR(kI_1),          & displayOffset100,  PS_kI, 255,0},
/*3*/{STR(kD_1),          & displayOffset100,  PS_kD, 200,0},
/*4*/{STR(kP_2),          & displayOffset100,  PS_kP_Secondary,200,0},
/*5*/{STR(kI_2),          & displayOffset100,  PS_kI_Secondary, 255,0},
/*6*/{STR(kD_2),          & displayOffset100,  PS_kD_Secondary, 200,0},
/*7*/{STR(kP_both),          & displayOffset100,  PS_kP_AllOn,200,0},
/*8*/{STR(kI_both),          & displayOffset100,  PS_kI_AllOn, 255,0},
/*9*/{STR(kD_both),          & displayOffset100,  PS_kD_AllOn, 200,0},
#else
/*1*/{STR(kP),          & displayOffset100,  PS_kP,200,0},
/*2*/{STR(kI),          & displayOffset100,  PS_kI, 255,0},
/*3*/{STR(kD),          & displayOffset100,  PS_kD, 200,0},
#endif
/*4,10*/{STR(SampleTime),  & displayMultiply250,PS_SampleTime,8000/250 /*3500/250*/,1500/250},
/*5,11*/{STR(WindowSet_ms),& displayMultiply250,PS_WindowSize,40000/250 /*7500/250*/,4000/250},
/*6,12*/{STR(Heat_in_Boil),& displayPercentage, PS_BoilHeat,100,0},
/*7,13*/{STR(Start_PID_In),& displayTempDivide10,   PS_PID_Start,35,10},
/*8,14*/{STR(SensorResolution),&displayResolution, 0 ,12,9,},
#if MaximumNumberOfSensors > 1
/*9,15*/{STR(Calibration), & displayTempShift50Divide10,0,100,0},
#else
/*9,15*/{STR(Calibration), & displayTempShift50Divide10,PS_Offset,100,0},
#endif
/*10,16*/{ STR(TwoPointCalibration), & displayYesNo,PS_EnableTwoPointCalibration,1,0},
/*11,17*/{STR(CalibrationPoint1), & displayTempDivide10,0,2200,0},
/*12,18*/{STR(CalibrationRefPoint1), & displayTempDivide10,0,2200,0},
/*13,19*/{STR(CalibrationPoint2), & displayTempDivide10,0,2200,0},
/*14,20*/{STR(CalibrationRefPoint2), & displayTempDivide10,0,2200,0}
};

#define C

#if SecondaryHeaterSupport == true
#define SensorResolutionIndex 14
#define SensorCalibrationIndex 15

#define TwoPointCalibrationIndex 16
#define CalibrationPoint1Index 17
#define CalibrationRefPoint1Index 18
#define CalibrationPoint2Index 19
#define CalibrationRefPoint2Index 20

#else
#define SensorResolutionIndex 8
#define SensorCalibrationIndex 9

#define TwoPointCalibrationIndex 10
#define CalibrationPoint1Index 11
#define CalibrationRefPoint1Index 12
#define CalibrationPoint2Index 13
#define CalibrationRefPoint2Index 14

#endif

byte _pidSettingAux;
/***********************************/
/* if 0 is specified in address field, these functions will be call
 *  to get/set data instead of reading from the address */
/************************************/
#if	MaximumNumberOfSensors > 1

int pidGetValue(int index)
{
    if(index == SensorResolutionIndex) return ResolutionDecode(gSensorResolution) + 9;
	else if (index == CalibrationPoint1Index)return readSettingWord(PS_CalibrationReadingP1Of(_pidSettingAux));
	else if (index == CalibrationRefPoint1Index)return readSettingWord(PS_CalibrationReferenceP1Of(_pidSettingAux));
	else if (index == CalibrationPoint2Index)return readSettingWord(PS_CalibrationReadingP2Of(_pidSettingAux));
	else if (index == CalibrationRefPoint2Index)return readSettingWord(PS_CalibrationReferenceP2Of(_pidSettingAux));
    else if (index == SensorCalibrationIndex) return (int)readSetting(CalibrationAddressOf(_pidSettingAux));
	return 0;
}

void pidSetValue(int index, int value)
{
    // only for sensor resolution

    byte res= value -9;
    if(index == SensorResolutionIndex){
    	for(byte i=0;i<gSensorNumber;i++)
            tpSetSensorResolution(gSensorAddresses[i],res);
    }
	else if (index == CalibrationPoint1Index){
		updateSettingWord(PS_CalibrationReadingP1Of(_pidSettingAux),value);
	}else if (index == CalibrationRefPoint1Index){
		updateSettingWord(PS_CalibrationReferenceP1Of(_pidSettingAux),value);
	}else if (index == CalibrationPoint2Index){
		updateSettingWord(PS_CalibrationReadingP2Of(_pidSettingAux),value);
	}else if (index == CalibrationRefPoint2Index){
		updateSettingWord(PS_CalibrationReferenceP2Of(_pidSettingAux),value);
	}
    else if (index == SensorCalibrationIndex) {
		changeSettingValue(CalibrationAddressOf(_pidSettingAux),value);
		calTemperatureCalibration;
    }
}
#else //MaximumNumberOfSensors > 1
int pidGetValue(int index)
{
    if(index == SensorResolutionIndex) return ResolutionDecode(gSensorResolution) + 9;
	else if (index == CalibrationPoint1Index)return readSettingWord(PS_CalibrationReadingP1);
	else if (index == CalibrationRefPoint1Index)return readSettingWord(PS_CalibrationReferenceP1);
	else if (index == CalibrationPoint2Index)return readSettingWord(PS_CalibrationReadingP2);
	else if (index == CalibrationRefPoint2Index)return readSettingWord(PS_CalibrationReferenceP2);
	return 0;
}

void pidSetValue(int index, int value)
{
    // only for sensor resolution

    byte res= value -9;
    if(index == SensorResolutionIndex){
        tpSetSensorResolution(NULL,res);
    }
	else if (index == CalibrationPoint1Index){
		updateSettingWord(PS_CalibrationReadingP1,value);
	}else if (index == CalibrationRefPoint1Index){
		updateSettingWord(PS_CalibrationReferenceP1,value);
	}else if (index == CalibrationPoint2Index){
		updateSettingWord(PS_CalibrationReadingP2,value);
	}else if (index == CalibrationRefPoint2Index){
		updateSettingWord(PS_CalibrationReferenceP2,value);
	}
}
#endif // MaximumNumberOfSensors > 1
void settingPidSetup(void)
{
	settingEditor.setup(pidSettingItems,& pidGetValue,& pidSetValue);
	settingEditor.displayItem();
}
#if MaximumNumberOfSensors > 1
bool settingPidEventHandler(byte)
{
	if(settingEditor.buttonHandler()){
		// 
    	if(settingEditor.index() == SensorResolutionIndex){
			// "enter" in resolution, next to SensorCalibrationIndex
            if (0 == gSensorNumber){
				// if no Sensor detected/assigned, just returns.
	            switchApplication(SETUP_SCREEN);
                return true;
            }
			// start Sesnor Calibration setup. 
            _pidSettingAux=0;
            settingEditor.nextItem();
    	    editItemTitleAppendNumber(_pidSettingAux+1);
    	    return true;
		}else if (settingEditor.index() == SensorCalibrationIndex) {
	        // process multi sensor calibration
    		_pidSettingAux++;
	        if(_pidSettingAux < gSensorNumber){
    	        settingEditor.displayItem();
    	        editItemTitleAppendNumber(_pidSettingAux+1);
    	        return true;
	        }else{
				// finish sensor calibration, enter 
				// 2point calibration setup.(next item)
			}
		}else if(settingEditor.index() == TwoPointCalibrationIndex){
			if(readSetting(PS_EnableTwoPointCalibration) == 0){
				// disable 2 point calibration, back to setup screen
				// last item
				calTemperatureCalibration();
	    	    switchApplication(SETUP_SCREEN);
            	return true;			
			}
			// start complicated 4 points input of each sensors.
			_pidSettingAux=0;
            settingEditor.nextItem();
    	    editItemTitleAppendNumber(_pidSettingAux+1);
		}else if(settingEditor.index() == CalibrationRefPoint2Index ){
			_pidSettingAux++;
	        if(_pidSettingAux < gSensorNumber){
				// back to point1
				settingEditor.setIndex(CalibrationPoint1Index);
    	        settingEditor.displayItem();
    	        editItemTitleAppendNumber(_pidSettingAux+1);
    	        return true;
	        }else{
				// finish all sensors
				calTemperatureCalibration();
		        switchApplication(SETUP_SCREEN);
	            return true;
			}	
	    }
        settingEditor.nextItem();
    }
	return true;
}
#else //MaximumNumberOfSensors > 1
bool settingPidEventHandler(byte)
{
	if(settingEditor.buttonHandler())
	{
	    if(settingEditor.index() == (sizeof(pidSettingItems)/sizeof(SettingItem) -1)
			||((settingEditor.index() == TwoPointCalibrationIndex) && (readSetting(PS_EnableTwoPointCalibration) == 0))){
	        // last
			calTemperatureCalibration();

	        switchApplication(SETUP_SCREEN);
            return true;

	    }
        settingEditor.nextItem();
    }
	return true;
}
#endif //MaximumNumberOfSensors > 1
// *************************
//*  Unit Parameters settings
// *************************
const SettingItem unitSettingItems[] PROGMEM={
/* 0 */{STR(Set_Degree),   &displayDegreeSymbol,       PS_TempUnit ,1,0},
/* 1 */{STR(Temp_Boil),    &displaySimpleTemperature,  PS_BoilTemp,105,90},  // celius
/*  *///{STR(Temp_Boil),    &displaySimpleTemperature,  PS_BoilTemp,221,194}, //fahrenheit
#if UsePaddleInsteadOfPump
/* 2 */{STR(Pump_Cycle),   &displayTimeSec,            PS_PumpCycle,30,5},
/* 3 */{STR(Pump_Rest),    &displayTimeSec,            PS_PumpRest,60,0},
#else
/*2 */ {STR(Pump_Cycle),   &displayTime,               PS_PumpCycle,15,5},
/* 3 */{STR(Pump_Rest),    &displayTime,               PS_PumpRest,5,0},
#endif
/* 4 */{STR(Pump_PreMash), &displayOnOff,              PS_PumpPreMash,1,0},
/* 5 */{STR(Pump_On_Mash), &displayOnOff,              PS_PumpOnMash,1,0},
/* 6 */{STR(Pump_Mashout), &displayOnOff,              PS_PumpOnMashOut,1,0},
/* 7 */{STR(Pump_On_Boil), &displayOnOff,              PS_PumpOnBoil,1,0},
/* 8 */{STR(Pump_Stop),    &displaySimpleTemperature,  PS_TempPumpRest,105,80},  // celius
/*  *///{STR(Pump_Stop),    &displaySimpleTemperature,  PS_TempPumpRest,221,176}, // fahrenheit
/* 9 */{STR(PID_STRIKE), &displayOnOff,              PS_PID_STRIKE,1,0},

/* 10 */{STR(PID_Dough_In), &displayOnOff,              PS_PID_DoughIn,1,0},
/* 11 */{STR(PID_MaltOut),  &displayOnOff,              PS_PidPipe, 1,0},
/* 12 */{STR(Skip_Add),     &displayYesNo,              PS_SkipAddMalt,1,0},
/* 13 */{STR(Skip_Remove),  &displayYesNo,              PS_SkipRemoveMalt,1,0},
/* 14 */{STR(Skip_Iodine),  &displayYesNo,              PS_SkipIodineTest,1,0},
/* 15 */{STR(IodineTime),   &displayTimeOff,            PS_IodineTime,90,0},
/* 16 */{STR(Whirlpool),    &displayHotColdOff,         PS_Whirlpool,2,0},
/* 17 */{STR(HeatOnPump),    &displayYesNo,         PS_HeatOnPump,1,0}}; 

void settingUnitSetup(void)
{
	settingEditor.setup(unitSettingItems);
	settingEditor.displayItem();
	
}

bool settingUnitEventHandler(byte)
{
	if(settingEditor.buttonHandler())
	{
	    int index = settingEditor.index();
		if(index  >=(int) (sizeof(unitSettingItems)/sizeof(SettingItem) -1)){
	        switchApplication(SETUP_SCREEN);
            return true;
	    }
		if( index == 0 ){ // degree setting
			temperatureUnitChange(readSetting(PS_TempUnit)  );
		}

		if(gIsUseFahrenheit && ((index +1)== 1)){
			settingEditor.nextItem(221,194);
        }else if(gIsUseFahrenheit && ((index +1)== 8)){
			settingEditor.nextItem(221,176);
        }else
            settingEditor.nextItem();
    }
	return true;
}

// *************************
//*  Automation settings
// *************************
#define ToTempForEditing(t) ((int)((t)*4))
#define TempFromEditing(t)  ((float)(t)/4.0)

void displayStageTemperature(int value)
{
	float temperature=TempFromEditing(value);
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
byte _postBoilHopIndex;

int _maxHopTime; // to make sure hop time is in order
				  // will be set at BOIL time setting
				  // and every hoptime

#define MAX_STAGE_TIME 140
#define MIN_STAGE_TIME 1

void settingAutomationDisplayItem(void)
{
	int value=0;

	if(_editingStage <= MashStepMashOut /*7*/) // from MashIn,Phytase,Glucanase,Protease,bAmylase,aAmylase1,aAmylase2,MashOut
	{
		if(_editingStageAux==0){
			value = ToTempForEditing(automation.stageTemperature(_editingStage));
		}else{
			value =automation.stageTime(_editingStage);
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
		int max=(gIsUseFahrenheit)? ToTempForEditing(167):ToTempForEditing(75);
		int min=(gIsUseFahrenheit)? ToTempForEditing(68):ToTempForEditing(20);
		// Mash In:temp only
		editItem(STR(Mash_In),value,max,min,&displayStageTemperature);
	}

	else if(_editingStage >0 && _editingStage < MashStepMashOut /*7*/)
	{
		int max=(gIsUseFahrenheit)? ToTempForEditing(169):ToTempForEditing(76);
		int minTemp;

		if(_editingStage==1){
			minTemp=(gIsUseFahrenheit)? ToTempForEditing(77):ToTempForEditing(25);
		}else{
			minTemp=ToTempForEditing(automation.stageTemperature(_editingStage-1)) +1;
		}

		if (_editingStageAux == 0){
			editItem(STR(Mash_x),value,max,minTemp,&displayStageTemperature);
		}else{
			editItem(STR(Mash_x),value,MAX_STAGE_TIME,MIN_STAGE_TIME,&displayTime);
		}
		editItemTitleAppendNumber(_editingStage);
	}

	else if(_editingStage == MashStepMashOut /*7*/)
	{
		// MashOut
		if (_editingStageAux == 0){
			int max=(gIsUseFahrenheit)? ToTempForEditing(176):ToTempForEditing(80);
			int min=(gIsUseFahrenheit)? ToTempForEditing(167):ToTempForEditing(75);

			editItem(STR(Mash_out),value,max,min,&displayStageTemperature);
		}
		else
			editItem(STR(Mash_out),value,MAX_STAGE_TIME,MIN_STAGE_TIME,&displayTime);
	}
	else if(_editingStage ==8)
	{
		// 8. number of hops
		value =automation.numberOfHops();
		// boiling, need to input
		editItem(STR(Number_Of_Hops),value,10,0,&displaySimpleInteger);
	}
	else if(_editingStage ==9)
	{
		// 9. boil time
		value =automation.boilTime();
		// boiling, need to input
		editItem(STR(Boil),value,MAX_STAGE_TIME,MIN_STAGE_TIME,&displayTime);
	}
	else if(_editingStage ==10)
	{
		//10. hops
		value=automation.timeOfHop(_editingStageAux);

		if(value>_maxHopTime) value=_maxHopTime;
		//create a number
		// hop number starts from 1

		editItem(STR(Hops_Number_x),value,_maxHopTime,0,&displayTime);
		editItemTitleAppendNumber(_editingStageAux+1); //
	}
	else if(_editingStage ==11) // hop session
	{
		//11. hopstand session number
		value=automation.numberOfHopStandSession();
		//DBG_PRINTF("numberOfHopStandSession:%d\n",value);

		if(value>MAXIMUM_HSSESSION_NUMBER) value=0;
		//DBG_PRINTF("numberOfHopStandSession:%d\n",value);
		//create a number
		// hop number starts from 1
		editItem(STR(HS_Session_Number),value,MAXIMUM_HSSESSION_NUMBER,0,&displaySimpleInteger);
	}
	else if(_editingStage ==12) // hop session start temperature
	{
        value =automation.sessionStartTemperature(_editingStageAux);

		int min=(gIsUseFahrenheit)? ToTempForEditing(77):ToTempForEditing(25);
		int max;
		if (_editingStageAux == 0){
		    max = (gIsUseFahrenheit)? ToTempForEditing(212):ToTempForEditing(100);
		}else{
		    max = ToTempForEditing(automation.sessionKeepTemperature(_editingStageAux -1)-1);
		}
		if(value > max || value < min) value = max;

        editItem(STR(HS_Start),value,max,min,&displayStageTemperature);
        editItemTitleAppendNumber(_editingStageAux +1);
	}
	else  if(_editingStage ==13) // hop session keep temperature
	{
        value =automation.sessionKeepTemperature(_editingStageAux);

		int min=(gIsUseFahrenheit)? ToTempForEditing(77):ToTempForEditing(25);
		int max=ToTempForEditing(automation.sessionStartTemperature(_editingStageAux));
        if(value > max || value < min) value = max;

        editItem(STR(HS_Keep),value,max,min,&displayStageTemperature);
        editItemTitleAppendNumber(_editingStageAux +1);
	}
	else  if(_editingStage ==14) //post hop time
	{
        value =automation.hopInSession(_editingStageAux,_postBoilHopIndex);

		int max=(_postBoilHopIndex ==0)? 255:automation.hopInSession(_editingStageAux,_postBoilHopIndex-1);

        editItem(STR(PBH_x),value,max,0,&displayTime);
        editItemTitleAppendNumber(automation.postBoilHopIndex(_editingStageAux,_postBoilHopIndex));
        uiButtonLabel(ButtonLabel(Up_Down_End_More));
	}
}

void settingAutoSetup(void)
{
	_editingStage=0;
	_editingStageAux=0;
	settingAutomationDisplayItem();
}

bool settingAutoEventHandler(byte)
{
	if(btnIsEnterPressed)
	{
		int value=editItemValue();

		if(_editingStage ==0)
		{

			_editingStageAux=0;
		}
		if(_editingStage <= MashStepMashOut /*7*/)
		{
			if(_editingStageAux ==0)
			{
				changeAutomationTemperature(_editingStage,TempFromEditing(value));

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
			automation.setNumberOfHops((uint8_t) value);
			//number of hops
			_editingStage++;
		}
		else if(_editingStage == 9)
		{
			automation.setBoilTime((uint8_t) value);
			// set the maxHopTime for the first hop
			_maxHopTime=value;
			// boiling time;

			_editingStageAux=0;
			int hopsNum=automation.numberOfHops();
			if(hopsNum){
				_editingStage++;

			}else{
				finishAutomationEdit();

				//uiClearSettingRow();
				switchApplication(SETUP_SCREEN);
				return true;
			}
		}
		else if(_editingStage == 10)
		{
			automation.setTimeOfHop(_editingStageAux,(byte)value);
			// update masxHoptime for next hop
			_maxHopTime = value-1;

			int hopsNum=automation.numberOfHops();

			if(_editingStageAux == (hopsNum-1))
			{
				//finish
				//finishAutomationEdit();

				//uiClearSettingRow();
				//switchApplication(SETUP_SCREEN);
				_editingStage ++;
			}
			else
			{
				_editingStageAux++;

			}
		}
		else if(_editingStage == 11)
		{
		    automation.setNumberOfHopStandSession(value);
		    if(value ==0){
				finishAutomationEdit();
				switchApplication(SETUP_SCREEN);
				return true;

		    }else{
    		    _editingStageAux =0;
    		    _editingStage ++;
		    }
		}
		else if(_editingStage == 12) //start
		{
		    automation.setSessionStartTemperature(_editingStageAux,TempFromEditing(value));
		    _editingStage ++;
		}
		else if(_editingStage == 13) // keep
		{
		    automation.setSessionKeepTemperature(_editingStageAux,TempFromEditing(value));
		    _editingStage ++;
		    _postBoilHopIndex=0;
		}
		else if(_editingStage == 14) //post hop time
		{
		    automation.setHopInSession(_editingStageAux,_postBoilHopIndex,value);

		    _postBoilHopIndex ++;

		    if(automation.postBoilHopIndex(_editingStageAux,_postBoilHopIndex) > MAXIMUM_POST_BOIL_HOP_NUMBER
		     || (_postBoilHopIndex >= MAXIMUM_HOP_IN_HSSESSION
		        && (_editingStageAux +1) == automation.numberOfHopStandSession())){
		        // force to stop
		        automation.setNumberOfHopStandSession(_editingStageAux +1);
		        finishAutomationEdit();
				switchApplication(SETUP_SCREEN);
				return true;
		    }
		    if(_postBoilHopIndex >= MAXIMUM_HOP_IN_HSSESSION){
		        // next session
		        automation.setNumberOfHopInSession(_editingStageAux,_postBoilHopIndex+1);
    		    _editingStageAux ++;
		        _postBoilHopIndex=0;
		        _editingStage=12;
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
			_editingStage = MashStepMashOut; //7;
			_editingStageAux=0;
			settingAutomationDisplayItem();
		}
		else if (_editingStage == 14)
		{
    		int value=editItemValue();
		    automation.setHopInSession(_editingStageAux,_postBoilHopIndex,value);
		    automation.setNumberOfHopInSession(_editingStageAux,_postBoilHopIndex+1);
		    _editingStageAux ++;
		    if(_editingStageAux == automation.numberOfHopStandSession()){
		        // finish
		        finishAutomationEdit();
				switchApplication(SETUP_SCREEN);
				return true;
		    }else{
		        // next
		        _postBoilHopIndex=0;
		        _editingStage=12;
		        settingAutomationDisplayItem();
		    }
		}
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
	return true;
}// end of void settingAutoEventHandler(byte)

// *************************
//*  misc settings
// *************************
void displayIntegerPlusOne(int data)
{
	uiSettingDisplayNumber((float)data+1,0);
}
#if SecondaryHeaterSupport == true
void displayHeaterSelection(int data)
{
	if (data==1) uiSettingDisplayText(STR(PrimaryHeater));
	else if (data==2) uiSettingDisplayText(STR(SecondaryHeater));
	else uiSettingDisplayText(STR(BothHeater));
}
#endif

const SettingItem miscSettingItems[] PROGMEM=
{
/*0*/{STR(Auto_Resume),&displayYesNo, PS_AutoResume_Enabled ,1,0},
/*1*/{STR(No_Delay_Start),&displayYesNo, PS_NoDelayStart ,1,0},
/*2*/{STR(Button_Buzz), &displayYesNo, PS_ButtonFeedback,1,0},
/*3*/{STR(PumpPrime), &displaySimpleInteger, PS_PumpPrimeCount,10,0},
/*4*/{STR(PrimeOn), &displayMultiply250, PS_PumpPrimeOnTime,40,1},
/*5*/{STR(PrimeOff), &displayMultiply250, PS_PumpPrimeOffTime,40,0},
/*6*/	 {STR(Pump_Inverse),&displayYesNo, PS_PumpActuatorInverted,1,0}
#if SpargeHeaterSupport == true
/*7*/,{STR(Sparge_Heater),&displayYesNo,PS_SpargeWaterEnableAddress,1,0,}
#if MaximumNumberOfSensors >1
/*8*/,{STR(Temp_Ctrl),    &displayYesNo,PS_SpargeWaterTemperatureControlAddress,1,0},
/*9*/{STR(Sparge_Sensor), &displayIntegerPlusOne,PS_SpargeWaterSensorIndexAddress,0,1,},
/*10*/{STR(Sparge_Temp),   &displaySimpleTemperature,PS_SpargeWaterTemperatureAddress,80,75,},
/*11*/{STR(Temp_Diff),    &displayTempDivide10,PS_SpargeWaterTemperatureDifferenceAddress,20,5}
#endif
#endif
#if SecondaryHeaterSupport == true
/**/,{STR(HeaterPreMash),    &displayHeaterSelection,PS_PreMashHeating,3,1},
/**/{STR(HeaterMashing),    &displayHeaterSelection,PS_MashingHeating,3,1},
/**/{STR(HeaterBoiling),    &displayHeaterSelection,PS_BoilingHeating,3,1},
/**/{STR(HeaterPostBoil),   &displayHeaterSelection,PS_PostBoilHeating,3,1}
#endif
#if EnableLevelSensor
	,{STR(Enable_Level_Sensor),    &displayOnOff, PS_EnableLevelSensor,1,0},
	{STR(Lv_Trig),    &displayMultiply50, PS_LevelSensorMinimumTime,20,1},
	{STR(Ext_Pump_Rest),    &displayTimeSec, PS_PumpRestExtendedTime,90,2}
#endif
};

#define SpargeHeaterEnableIndex 7
#define SpargeHeaterSettingNumber 4

#define  SpargeTemperatureControlIndex 9
#define  SpargeSensorIndex 9
#define  SpargeTemperatureIndex 10

void miscSettingSetup(void)
{
	settingEditor.setup(miscSettingItems);
	settingEditor.displayItem();
}

bool miscSettingEventHandler(byte)
{
	if(settingEditor.buttonHandler())
	{
	    int index=settingEditor.index();

#if SpargeHeaterSupport == true
#if MaximumNumberOfSensors >1
        if(index == SpargeHeaterEnableIndex){
            if(!readSetting(PS_SpargeWaterEnableAddress)){
                // disable. sparge heating control. skip the following 5 items
                index += SpargeHeaterSettingNumber;
                settingEditor.setIndex(index);
            }
        }
#endif
#endif
	    if(index >=(int)(sizeof(miscSettingItems)/sizeof(SettingItem) -1)){
	        switchApplication(SETUP_SCREEN);
            return true;
	    }
#if SpargeHeaterSupport == true
#if MaximumNumberOfSensors >1
	    if( (index +1 ) == SpargeTemperatureControlIndex)
    	    settingEditor.nextItem((gSensorNumber>1)? 1:0,0);
	    else  if((index +1 ) == SpargeSensorIndex)
	        settingEditor.nextItem(gSensorNumber,1);
	    else  if( gIsUseFahrenheit && ((index +1 ) == SpargeTemperatureIndex))
            settingEditor.nextItem(176,167);
        else
#endif
#endif
            settingEditor.nextItem();
    }
	return true;
}
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
				uiSettingTitle(STR(Sensor_PostBoil));
			else
				uiSettingTitle(STR(AuxSensor_PostBoil));
		}else if(_sensorSettingIndex ==(SensorForManual+1)){
			if(_sensorSettingAux==0)
				uiSettingTitle(STR(Sensor_Manual));
			else
				uiSettingTitle(STR(AuxSensor_Manual));
		}
#if SupportDistilling
		else if(_sensorSettingIndex ==(SensorForDistilling+1)){
			if(_sensorSettingAux==0)
				uiSettingTitle(STR(Sensor_Distill));
			else
				uiSettingTitle(STR(AuxSensor_Distill));
		}
#endif
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
	for(byte i=0;i<MaximumNumberOfSensors;i++) resetSensorCorrection(i);

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
//	char pbuf[20];
//	printSensorAddress(pbuf,address);
//	Serial.printf("Save sensor %d 0x%s to %d\n",idx,pbuf,addr);
}


bool sensorMenuEventHandler(byte)
{
	if(_sensorSettingIndex==0  && gSensorNumber==0)
	{
		if(btnIsEnterPressed){
			//uiClearSettingRow();
			switchApplication(SETUP_SCREEN);
			return true;
		}
		return false;
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
			resetSensorCorrection(_sensorSettingAux);
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
				return true;
			}
			sensorMenuItem();
		}
	}
	return true;
}
#endif //MaximumNumberOfSensors

// *************************
//*  Level 1 Menu (settings)
// *************************

str_t const level1Menu[]={STR(PID_PWM),STR(Unit_Parameters),STR(Set_Automation),
STR(Misc_Setting)
#if MaximumNumberOfSensors > 1
,STR(Sensor_Setting)
#endif
#if SupportDistilling
,STR(Distill_Recipe)
#endif
};
const ScreenIdType level1Screens[]={
	PID_SETTING_SCREEN,
	UNIT_SETTING_SCREEN,
	AUTO_SETTING_SCREEN,
    MISC_SETTING_SCREEN
#if MaximumNumberOfSensors > 1
	,SENSOR_SCREEN
#endif
#if SupportDistilling
	,DISTILL_SETTING_SCREEN
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

	wiReportCurrentStage(StageSetting);
}

bool menuEventHandler(byte event)
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
	} else return false;
	return true;
}

// ***************************************************************************
//*  Common function to handle Setting temperature
//*
// ***************************************************************************

float _maxAdjustTemp;
float _minAdjustTemp;

bool togglePwmInput(void)
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

			wiReportPwm();
			wiTogglePwm();
			return true;
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

			wiTogglePwm();
			return true;
		}
	}
	return false;
}

void setAdjustTemperature(float max, float min)
{
	_maxAdjustTemp=max;
	_minAdjustTemp=min;
}

void adjustSp(float adjust)
{
	float temp = gSettingTemperature + adjust;

	if(temp > _maxAdjustTemp) temp=_maxAdjustTemp;
	if(temp < _minAdjustTemp) temp=_minAdjustTemp;

	setSettingTemperature(temp);
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
	wiReportPwm();
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
void heatLoadParameters(void);

void loadBrewParameters(void)
{
	heatLoadParameters();
	pump.loadParameters();
	#if EnableLevelSensor
	lvMonitor.loadParameters();
	#endif

}


void togglePumpRest(void){
				//
			if(pump.isRest())
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
				heatOn();
				buzzPlaySound(PumpRestEndSoundId);
				#endif

				wiReportEvent(RemoteEventPumpRestEnd);
			}
}
// ***************************************************************************
//*  Manual Mode Screen
//*
// ***************************************************************************

#if SecondaryHeaterSupport == true
void toggleHeater(byte heater)
{
    byte result= _gElementInUseMask ^ heater;
    // off all, and turn on again.
    //
    heatOff();
    setHeatingElementsInUse(result);
    if(result) heatOn();
}
#endif

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
bool isCountDownTimeBlinking=false;
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
	wiReportSettingTemperature();

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

	// displace current temperature
	uiTempDisplaySetPosition(TemperatureManualModePosition);
	// Setpoint temperature
	uiSetSettingTemperature(gSettingTemperature);

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

	wiReportCurrentStage(StageManualMode);
	wiReportSettingTemperature();

}

void finishAutoTuneBackToManual(void)
{
	_state = MSManualMode;

	float temp =gIsUseFahrenheit? 95:35;
	setSettingTemperature(temp);

	uiRunningTimeShowInitial(0);

	uiTitle(STR(Manual_Mode));
	uiButtonLabel(ButtonLabel(Up_Down_Heat_Pmp));

	if(gIsHeatOn) heatOff();
	pump.off();

	brewLogger.stage(StageManualMode);

	wiReportCurrentStage(StageManualMode);
}

bool manualModeEventHandler(byte event)
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
			return true;
		}
		else if(btnIsEnterPressed)
		{
			// NO. back to main
			switchApplication(MAIN_SCREEN);
			return true;
		}
	}
#if	EnablePidAutoTune == true
	else if(_state == MSAskAutoTune)
	{
		if(event != ButtonPressedEventMask) return false;

		if(btnIsStartPressed)
		{
			// NO, back to manual mode
			_state = MSManualMode;
			uiButtonLabel(ButtonLabel(Up_Down_Heat_Pmp));
			return true;
		}
		else if(btnIsEnterPressed)
		{
			// YES, enter Auto tune mode
			_state = MSRunningAutoTune;

			uiTitle(STR(PID_AUTOTUNE));
			uiButtonLabel(ButtonLabel(x_x_Exit_Pmp));
			setSettingTemperature(gCurrentTemperature);

			uiRunningTimeShowInitial(0);
			uiRunningTimeStart();

			startAutoTune();
			heatOn();

			brewLogger.stage(StagePIDAutoTune);

			wiReportCurrentStage(StagePIDAutoTune);
			return true;
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
				return true;
			}
			else if(btnIsEnterPressed)
			{
				// Pump
				pump.toggle();
				return true;
			}
		}
		if(!_isRunningAutoTune)
		{
			// auto tuning finished
			buzzPlaySound(SoundIdAutoTuneFinished);
			finishAutoTuneBackToManual();
			return true;
		}
	}
	else if(_state == MSAskExitingAutoTune)
	{
		if(event != ButtonPressedEventMask) return false;

		if(btnIsStartPressed)
		{
			//No
			_state = MSRunningAutoTune;
			uiButtonLabel(ButtonLabel(x_x_Exit_Pmp));
			return true;
		}
		else if(btnIsEnterPressed)
		{
			// YES
			// back to Manual Mode.
			cancelAutoTune();
			finishAutoTuneBackToManual();
			return true;
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
					pump.off();
				}
				else
#endif
				{
                    #if SecondaryHeaterSupport == true
                    toggleHeater(PrimaryHeaterMask);
                    #else
					//turn heating on/off
					if(gIsHeatOn) heatOff();
					else heatOn();
				    #endif
				}
			}
			else if(btnIsEnterPressed)
			{
				if(btnIsEnterLongPressed)
				{
					pump.setRestEnabled(!pump.isRestEnabled());
				}
				else
				{
					// turn pump on/off
					pump.toggle();
				}
			}
            #if SecondaryHeaterSupport == true
			else if(isExactButtonsPressed(ButtonUpMask | ButtonStartMask))
			{
			    toggleHeater(SecondaryHeaterMask);
            }
            #endif
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
				bool update=false;
				long value=(long)manualModeChangeCountDownTime;
				if(btnIsUpPressed)
				{
					value +=1; update=true;
				}
				else if(btnIsDownPressed)
				{
					value -= 1; update=true;
				}			
				else if(btnIsUpContinuousPressed)
				{
					value += 5; update=true;
				}
				else if(btnIsDownContinuousPressed)
				{
					value -= 5; update=true;
				}
				if(update)
				{
					if(value < 0) value=0;
					else if(value > 600) value=600;
					manualModeChangeCountDownTime = value;
					uiRunningTimeShowInitial(manualModeChangeCountDownTime * 60);
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
			return true;
		} // end of button presss
		else if(event == TemperatureEventMask)
		{
			bool ret=false;
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
					wiReportEvent(RemoteEventTemperatureReached);
					ret =true;
				}
			}
			// Temperate Reached state
			bool toggled = togglePwmInput();
			return ret || toggled;
		} // end of temperature handling
		else if(event == PumpRestEventMask)
		{
			togglePumpRest();
			return true;
		}
		#if SupportManualModeCountDown == true
		else if(event == TimeoutEventMask)
		{
			buzzPlaySound(SoundIdCountDown);
			isManualModeCountDownMode=false;
			uiRunningTimeStart();
			return true;
		}
		#endif
	} // else of if(_state == MSAskWater)
	return false;
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
#define AS_HopStandChilling       19
#define AS_HopStand            20

#define AS_AutoResumeWaiting 21

#define HOP_ALTERTING_TIME 10
#define ADVANCE_BEEP_TIME 5
#define AutoStateIs(s) (_state==(s))

byte _primePumpCount;


//************************************
// for recovery
// ony necessary for StageDelayStart & DoughIn
// after that, the status can be retrieved from log
#define InvalidStage 0xFF
void autoModeEnterAutoResumeWaiting();
void saveStatus(uint8_t stage,uint16_t time){
	if(! readSetting(PS_AutoResume_Enabled)) return;

	updateSetting(PS_Saved_Stage,stage);
	updateSettingWord(PS_Time2Resume,time);
	commitSetting();
}

void clearStatus(){
	// clear only if not
	if(! readSetting(PS_AutoResume_Enabled)) return;

	if(readSetting(PS_Saved_Stage) != InvalidStage){
		updateSetting(PS_Saved_Stage,InvalidStage);
		commitSetting();
	}
}
bool checkResume(){
	if(! readSetting(PS_AutoResume_Enabled)) return false;

	uint8_t savedStage=readSetting(PS_Saved_Stage);
	DBG_PRINTF("checkResume %d\n",savedStage);

	if(savedStage == InvalidStage || (savedStage == AS_Mashing && !brewLogger.checkRecovery())) return false;
	
	return true;
}
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
	saveStatus(AS_DoughIn,0);
	// setup temperature event mask request after this.
	#if EnableLevelSensor
	setEventMask(TemperatureEventMask | ButtonPressedEventMask | PumpRestEventMask);
	#else
	setEventMask(TemperatureEventMask /*| ButtonPressedEventMask */);
	#endif

	//load temperature value
	float doughinTemp = automation.stageTemperature(0);

	// setup screen
	uiClearTitle();
	uiAutoModeTitle();
	uiClearSubTitleRow();

	uiAutoModeStage(0); // 0 is Mash-In

	// displace temperature
	uiTempDisplaySetPosition(TemperatureAutoModePosition);

	setSettingTemperature(doughinTemp);


	#if MANUAL_PUMP_MASH == true
	uiButtonLabel(ButtonLabel(Up_Down_Pause_Pmp));
	#else
	uiButtonLabel(ButtonLabel(Up_Down_Pause_x));
	#endif
	// start pump, if request,

	if(readSetting(PS_PumpPreMash)) pump.on();
	else pump.off();

	#if EnableLevelSensor
	if(readSetting(PS_EnableLevelSensor))
		lvMonitor.startMonitor();
	#endif

#if SpargeHeaterSupport == true
	if(gEnableSpargeWaterHeatingControl)
	{
		startHeatingSpargeWater();
	}
#endif


#if MaximumNumberOfSensors > 1
	setSensorForStage(SensorForPreMash);
#endif

	// start heat
#if SecondaryHeaterSupport
	setHeatingElementForStage(HeatingStagePreMash);
#endif
	// PID_STRIKE
	#if PwmHeatingSupport	
	heatOn(readSetting(PS_PID_STRIKE)? HeatingModeBoil:HeatingModePID);
	#else
	heatOn(readSetting(PS_PID_STRIKE) !=0);
	#endif


	if(gIsUseFahrenheit)
		setAdjustTemperature(167,77);
	else
		setAdjustTemperature(75.0,25.0);
	gIsEnterPwm=false;

	//

	#if MaximumNumberOfSensors > 1
		brewLogger.startSession(gSensorNumber,TemperatureChartPeriod,gIsUseFahrenheit);
	#else
		brewLogger.startSession(1,TemperatureChartPeriod,gIsUseFahrenheit);
	#endif

	#if SpargeHeaterSupport
		if(gEnableSpargeWaterHeatingControl){
			brewLogger.event(RemoteEventSpargeWaterAdded); // Log sparge enabled as event for recovery purposes
		}
	#endif
	brewLogger.stage(StageDoughIn);

	#if MaximumNumberOfSensors > 1
		brewLogger.temperatures(gTemperatureReading);
	#else
		brewLogger.temperature(gCurrentTemperature);
	#endif

	wiReportCurrentStage(StageDoughIn);
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

void autoModeEnterMashingExtension(unsigned long seconds)
{
	uiSetMashExtensionStatus(MashExtensionRunning);
	_mashingStageExtending = true;
	uiRunningTimeStartFrom(seconds);
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
		_mashingStep = MashStepMashOut; //7;
	}
	time = automation.stageTime(_mashingStep);
	if(time==0) time=1;

	// 	if(_mashingStep > 7), mashout time will always more than 1

	if(_mashingStep > 0 && _mashingStep < MashStepMashOut /*7*/)
		uiAutoModeMashTitle(_mashingStep,_numberMashingStep);
	else
		uiAutoModeStage(_mashingStep);

	uiRunningTimeSetPosition(RunningTimeNormalPosition);
	uiRunningTimeShowInitial(time * 60);

	if(gIsUseFahrenheit)
		setAdjustTemperature(176.0,68.0);
	else
		setAdjustTemperature(80.0,20.0);

	setSettingTemperature( automation.stageTemperature(_mashingStep));

	#if	MANUAL_PUMP_MASH == true
	uiButtonLabel(ButtonLabel(Up_Down_PmPus_STP));
	#else
	uiButtonLabel(ButtonLabel(Up_Down_Pause_STP));
	#endif
	_mashingTemperatureReached=false;

	pump.setRestEnabled(false);
	heatOn();

#if	MANUAL_PUMP_MASH == true
	if(!gManualPump)
	{
#endif
	if(_mashingStep <=6)
	{
		// pump is off at the time AddMalt
		if(readSetting(PS_PumpOnMash)) pump.on();
		else pump.off();
#if MaximumNumberOfSensors > 1
	setSensorForStage(SensorForMash);
#endif

	}
	else if(_mashingStep == MashStepMashOut /*7*/)
	{
		if(readSetting(PS_PumpOnMashOut)) pump.on();
		else pump.off();
#if MaximumNumberOfSensors > 1
	setSensorForStage(SensorForMash);
#endif
	}

#if	MANUAL_PUMP_MASH == true
	}
#endif

	if(!resume) {
		brewLogger.stage(_mashingStep);
	}

#if EnableExtendedMashStep
	autoModeResetMashExtension();
#endif
	wiReportCurrentStage(_mashingStep);

}

void autoModeGetMashStepNumber(void)
{
	byte idx=1;
	byte time;
	while(idx < MashStepMashOut /*7*/ && (time = automation.stageTime(idx)) != 0)
	{
		idx++;
	}
	_numberMashingStep = idx -1; // total mash steps.
}

void autoModeEnterMashing(void)
{
	_state = AS_Mashing;
	saveStatus(AS_Mashing,0);
	setEventMask(TemperatureEventMask | ButtonPressedEventMask | TimeoutEventMask | PumpRestEventMask);

	_askingSkipMashingStage = false;
	_mashingStep = 0; // 0 is mash in , real mashing starts from 1, this number will be increased in
					  // autoModeNextMashingStep() later.

#if	MANUAL_PUMP_MASH == true
	gManualPump=false;
#endif
#if SecondaryHeaterSupport
    setHeatingElementForStage(HeatingStageMashing);
#endif
	heatOn();

	autoModeGetMashStepNumber();
	autoModeNextMashingStep(false);
}

void autoModeEnterIodineTest(void)
{
	_state = AS_IodineTest;

	uiPreparePasueScreen(STR(IODINE_TEST));
	uiButtonLabel(ButtonLabel(x_x_Mashout_Extend));

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

	wiReportEvent(RemoteEventIodineTest);

}

void autoModeIodineTestToMashExtension(unsigned long time)
{
    // back to last mashing step.
    _state = AS_Mashing;
	buzzMute();
    tmPauseTimer();
    uiClearScreen();

    uiAutoModeTitle();
	pump.updateUI();
	
	uiAutoModeMashTitle(_mashingStep,_numberMashingStep);
    uiTempDisplaySetPosition(TemperatureAutoModePosition);
	uiRunningTimeSetPosition(RunningTimeNormalPosition);
	uiRunningTimeShowInitial(time);
	if(gIsUseFahrenheit)
		setAdjustTemperature(176.0,68.0);
	else
		setAdjustTemperature(80.0,20.0);

	setSettingTemperature( automation.stageTemperature(_mashingStep));

	#if	MANUAL_PUMP_MASH == true
	uiButtonLabel(ButtonLabel(Up_Down_PmPus_STP));
	#else
	uiButtonLabel(ButtonLabel(Up_Down_Pause_STP));
	#endif
    autoModeEnterMashingExtension(time);
    _mashingStageExtendEnable = false;
}

void autoModeIodineTestToMashout(void)
{
    tmPauseTimer();
	uiRunningTimeStop();
	buzzMute();
	// restore Screen
	uiClearScreen();

	uiAutoModeTitle();
	pump.updateUI();
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
	pump.off();

	uiRunningTimeStop();
	//uiClearPrompt();
	uiPrompt(STR(Remove_Malt));
	uiButtonLabel(ButtonLabel(Continue_Yes_No));
	// skip event mask, just filter it out in handling code

	buzzPlaySoundRepeat(SoundIdWaitUserInteraction);

//	if(!(readSetting(PS_PidPipe) == 1 && readSetting(PS_SensorType) == SensorInside))

	if(! readSetting(PS_PidPipe))
		heatProgramOff(); // heat off, programming

	wiReportEvent(RemoteEventRemoveMalt);
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
	_savedPump = pump.isOn();
	heatOff();
	pump.off();
	// just wait for user button

	uiPreparePasueScreen(STR(In_Pause));
	uiButtonLabel(ButtonLabel(x_x_Exit_x));

	_savedTime=time;

	uiRunningTimeShowInitial(_savedTime/1000);
	brewLogger.event(RemoteEventPause);
	wiReportEvent(RemoteEventPause);
}

void autoModeExitPause(void)
{
	// restore state
	_state = _stateBeforePause;
	gIsPaused=false;
	// restore timer, if any

	#if EnableExtendedMashStep
	if(!_mashingStageExtending && _savedTime > 0)
	#else
	if(_savedTime > 0)
	#endif
	{
		tmSetTimeoutAfter(_savedTime);
		if(_savedTime > ADVANCE_BEEP_TIME*1000)
			tmSetAuxTimeoutAfter(_savedTime -ADVANCE_BEEP_TIME*1000);
	}
	// restore screen
	uiClearScreen();

	uiAutoModeTitle();

	if(_mashingStep > 0 && _mashingStep < MashStepMashOut /*7*/)
		uiAutoModeMashTitle(_mashingStep,_numberMashingStep);
	else
		uiAutoModeStage(_mashingStep);


	// temperateure position
	uiTempDisplaySetPosition(TemperatureAutoModePosition);
	// set temperature point
	uiSetSettingTemperature(gSettingTemperature);

	// counting time
	uiRunningTimeSetPosition(RunningTimeNormalPosition);

	if(_savedTime==0)
	{
		if(_state != AS_DoughIn)
		{
			byte time = automation.stageTime(_mashingStep);
			uiRunningTimeShowInitial(time * 60);
			#if EnableExtendedMashStep
			uiSetMashExtensionStatus(_mashingStageExtendEnable? MashExtensionEnabled:MashExtensionNone);
			#endif
		}
	}
	else //if(_savedTime==0)
	{
		// temperature reached, timer already started
		// this should always NOT be MashIn
		if(_state == AS_Mashing)
		{
			uiRunningTimeShowInitial(_savedTime/1000);

			#if EnableExtendedMashStep
			if(_mashingStageExtending){
				uiSetMashExtensionStatus(MashExtensionRunning);
				uiRunningTimeStartFrom(_savedTime/1000);
			}else{
				uiSetMashExtensionStatus(_mashingStageExtendEnable? MashExtensionEnabled:MashExtensionNone);

				uiRunningTimeStartCountDown(_savedTime/1000);
			}
			#else
			uiRunningTimeStartCountDown(_savedTime/1000);
			#endif
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
	if(_savedHeating) heatOn();
	if(_savedPump) pump.on();

	brewLogger.event(RemoteEventResume);
	wiReportEvent(RemoteEventResume);
}
void autoModeEnterBoiling(void);
void autoModeMashingStageFinished(void)
{
	//[TODO:] make sure step 6 , beta 2 is non-skippable.
	if(_mashingStep < MashStepMashOut /*7*/) // step 7 = mashout
	{
    	DBG_PRINTF("autoModeMashingStageFinished:%d @%d iodine skip%d, extended:%d\n",_mashingStep,_numberMashingStep,readSetting(PS_SkipIodineTest),_mashingStageExtending);
		if((_mashingStep == _numberMashingStep)
			&& readSetting(PS_SkipIodineTest) ==0
			&& ! _mashingStageExtending)
		{
			// before MashOut(7) and not Skip Iodine Test
			DBG_PRINTF("autoModeMashingStageFinished autoModeEnterIodineTest\n" );
			autoModeEnterIodineTest();
		}
		else
		{
			autoModeNextMashingStep(false);
		}
	}
	else
	{
		#if EnableLevelSensor
		lvMonitor.stopMonitor();
		#endif

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
	setSettingTemperature ( gBoilStageTemperature);

	// display time
	isCountDownTimeBlinking=false;
	byte boilTime=automation.boilTime();

	brewLogger.stage(StageBoil);

	uiRunningTimeShowInitial(boilTime * 60);

	uiAutoModeStage(BoilingStage);
	uiButtonLabel(ButtonLabel(Up_Down_x_Pmp));

	if(readSetting(PS_PumpOnBoil)) pump.on();
	else pump.off();

#if MaximumNumberOfSensors > 1
	setSensorForStage(SensorForBoil);
#endif

	if(gIsUseFahrenheit)
		setAdjustTemperature(230.0,176.0);
	else
		setAdjustTemperature(110.0,80.0);

	gIsEnterPwm =false;
#if SecondaryHeaterSupport
	setHeatingElementForStage(HeatingStageBoiling);
#endif
#if PwmHeatingSupport
	heatOn(HeatingModeBoil);
#else
	heatOn(false); // NO need of PID, just full power until boiling
#endif
	wiReportCurrentStage(StageBoil);
}



// reuse the variable
//#define _restoreBoilingTimer _finishedTimer
#define _numHopToBeAdded _primePumpCount

void autoModeShowHopAdding(void)
{
	uiAutoModeShowHopNumber(automation.numberOfHops() - _numHopToBeAdded +1);
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

			wiReportEvent(RemoteEventAddHop);
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
		byte idx=automation.numberOfHops() - _numHopToBeAdded;

		unsigned long nextHopTime=(unsigned long)automation.timeOfHop(idx) * 60 * 1000;
		
		if(_remainingBoilTime <= nextHopTime)
		{
			// alert directly, start timer to restore
			autoModeAddHopNotice();
		}
		else
		{
			recoveryTimer = false;
			tmSetAuxTimeoutAfter(_remainingBoilTime - nextHopTime);
		}
	}
}

void autoModeStartBoilingTimer(void)
{
	// [IMPORTANT!] cast to (unsigned long) is needed
	// NO hop adding. just start last before

	byte boilTime=automation.boilTime();

	_remainingBoilTime= (unsigned long)boilTime * 60 *1000;

	_numHopToBeAdded =  automation.numberOfHops();

	autoModeReStartBoilingTimer();
}


void autoModeStartNextHopTimer(void)
{
	// it is done at hop timer expires :_numHopToBeAdded--;
	// this function is called after Screen is restored.(restore timer expires)

	byte lastHopIdx=automation.numberOfHops() - _numHopToBeAdded -1;


	if(_numHopToBeAdded > 0) // there are next timer
	{
		byte lastHopTime=automation.timeOfHop(lastHopIdx);

		byte nextHopTime= automation.timeOfHop(lastHopIdx+1);

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

void autoModeCoolingAsk(const char* msg,byte stage)
{
	_stageConfirm=false;

	//dismiss Temperature & running time
	uiTempDisplayHide();
	uiRunningTimeStop();

	uiClearScreen();

	uiSubTitle(msg);

	uiButtonLabel(ButtonLabel(Continue_Yes_No));

	wiReportCurrentStage(stage);
}

void autoModeEnterCooling(unsigned long elapsed)
{
	// state setting and brewlogger is handled outside.
	// this function is used after "confirmation"
	uiClearPrompt();
	uiClearSubTitleRow();

	uiAutoModeTitle();
	uiAutoModeStage(CoolingStage);
	float temp =gIsUseFahrenheit? 68.0:20.0;
	_coolingTempReached=false;
	// temperature at automode
	uiTempDisplaySetPosition(TemperatureAutoModePosition);

	setSettingTemperature(temp);

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
	pump.off();

	uiTempDisplayHide();
	uiRunningTimeStop();

	_state = AS_Finished;

	uiClearScreen();

	uiAutoModeFinishScreen();

	buzzPlaySoundRepeat(SoundIdBrewEnd);
	tmSetTimeoutAfter(BREW_END_STAY_DURATION * 1000);

	brewLogger.endSession();

	wiReportEvent(RemoteEventBrewFinished);
}


//#if NoWhirlpool != true
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

void autoModeWhirlpool(unsigned long elapsed)
{
	uiClearSubTitleRow();
	uiClearPrompt();

	uiAutoModeTitle();
	uiAutoModeStage(WhirlpoolStage);
	if(readSetting(PS_Whirlpool)== WhirlpoolCold) gSettingTemperature =(gIsUseFahrenheit)? 86:30;
	else gSettingTemperature = gIsUseFahrenheit? 185:85;

	// temperature at automode
	uiTempDisplaySetPosition(TemperatureAutoModePosition);
	uiSetSettingTemperature(gSettingTemperature);
	unsigned long time= (unsigned long)(_whirlpoolTime - elapsed) * 60;
	uiRunningTimeShowInitial( time);
	uiButtonLabel(ButtonLabel(x_x_x_Pmp));

	_pumpRunning = true;
	pump.on();
	tmSetTimeoutAfter((unsigned long)time *1000);
	uiRunningTimeStartCountDown(time);

	//wiReportCurrentStage(StageWhirlpool);

}

void autoModeWhirlpoolFinish(void)
{
	pump.off();

	// if cool whirlpool, got to whirlpool, or go to end
	if(readSetting(PS_Whirlpool) == WhirlpoolHot)
	{
		_state = AS_Cooling;
		autoModeCoolingAsk(STR(START_COOLING),StageCooling);
	}
	else
	{
		autoModeBrewEnd();
	}

}
//#endif

void autoModeCoolingFinish(void)
{
//#if NoWhirlpool != true
	if(readSetting(PS_Whirlpool) == WhirlpoolCold)
	{
		_state = AS_Whirlpool;
		autoModeCoolingAsk(STR(WHIRLPOOL),StageWhirlpool);
	}
	else
//#endif
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
	pump.on();
	_primePumpCount=0;
	tmSetTimeoutAfter((uint32_t)readSetting(PS_PumpPrimeOnTime) * 250); // 1sec
}
#endif


void autoModeCoolingOrWhirlpool(void)
{
    //#if NoWhirlpool != true
	if(readSetting(PS_Whirlpool) == WhirlpoolHot)
	{
	    _state = AS_Whirlpool;
		autoModeCoolingAsk(STR(WHIRLPOOL),StageWhirlpool);
	}
	else
	{
	    _state = AS_Cooling;
		autoModeCoolingAsk(STR(START_COOLING),StageCooling);
	}
	//#else
	//_state = AS_Cooling;
	//autoModeCoolingAsk(STR(START_COOLING),StageCooling);
	//#endif
}

//************************************
// HopStand
//
#define  _hopStandSession _mashingStep
uint8_t  _hopStandSessionHopIndex;

void autoModeEnterHopStandChilling(void)
{
    heatOff();
    //set temperature as "start" temperature.
    float start=automation.sessionStartTemperature(_hopStandSession);
    setSettingTemperature(start);
    float max=(_hopStandSession)? (automation.sessionKeepTemperature(_hopStandSession-1)-1):gBoilStageTemperature;
    setAdjustTemperature(max,automation.sessionKeepTemperature(_hopStandSession));
    // display chilling
	uiAutoModeTitle();
	uiAutoModeStage(HopStandChillingStage);

	brewLogger.stage(StageHopStandChill);
	// notify for interaction.
	uiButtonLabel(ButtonLabel(x_x_Ok_x));
	buzzPlaySoundRepeat(SoundIdWaitUserInteraction);
	_stageConfirm=false;
	uiRunningTimeShowInitial(0);
	uiRunningTimeStart();

	wiReportCurrentStage(StageHopStandChill);
}


void autoModeShowPostBoilHop(void)
{
    uint8_t hopIdx=automation.postBoilHopIndex(_hopStandSession,_hopStandSessionHopIndex);
	uiAutoModeShowPostBoilHopNumber(hopIdx);
    buzzPlaySoundRepeat(SoundIdAddHop);
	wiReportEvent(RemoteEventAddHop);
    // NOTE: the hop time should not be ZERO.
    recoveryTimer=true;
    tmSetAuxTimeoutAfter(HOP_ALTERTING_TIME * 1000);
}

void autoModeStartHopStandHopTimer(void)
{
    //
    uint32_t remainingTime=tmGetRemainingTime();
    // remainingTime should not be zero.
    if(!remainingTime) return;

    int idx;
    uint32_t hopTime=0;
    for(idx=0;idx< automation.numberOfHopInSession(_hopStandSession);idx++){
        hopTime=(uint32_t)automation.hopInSession(_hopStandSession,idx) * 60000;
        if( remainingTime > hopTime) break;
    }
    _hopStandSessionHopIndex = idx;
    if(idx >= automation.numberOfHopInSession(_hopStandSession)){
        DBG_PRINTF("autoModeStartHopStandHopTimer, no more hop\n");
    }else{
        uint32_t hstime=  remainingTime - hopTime;
        DBG_PRINTF("autoModeStartHopStandHopTimer, %d,%d, time:%d\n",_hopStandSession,_hopStandSessionHopIndex,hstime);
        recoveryTimer=false;
        tmSetAuxTimeoutAfter(hstime);
    }
}

void autoModeEnterHopStand(uint32_t elapsed=0)
{
    hsHeatOn();
    pump.off();
    //set temperature as "start" temperature.
    float temp=automation.sessionKeepTemperature(_hopStandSession);
    setSettingTemperature(temp);

    float min;
    if(_hopStandSession == (automation.numberOfHopStandSession() -1)){
        // last one
        min=(gIsUseFahrenheit)? 68:20;
    }else{
        min = automation.sessionStartTemperature(_hopStandSession +1)+1;
    }
    float max=(_hopStandSession)? (automation.sessionKeepTemperature(_hopStandSession-1)-1):gBoilStageTemperature;
    setAdjustTemperature(max,min);
    // display Hopstand
	uiButtonLabel(ButtonLabel(Up_Down_Skip_Pmp));

	uiAutoModeTitle();
    //
    if(elapsed > 0){
    	uiAutoModeStage(StageHopStand);
    	// derive the correct time elapse.
    	uint32_t hsTotal=(uint32_t)automation.hopInSession(_hopStandSession,0);
    	uint32_t hsLeft = hsTotal - elapsed; // time left

        uiRunningTimeShowInitial(hsLeft * 60);
        uiRunningTimeStartCountDown(hsLeft * 60);
        tmSetTimeoutAfter(hsLeft * 60 * 1000);
        // the main timer should start first.

        autoModeStartHopStandHopTimer();

    }else{
        // display adding hop first hop

         // display time
        uint32_t hstimeSecond=(uint32_t)automation.hopInSession(_hopStandSession,0) * 60;
        uiRunningTimeShowInitial(hstimeSecond);
        uiRunningTimeStartCountDown(hstimeSecond);
        // start timer
        tmSetTimeoutAfter(hstimeSecond * 1000);
        // the main timer should start first.

        // prompt for hop adding
        autoModeShowPostBoilHop();
    }

	wiReportCurrentStage(StageHopStand);
}

void autoModeEndHopStandSession(void)
{
    DBG_PRINTF("autoModeEndHopStandSession, %d,%d\n",_hopStandSession,_hopStandSessionHopIndex);
    tmPauseTimer();

    _hopStandSession ++;
    _hopStandSessionHopIndex=0;
    if(_hopStandSession >= automation.numberOfHopStandSession()){
        // end of hopstand
        DBG_PRINTF("HopStand Finished.\n");
        heatOff();
        pump.off();
        autoModeCoolingOrWhirlpool();
    }else{
        // next hopstand, enter chilling
        _state = AS_HopStandChilling;
        brewLogger.stage(StageHopStandChill);
        autoModeEnterHopStandChilling();
    }
}

void autoModeHopStandTimeout(void)
{
    // end of session
    autoModeEndHopStandSession();
}

void autoModeAuxTimeout(void)
{
    if(recoveryTimer){
        uiAutoModeStage(HopStandStage);
        buzzMute();
        autoModeStartHopStandHopTimer();
    }else{ // hop time
        // show hop if any
//        _hopStandSessionHopIndex ++;
//        if(_hopStandSessionHopIndex < automation.numberOfHopInSession(_hopStandSession)){
            autoModeShowPostBoilHop();
//        }
    }
}

void autoModeStartHopStand(void)
{
	uiClearSubTitleRow();
	uiClearPrompt();
	
    _hopStandSession=0;
    _hopStandSessionHopIndex=0;
    // in case of knock off, (stating temp == boil temp, or current temperature)
    // skip chilling stage, go direct to hopstand
    float start=automation.sessionStartTemperature(0);

    if(start >= gBoilStageTemperature || start >= gCurrentTemperature){
        _state = AS_HopStand;
        brewLogger.stage(StageHopStand);
        autoModeEnterHopStand();
    }else{
        _state = AS_HopStandChilling;
         brewLogger.stage(StageHopStandChill);
        autoModeEnterHopStandChilling();
    }
}


//************************************
// for recovery
//
byte hopStandSessionByTime(uint32_t elapsed)
{
    int i;
	uint32_t time=0;
	for(i=0;i<automation.numberOfHopStandSession();i++){
	    time += automation.hopInSession(i,0);
	    if(elapsed < time) break;
    }
    return i;
}

void autoModeResumeProcess(void)
{
	// get stage
	byte stage;
	uint32_t elapsed;

	#if SpargeHeaterSupport == true	
	bool resume_sparge = false;
	bool success=brewLogger.resumeSession(&stage,&elapsed,&resume_sparge);
	DBG_PRINTF("resume state:%d, elapsed:%d, sparge:%d\n",stage, elapsed,resume_sparge);

	#else
	bool success=brewLogger.resumeSession(&stage,&elapsed);
	DBG_PRINTF("resume state:%d, elapsed:%d\n",stage, elapsed);

	#endif


	if(!success){
		_state = AS_Finished;

		uiClearScreen();
		uiPrompt(STR(Resume_Failed));
		buzzPlaySound(SoundIdWarnning);
		tmSetTimeoutAfter(BREW_END_STAY_DURATION * 1000);
		return;
	}
	// after mashing sharts, the progress is recorded in log file. so just use AS_Mashing
	saveStatus(AS_Mashing,0);
	//
	setEventMask(TemperatureEventMask | ButtonPressedEventMask | TimeoutEventMask | PumpRestEventMask);

	uiClearScreen();

	uiAutoModeTitle();
	//uiSetSettingTemperature(gSettingTemperature); will be set later in each entering procedure
	uiTempDisplaySetPosition(TemperatureAutoModePosition);
	uiRunningTimeSetPosition(RunningTimeNormalPosition);
	// time may be 0xFF, invalid, => not just enter
	// less then stage time. temperature reached
	//
	loadBrewParameters();

	if (stage == StageBoil) // boiling
	{
		autoModeEnterBoiling();
		// if 0xfFF, assume not
		if(elapsed != INVALID_RECOVERY_TIME)
		{
			byte boilTime=automation.boilTime();
			byte time = boilTime - (byte)elapsed;

				// findout whihc hop is current

				_isBoilTempReached=true;
				unsigned long sec=(unsigned long)time *60;
				uiRunningTimeStartCountDown(sec);
				tmSetTimeoutAfter(sec *1000);

				//wiReportEvent(RemoteEventTemperatureReached);

				// start hop & boiling out timer
				byte hopnum =  automation.numberOfHops();
				_numHopToBeAdded=hopnum;
				if(hopnum > 0)
				{
					byte i;
					byte nextHopTime=0;
					for(i=0;i<hopnum;i++)
					{
						nextHopTime=automation.timeOfHop(i);
						if(nextHopTime > time) _numHopToBeAdded--;
						else break;
					}
					if(_numHopToBeAdded > 0)
					{
						unsigned long hopTimer =(uint32_t)(time - nextHopTime);
						recoveryTimer = false;
						tmSetAuxTimeoutAfter((unsigned long)hopTimer * 60 * 1000);
					}
				}
		}
	}
	else if (stage == StageCooling) // cooling
	{
	    heatOff();
	    if(elapsed != INVALID_RECOVERY_TIME)
        	autoModeEnterCooling((uint32_t)elapsed*60);
        else
            autoModeEnterCooling(0);

    	_state = AS_Cooling;
    	_stageConfirm= (elapsed !=0);
	}
	else if (stage == StageWhirlpool) // Whirlpool
	{
    	heatOff();
    	if(elapsed != INVALID_RECOVERY_TIME){
    	    autoModeWhirlpool(elapsed);
		}else{
    	    autoModeWhirlpool(0);
		}
	    _state = AS_Whirlpool;
	}
	else if (stage == StageHopStandChill) // HopStandChill
	{
	    // need to 'recover' which session it is
	    _hopStandSessionHopIndex=0;
	    if(elapsed == INVALID_RECOVERY_TIME){
	        _hopStandSession=0;
	    }else{
	        _hopStandSession=hopStandSessionByTime(elapsed);
        }
         _state = AS_HopStandChilling;
        autoModeEnterHopStandChilling();
	}
	else if (stage == StageHopStand) // StageHopStand
	{
        _hopStandSession=hopStandSessionByTime(elapsed);

        int elapsedPre=0;

    	for(int i=0;i<_hopStandSession;i++){
	        elapsedPre += automation.hopInSession(i,0);
        }
        DBG_PRINTF("Resume: elapsed:%d, elaspsedPre:%d\n",elapsed,elapsedPre);
        _state = AS_HopStand;
        autoModeEnterHopStand(elapsed - elapsedPre);
	}
	else if (stage == StageDoughIn) // Daugh-in
	{
		heatOn();
		autoModeEnterDoughIn();
	}
	else if (stage <  StageBoil)
	{
		// everything else is in MASHING state
		// just enter mashing step ...
#if SecondaryHeaterSupport
		setHeatingElementForStage(HeatingStageMashing);
#endif
#if SpargeHeaterSupport == true	 
	if((gEnableSpargeWaterHeatingControl) && (resume_sparge))
	{
		// Sparge heating was active, we should resume it
		startHeatingSpargeWater();	
	}
#endif
		heatOn();
		_state = AS_Mashing;
		_askingSkipMashingStage = false;

		autoModeGetMashStepNumber();

		_mashingStep = stage - 1; // next step will increase the step
		autoModeNextMashingStep(true);

		#if EnableLevelSensor
		if(readSetting(PS_EnableLevelSensor))
			lvMonitor.startMonitor();
		#endif

		// adjust timer if necessary
		if(elapsed != INVALID_RECOVERY_TIME)
		{
				byte stagetime = automation.stageTime(_mashingStep);
				_mashingTemperatureReached = true;

				if (elapsed > stagetime){
					// extended mode
					unsigned long seconds=(unsigned long)(elapsed - stagetime) * 60;

					autoModeIodineTestToMashExtension(seconds);
					pump.setRestEnabled(true);
				}else{
					byte time = stagetime - elapsed;
					unsigned long seconds=(unsigned long)time * 60;

					tmSetTimeoutAfter( seconds *1000);
					tmSetAuxTimeoutAfter((seconds-ADVANCE_BEEP_TIME) *1000);
					uiRunningTimeStartCountDown(seconds);
					pump.setRestEnabled(true);
				}
		}

	}
}
void autoModeDelayWaiting(uint32_t time);

void autoModeAutoResume(){
	// handles DelayStart & Dough-in, other resuming is performed in autoModeResumeProcess()
	uint8_t stage=readSetting(PS_Saved_Stage);
	DBG_PRINTF("\n\n***\nAuto Resume: %d\n",stage);
	if(stage == AS_DelayWaiting){
		
		loadBrewParameters();
		uint16_t remaining = readSettingWord(PS_Time2Resume);
		DBG_PRINTF("Resume Delay waiting:%d\n",remaining);
		if(remaining > 2) remaining--;
		autoModeDelayWaiting(remaining * 60);
	}else if(stage == AS_DoughIn){
		DBG_PRINTF("Resume dough-in\n");
		loadBrewParameters();
		autoModeEnterDoughIn();
	}else if(brewLogger.checkRecovery()){
		DBG_PRINTF("resume auto\n");
		autoModeResumeProcess();
	}else{
		// error case.		
		backToMain();
		return;
	}
}

//******************************
// Auto Mode Event Handling

bool autoModeAskResumeHandler(byte event)
{
	if(event != ButtonPressedEventMask) return false;
	if(btnIsStartPressed){
		// YES
		autoModeResumeProcess();
		return true;
	}else if(btnIsEnterPressed){
		// clear the flag
		brewLogger.clearRecovery();
		autoModeSetup();
		return true;
	}
	return false;
}

#if NoDelayStart == false
bool autoModeAskDelayStartHandler(byte event){
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
		return true;
	}
	return false;
}
#endif


#if SpargeHeaterSupport == true
bool autoModeAskSpargeWaterAddedHandler(byte event)
{
	if(btnIsStartPressed)
	{
		// No sparge
		gEnableSpargeWaterHeatingControl = false;
		#if UsePaddleInsteadOfPump
		autoModeStartWithoutPumpPrimming();
		#else
		autoModeEnterPumpPriming();
		#endif
		return true;
	}
	else if(btnIsEnterPressed)
	{
		// sparge
		gEnableSpargeWaterHeatingControl = true;
		// session not started yet. brewLogger.event(RemoteEventSpargeWaterAdded); // Log sparge enabled as event for recovery purposes
		#if UsePaddleInsteadOfPump
		autoModeStartWithoutPumpPrimming();
		#else
		autoModeEnterPumpPriming();
		#endif
		return true;
	} 
	return false;
}  //end of state AS_AskWaterAdded

#endif



bool autoModePumpPrimingHandler(byte event)
{
	if(event == TimeoutEventMask){
		if(pump.isOn()){
			pump.off();
			tmSetTimeoutAfter((uint32_t)readSetting(PS_PumpPrimeOffTime) * 250);
		}else{
			_primePumpCount++;

			if(_primePumpCount < readSetting(PS_PumpPrimeCount)){
				pump.on();
				tmSetTimeoutAfter((uint32_t)readSetting(PS_PumpPrimeOnTime) * 250);

			}else{
					// next stage is setting delay or mash start
				#if NoDelayStart == false
				if(_delayRequested){
					autoModeEnterDelayTimeInput();
				}else{
					//_state = AS_DoughIn;
					autoModeEnterDoughIn();
				}
				#else
				autoModeEnterDoughIn();
				#endif
			} // else of prime pump < 5
		} // end of else if pump on
		return true;
	} // end of handling of TimeoutEventMask
	return false;
} // end of state AS_PumpPrime

bool autoModeAskWaterAddedHandler(byte event){
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
		return true;
	}
	else if(btnIsEnterPressed)
	{
		// NO; before heat & pump are used, it is safe to switch directly
		// instead of "backToMain"
		switchApplication(MAIN_SCREEN);
		return true;
	}
	return false;
}

#if NoDelayStart == false

bool autoModeDelayTimerInputHandler(byte event)
{
	// input delay timer
	if(event != ButtonPressedEventMask) return false;

	if(btnIsUpPressed){
		if( (_delayTime +1) < MAX_DELAY_TIME ){
			_delayTime ++;
			uiRunningTimeShowInitial(_delayTime * 15 * 60);
		}
	}else if(btnIsDownPressed){
		if(_delayTime > 1){
			_delayTime --;
			uiRunningTimeShowInitial(_delayTime * 15 * 60);
		}
	}else if(btnIsStartPressed){
		// quit
		backToMain();
	}else if(btnIsEnterPressed){
		_state = AS_DelayTimeConfirm;
		uiButtonLabel(ButtonLabel(Continue_Yes_No));
	}
	return true;
} 

void autoModeDelayWaiting(uint32_t time){
	_state = AS_DelayWaiting;
	uiClearSubTitleRow();
	uiSubTitle(STR(To_be_started_in));
	uiButtonLabel(ButtonLabel(x_x_Quit_Go));

	uiRunningTimeSetPosition(RunningTimeDelayInputPosition);
	tmSetTimeoutAfter(time * 1000);
	uiRunningTimeStartCountDown(time);
	setEventMask(TimeoutEventMask | ButtonPressedEventMask );

	wiReportCurrentStage(StageDelayStart);	
	tmSetAuxTimeoutAfter(60*1000); // one minute
}

bool autoModeDelayTimerConfirmHandler(byte event)
{
	if(event != ButtonPressedEventMask) return false;

	if(btnIsStartPressed){
		// YES
		autoModeDelayWaiting(_delayTime * 15 * 60);
		saveStatus(AS_DelayWaiting,_delayTime * 15 );
		return true;
	}else if(btnIsEnterPressed){
		//NO
		backToMain();
		return true;
	}
	return false;
} 

bool autoModeDelayWaitingHandler(byte event)
{
	if(event == ButtonPressedEventMask){
		if(btnIsStartPressed){
			// Quit
			uiRunningTimeStop();
			tmPauseTimer();
			backToMain();
			return true;
		}else if(btnIsEnterPressed){
			//GO
			// cancel timer
			uiRunningTimeStop();
			tmPauseTimer();
			uiClearSettingRow();
			//_state = AS_DoughIn;
			autoModeEnterDoughIn();
			return true;
		}
	}else if(event == TimeoutEventMask){
		if(IsAuxTimeout){
			uint32_t remaining=tmGetRemainingTime();
			saveStatus(AS_DelayWaiting,remaining/(60*1000));
			tmSetAuxTimeoutAfter(60*1000); // one minute
		}else{
			buzzPlaySound(SoundIdDelayTimeout);
			uiRunningTimeStop();
			uiClearSettingRow();
			autoModeEnterDoughIn();
			return true;
		}
	}
	return false;
}//AS_DelayWaiting

#endif

// 
void autoModeEnterAutoResumeWaiting(){
	_state = AS_AutoResumeWaiting;
	uiLcdClearAll();
	uiTitle(STR(AutomaticMode));
	uiSubTitle(STR(Resume_in));
	uiButtonLabel(ButtonLabel(x_x_Quit_Go));


	tmSetTimeoutAfter( AUTO_RESUME_TIMEOUT  * 1000);
	uiRunningTimeSetPosition(RunningTimeDelayInputPosition);
	uiRunningTimeShowInitial(AUTO_RESUME_TIMEOUT);
	uiRunningTimeStartCountDown(AUTO_RESUME_TIMEOUT);
	setEventMask(TimeoutEventMask | ButtonPressedEventMask );

	wiReportCurrentStage(StageAutoResume);
}

bool autoModeAutoResumeHandler(byte event){
	if(event == ButtonPressedEventMask){
		if(btnIsStartPressed){
			// Quit
			uiRunningTimeStop();
			tmPauseTimer();
			backToMain();
			return true;
		}else if(btnIsEnterPressed){
			//GO
			// cancel timer
			uiRunningTimeStop();
			tmPauseTimer();
			uiClearSettingRow();
			//_state = AS_DoughIn;
			autoModeAutoResume();
			return true;
		}
	}else if(event == TimeoutEventMask){
		uiRunningTimeStop();
		uiClearSettingRow();
		autoModeAutoResume();
		return true;
	}
	return false;
}

bool autoModeDoughInHandler(byte event)
{
	if(event == TemperatureEventMask){
		if(gCurrentTemperature >=gSettingTemperature){
			// PID_STRIKE
			heatOn(); // switch back to PID mode, whatever it was

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
			wiReportEvent(RemoteEventTemperatureReached);
			return true;
		}
	}//TemperatureEventMask
	else if(event == ButtonPressedEventMask){
		// up/down/pause
		if(btnIsStartPressed){
			autoModePause(0);
		}
#if MANUAL_PUMP_MASH == true
		else if(btnIsEnterPressed){
				pump.toggle();
		}
#endif
		else{
			//for up/down
			processAdjustButtons();
		}
		return true;
	}//ButtonPressedEventMask
	#if EnableLevelSensor
	else if(event == PumpRestEventMask){
		togglePumpRest();
		return true;
	}
	#endif
	return false;
} // endof state AS_DoughIn

bool autoModePauseHandler(byte event)
{
	if(event == ButtonPressedEventMask){
		if(btnIsStartPressed){
			autoModeExitPause();
			return true;
		}
	}
	return false;
}

bool autoModeMashInAskContinueHandler(byte event)
{
	if(btnIsStartPressed){

		buzzMute();
		if(readSetting(PS_PID_DoughIn)){
		}else{
			heatOff(); // turn off heat. during "dough-in"
		}
		// goto next stage, Mashing or ask MaltADD
		if(readSetting(PS_SkipAddMalt)){
			uiClearPrompt();
			// skip Add Malt , enter mashing state
			autoModeEnterMashing();
		}else{
			pump.off();
			// ask Add Malt
			//uiClearPrompt();
			uiPrompt(STR(Add_Malt));
			//{ADD_MALT_MOD
			//uiButtonLabel(ButtonLabel(Continue_Yes_No));
			uiButtonLabel(ButtonLabel(Continue_Yes_Pmp));
			//}ADD_MALT_MOD

			_state = AS_AskAddMalt;

			wiReportEvent(RemoteEventAddMalt);

		}
		return true;
	}else if(btnIsEnterPressed){
		// NO
		// heater & pump might started, so use back to main
		backToMain();
		return true;
	}
	return false;
} /// AS_MashInAskContinue

bool autoModeAskAddMaltHandler(byte event)
{
	if(btnIsStartPressed)
	{
		// YES
		uiClearPrompt();
		
		autoModeEnterMashing();
		return true;
	}
	else if(btnIsEnterPressed)
	{
		pump.toggle();
		return true;
	}
	return false;
}

bool autoModeMashingHandler(byte event)
{
	// handle key event together.
	// the same way reached or not.
	if(event == ButtonPressedEventMask){
		if(_askingSkipMashingStage){
			#if EnableExtendedMashStep
			if(btnIsUpPressed){
				autoModeToggleMashExtension();
			} else
			#endif //#if EnableExtendedMashStep
			if(btnIsStartPressed){
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
			}else if(btnIsEnterPressed){
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
			return true;
		}
		// else, not asking skipe mashing
		if(btnIsStartPressed){

			#if	MANUAL_PUMP_MASH == true
			if(btnIsStartLongPressed){
			#endif
				// if in
				if(_mashingTemperatureReached){
					buzzMute();
					if(_mashingStageExtending){
						autoModePause(uiGetDisplayTime() * 1000);
					}else{
						autoModePause(tmPauseTimer());
					}
				}else{
					autoModePause(0);
				}
			#if	MANUAL_PUMP_MASH == true
			}else{
				pump.toggle();
				gManualPump = true;
			}
			#endif
		}else if(btnIsEnterPressed){
			#if EnableExtendedMashStep
			if(_mashingStageExtending){
					// go to next step.
				autoModeMashingStageFinished();
			}else
			#endif // #if EnableExtendedMashStep

			// Skip, go to next stage
			// long pressed is "cover" in normal pressed
			if(btnIsEnterLongPressed)  {

				if(_mashingTemperatureReached) buzzMute();
				
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
		}else{
			//up, down etc.
			if(_askingSkipMashingStage) return false; // ignore

			processAdjustButtons();
		}
		return true;
	}else if(event == PumpRestEventMask){
		togglePumpRest();
		return true;
	}else {
		// else of PumpRestEvent & Button,
		//DBG_PRINTF("reach:%d, setting:%d\n",_mashingTemperatureReached,(int)gSettingTemperature);
		if(_mashingTemperatureReached){
			if(event == TimeoutEventMask){
				// counting time
				// except button, we care also two timer
				// one for 10  or 5 seconds before time out
				// the other for end of phase timeout
				if(IsAuxTimeout){
					buzzPlaySound(SoundIdCountDown);
				}else{
					// next stage
					if(_askingSkipMashingStage){
						uiClearPrompt();
						uiRunningTimeHide(false);
						_askingSkipMashingStage = false;
					}

					#if EnableExtendedMashStep
					if(_mashingStageExtendEnable){
						autoModeEnterMashingExtension(0);
					}else
					#endif //#if EnableExtendedMashStep
					{
						autoModeMashingStageFinished();
					}
				}
				return true;
			} // end of event == TimeoutEventMask
		}else {
			// else of if(_mashingTemperatureReached)
			if(event == TemperatureEventMask){
				// rising temperature
				if(gCurrentTemperature >= gSettingTemperature){
					brewLogger.event(RemoteEventTemperatureReached);

					_mashingTemperatureReached = true;
					unsigned long seconds=(unsigned long)automation.stageTime(_mashingStep) * 60;

					tmSetTimeoutAfter( seconds *1000);
					tmSetAuxTimeoutAfter((seconds-ADVANCE_BEEP_TIME) *1000);

					uiRunningTimeStartCountDown(seconds);

					buzzPlaySound(SoundIdTemperatureReached);

					pump.setRestEnabled(true);

					wiReportEvent(RemoteEventTemperatureReached);
					return true;
				}
			}
		} 	// end of else if(_mashingTemperatureReached)
	}		// end of temperature and timeout handling
	return false;
}//AS_Mashing

bool autoModeIodineTestHandler(byte event)
{
	// timeout or user press ok
	if(event ==ButtonPressedEventMask){
		if(btnIsStartPressed){
			uiClearPrompt();
			// back to next mashing step: Mashout
			autoModeIodineTestToMashout();
			return true;
		}else  if(btnIsEnterPressed){
			// extend mash
			autoModeIodineTestToMashExtension(0);
			return true;
		}
	}else if(event ==TimeoutEventMask){
		uiClearPrompt();
		//[TODO:] make sure not other timeout event
		autoModeIodineTestToMashout();
		return true;
	}
	return false;
}//AS_IodineTest

bool autoModeAskMaltRemoveHandler(byte event)
{
	if(event ==ButtonPressedEventMask){
		if(btnIsStartPressed){
			buzzMute();
			// yes
			uiClearPrompt();
			autoModeEnterBoiling();
			return true;
		}else if(btnIsEnterPressed){
			// back to main
			backToMain();
			return true;
		}
	}
	return false;
}

void autoModeBoilingTimeChange(int change){
	int bt =(int) automation.boilTime();
	bt +=  change;
	if(bt < 0) bt=0;
	if(bt > 240) bt=240;
	automation.setBoilTime(bt);
	uiRunningTimeShowInitial(bt * 60);
}

bool autoModeBoilingHandler(byte event)
{
	if(event ==ButtonPressedEventMask){
		if(isCountDownTimeBlinking){
			if(btnIsUpPressed){
				autoModeBoilingTimeChange(1);
			}else if(btnIsUpContinuousPressed){
				autoModeBoilingTimeChange(5);
			}else if(btnIsDownPressed){
				autoModeBoilingTimeChange(-1);
			}else if(btnIsDownContinuousPressed){
				autoModeBoilingTimeChange(-5);
			}else{
				// other button actions are considered as ending of time editing
				isCountDownTimeBlinking=false;
				uiRunningTimeBlink(false);
				wiRecipeChange();
			}
			return true;
		}
		//else
		if (btnIsEnterPressed){
			// pump control
			pump.toggle();
		}else if(btnIsStartPressed){
			if(_isBoilTempReached){
				autoModeBoilingPauseHandler();
			}
		}else if(isExactButtonsPressed(ButtonEnterMask | ButtonStartMask)){
			if(!_isBoilTempReached){
				// to change boiling time
				isCountDownTimeBlinking = true;
				uiRunningTimeBlink(isCountDownTimeBlinking);
			}
		}else{
			processAdjustButtons();
		}
		return true;
	}else if(event ==TimeoutEventMask){
		if(IsAuxTimeout){
			// start next timer to end notice of hop adding
			if(recoveryTimer){
				uiAutoModeStage(BoilingStage);
				autoModeStartNextHopTimer();
			}else{
				// start next timer
				autoModeAddHopNotice();
			}
		}else{
			//boiling timer timeout
			// next stage
			heatOff(); // heat OFF
			// switch to post boil for next starts.
			#if SecondaryHeaterSupport
			setHeatingElementForStage(HeatingStagePostBoil);
			#endif
			pump.off();

			brewLogger.event(RemoteEventBoilFinished);
			wiReportEvent(RemoteEventBoilFinished);

			buzzPlaySoundRepeat(SoundIdWaitUserInteraction);
			gIsEnterPwm = false;

			if(automation.numberOfHopStandSession() ==0) autoModeCoolingOrWhirlpool();
    		else autoModeStartHopStand();
		}
		return true;
	}else{ // if(event ==TemperatureMask)
		bool ret=false;
		if(gCurrentTemperature >= gBoilStageTemperature){
			if(_isBoilTempReached == false){
				brewLogger.event(RemoteEventTemperatureReached);
				_isBoilTempReached=true;

				// forced stop time editing
				if(isCountDownTimeBlinking){
					isCountDownTimeBlinking = false;
					uiRunningTimeBlink(isCountDownTimeBlinking);
					wiRecipeChange();
				}
				//buzz temperature reach first
				// because later "add hop" buzz may interrupt
				// it
				wiReportEvent(RemoteEventTemperatureReached);
				buzzPlaySound(SoundIdBoil);
				// start counting down
				byte boilTime=automation.boilTime();
				uiRunningTimeStartCountDown((unsigned long)boilTime *60);
				// start hop & boiling out timer
				autoModeStartBoilingTimer();

				uiButtonLabel(ButtonLabel(Up_Down_Pause_Pmp));
				ret=true;
			}
		}
		bool toggled=togglePwmInput();
		return ret || toggled;
	}
	return false;
} //AS_Boiling

bool autoModeHopStandChillingHandler(byte event)
{
	if(event == ButtonPressedEventMask){
	    if(!_stageConfirm){
	        if (btnIsStartPressed){
            	_stageConfirm=true;
            	buzzMute();
	            uiButtonLabel(ButtonLabel(Up_Down_Skip_Pmp));
				return true;
	        }
	        return false;
	    }
	    // else stageConfirm
		if (btnIsEnterPressed){
			// pump control
			pump.toggle();
		}else if (btnIsStartPressed){
			// skip
    		if(btnIsStartLongPressed) {
    		    _state = AS_HopStand;
    		    brewLogger.stage(StageHopStand);
		        autoModeEnterHopStand();
		    }
		}else{
			processAdjustButtons();
		}
		return true;
	}else if(event == TemperatureEventMask) {
		if(gCurrentTemperature <= gSettingTemperature){
    		_state = AS_HopStand;
    		brewLogger.stage(StageHopStand);
		    autoModeEnterHopStand();
			return true;
		}
	}
	return false;
} //AS_HopStandChilling

bool autoModeHopStandHandler(byte event)
{
	if(event == ButtonPressedEventMask){
		if (btnIsEnterPressed){
			// pump control
			pump.toggle();
		}else if (btnIsStartPressed){
			// Skip
    		if(btnIsStartLongPressed) {
    			 autoModeEndHopStandSession();
    		}
		}else{
			processAdjustButtons();
		}
		return true;
    }else if(event == TimeoutEventMask) {
    	if(IsAuxTimeout){
           	autoModeAuxTimeout();
		}else{
		    autoModeHopStandTimeout();
		}
		return true;
    }
	return false;
} //AS_HopStand

bool autoModeCoolingHandler(byte event)
{
	if(_stageConfirm){
		if(event == ButtonPressedEventMask){
			if (btnIsEnterPressed){
				// pump control
				pump.toggle();
			}else if (btnIsStartPressed){
				// next stage
				autoModeCoolingFinish();
			}else{
				processAdjustButtons();
			}
			return true;
		}else if(event == TemperatureEventMask){
			// if temperature drop to desire temp
			// end this phase
			if(gCurrentTemperature <= gSettingTemperature ){
				// next stage
				//Manual END. no Auto end
				// autoModeCoolingFinish();
				if(!_coolingTempReached){
					buzzPlaySound(SoundIdTemperatureReached);
					wiReportEvent(RemoteEventTemperatureReached);
					_coolingTempReached=true;
					return true;
				}
			}
		}
	}else{
		// of if(_stageConfirm), in state of asking Enter Cooling
		// wait confirm
		if(event != ButtonPressedEventMask) return false;
		
		buzzMute();
		if(btnIsStartPressed){
			brewLogger.stage(StageCooling);
			// yes
			_stageConfirm=true;
			autoModeEnterCooling(0);
			brewLogger.setPoint(gSettingTemperature);
			return true;
		}else if (btnIsEnterPressed){
			// no
			autoModeCoolingFinish();
			return true;
		}
	} // end of else of if(_stageConfirm)
	return false;
} //AS_Cooling


bool autoModeWhirlpoolHandler(byte event)
{
	if(_stageConfirm){
		if(_whirlpoolInput){
			// input screen of Whirlpool time
			if(event != ButtonPressedEventMask) return false;

			if(btnIsUpPressed){
				//up
				if((_whirlpoolTime + 1) <= MAX_WHIRLPOOL_TIME){
					_whirlpoolTime++;
					uiRunningTimeShowInitial(_whirlpoolTime * 60);
				}
			}else if (btnIsDownPressed){
				// down
				if((_whirlpoolTime - 1) >= MIN_WHIRLPOOL_TIME){
					_whirlpoolTime--;
					uiRunningTimeShowInitial(_whirlpoolTime * 60);
				}
			}else if (btnIsStartPressed){
				//Quit
				autoModeWhirlpoolFinish();
			}else if (btnIsEnterPressed){
				//OK
				_whirlpoolInput=false;
				autoModeWhirlpool(0);
			}
			return true;
		}else{
			 // of _whirlpoolInput
			// Whirlpool stage running
			// counting time & running pump
			if(event == ButtonPressedEventMask){

				if(btnIsStartPressed){
					if(!_pumpRunning){
						// time, back to time setting
						autoModeWhirlpoolInputTime();
					}
				}else if(btnIsEnterPressed){
					if(_pumpRunning){
							// stop pump ,and stop & reset time
						_pumpRunning = false;
						pump.off();
						uiRunningTimeShowInitial(_whirlpoolTime * 60);
						tmPauseTimer();

						uiButtonLabel(ButtonLabel(x_x_Time_Pmp));
					}else{
						_pumpRunning = true;
						pump.on();
						uiRunningTimeStartCountDown(_whirlpoolTime * 60);
						tmSetTimeoutAfter((unsigned long)_whirlpoolTime*60*1000);

						uiButtonLabel(ButtonLabel(x_x_x_Pmp));

					}
				}
				return true;
			}else if(event == TemperatureEventMask){
					//[TODO:] temperature control
			}
			else if(event == TimeoutEventMask){
					autoModeWhirlpoolFinish();
					return true;
			}
		}
	}else{ // if(_stageConfirm)
			// wait confirm
		if(event != ButtonPressedEventMask) return false;

		buzzMute();

		if(btnIsStartPressed){
			brewLogger.stage(StageWhirlpool);
			// yes
			_stageConfirm=true;
			autoModeWhirlpoolInputTime();
		}else if (btnIsEnterPressed){
			// no
			autoModeWhirlpoolFinish();
		}
		return true;
	} // of else // if(_stageConfirm)
	return false;
}//AS_Whirlpool

bool autoModeFinishedHandler(byte event)
{
	if(event == TimeoutEventMask)
	{
		backToMain();
		return true;
	}
	return false;
}

bool autoModeEventHandler(byte event)
{
	// switch-case uses more memory, though it looks better
	//
	if(AutoStateIs( AS_AskResume)){
		return autoModeAskResumeHandler(event);
	}else

#if NoDelayStart == false
	if(AutoStateIs( AS_AskDelayStart)){
		return autoModeAskDelayStartHandler(event);
	}else
#endif
	if(AutoStateIs(AS_AskWaterAdded)){
		return autoModeAskWaterAddedHandler(event);
	}  //end of state AS_AskWaterAdded
#if SpargeHeaterSupport == true
	else if(AutoStateIs(AS_AskSpargeWaterAdded)){
		return autoModeAskSpargeWaterAddedHandler(event);
	}  //end of state AS_AskSpargeWaterAdded
#endif
	else if(AutoStateIs(AS_PumpPrime)){
		return autoModePumpPrimingHandler(event);
	} // end of state AS_PumpPrime
#if NoDelayStart == false
	else if(AutoStateIs(AS_DelayTimeInput)){
		return autoModeDelayTimerInputHandler(event);
	}else if(AutoStateIs(AS_DelayTimeConfirm)){
		return autoModeDelayTimerConfirmHandler(event);
	}else if(AutoStateIs(AS_DelayWaiting)){
		return autoModeDelayWaitingHandler(event);
	}//AS_DelayWaiting
#endif
	else if(AutoStateIs(AS_DoughIn)){
		return autoModeDoughInHandler(event);
	}else if(AutoStateIs(AS_Pause)){
		return autoModePauseHandler(event);
	}else if(AutoStateIs(AS_MashInAskContinue)){
		return autoModeMashInAskContinueHandler(event);
	}else if(AutoStateIs(AS_AskAddMalt)){
		return autoModeAskAddMaltHandler(event);
	} else if(AutoStateIs(AS_Mashing)){
		return autoModeMashingHandler(event);
	}else if(AutoStateIs(AS_IodineTest)){
		return autoModeIodineTestHandler(event);
	}else if(AutoStateIs(AS_AskMaltRemove)){
		return autoModeAskMaltRemoveHandler(event);
	}else if(AutoStateIs(AS_Boiling)){
		return autoModeBoilingHandler(event);
	}else if(AutoStateIs(AS_HopStandChilling)){
		return autoModeHopStandChillingHandler(event);
	}else if(AutoStateIs(AS_HopStand)){
		return autoModeHopStandHandler(event);
	}else if(AutoStateIs(AS_Cooling)){
		return autoModeCoolingHandler(event);
	}else if(AutoStateIs(AS_Whirlpool)){
		return autoModeWhirlpoolHandler(event);
	}else if(AutoStateIs(AS_Finished)){
		return autoModeFinishedHandler(event);
	}else if(AutoStateIs(AS_AutoResumeWaiting)){
		return autoModeAutoResumeHandler(event);
	}
	return false;
} // end of autoModeEventHandler


// *************************
//* Distilling
// *************************
#if SupportDistilling
typedef enum _DistillingState{
	DistillingStateConfirmation,
	DistillingStateBeforeHead,
	DistillingStateHeadConfirmation,
	DistillingStateHead,
	DistillingStateHeart,
	DistillingStateTail,
	DistillingStateEnd,
	DistillingStateManual	
} DistillingState;

class DistillingController{
	DistillingState _state;
	bool _blinkingSettingTemperature;
public:
	DistillingController(){}

	void setup(void){
		_state = DistillingStateConfirmation;
		// display confirmation temperature
		// 
		uiTitle(STR(Distilling));
		uiPrompt(STR(StartDistilling));
		uiButtonLabel(ButtonLabel(DistillConfirm));
	}
	bool eventHanlder(byte event){
		if(_state == DistillingStateConfirmation)  return eventHanlderConfirmation(event);
		else if(_state == DistillingStateBeforeHead) return eventHanlderBeforeHead(event);
		else if(_state == DistillingStateHeadConfirmation) return eventHanlderHeadConfirmation(event);
		else if(_state == DistillingStateHead) return eventHanlderHead(event);
		else if(_state == DistillingStateHeart) return eventHanlderHeart(event);
		else if(_state == DistillingStateTail) return eventHanlderTail(event);
		else if(_state == DistillingStateEnd) return eventHanlderEnd(event);
		else if(_state == DistillingStateManual) return eventHanlderManualDistilling(event);
		return false;
	}

protected:
	#if SecondaryHeaterSupport == true
	void toggleHeaterPwm(byte heater)
	{
		byte result= _gElementInUseMask ^ heater;
		// off all, and turn on again.
		heatOff();
		setHeatingElementsInUse(result);
		if(result) heatOn(HeatingModePWM);
	}
	#endif

	void enterManualDistilling(void){
		_state = DistillingStateManual;
		#if MaximumNumberOfSensors > 1
		setSensorForStage(SensorForDistilling);
		#endif
		heatLoadParameters();
		uiClearScreen();
		uiTitle(STR(Manual_Distill));		
		uiButtonLabel(ButtonLabel(Up_Down_Heat_Pmp));
		uiTempDisplaySetPosition(TemperatureManualModePosition);
		uiShowPwmLabel();
		uiShowPwmValue(gBoilHeatOutput);
		gIsEnterPwm=true;
		gSettingTemperature = DEFAULT_MANUL_MODE_TEMPERATURE;
		wiReportSettingTemperature();
		uiSetSettingTemperature(gSettingTemperature);
		// display counting time
		uiRunningTimeSetPosition(RunningTimeNormalPosition);
		uiRunningTimeShowInitial(0);
		isCountDownTimeBlinking=false;
		_blinkingSettingTemperature = false;

		setAdjustTemperature(gIsUseFahrenheit? 220:110, gIsUseFahrenheit? 68:20);

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
		setEventMask(TemperatureEventMask | ButtonPressedEventMask | TimeoutEventMask);
		if(gIsUseFahrenheit)
			setAdjustTemperature(230.0,68.0);
		else
			setAdjustTemperature(110.0,20.0);
	
		wiReportCurrentStage(StageManualMode);
	}

	bool eventHanlderManualDistilling(byte event){
		if(event == ButtonPressedEventMask)
		{
			if(btnIsStartPressed)
			{
				if(btnIsStartLongPressed){
					_blinkingSettingTemperature = ! _blinkingSettingTemperature;
					uiSettingTemperatureBlinking(_blinkingSettingTemperature);
				}else{
					#if SecondaryHeaterSupport == true
					toggleHeaterPwm(PrimaryHeaterMask);
					#else
						//turn heating on/off
					if(gIsHeatOn) heatOff();
					else heatOn(HeatingModePWM);
					#endif
				}
			}
			else if(btnIsEnterPressed)
			{
				if(btnIsEnterLongPressed)
				{
					pump.setRestEnabled(!pump.isRestEnabled());
				}
				else
				{
						// turn pump on/off
					pump.toggle();
				}
			}
			#if SecondaryHeaterSupport == true
			else if(isExactButtonsPressed(ButtonUpMask | ButtonStartMask))
			{
				toggleHeaterPwm(SecondaryHeaterMask);
			}
			#endif
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
				bool update=false;
				long value=(long)manualModeChangeCountDownTime;
				if(btnIsUpPressed)
				{
					value +=1; update=true;
				}
				else if(btnIsDownPressed)
				{
					value -= 1; update=true;
				}			
				else if(btnIsUpContinuousPressed)
				{
					value += 5; update=true;
				}
				else if(btnIsDownContinuousPressed)
				{
					value -= 5; update=true;
				}
				if(update)
				{
					if(value < 0) value=0;
					else if(value > 600) value=600;
					manualModeChangeCountDownTime = value;
					uiRunningTimeShowInitial(manualModeChangeCountDownTime * 60);
				}
			}
			else if(_blinkingSettingTemperature)
			{
				//bool update=false;
				//long value=(long)manualModeChangeCountDownTime;
				if(btnIsUpPressed)
				{
					adjustSp(1);
				}
				else if(btnIsDownPressed)
				{
					adjustSp(-1);
				}			
				else if(btnIsUpContinuousPressed)
				{
					adjustSp(4);
				}
				else if(btnIsDownContinuousPressed)
				{
					adjustSp(-4);
				}				
			}
			else
			{
				handleAdjustPwm();
			}
			return true;
		}
		else if(event == TemperatureEventMask)
		{
			// Handle temperature change or other states
			if (! gIsTemperatureReached)
			{
				if(gCurrentTemperature >= gSettingTemperature)
				{
						// beep & start counting time
	
					buzzPlaySound(SoundIdTemperatureReached);
	
					gIsTemperatureReached=true;
	
					if(isManualModeCountDownMode)
					{
						uiRunningTimeStartCountDown(manualModeChangeCountDownTime*60);
						tmSetTimeoutAfter(manualModeChangeCountDownTime*60 * 1000);
					}
					else
						uiRunningTimeStart();
	
					brewLogger.event(RemoteEventTemperatureReached);
					wiReportEvent(RemoteEventTemperatureReached);
					return true;
				}
			}
				// Temperate Reached state	
		} // end of temperature handling
		else if(event == TimeoutEventMask)
		{
			buzzPlaySound(SoundIdCountDown);
			isManualModeCountDownMode=false;
			uiRunningTimeStart();
			return true;
		}
		return false;
	}

	void enterAutomaticDistilling(void){
		_state = DistillingStateBeforeHead;
		// setup distilling mode screen
		distillRecipe.load();
		
		setEventMask(TemperatureEventMask /*| ButtonPressedEventMask */);

		//load temperature value
		// setup screen
		uiClearTitle();
		uiDistillingModeTitle();
		uiClearSubTitleRow();
		uiClearPrompt();

		uiDistillingModeStage(DistillStageStart); // 0 is Start
		uiRunningTimeSetPosition(RunningTimeNormalPosition);
		uiRunningTimeStart();
		// displace temperature
		uiTempDisplaySetPosition(TemperatureAutoModePosition);
		setSettingTemperature((float)distillRecipe.tempOf(DistillHeadTemp));

		uiButtonLabel(ButtonLabel(Up_Down_x_Pmp));

#if MaximumNumberOfSensors > 1
		setSensorForStage(SensorForDistilling);
#endif		

		heatLoadParameters();

		heatOn(HeatingModePWM);
		gIsEnterPwm=true;
		uiShowPwmLabel();

		changePwmValue(100);
		
		#if MaximumNumberOfSensors > 1
		brewLogger.startSession(gSensorNumber,TemperatureChartPeriod,gIsUseFahrenheit,false);
		#else
		brewLogger.startSession(1,TemperatureChartPeriod,gIsUseFahrenheit,false);
		#endif

		brewLogger.stage(StageDistillingPreHeat);
		wiReportCurrentStage(StageDistillingPreHeat);
	}

	bool eventHanlderConfirmation(byte event){
		if(btnIsStartPressed){
			enterAutomaticDistilling();
		}else if(btnIsDownPressed){
			enterManualDistilling();			
		}else if(btnIsEnterPressed){
			// back to main
			backToMain();
		}else return false;
		return true;
	}

	void automaticDistillFinished(void)
	{
		heatOff();
		pump.off();
		// buzz and display to end

		uiTempDisplayHide();
		uiRunningTimeStop();

		uiClearScreen();

		uiPrompt(STR(Distill_finished));

		buzzPlaySoundRepeat(SoundIdBrewEnd);
		setEventMask(TimeoutEventMask);
		tmSetTimeoutAfter(BREW_END_STAY_DURATION * 1000);

		brewLogger.endSession();

		wiReportEvent(RemoteEventBrewFinished);
	}

	bool eventHanlderEnd(byte event){
		if(event == TimeoutEventMask){
			buzzMute();
			backToMain();
			return true;
		}
		return false;
	}
	
	void changePwmValue(uint8_t pwm){
		gBoilHeatOutput = pwm;
		uiShowPwmValue(gBoilHeatOutput);
		wiReportPwm();
	}

	bool handleAdjustPwm(void){
			if(btnIsUpPressed)
			{
				adjustPwm(+1);
			}
			else if(btnIsDownPressed)
			{
				adjustPwm(-1);
			}
			else if(btnIsUpContinuousPressed)
			{
				adjustPwm(+4);
			}
			else if(btnIsDownContinuousPressed)
			{
				adjustPwm(-4);
			}
			else return false;
		return true;
	}
	
	bool autoDistillerButtonHandler(void){
		if(btnIsEnterPressed)
		{
			if(btnIsEnterLongPressed)
			{
				pump.setRestEnabled(!pump.isRestEnabled());
			}
			else
			{
				pump.toggle();
			}			
		}else{
			handleAdjustPwm();
		}
		return true;
	}

	bool eventHanlderBeforeHead(byte event){
		if(event == ButtonPressedEventMask){
			return autoDistillerButtonHandler();
		} else if(event == TemperatureEventMask){
			if(gCurrentTemperature >= gSettingTemperature ){
				// stop heating
				heatProgramOff();
				// prompt for start head
				uiRunningTimeStop();
//				uiRunningTimeHide();
				uiClearPrompt();
				uiPrompt(STR(TurnOnCoolWater));
				uiButtonLabel(ButtonLabel(x_x_x_Ok));
				_state = DistillingStateHeadConfirmation;

				buzzPlaySoundRepeat(SoundIdConfirmUser);
				return true;
			}
		} // TemperatureEventMask
		return false;
	}

	bool eventHanlderHeadConfirmation(byte event){
		if(event == ButtonPressedEventMask){
			if(btnIsEnterPressed){
				buzzMute();
				_state = DistillingStateHead;
				uiClearPrompt();
				uiDistillingModeStage(DistillStageHead); 
				uiRunningTimeStart();
				uiButtonLabel(ButtonLabel(Up_Down_x_Pmp));
				uiShowPwmLabel();

				heatOn(HeatingModePWM);
				changePwmValue(distillRecipe.pwmOf(DistillHeadPwm));
				setSettingTemperature((float)distillRecipe.tempOf(DistillHeartTemp));

				brewLogger.stage(StageDistillingHead);
				wiReportCurrentStage(StageDistillingHead);
				return true;
			}
		} 
		return false;
	}

	bool eventHanlderHead(byte event){
		if(event == ButtonPressedEventMask){
			return autoDistillerButtonHandler();

		} else if(event == TemperatureEventMask){
			if(gCurrentTemperature >= gSettingTemperature ){
				_state = DistillingStateHeart;
				uiDistillingModeStage(DistillStageHeart); 
				changePwmValue(distillRecipe.pwmOf(DistillHeartPwm));
				setSettingTemperature((float)distillRecipe.tempOf(DistillTailTemp));
				
				brewLogger.stage(StageDistillingHeart);
				wiReportCurrentStage(StageDistillingHeart);

				buzzPlaySound(SoundIdTemperatureReached);
				return true;
			}
		} // TemperatureEventMask
		return false;
	}

	bool eventHanlderHeart(byte event){
		if(event == ButtonPressedEventMask){
			return autoDistillerButtonHandler();
		} else if(event == TemperatureEventMask){
			if(gCurrentTemperature >= gSettingTemperature ){
				_state = DistillingStateTail;
				uiDistillingModeStage(DistillStageTail); 
				changePwmValue(distillRecipe.pwmOf(DistillTailPwm));
				setSettingTemperature(distillRecipe.tempOf(DistillEndTemp));

				brewLogger.stage(StageDistillingTail);
				wiReportCurrentStage(StageDistillingTail);
				buzzPlaySound(SoundIdTemperatureReached);
				return true;
			}
		} // TemperatureEventMask
		return false;
	}

	bool eventHanlderTail(byte event){
		if(event == ButtonPressedEventMask){
			return autoDistillerButtonHandler();
		} else if(event == TemperatureEventMask){
			if(gCurrentTemperature >= gSettingTemperature ){
				_state = DistillingStateEnd;
				automaticDistillFinished();	
				return true;				
			}
		} // TemperatureEventMask
		return false;
	}


} distiller;


void distillingSetup(void)
{
	distiller.setup();
}

bool distillingEventHandler(byte event)
{
	return distiller.eventHanlder(event);
}
#endif

// *************************
//*  Main Screen
// *************************

void mainSetup(void)
{
	uiClearScreen();

	uiTitle(STR(welcome));
	uiButtonLabel(ButtonLabel(Manual_Auto_Setup));
    uiTempDisplaySetPosition(TemperatureMainScreenPosition);

    uiPrintIpAddress();
    wiReportCurrentStage(StageIdleScreen);

    #if MaximumNumberOfSensors > 1
    setSensorForStage(SensorForIdle);
    #endif
}

// main screen
//  -down button-> manual
//  -start button->auto
//  -ener button ->setup

bool mainEventHandler(byte event)
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
#if SupportDistilling
  	else if(btnIsUpPressed)
  	{
      	switchApplication(DISTILLING_MODE_SCREEN);
  	}
#else
	else return false;
#endif
	return true;
}

// *************************
//*  Screen switch
// *************************


const CScreen *currentScreen;
void switchEventHandler(byte screenId){
	currentScreen=allScreens+screenId;
}
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
	clearStatus(); // manually quit won't start "auto resume"
	// turn pump & heat off
	heatOff();
#if SpargeHeaterSupport == true
	stopHeatingSpargeWater();
#endif
	pump.off();
	buzzMute();
	#if EnableLevelSensor
	lvMonitor.reset();
	#endif
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
/*
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
*/
void startBrewManiac()
{
	// auto resume if necessary.
	if(checkResume()){

		DBG_PRINTF("Auto resume waiting\n");
		autoModeEnterAutoResumeWaiting();
		switchEventHandler(AUTO_MODE_SCREEN);
	}else{
		switchApplication(MAIN_SCREEN);
	}
}

bool brewmaniac_setup() {

	EepromInit();
	automation.load();
	// ESP8266
  	Wire.begin(I2C_SDA,I2C_SCL);

	initIOPins();

	gIsUseFahrenheit = readSetting(PS_TempUnit);

	tmInitialize();
	btnInitialize();
	tpInitialize();

	uiInitialize();

	heatInitialize();
	//pumpInitialize();


	uiPrintInitialScreen();

	wiInitialize();
	// RecoveryMode
	DBG_PRINTF("delay\n");
	DBG_PRINTF("gButtonPressed=%d\n",gButtonPressed);
	delay(3000);
	DBG_PRINTF("gButtonPressed=%d\n",gButtonPressed);
	btnReadButtons();
	delay(150);
	btnReadButtons();
	DBG_PRINTF("gButtonPressed=%d\n",gButtonPressed);

	if(btnIsUpPressed){
		delay(500);
		if(btnIsUpPressed){
			return true;
		}
	}
	return false;
	// RecoveryMode
}

//*********************************************************************
//*********************************************************************



void brewmaniac_loop() {

	// Process Events
	//    button, temperature, time
	//  Event: BUTTON,  TEMP Reach, TimeOut.

	tpReadTemperature();
	bool updateLcd;
	// let the handler compare the temperatures themselves if they request it.
	if(_currentEventMask & TemperatureEventMask){
		wiLcdBufferBegin();
		updateLcd=(*currentScreen->eventHandler)(TemperatureEventMask);
		wiLcdBufferEnd(updateLcd);
	}
	if(tmTiming())
	{
		if(_currentEventMask & TimeoutEventMask){
			wiLcdBufferBegin();
			updateLcd=(*currentScreen->eventHandler)(TimeoutEventMask);
			wiLcdBufferEnd(updateLcd);
		}
	}

	if(btnReadButtons())
	{
		if(isExactButtonsPressed(ButtonUpMask | ButtonDownMask)){
			wiLcdBufferBegin();
			backToMain();
			wiLcdBufferEnd(true);
		}else
	#if UseLcdBuffer
		if(isExactButtonsPressed(ButtonUpMask | ButtonDownMask | ButtonStartMask ))
			refreshLcdDisplay();
		else
	#endif
		{
			// if(_currentEventMask & ButtonPressedEventMask) button event is always handled in all
			// screen!
			wiLcdBufferBegin();
			(*currentScreen->eventHandler)(ButtonPressedEventMask);
			wiLcdBufferEnd(true);
		}
	}

	if(pump.restEvent())
	{
		if(_currentEventMask & PumpRestEventMask){
			wiLcdBufferBegin();
			(*currentScreen->eventHandler)(PumpRestEventMask);
			wiLcdBufferEnd(true);
		}
	}

	#if MaximumNumberOfSensors > 1
	brewLogger.temperatures(gTemperatureReading);
	#else
	brewLogger.temperature(gCurrentTemperature);
	#endif

	//update Time & temperature
	uiLoop();

	//
	// threads
	heatThread();
	#if EnableLevelSensor
	lvMonitor.run();
	#endif
	pump.run();
	buzzThread();

	wiThread();
	// handler state machine

}// end of loop();
