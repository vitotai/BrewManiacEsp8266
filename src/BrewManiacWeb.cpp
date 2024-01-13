#include <Arduino.h>
#include <pgmspace.h>
#include <EEPROM.h>
#include <FS.h>
#if ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <ArduinoJson.h>
#include "ps.h"
#include "BrewManiacWeb.h"
#include "automation.h"

String _jsonDisconnected="{\"code\":-1,\"result\":\"BM disc.\"}";

#if SerialDebug == true
#define DEBUGF(...)  DebugPort.printf(__VA_ARGS__)
#else
#define DEBUGF(...)
#endif
extern FS& FileSystem;
extern float gCurrentTemperature;
extern float gSettingTemperature;
extern float gBoilStageTemperature;

extern byte gBoilHeatOutput;
extern bool gIsEnterPwm;
extern bool gIsTemperatureReached;

extern bool uiIsTimerRunning(void);
extern bool uiIsTimerRunningUp(void);
extern int  uiGetDisplayTime(void);
extern char _lcdBuffer[4][21];

extern void virtualButtonPress(byte mask,boolean longPressed);


extern byte readSetting(int addr);

extern void wiSetDeviceAddress(byte ip[],bool apmode);
extern void wiUpdateSetting(int address,byte value);
extern void wiUpdateSettingWord(int address,int16_t value);

#if MaximumNumberOfSensors > 1
extern byte gSensorAddresses[MaximumNumberOfSensors][8];
extern byte gSensorNumber;
extern byte gPrimarySensorIndex;
extern byte gAuxSensorIndex;
extern float gAuxTemperature;
extern byte wiReadCalibrationOfSensor(byte i);
extern void  wiUpdateCalibrationOfSensor(byte i,byte value);

extern float gTemperatureReading[MaximumNumberOfSensors];

extern byte wiUpdatePrimarySensor(byte i,byte v);

extern void wiUpdateAuxSensor(byte i,byte v);
extern byte wiReadPrimarySensor(byte i);
extern byte wiReadAuxSensor(byte i);
//extern byte scanSensors(byte max,byte addresses[][8]);
extern byte wiScanSensors(byte max,byte addresses[][8]);
extern void saveSensor(byte idx,byte address[]);
#endif


/* from BM */
void BrewManiacWeb::holdStatusUpdate(void){
	_holdingStatusUpdate =true;
}

void BrewManiacWeb::unHoldStatusUpdate(bool update){
	_holdingStatusUpdate=false;
	if(update){
		if(_eventHandler) _eventHandler(this,BmwEventStatusUpdate);
	}
}

void BrewManiacWeb::statusChange(void)
{
	if(_holdingStatusUpdate) return;
    if(_eventHandler) _eventHandler(this,BmwEventStatusUpdate);
}

void BrewManiacWeb::setButtonLabel(byte btns)
{
	_buttonLabel=btns;
	// the result is the same as BmwEventStatusUpdate
	if(_holdingStatusUpdate) return;
	if(_eventHandler) _eventHandler(this,BmwEventButtonLabel);
}
void BrewManiacWeb::setBrewStage(uint8_t stage)
{
	_stage=stage;
	if(_holdingStatusUpdate) return;
	if(_eventHandler) _eventHandler(this,BmwEventStatusUpdate);
}

void BrewManiacWeb::brewEvent(uint8_t event)
{
	_lastEvent=event;
	if(_eventHandler) _eventHandler(this,BmwEventBrewEvent);
}

void BrewManiacWeb::updateSettingTemperature(void)
{
	if(_eventHandler) _eventHandler(this,BmwEventSettingTemperatureChanged);
}
void BrewManiacWeb::updatePwm(uint8_t pwm)
{
    _pwm = pwm;
    if(_eventHandler) _eventHandler(this,BmwEventPwmChanged);
}

void BrewManiacWeb::settingChanged(int address,byte value)
{
	if(_eventHandler) _eventHandler(this,BmwEventSettingChanged);
}

void BrewManiacWeb::automationChanged(void)
{
	if(_eventHandler) _eventHandler(this,BmwEventAutomationChanged);
}

/* end of from BM */

#if	MaximumNumberOfSensors > 1
byte BrewManiacWeb::scanSensors(byte max,byte addresses[][8]) {

	return wiScanSensors(max, addresses);
}

