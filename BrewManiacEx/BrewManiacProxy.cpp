#include "BrewManiacProxy.h"

extern float gCurrentTemperature;
extern float gSettingTemperature;
extern float gBoilStageTemperature;

extern byte gBoilHeatOutput;
extern bool gIsEnterPwm;
extern bool gIsTemperatureReached;

extern bool uiIsTimerRunning(void);
extern bool uiIsTimerRunningUp(void);
extern int  uiGetDisplayTime(void);

extern void virtualButtonPress(byte mask,boolean longPressed);

extern void wiSetRecipe(AutomationRecipe *recipe);
extern void wiReadRecipe(AutomationRecipe *recipe);

extern byte readSetting(int addr);
extern void updateSetting(int addr,byte value);

extern void wiSetDeviceAddress(byte ip[],bool apmode);
extern void wiUpdateSetting(int address,byte value);

#if MaximumNumberOfSensors > 1
extern byte gSensorAddresses[MaximumNumberOfSensors][8];
extern byte gSensorNumber;
extern byte gPrimarySensorIndex;
extern byte gAuxSensorIndex;
extern float gAuxTemperature;
extern byte wiReadCalibrationOfSensor(byte i);
extern void  wiUpdateCalibrationOfSensor(byte i,byte value);

extern float gTemperatureReading[MaximumNumberOfSensors];

extern byte scanSensors(byte max,byte addresses[][8]);


byte   BrewManiacProxy::sensorCalibrationOfIndex(byte i)
{
	return wiReadCalibrationOfSensor(i);
}

void   BrewManiacProxy::setSensorCalibrationOfIndex(byte i,byte value)
{
	wiUpdateCalibrationOfSensor(i,value);
}

byte   BrewManiacProxy::sensorNumber(void)
{
	return gSensorNumber;
}
float* BrewManiacProxy::sensorReading(void)
{
	return gTemperatureReading;
}

SensorAddressType *BrewManiacProxy::sensorAddresses(void)
{
	return gSensorAddresses;
}

extern byte wiReadPrimarySensor(byte i);
extern byte wiReadAuxSensor(byte i);
extern void wiUpdatePrimarySensor(byte i,byte v);
extern void wiUpdateAuxSensor(byte i,byte v);

void   BrewManiacProxy::sensorUsage(byte *primary,byte *auxiliary)
{
	for(byte i=0;i<NumberSensorStage;i++)
	{
		byte p=wiReadPrimarySensor(i);
		byte a=wiReadAuxSensor(i);;
		primary[i]=(p>=gSensorNumber)? 0:p;
		auxiliary[i]=(a>=gSensorNumber)? 0:a;
	}
}

void   BrewManiacProxy::setSensorUsage(byte *primary,byte *auxiliary)
{
	for(byte i=0;i<NumberSensorStage;i++)
	{
		wiUpdatePrimarySensor(i,primary[i]);
		wiUpdateAuxSensor(i,auxiliary[i]);
	}
}
extern void wiStartSensorScan(void);

void   BrewManiacProxy::scanSensorStart(void)
{
	wiStartSensorScan();
}

void   BrewManiacProxy::scanSensorDone(void)
{
	if(_eventHandler) _eventHandler(BMNotifySensorScanDone);
}

#endif //#if MaximumNumberOfSensors > 1



void BrewManiacProxy::begin(BmProxyEventHandlerFunc handler)
{
	_eventHandler=handler;
	// read recipe from EEPROM.
	wiReadRecipe(&automationRecipe);
}

bool BrewManiacProxy::isPwmOn(void)
{
	return gIsEnterPwm;
}

bool BrewManiacProxy::isTempReached(void)
{
	return gIsTemperatureReached;
}

bool BrewManiacProxy::isPaused(void)
{
	return !uiIsTimerRunning();
}

bool BrewManiacProxy::isCountingUp(void)
{
	return uiIsTimerRunningUp();
}

float BrewManiacProxy::currentTemperature(void)
{
	return gCurrentTemperature;
}

int  BrewManiacProxy::runningTimer(void)
{
	return uiGetDisplayTime();
}
	
int   BrewManiacProxy::setPwm(void)
{
	return gBoilHeatOutput;
} 
	
float BrewManiacProxy::setTemperature(void)
{
	return gSettingTemperature;
}


void BrewManiacProxy::sendButton(byte mask,bool longPressed)
{
	virtualButtonPress(mask & 0xF,longPressed);
}
// recipe
void BrewManiacProxy::setAutomationRecipe(AutomationRecipe *recipe)
{
	memcpy(&automationRecipe,recipe,sizeof(AutomationRecipe));
	wiSetRecipe(recipe);
	if(_eventHandler) _eventHandler(BMNotifyAutomationChanged);
}

void BrewManiacProxy::recipeChanged(void)
{
	wiReadRecipe(&automationRecipe);
	if(_eventHandler) _eventHandler(BMNotifyAutomationChanged);
}

// setting
byte BrewManiacProxy::getSetting(int addr)
{
	return readSetting(addr);
}

void BrewManiacProxy::updateSetting(int address,byte value)
{
	wiUpdateSetting(address,value);
}


void BrewManiacProxy::settingChanged(int address,byte value)
{
	if(_eventHandler) _eventHandler(BMNotifySettingChanged);
}

void BrewManiacProxy::statusChange(void)
{
	if(_eventHandler) _eventHandler(BMNotifyStatusChanged);
}

void BrewManiacProxy::brewEvent(byte event)
{
	lastEvent=event;
	if(_eventHandler) _eventHandler(BMNotifyEvent);
}

void BrewManiacProxy::updatePwm(int pwm)
{
	if(_eventHandler) _eventHandler(BMNotifySetPwm);
}

void BrewManiacProxy::updateSettingTemp(float st)
{
	if(_eventHandler) _eventHandler(BMNotifySetTemperature);
}


void BrewManiacProxy::setButtonLabel(byte btns)
{
	buttonLabel=btns;
	
	if(_eventHandler) _eventHandler(BMNotifyButtonLabel);
}

void BrewManiacProxy::setIp(byte ip[], bool apmode)
{
	wiSetDeviceAddress(ip,apmode);
}




