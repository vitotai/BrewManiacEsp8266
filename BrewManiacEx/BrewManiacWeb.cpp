#include <EEPROM.h>
#include "BrewManiacWeb.h"

BrewManiacProxy bmproxy;

String _jsonDisconnected="{\"code\":-1,\"result\":\"BM disc.\"}";

#if SerialDebug == true
#define DEBUGF(...)  DebugPort.printf(__VA_ARGS__)
#else
#define DEBUGF(...)
#endif

void BrewManiacWeb::_bmProxyEventHandler(uint8_t event)
{
	if(event == BMNotifyAutomationChanged){
			DEBUGF("BMNotifyAutomationChanged\n");
			if(_eventHandler) _eventHandler(this,BmwEventAutomationChanged);
		
	}else if(event == BMNotifySettingChanged){
			DEBUGF("BMNotifySettingChanged\n");
			if(_eventHandler) _eventHandler(this,BmwEventSettingChanged);
		
	}else if(event == BMNotifyStatusChanged){
			//DEBUGF("BMNotifyStatusChanged"); // too much
			if(_eventHandler) _eventHandler(this,BmwEventStatusUpdate);
		
	}else if(event == BMNotifyEvent){
			DEBUGF("Event:%d\n",bmproxy.lastEvent);
			if(_eventHandler) _eventHandler(this,BmwEventBrewEvent);		
	}else if(event == BMNotifySetPwm){
			DEBUGF("setting PWM:%d\n",bmproxy.setPwm());
			if(_eventHandler) _eventHandler(this,BmwEventPwmChanged);		
	}else if(event == BMNotifySetTemperature){
//			DEBUGF("setting Temp:%f\n",bmproxy.setTemperature());
			if(_eventHandler) _eventHandler(this,BmwEventSettingTemperatureChanged);
	}else if(event == BMNotifyButtonLabel){
//			DEBUGF("button label:%d\n",bmproxy.buttonLabel);
			if(_eventHandler) _eventHandler(this,BmwEventButtonLabel);
	}else if(event == BMNotifySensorScanDone){
			DEBUGF("Scan done\n");
			if(_eventHandler) _eventHandler(this,BmwEventSettingChanged);
	}

}


#if	MaximumNumberOfSensors > 1
void BrewManiacWeb::scanSensors(void)
{
	bmproxy.scanSensorStart();
}

void BrewManiacWeb::updateSensorSetting(String& json)
{
}
#endif


void BrewManiacWeb::setIp(uint8_t ip[],bool apmode)
{
	bmproxy.setIp(ip,apmode);
}

BrewManiacWeb::BrewManiacWeb(void)
{
	_reportPeriod=DEFAULT_REPORT_PERIOD;
	_lastReportTime=0;

	_eventHandler=NULL;
}
void BrewManiacWeb::onEvent(BmwEventHandler handler)
{
	_eventHandler=handler;
	bmproxy.begin([&](uint8_t event){
		_bmProxyEventHandler(event);
		});
}

static const char* SettingMap[]={
//#define PS_UseGas   0  //	Use Gas
	NULL,
//#define PS_kP      1  // 	kP
  	"s_kp",
//#define PS_kI      2  //	kI
  	"s_ki",
//#define PS_kD      3  //     kD
  	"s_kd",
//#define PS_SampleTime      4  //     SampleTime
	"s_sample_time",
//#define PS_WindowSize      5  //     WindowSize
  	"s_window",
//#define PS_BoilHeat      6    //   Boil Heat %
	"s_pwm",
//#define PS_Offset     7      // Offset
  	"s_cal",
//#define PS_Hysteresi     8   //    Hysteresi 
	"s_pidstart",
	"s_piddoughin", // 9 
//#define PS_TempUnit   10     //  Scale Temp
	"s_unit",
//#define PS_SensorType     11      // Sensor Type
  	"s_nodelay",
//#define PS_BoilTemp     12       //Temp Boil °C
	"s_boil",
//     13       Temp Boil °F
	NULL,
//#define PS_PumpCycle     14  //     Time Pump Cycle
	"s_pumpcycle",
//#define PS_PumpRest     15   //    Time Pump Rest
	"s_pumprest",
//#define PS_PumpPreMash     16  //     Pump PreMash
	"s_pumppremash",
//#define PS_PumpOnMash     17   //    Pump on Mash
	"s_pumpmash",
//#define PS_PumpOnMashOut     18  //     Pump on MashOut
	"s_pumpmashout",
//#define PS_PumpOnBoil      19     //  Pump on Boil
	"s_pumpboil",
//#define PS_TempPumpRest     20    //   Temp Pump Rest °C
	"s_pumpstop",
//     21       Temp Pump Rest °F
	NULL,
//#define PS_PidPipe     22     //  PID Pipe
	"s_pipe",
//#define PS_SkipAddMalt     23  //     Skip Add Malt
	"s_skipadd",
//#define PS_SkipRemoveMalt     24  //     Skip Remove Malt
	"s_skipremove",
//#define PS_SkipIodineTest     25    //   Skip Iodine Test
	"s_skipiodine",
//#define PS_IodineTime     26   //    Iodine Time
	"s_iodine",
//#define PS_Whirlpool     27     //  Whirlpool  	
	"s_whirlpool",
	// 28: B
	NULL,
	// 29  M
	NULL,
	// 30  E
	NULL,
	// 31  X
	NULL,
	// 32
	"s_spenable",  //PS_SpargeWaterEnableAddress    32
	//33 
	"s_sptempctrl", //PS_SpargeWaterTemperatureControlAddress    33
	//34
	"s_spsensor", //PS_SpargeWaterSensorIndexAddress 34
	//35
	"s_sptemp", // PS_SpargeWaterTemperatureAddress	35
	//36
	"s_spdiff"  // PS_SpargeWaterTemperatureDifferenceAddress	36
};

