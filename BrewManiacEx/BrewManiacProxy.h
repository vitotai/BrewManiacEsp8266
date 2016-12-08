#ifndef BrewManiacProxy_H
#define BrewManiacProxy_H

#include <Arduino.h>
#include <functional>
#include "config.h"

typedef byte SensorAddressType[8];

#define BMNotifySensorScanDone 2

#define BMNotifyAutomationChanged 3
#define	BMNotifySettingChanged 4
#define	BMNotifyStatusChanged 5
#define	BMNotifyEvent 6
#define	BMNotifySetPwm 7
#define	BMNotifySetTemperature 8
#define	BMNotifyButtonLabel 9

typedef struct _autoRecipe{
	byte restTime[8];
	float restTemp[8];
	byte boilTime;
	byte hops[10];
	byte numberHops;
}AutomationRecipe;
#define NumberSensorStage 6

typedef std::function<void(uint8_t event)> BmProxyEventHandlerFunc;

class BrewManiacProxy{

	BmProxyEventHandlerFunc _eventHandler;
public:
	BrewManiacProxy(void):_eventHandler(NULL){}
	
	void begin(BmProxyEventHandlerFunc handler);

	// from Web
	void sendButton(byte mask,bool longPressed);
	void setIp(byte ip[]);

	// recipe
	AutomationRecipe automationRecipe;	
	void recipeChanged(void);
	void setAutomationRecipe(AutomationRecipe *recipe);

	// settings
	byte getSetting(int addr);
	void settingChanged(int address,byte value);
	void updateSetting(int address,byte value);	
	// status from BrewManiac
	byte stage;
	byte pumpStatus;
	byte heaterStatus;
	byte auxHeaterStatus;

 	bool isPwmOn(void);
	bool isTempReached(void);
	bool isPaused(void);
	bool isCountingUp(void);

	float currentTemperature(void);
	int  runningTimer(void);
	
	void statusChange(void);
	byte lastEvent;
	void brewEvent(byte event);
	
	int  setPwm(void);
	void updatePwm(int pwm);
	
	float setTemperature(void);
	void updateSettingTemp(float st);

	// from BrewManiac
	byte buttonLabel;
	void setButtonLabel(byte btns);
	
	int brewManiacVersion;
	
#if MaximumNumberOfSensors > 1
	byte   sensorNumber(void);
	float *sensorReading(void);
	SensorAddressType*  sensorAddresses(void);
	void   sensorUsage(byte *primary,byte *auxiliary);
	void   setSensorUsage(byte *primary,byte *auxiliary);
	byte   sensorCalibrationOfIndex(byte i);
	void   setSensorCalibrationOfIndex(byte i,byte value);
	
	void   scanSensorStart(void);
	void   scanSensorDone(void);
#endif
};

#endif










