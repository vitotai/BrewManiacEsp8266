#include <FS.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "automation.h"

#define BUFFER_SIZE 512


#if SerialDebug == true
#define DebugOut(a) DebugPort.print(a)
#define DBGPRINTF(...) DebugPort.printf(__VA_ARGS__)
#else
#define DebugOut(a)
#define DBGPRINTF(...)
#endif


const char DefaultAutomation[] PROGMEM =
R"END(
{"rest_tp":[55,65,0,0,0,0,0,76],
"rest_tm":[1,60,0,0,0,0,0,10],
"boil":60,
"hops":[60]
}
)END";

CAutomation automation;

bool CAutomation::load(void)
{
    char strJsonBuffer[BUFFER_SIZE];

    File f=SPIFFS.open(AUTOMATION_FILE,"r");

	if(!f){
	    // no file exists. use default file
	    DBGPRINTF("No automation file!\n");
        strcpy_P(strJsonBuffer,DefaultAutomation);
        f=SPIFFS.open(AUTOMATION_FILE,"w+");
        if(f){
            f.print(strJsonBuffer);
            f.close();
        }
	}else{
	    size_t len=f.readBytes(strJsonBuffer,BUFFER_SIZE-1);
	    strJsonBuffer[len]='\0';
	    f.close();
    }

	DBGPRINTF("load:\"%s\"\n",strJsonBuffer);

	StaticJsonBuffer<1024> jsonBuffer;

	JsonObject& root = jsonBuffer.parseObject(strJsonBuffer);

	if (!root.success()){
		DBGPRINTF("wrong JSON string\n");
		return false;
	}

	for(byte i=0;i<MAXIMUM_STAGE_NUMBER;i++){
		_stageTimes[i]=root["rest_tm"][i];
		_stageTemperatures[i]=root["rest_tp"][i];
	}
	_stageTimes[0]=1;

    byte i=2;
    for(;i<(MAXIMUM_STAGE_NUMBER -1);i++){
        if(_stageTimes[i] ==0) break;
	}
    for(;i<(MAXIMUM_STAGE_NUMBER -1);i++){
        _stageTimes[i]=0;
	}

	_boilTime=root["boil"];

	JsonArray& hopArray = root["hops"];

	byte idx=0;
	for(JsonArray::iterator it=hopArray.begin(); it!=hopArray.end(); ++it)
	{
    	_hopTimes[idx] = it->as<unsigned char>();
    	idx++;
	}
	_numberOfHops = idx;

	if(root.containsKey("hs")){

    	JsonArray& hsArray = root["hs"];

	    byte hsidx=0;
	    for(JsonArray::iterator it=hsArray.begin(); it!=hsArray.end(); ++it)
	    {
	        JsonObject& hs=it->as<JsonObject>();
	        _hopStandSessions[hsidx].startTemperature=hs["s"];
	        _hopStandSessions[hsidx].keepTemperature=hs["k"];

        	JsonArray& hsHopArray = hs["h"];

	        byte hidx=0;
	        for(JsonArray::iterator hit=hsHopArray.begin(); hit!=hsHopArray.end(); ++hit)
	        {
    	        _hopStandSessions[hsidx].hopTime[hidx] = hit->as<unsigned char>();
    	        hidx++;
	        }
            _hopStandSessions[hsidx].numberHops=hidx;
	        DBGPRINTF("HS session#%d hops:%d\n",hsidx,hidx);
    	    hsidx++;
	    }
    	_numberOfHopStandSession=hsidx;
    	DBGPRINTF("HS session:%d\n",_numberOfHopStandSession);

	}else{
	    _numberOfHopStandSession=0;
	    DBGPRINTF("NO HS session\n");
	}

	return true;
}

size_t CAutomation::formatJson(char* buffer,size_t size)
{
    StaticJsonBuffer<1024> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["boil"] =_boilTime;

    JsonArray& rest_tp = root.createNestedArray("rest_tp");
	for(byte i=0;i<MAXIMUM_STAGE_NUMBER;i++){
		rest_tp.add(_stageTemperatures[i]);
	}

    JsonArray& rest_tm = root.createNestedArray("rest_tm");
	for(byte i=0;i<MAXIMUM_STAGE_NUMBER;i++){
		rest_tm.add(_stageTimes[i]);
	}

    JsonArray& hops = root.createNestedArray("hops");

	for(byte i=0;i<_numberOfHops;i++){
		hops.add(_hopTimes[i]);
	}

    if(_numberOfHopStandSession >0){
         JsonArray& hs = root.createNestedArray("hs");

	    for(byte i=0;i<_numberOfHopStandSession;i++){
    	    JsonObject& session = hs.createNestedObject();
    	    session["s"]=_hopStandSessions[i].startTemperature;
    	    session["k"]=_hopStandSessions[i].keepTemperature;
    	    JsonArray& pbh = session.createNestedArray("h");
    	    for(int j=0;j< _hopStandSessions[i].numberHops; j++){
    	        pbh.add(_hopStandSessions[i].hopTime[j]);
    	    }
	    }
    }

    return root.printTo(buffer,size);
}

void CAutomation::save(void)
{
    char buffer[BUFFER_SIZE];
    size_t length=formatJson(buffer,BUFFER_SIZE);

    File f=SPIFFS.open(AUTOMATION_FILE,"w+");

    if(f){
        f.write((const uint8_t*)buffer,length);
        f.close();
    }
}

String CAutomation::json(void)
{
    char buffer[BUFFER_SIZE];
    size_t length=formatJson(buffer,BUFFER_SIZE);
    buffer[length]='\0';
    return String(buffer);
}


uint8_t  CAutomation::postBoilHopIndex(uint8_t hs,uint8_t hopIdx)
{
    uint8_t index=hopIdx+1;
    for(int i=0;i< hs;i++)
        index += _hopStandSessions[i].numberHops;

    return index;
}