extern void printSensorAddress(char *buf, byte *addr);

float BrewManiacWeb::temperature(void)
{
	return bmproxy.currentTemperature();
}

#if MaximumNumberOfSensors > 1

float* BrewManiacWeb::temperatures(void)
{
	return bmproxy.sensorReading();
}
byte BrewManiacWeb::sensorNumber(void)
{
	return bmproxy.sensorNumber();
}
#endif

bool   BrewManiacWeb::isBrewing(void)
{
	return (bmproxy.stage <= 10   // auto mode
		|| bmproxy.stage == 100   // manual mode
		|| bmproxy.stage == 103); // pid auto tune
}

byte    BrewManiacWeb::brewingStage(void)
{
	return bmproxy.stage;
}

byte    BrewManiacWeb::lastBrewEvent(void)
{
	return bmproxy.lastEvent;
}


void BrewManiacWeb::getSettings(String& json)
{
	
    json = "{\"code\":0,\"result\":\"OK\", \"data\":{";

	bool comma=false;
    for(int i=0;i< sizeof(SettingMap)/sizeof(const char*);i++)
    {
    	if(SettingMap[i]){
    		if(!comma){
    			comma=true; // don't append comma before the first object
    		}else{
    			json += ",";
    		}
    		json += "\"" + String(SettingMap[i])  +"\":"+String(bmproxy.getSetting(i));
    	}
    }
    
#if MaximumNumberOfSensors > 1
	char buff[20];
	byte numSensor = bmproxy.sensorNumber();

	for(byte i=0;i<numSensor;i++)
	{
		json += ",\"s_cal_" + String(i+1) + "\":" + bmproxy.sensorCalibrationOfIndex(i);
	}


	json += ",\"sensors\":[";
	
	SensorAddressType *address=bmproxy.sensorAddresses();
	for(byte i=0;i<numSensor;i++)
	{
		printSensorAddress(buff,address[i]);
		if(i > 0) json += ",";
		json += "\"0x" +String(buff) +"\"";
	}
	
	json += "]";
	byte primary[NumberSensorStage],auxiliary[NumberSensorStage];
	
	bmproxy.sensorUsage(primary,auxiliary);

	json += ",\"primary\":[";
	byte p=0;
	for(byte i=0;i<NumberSensorStage;i++) {
		if(i>0) buff[p++]=',';
		buff[p++]= '0' + primary[i];
	}
	buff[p]=0;
	json += String(buff);
	json += "]";

	json += ",\"auxiliary\":[";
	p=0;
	for(byte i=0;i<NumberSensorStage;i++) {
		if(i>0) buff[p++]=',';
		buff[p++]= '0' + auxiliary[i];
	}
	buff[p]=0;
	json += String(buff);

	json += "]";
#endif
    
    json += "}}";

}


void BrewManiacWeb::getAutomation(String& json)
{
	AutomationRecipe* recipe=& bmproxy.automationRecipe;
	
    json = "{\"code\":0,\"result\":\"OK\", \"data\":{\"rest_tm\":[";

	for(int i=0;i<8;i++){
			json += String(recipe->restTime[i]);
			if(i!=7) json += ",";
	}
	json += "], \"rest_tp\":[";

	for(int i=0;i<8;i++){
			json += String(recipe->restTemp[i]);
			if(i!=7) json += ",";
	}
	
	json += "], \"boil\":";
	json += String(recipe->boilTime);
	json += ", \"hops\":[";
	for(int i=0;i<recipe->numberHops;i++){
		json +=String(recipe->hops[i]);
		if(i!= (recipe->numberHops-1)) json += ",";
	}	
	json += "] }}";
}