#endif


void BrewManiacWeb::setIp(uint8_t ip[],bool apmode){
	wiSetDeviceAddress(ip,apmode);
}

BrewManiacWeb::BrewManiacWeb(void){
	_reportPeriod=DEFAULT_REPORT_PERIOD;
	_lastReportTime=0;

	_eventHandler=NULL;
	_holdingStatusUpdate=false;
}
void BrewManiacWeb::onEvent(BmwEventHandler handler)
{
	_eventHandler=handler;
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
//     13       
	"s_hop",
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
#if SpargeHeaterSupport == true
	// 32
	"s_spenable",  //PS_SpargeWaterEnableAddress    32
	//33
	"s_sptempctrl", //PS_SpargeWaterTemperatureControlAddress    33
	//34
	"s_spsensor", //PS_SpargeWaterSensorIndexAddress 34
	//35
	"s_sptemp", // PS_SpargeWaterTemperatureAddress	35
	//36
	"s_spdiff",  // PS_SpargeWaterTemperatureDifferenceAddress	36
#else
    NULL,NULL,NULL,NULL,NULL,
#endif
	"s_autoresume",  // PS_AutoResume_Enabled 37
	NULL, // 38, PS_Saved_Stage
	"s_pidstrike", // PID_STRIKE
	"s_pom",
	"s_pinvert",
	//42
	"s_btnbuzz",
	//43
	"s_pprime",
	// 44
	"s_ppon",
	// 45
	"s_ppoff",
	// 46-47,
	#if EnableLevelSensor
	//46	
	"s_wlv",
	//47
	"s_wlvtrigger",
	//48
	"s_pmpminrest",
	#else
	NULL,NULL,NULL,
	#endif
	//49
	NULL,
#if SecondaryHeaterSupport == true
	//50,
	"s_preheat",
	//51
	"s_mashheat",
	//52
	"s_boilheat",
	// 53
	"s_pbheat",
	// 54
	NULL,
	//55,
	"s_kp2",
	//56
	"s_ki2",
	//57
	"s_kd2",
	// 58
	"s_kpall",
	// 59
	"s_kiall",
	// 60
	"s_kdall",
#endif
	NULL
};

#define EnableTwoPointCalibrationKey "s_2ptc"
#define CalibrationReadingP1Key "s_pt1"
#define CalibrationReferenceP1Key "s_pr1"
#define CalibrationReadingP2Key "s_pt2"
#define CalibrationReferenceP2Key "s_pr2"

typedef struct _SettingMapEntry{
const char* key;
uint8_t   address;
uint8_t   size;
}SettingMapEntry;

#if MaximumNumberOfSensors > 1
static const SettingMapEntry settingMap2[]={
{EnableTwoPointCalibrationKey,PS_EnableTwoPointCalibration,1},
{"s_pt1_1",PS_CalibrationReadingP1Of(0),2},
{"s_pr1_1",PS_CalibrationReferenceP1Of(0),2},
{"s_pt2_1",PS_CalibrationReadingP2Of(0),2},
{"s_pr2_1",PS_CalibrationReferenceP2Of(0),2},

{"s_pt1_2",PS_CalibrationReadingP1Of(1),2},
{"s_pr1_2",PS_CalibrationReferenceP1Of(1),2},
{"s_pt2_2",PS_CalibrationReadingP2Of(1),2},
{"s_pr2_2",PS_CalibrationReferenceP2Of(1),2},

{"s_pt1_3",PS_CalibrationReadingP1Of(2),2},
{"s_pr1_3",PS_CalibrationReferenceP1Of(2),2},
{"s_pt2_3",PS_CalibrationReadingP2Of(2),2},
{"s_pr2_3",PS_CalibrationReferenceP2Of(2),2},

{"s_pt1_4",PS_CalibrationReadingP1Of(3),2},
{"s_pr1_4",PS_CalibrationReferenceP1Of(3),2},
{"s_pt2_4",PS_CalibrationReadingP2Of(3),2},
{"s_pr2_4",PS_CalibrationReferenceP2Of(3),2},

{"s_pt1_5",PS_CalibrationReadingP1Of(4),2},
{"s_pr1_5",PS_CalibrationReferenceP1Of(4),2},
{"s_pt2_5",PS_CalibrationReadingP2Of(4),2},
{"s_pr2_5",PS_CalibrationReferenceP2Of(4),2}
};
#else
static const SettingMapEntry settingMap2[]={
{EnableTwoPointCalibrationKey,PS_EnableTwoPointCalibration,1},
{CalibrationReadingP1Key,PS_CalibrationReadingP1,2},
{CalibrationReferenceP1Key,PS_CalibrationReferenceP1,2},
{CalibrationReadingP2Key,PS_CalibrationReadingP2,2},
{CalibrationReferenceP2Key,PS_CalibrationReferenceP2,2}
};
#endif
extern void printSensorAddress(char *buf, byte *addr);

