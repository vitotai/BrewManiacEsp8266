#ifndef BrewManiacWeb_H
#define BrewManiacWeb_H

#include <Arduino.h>
#include <functional>
#include <ArduinoJson.h>

#include "config.h"

class BrewManiacWeb;


typedef byte SensorAddressType[8];

typedef enum _BmwEventType{
	BmwEventAutomationChanged,
	BmwEventSettingChanged,
	BmwEventStatusUpdate,
	BmwEventBrewEvent,
	BmwEventPwmChanged,
	BmwEventSettingTemperatureChanged,
	BmwEventButtonLabel,
	BmwEventSensorScanComplete
}BmwEventType;

typedef std::function<void(BrewManiacWeb* bmw, BmwEventType event)> BmwEventHandler;

class BrewManiacWeb
{
protected:
	BmwEventHandler _eventHandler;

	unsigned long _reportPeriod;
	unsigned long _lastReportTime;

	// status from BrewManiac
	uint8_t _stage;
	uint8_t _pumpStatus;
	uint8_t _heaterStatus;

	#if SecondaryHeaterSupport == true
	uint8_t _secondaryHeaterStatus;
	#endif

	#if SpargeHeaterSupport == true
	uint8_t _auxHeaterStatus;
	#endif
    uint8_t _buttonLabel;
    uint8_t _lastEvent;
    uint8_t _pwm;
public:

	BrewManiacWeb(void);
	/* from BM */
	void statusChange(void);

	void setButtonLabel(uint8_t btns);
	void setBrewStage(uint8_t stage);

    #if SpargeHeaterSupport == true
	void setAuxHeaterStatus(uint8_t value){_auxHeaterStatus=value; }

    #if SecondaryHeaterSupport == true

	void setHeaterStatus(uint8_t value,uint8_t sec){ _heaterStatus=value; _secondaryHeaterStatus=sec;}

	#else
	void setHeaterStatus(uint8_t value){ _heaterStatus=value; }
    #endif // #if SecondaryHeaterSupport == true

    #else //#if SpargeHeaterSupport == true

    #if SecondaryHeaterSupport == true
	void setHeaterStatus(uint8_t value,uint8_t sec){ _heaterStatus=value; _secondaryHeaterStatus=sec;  statusChange();}

    #else
	void setHeaterStatus(uint8_t value){ _heaterStatus = value; statusChange(); }
    #endif

    #endif //#if SpargeHeaterSupport == true

	void setPumpStatus(uint8_t value){ _pumpStatus=value; statusChange(); }

	void brewEvent(uint8_t event);
	void updatePwm(uint8_t pwm);
	void updateSettingTemperature(void);

    void settingChanged(int address,byte value);
    void automationChanged(void);

	/* */
	float temperature(void);
	bool  isBrewing(void);
	byte   brewingStage(void);
	byte   lastBrewEvent(void);
	void setIp(uint8_t ip[],bool apmode=false);

	void loop(void);
	// get data
	void getSettings(String& json);
	void getAutomation(String& json);
	// update data
	bool updateSettings(String& json);
	bool updateAutomation(String& json);
	void sendButton(byte mask,bool longPressed);
	// get current status
	void getCurrentStatus(String& json,bool initial=false);

	// 'push' data
	void getLastEvent(String& json);
	void getSettingPwm(String& json);
	void getSettingTemperature(String& json);
	void getButtonLabel(String& json);
	// event handler
	void onEvent(BmwEventHandler handler);

#if	MaximumNumberOfSensors > 1
	void scanSensors(void);
	void updateSensorSetting(String& json);
	float *temperatures(void);
	byte sensorNumber(void);

	void   scanSensorDone(void);
#endif

};

#endif