void BrewManiacWeb::getCurrentStatus(String& json,bool initial)
{

   	json = "{\"state\":";	
	json += String(bmproxy.stage);
	
	if(initial){
		json += ",\"version\":";
		json += String(bmproxy.brewManiacVersion);		
   	}
	json += ",\"btn\":";
	json += String(bmproxy.buttonLabel);

	json += ",\"pump\":";
	json += String(bmproxy.pumpStatus);
	json += ",\"heat\":";
	json += String(bmproxy.heaterStatus);

#if SpargeHeaterSupport == true
	json += ",\"spgw\":";
	json += String(bmproxy.auxHeaterStatus);
#endif

#if MaximumNumberOfSensors > 1
	json += ",\"temps\":[";
	float *ts=bmproxy.sensorReading();
	for(byte i=0;i< bmproxy.sensorNumber();i++)
	{
		if(i>0) json += ",";
		json += String(ts[i]);
	}
	json +="]";
#else
	json += ",\"temp\":";
	json += String(bmproxy.currentTemperature());
#endif
	json += ",\"tr\":";
	json += String((bmproxy.isTempReached())? 1:0);
	json += ",\"pwmon\":";
	json += String((bmproxy.isPwmOn())? 1:0);
	json += ",\"paused\":";
	json += String((bmproxy.isPaused())? 1:0);
	json +=",\"counting\":";
	json += String((bmproxy.isCountingUp())? 1:0);	
	json += ",\"timer\":";
	json += String(bmproxy.runningTimer());	
	json += "}";
}

void BrewManiacWeb::loop(void)
{
	// the loop function will handle retransmission.
	// periodically report status
	unsigned long now=millis();
	if( (now-_lastReportTime) > _reportPeriod){
		_lastReportTime=now;
		_bmProxyEventHandler(BMNotifyStatusChanged);
	}
}

void BrewManiacWeb::getLastEvent(String &json)
{
	json = "{\"event\":" 
		+ String(bmproxy.lastEvent)
		+ "}";
}

void BrewManiacWeb::getSettingPwm(String& json)
{
	json = "{\"pwm\":" 
		+ String(bmproxy.setPwm())
		+ "}";

}

void BrewManiacWeb::getSettingTemperature(String& json)
{
	json = "{\"stemp\":" 
		+ String(bmproxy.setTemperature())
		+ "}";
}
// need to parse JSON object

/*
StaticJsonBuffer<1024> jsonBuffer;
#define JSONBUFFER_SIZE 1024
char _strJsonBuffer[JSONBUFFER_SIZE];
*/

extern void commitSetting(void);

bool BrewManiacWeb::updateSettings(String& json)
{
	uint16_t size=json.length();
	char *strJsonBuffer=(char*) malloc(size +1);
	if(!strJsonBuffer){
		DEBUGF("error alloc mem.\n");
		return false;
	}
	strcpy(strJsonBuffer,json.c_str());

	const int BUFFER_SIZE = JSON_OBJECT_SIZE(40) + JSON_ARRAY_SIZE(3);
	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

	JsonObject& root = jsonBuffer.parseObject(strJsonBuffer);
	if (!root.success()){
		DEBUGF("wrong JSON string\n");
		free(strJsonBuffer);
		return false;
	}
	
	for(int i=0;i< sizeof(SettingMap)/sizeof(const char*);i++){
		if(SettingMap[i] && root.containsKey(SettingMap[i])){
			byte value =  root[SettingMap[i]].as<byte>();
			bmproxy.updateSetting(i,value);

			DEBUGF("update %s %d to %d\n",SettingMap[i],i,value);
		}
	}
#if MaximumNumberOfSensors > 1
	for(int i=1;i<=MaximumNumberOfSensors;i++){
		String cal= "s_cal_" + String(i);
		if(root.containsKey(cal)){
			byte value =  root[cal].as<byte>();
			bmproxy.setSensorCalibrationOfIndex(i-1,value);
		}
	}
#endif
	commitSetting();
	_bmProxyEventHandler(BMNotifySettingChanged);
	free(strJsonBuffer);
	return true;
}

bool BrewManiacWeb::updateAutomation(String& json)
{
	uint16_t size=json.length();
	char *strJsonBuffer=(char*) malloc(size +1);
	if(!strJsonBuffer){
		DEBUGF("error alloc mem.\n");
		return false;
	}
	strcpy(strJsonBuffer,json.c_str());

	DEBUGF("updateAutomation:\"%s\"\n",strJsonBuffer);

	const int BUFFER_SIZE = JSON_OBJECT_SIZE(6) + JSON_ARRAY_SIZE(4);
	StaticJsonBuffer<1024> jsonBuffer;

	JsonObject& root = jsonBuffer.parseObject(strJsonBuffer);
	
	if (!root.success()){
		DEBUGF("wrong JSON string\n");
		free(strJsonBuffer);
		return false;
	}
	
	AutomationRecipe recipe;

	for(byte i=0;i<8;i++){
		recipe.restTime[i]=root["rest_tm"][i];
		recipe.restTemp[i]=root["rest_tp"][i];
	}
	recipe.restTime[0]=1;
	
	recipe.boilTime=root["boil"];
	
	JsonArray& hopArray = root["hops"];

	byte idx=0;
	for(JsonArray::iterator it=hopArray.begin(); it!=hopArray.end(); ++it) 	
	{
    	recipe.hops[idx] = it->as<unsigned char>();
    	idx++;
	}
	recipe.numberHops = idx;
	bmproxy.setAutomationRecipe(&recipe);
	free(strJsonBuffer);
	return true;
}

void BrewManiacWeb::sendButton(byte mask,bool longPressed)
{
	bmproxy.sendButton(mask,longPressed);
}