float BrewManiacWeb::temperature(void)
{
	return gCurrentTemperature;
}

#if MaximumNumberOfSensors > 1

float* BrewManiacWeb::temperatures(void)
{
	return gTemperatureReading;
}
byte BrewManiacWeb::sensorNumber(void)
{
	return gSensorNumber;
}
#endif

bool   BrewManiacWeb::isBrewing(void)
{
	return (_stage <= 10   // auto mode
		|| _stage == 100   // manual mode
		|| _stage == 103); // pid auto tune
}

byte    BrewManiacWeb::brewingStage(void)
{
	return _stage;
}

byte    BrewManiacWeb::lastBrewEvent(void)
{
	return _lastEvent;
}


void BrewManiacWeb::getSettings(String& json)
{

    //json = "{\"code\":0,\"result\":\"OK\", \"data\":{";
    json = "{";
	bool comma=false;
    for(int i=0;i<(int)( sizeof(SettingMap)/sizeof(const char*));i++)
    {
    	if(SettingMap[i]){
    		if(!comma){
    			comma=true; // don't append comma before the first object
    		}else{
    			json += ",";
    		}
    		json += "\"" + String(SettingMap[i])  +"\":"+String(readSetting(i));
    	}
    }

    for(int i=0;i<(int)( sizeof(settingMap2)/sizeof(SettingMapEntry));i++)
    {
			int value=0;
			if(settingMap2[i].size ==1){
				value=readSetting(settingMap2[i].address);
			}else if(settingMap2[i].size ==2){
				value=readSettingWord(settingMap2[i].address);
			}
    		json += ",\"" + String(settingMap2[i].key)  +"\":"+String(value);
    }


#if MaximumNumberOfSensors > 1
	char buff[20];
	byte numSensor = gSensorNumber;

	for(byte i=0;i<numSensor;i++)
	{
		json += ",\"s_cal_" + String(i+1) + "\":" + wiReadCalibrationOfSensor(i);
	}


	json += ",\"sensors\":[";

	SensorAddressType *address=gSensorAddresses;
	for(byte i=0;i<numSensor;i++)
	{
		printSensorAddress(buff,address[i]);
		if(i > 0) json += ",";
		json += "\"0x" +String(buff) +"\"";
	}

	json += "]";
	byte primary[NumberSensorStage],auxiliary[NumberSensorStage];

	for(byte i=0;i<NumberSensorStage;i++)
	{
		byte p=wiReadPrimarySensor(i);
		byte a=wiReadAuxSensor(i);;
		primary[i]=(p>=gSensorNumber)? 0:p;
		auxiliary[i]=(a>=gSensorNumber)? 0:a;
	}

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

    json += "}";
}

void BrewManiacWeb::getAutomation(String& json)
{
    automation.json(json);
}
#define HEXCode(a) (((a) < 10)? ('0'+(a)):('A'-10+(a)))

