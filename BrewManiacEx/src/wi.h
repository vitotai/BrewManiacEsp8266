#ifndef WI_H
#define WI_H

#define	wiIndicationStart(a)
#define	wiIndicationPut(a)
#define	wiIndicationEnd()

#include "BrewManiacWeb.h"
extern BrewManiacWeb bmWeb;

void wiSendButtonLabel(const byte labelId)
{
	bmWeb.setButtonLabel(labelId);
}
byte _currentStage=101;
void wiReportCurrentStage(byte stage)
{
    _currentStage = stage;
    bmWeb.setBrewStage(stage);
}

#if SpargeHeaterSupport == true
void wiReportAuxHeater(byte value)
{
	bmWeb.setAuxHeaterStatus(value);
}
void wiUpdateHeaterStatus(void)
{
	bmWeb.statusChange();
}
#endif //#if SpargeHeaterSupport == true

#if SecondaryHeaterSupport == true
void wiReportHeater(byte primary,byte secondary)
{
	bmWeb.setHeaterStatus(primary,secondary);
}

#else
void wiReportHeater(byte value)
{
	bmWeb.setHeaterStatus(value);
}
#endif


void wiReportPump(byte value)
{
	bmWeb.setPumpStatus(value);
}

void wiReportEvent(byte event)
{
	bmWeb.brewEvent(event);
}

void wiReportPwm(void)
{
	bmWeb.updatePwm(gBoilHeatOutput);
}

void wiReportSettingTemperature(void)
{
	bmWeb.updateSettingTemperature();
}

void wiTogglePwm(void)
{
	bmWeb.statusChange();
}

void wiRecipeChange(void)
{
	bmWeb.automationChanged();
}

void wiSettingChanged(int address,byte value)
{
	bmWeb.settingChanged(address,value);
}


void wiSetDeviceAddress(byte ip[], bool apmode)
{
	// support only IPv4 for now
	if(	ip[0] != 0 && ip[1] != 0 && ip[2] != 0 && ip[3] != 0){
		uiSetWirelessStatus(apmode? WiStateAccessPoint:WiStateConnected);
		uiSetIp(ip);
		if(_currentStage == StageIdleScreen)
			uiPrintIpAddress();

	}else{ // clear
		uiSetWirelessStatus(WiStateNotConnected);
		buzzPlaySound(SoundIdWarnning);

		uiSetIp(ip);
		if(_currentStage == StageIdleScreen)
			uiClearIpAddress();
	}
}

#if	MaximumNumberOfSensors	> 1

extern byte scanSensors(byte max,byte addresses[][8]);
extern byte gSensorNumber;
extern byte gSensorAddresses[MaximumNumberOfSensors][8];

byte wiReadCalibrationOfSensor(byte i)
{
	readSetting(CalibrationAddressOf(i));
}

void wiUpdateCalibrationOfSensor(byte i,byte value)
{
	updateSetting(CalibrationAddressOf(i),value);
	gSensorCalibrations[i]=((float)value -50.0)/10.0;
	Serial.printf("cal:%d, %value\n",i,value);
}

byte wiReadPrimarySensor(byte i)
{
	return readSetting(PS_SensorUseAddressOf(i));
}

byte wiReadAuxSensor(byte i)
{
	return readSetting(PS_AuxSensorAddressOf(i));
}

void wiUpdatePrimarySensor(byte i,byte v)
{
	updateSetting(PS_SensorUseAddressOf(i),v);
}

void wiUpdateAuxSensor(byte i,byte v)
{
	updateSetting(PS_AuxSensorAddressOf(i),v);
}

static byte _wiSensorScanRequest;

void wiStartSensorScan(void)
{
	_wiSensorScanRequest =1;
}
#endif //#if	MaximumNumberOfSensors	> 1

extern void temperatureUnitChange(bool useF);

void wiUpdateSetting(int address,byte value)
{
	updateSetting(address,value);
	if(PS_TempUnit ==address){
		temperatureUnitChange((boolean)value);
	}

#if	MaximumNumberOfSensors	== 1
	if(address == PS_Offset){
		gSensorCalibration = ((float)value -50.0)/10.0;
	}
#endif
}

void wiInitialize(){
#if	MaximumNumberOfSensors	> 1
	_wiSensorScanRequest =0;
#endif
}

void wiThread()
{
#if	MaximumNumberOfSensors	> 1
	if(_wiSensorScanRequest)
	{
		gSensorNumber=scanSensors(MaximumNumberOfSensors,gSensorAddresses);
		_wiSensorScanRequest =0;
		bmWeb.scanSensorDone();
	}
#endif
}


#endif
