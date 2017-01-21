#ifndef WI_H
#define WI_H

#define	wiIndicationStart(a) 
#define	wiIndicationPut(a)
#define	wiIndicationEnd()	

#include "BrewManiacProxy.h"
extern BrewManiacProxy bmproxy;


void wiSendButtonLabel(const byte labelId)
{
	bmproxy.setButtonLabel(labelId);
}


void wiReportCurrentStage(byte stage)
{
	bmproxy.stage=stage;
	bmproxy.statusChange();
}

#if SpargeHeaterSupport == true
void wiReportAuxHeater(byte value)
{
	bmproxy.auxHeaterStatus=value;
}
void wiReportHeater(byte value)
{
	bmproxy.heaterStatus=value;
}

void wiUpdateHeaterStatus(void)
{
	bmproxy.statusChange();
}
#else
void wiReportHeater(byte value)
{
	bmproxy.heaterStatus=value;
	bmproxy.statusChange();
}
#endif //#if SpargeHeaterSupport == true



void wiReportPump(byte value)
{
	bmproxy.pumpStatus=value;
	bmproxy.statusChange();
}

void wiReportEvent(byte event)
{
	bmproxy.brewEvent(event);
}

void wiReportPwm(void)
{
	bmproxy.updatePwm(gBoilHeatOutput);
}

void wiReportSettingTemperature()
{
	bmproxy.updateSettingTemp(gSettingTemperature);
}

void wiTogglePwm(void)
{
	bmproxy.statusChange();
}

void wiRecipeChange(void)
{
	bmproxy.recipeChanged();
}
// read recipe
void wiReadRecipe(AutomationRecipe *recipe)
{
	byte stage;
	bool endMash=false;
	for(stage=0;stage<8;stage++)
	{
		byte time=readSetting(PS_StageTimeAddr(stage));

		if(stage ==0) time =1;
		if(endMash && stage<7) time=0;

		recipe->restTime[stage]=time;

		if(time==0)
		{
			recipe->restTemp[stage]=0.0;
			endMash=true;
		}
		else
		{
			int temp=readSettingWord(PS_StageTemperatureAddr(stage));
			recipe->restTemp[stage]=TempFromStorage(temp);
		}
	}
	// finish mash schedule. now boil & hop
	byte tNum=readSetting(PS_NumberOfHops);
	if(tNum > 10) tNum=0;
	recipe->numberHops=tNum;
	recipe->boilTime=readSetting(PS_BoilTime);
	
	byte i=0;
	while(i < tNum)
  	{
    	byte time=readSetting(PS_BoilTime+1+i);
		recipe->hops[i]=time;
    	i++;
  	}
}

void wiSetRecipe(AutomationRecipe *recipe)
{
	byte stage;
	
	for(stage=0;stage<8;stage++)
	{
		byte time=recipe->restTime[stage];
		if(stage ==0) time=1;
		else if(stage >=6 && time==0) time=1;
 
  		if(time !=0)
 	 		updateSettingWord(PS_StageTemperatureAddr(stage),ToTempInStorage(recipe->restTemp[stage]));
  	
  		updateSetting(PS_StageTimeAddr(stage),time);
	}
  	// write number 
  	updateSetting(PS_NumberOfHops,recipe->numberHops);
	updateSetting(PS_BoilTime,recipe->boilTime);

  	for(byte idx=0;idx < recipe->numberHops; idx++)
  	{
    	updateSetting(PS_BoilTime+1+idx,recipe->hops[idx]);
  	}
	commitSetting();
}

void wiSettingChanged(int address,byte value)
{
	bmproxy.settingChanged(address,value);
}


void wiSetDeviceAddress(byte ip[], bool apmode)
{
	// support only IPv4 for now
	if(	ip[0] != 0 && ip[1] != 0 && ip[2] != 0 && ip[3] != 0){
		uiSetWirelessStatus(apmode? WiStateAccessPoint:WiStateConnected);
		uiSetIp(ip);
		if(bmproxy.stage == StageIdleScreen)
			uiPrintIpAddress();

	}else{ // clear
		uiSetWirelessStatus(WiStateNotConnected);
		buzzPlaySound(SoundIdWarnning);

		uiSetIp(ip);
		if(bmproxy.stage == StageIdleScreen)		
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


void wiUpdateSetting(int address,byte value)
{
	updateSetting(address,value);

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
		bmproxy.scanSensorDone();
	}
#endif
}


#endif