void BrewManiacWeb::getCurrentStatus(String& json,bool initial)
{
	#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(1024);
	#else
	const int BUFFER_SIZE = JSON_OBJECT_SIZE(17+3) + JSON_ARRAY_SIZE(5+4+3);
	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
   	JsonObject& root = jsonBuffer.createObject();
	#endif

	root["state"]=_stage;
	root["btn"]=_buttonLabel;
	root["pump"]=_pumpStatus;
	root["heat"]=_heaterStatus;

#if SecondaryHeaterSupport == true
	root["heat2"]=_secondaryHeaterStatus;
#endif

#if SpargeHeaterSupport == true
	root["spgw"]=_auxHeaterStatus;
#endif

#if MaximumNumberOfSensors > 1
	#if ARDUINOJSON_VERSION_MAJOR == 6
	JsonArray temps = root.createNestedArray("temps");
	#else
	JsonArray& temps = root.createNestedArray("temps");
	#endif
	float *ts=gTemperatureReading;
	for(byte i=0;i< gSensorNumber;i++)
	{
		temps.add(ts[i]);
	}
#else
	root["temp"]=gCurrentTemperature;
#endif
	root["tr"]=gIsTemperatureReached? 1:0;
	root["pwmon"]=gIsEnterPwm? 1:0;

	if(gIsEnterPwm){
		root["pwm"]= _pwm;	
	}
	root["paused"]=(!uiIsTimerRunning())? 1:0;
	root["counting"]=(uiIsTimerRunningUp())? 1:0;
	root["timer"]=uiGetDisplayTime();
	root["stemp"]=gSettingTemperature;
	root["rssi"]=WiFi.RSSI();
// lcd.
	#if ARDUINOJSON_VERSION_MAJOR == 6
	JsonArray lcds = root.createNestedArray("lcd");
	#else
	JsonArray& lcds = root.createNestedArray("lcd");
	#endif
	char lcdcol[4][41];
	int idx=0;
	for(int i=0;i<4;i++){
		idx=0;
		for(int j=0;j<20;j++){
			uint8_t ch=_lcdBuffer[i][j];
			lcdcol[i][idx++]=HEXCode(ch >> 4);
			lcdcol[i][idx++]=HEXCode(ch&0xF);
		}
		lcdcol[i][40]='\0';
		lcds.add(lcdcol[i]);
	}
	#if ARDUINOJSON_VERSION_MAJOR == 6
	serializeJson(root,json);
	#else
	root.printTo(json);
	#endif
}

void BrewManiacWeb::loop(void)
{
	// the loop function will handle retransmission.
	// periodically report status
	unsigned long now=millis();
	if( (now-_lastReportTime) > _reportPeriod){
		_lastReportTime=now;
		statusChange();
	}
}

void BrewManiacWeb::getLastEvent(String &json)
{
	json = "{\"event\":"
		+ String(_lastEvent)
		+ "}";
}

void BrewManiacWeb::getSettingPwm(String& json)
{
	json = "{\"pwm\":"
		+ String(_pwm)
		+ "}";

}

void BrewManiacWeb::getSettingTemperature(String& json)
{
	json = "{\"stemp\":"
		+ String(gSettingTemperature)
		+ "}";
}

extern void commitSetting(void);

byte hex2Int(char hex){
    if(hex >= 'A' && hex <='F'){
        return 10 + hex - 'A';
    }else if(hex >= 'a' && hex <='f'){
        return 10 + hex - 'a';
    }else{
        return hex - '0';
    }
}

bool BrewManiacWeb::updateSettings(String& json)
{
	#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(2048); // ArduinoJson assistant suggest 14xx at least.
	auto error=deserializeJson(root,json);
	if(error)
	#else
	uint16_t size=json.length();
	char *strJsonBuffer=(char*) malloc(size +1);
	if(!strJsonBuffer){
		DEBUGF("error alloc mem.\n");
		return false;
	}
	strcpy(strJsonBuffer,json.c_str());

	const int BUFFER_SIZE = JSON_ARRAY_SIZE(3+2) + 2*JSON_ARRAY_SIZE(6+2) + JSON_OBJECT_SIZE(35+6);
	StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

	JsonObject& root = jsonBuffer.parseObject(strJsonBuffer);
	if (!root.success())
	#endif
	{
		DEBUGF("wrong JSON string\n");
		#if ARDUINOJSON_VERSION_MAJOR == 5
		free(strJsonBuffer);
		#endif

		return false;
	}

	for(int i=0;i<(int) (sizeof(SettingMap)/sizeof(const char*));i++){
		if(SettingMap[i] && root.containsKey(SettingMap[i])){
			byte value =  root[SettingMap[i]].as<byte>();
			wiUpdateSetting(i,value);

			DEBUGF("update %s %d to %d\n",SettingMap[i],i,value);
		}
	}

	for(int m=0;m<sizeof(settingMap2)/sizeof(SettingMapEntry);m++){
		if(root.containsKey(settingMap2[m].key)){
			if(settingMap2[m].size ==1){
				byte value =  root[settingMap2[m].key].as<byte>();
				wiUpdateSetting(settingMap2[m].address,value);
			}else if(settingMap2[m].size ==2){
				int value =  root[settingMap2[m].key].as<int>();
				wiUpdateSettingWord(settingMap2[m].address,(int16_t)value);
			}
		}
	}
	
#if MaximumNumberOfSensors > 1
	for(int i=1;i<=MaximumNumberOfSensors;i++){
		String cal= "s_cal_" + String(i);
		if(root.containsKey(cal)){
			byte value =  root[cal].as<byte>();
			wiUpdateCalibrationOfSensor(i-1,value);
		}
	}
	// sensor setup
	//"sensors":["0x0011223344556687","0x2211223344556687","0x3311223344556687"],
	//"primary":[0,1,1,1,1,1],"auxiliary":[1,0,0,0,2,2]
	if(root.containsKey("sensors")){
	
		#if ARDUINOJSON_VERSION_MAJOR == 6
    	JsonArray sensors = root["sensors"];
		#else
		JsonArray& sensors = root["sensors"];
		#endif

    	int idx=0;
    	byte *address;
    	for(JsonArray::iterator it=sensors.begin(); it!=sensors.end(); ++it)
	    {
	        address = gSensorAddresses[idx];

    	    const char* addressString= it->as<const char*>();
    	    // convert from stiring

    	    for(int i=0;i<8;i++){
                char hh= addressString[2 + i *2];
                char ll= addressString[2 + i *2 + 1];

    	        address[i] = (hex2Int(hh)<<4) + hex2Int(ll);
    	    }
    	    saveSensor(idx,address);

    	    DEBUGF("update sensor %d -",idx);
    	    #if SerialDebug
    	    char buff[20];
    	    printSensorAddress(buff,address);
    	    DEBUGF("%s\n",buff);
    	    #endif

    	    idx++;
	    }
	    gSensorNumber = idx;
	    // clear the next address
	    if(gSensorNumber < MaximumNumberOfSensors){
    	    address=gSensorAddresses[idx];
    	    address[0]=0xFF;
	        saveSensor(idx,address);
	    }
	}

	if(root.containsKey("primary")){
	    // primary
	    int idx=0;
		#if ARDUINOJSON_VERSION_MAJOR == 6
	    JsonArray primary = root["primary"];
		#else
	    JsonArray& primary = root["primary"];
        #endif
		for(JsonArray::iterator it=primary.begin(); it!=primary.end(); ++it)
	    {
    	    uint8_t sensor= it->as<unsigned char>();
    	    wiUpdatePrimarySensor(idx,sensor);
    	    DEBUGF("Primary sensor %d - %d\n",idx,sensor);
			idx++;
    	}
	}
	if(root.containsKey("auxiliary")){

	    // auxiliary
	    int idx=0;
		#if ARDUINOJSON_VERSION_MAJOR == 6
	    JsonArray auxliary = root["auxiliary"];
		#else
	    JsonArray& auxliary = root["auxiliary"];
		#endif
        for(JsonArray::iterator it=auxliary.begin(); it!=auxliary.end(); ++it)
	    {
    	    uint8_t sensor= it->as<unsigned char>();
    	    wiUpdateAuxSensor(idx,sensor);
    	    DEBUGF("Auxiliary sensor %d - %d\n",idx,sensor);
			idx++;
    	}
	}
#endif
	commitSetting();
	if(_eventHandler) _eventHandler(this,BmwEventSettingChanged);

	#if ARDUINOJSON_VERSION_MAJOR == 5
	free(strJsonBuffer);
	#endif

	return true;
}

bool BrewManiacWeb::updateAutomation(String& json)
{
    File f=FileSystem.open(AUTOMATION_FILE,"w+");
	if(f){
	    /*size_t len=*/f.print(json.c_str());
		f.close();
	}else{
	    return false;
    }

    // reload automation
    automation.load();
    if(_eventHandler) _eventHandler(this,BmwEventAutomationChanged);

    return true;
}

void BrewManiacWeb::sendButton(byte mask,bool longPressed)
{
	virtualButtonPress(mask,longPressed);
}
