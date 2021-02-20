#include <FS.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"
#include "automation.h"

#define BUFFER_SIZE 512

extern FS& FileSystem;

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

    File f=FileSystem.open(AUTOMATION_FILE,"r");

	if(!f){
	    // no file exists. use default file
	    DBGPRINTF("No automation file!\n");
        strcpy_P(strJsonBuffer,DefaultAutomation);
        f=FileSystem.open(AUTOMATION_FILE,"w+");
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

#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(1024);
	auto jsonerror=deserializeJson(root,strJsonBuffer);
	if(jsonerror)

#else
	StaticJsonBuffer<1024> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(strJsonBuffer);

	if (!root.success())
#endif

	{
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

#if ARDUINOJSON_VERSION_MAJOR == 6

	JsonArray hopArray = root["hops"];
#else

	JsonArray& hopArray = root["hops"];
#endif

	byte idx=0;
	for(JsonArray::iterator it=hopArray.begin(); it!=hopArray.end(); ++it)
	{
    	_hopTimes[idx] = it->as<unsigned char>();
    	idx++;
	}
	_numberOfHops = idx;

	if(root.containsKey("hs")){

#if ARDUINOJSON_VERSION_MAJOR == 6
    	JsonArray hsArray = root["hs"];
#else
    	JsonArray& hsArray = root["hs"];
#endif
	    byte hsidx=0;
	    for(JsonArray::iterator it=hsArray.begin(); it!=hsArray.end(); ++it)
	    {
#if ARDUINOJSON_VERSION_MAJOR == 6
	        JsonObject hs=it->as<JsonObject>();
#else
	        JsonObject& hs=it->as<JsonObject>();
#endif
	        _hopStandSessions[hsidx].startTemperature=hs["s"];
	        _hopStandSessions[hsidx].keepTemperature=hs["k"];

#if ARDUINOJSON_VERSION_MAJOR == 6
        	JsonArray hsHopArray = hs["h"];
#else
        	JsonArray& hsHopArray = hs["h"];
#endif
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
	#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(1024);
	#else
    StaticJsonBuffer<1024> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
	#endif

    root["boil"] =_boilTime;

	#if ARDUINOJSON_VERSION_MAJOR == 6
    JsonArray rest_tp = root.createNestedArray("rest_tp");
	#else
    JsonArray& rest_tp = root.createNestedArray("rest_tp");
	#endif
	for(byte i=0;i<MAXIMUM_STAGE_NUMBER;i++){
		rest_tp.add(_stageTemperatures[i]);
	}

	#if ARDUINOJSON_VERSION_MAJOR == 6
    JsonArray rest_tm = root.createNestedArray("rest_tm");
	#else
    JsonArray& rest_tm = root.createNestedArray("rest_tm");
	#endif

	for(byte i=0;i<MAXIMUM_STAGE_NUMBER;i++){
		rest_tm.add(_stageTimes[i]);
	}
	#if ARDUINOJSON_VERSION_MAJOR == 6

    JsonArray hops = root.createNestedArray("hops");
	#else
	JsonArray& hops = root.createNestedArray("hops");
	#endif
	for(byte i=0;i<_numberOfHops;i++){
		hops.add(_hopTimes[i]);
	}

    if(_numberOfHopStandSession >0){
		#if ARDUINOJSON_VERSION_MAJOR == 6

        JsonArray hs = root.createNestedArray("hs");
		#else
        JsonArray& hs = root.createNestedArray("hs");
		#endif

	    for(byte i=0;i<_numberOfHopStandSession;i++){
			#if ARDUINOJSON_VERSION_MAJOR == 6

    	    JsonObject session = hs.createNestedObject();
			#else
    	    JsonObject& session = hs.createNestedObject();
			#endif

			session["s"]=_hopStandSessions[i].startTemperature;
    	    session["k"]=_hopStandSessions[i].keepTemperature;
		
			#if ARDUINOJSON_VERSION_MAJOR == 6
    	    JsonArray pbh = session.createNestedArray("h");
			#else
    	    JsonArray& pbh = session.createNestedArray("h");
			#endif

    	    for(int j=0;j< _hopStandSessions[i].numberHops; j++){
    	        pbh.add(_hopStandSessions[i].hopTime[j]);
    	    }
	    }
    }
	#if ARDUINOJSON_VERSION_MAJOR == 6
	return serializeJson(root,buffer,size);
	#else
    return root.printTo(buffer,size);
	#endif

}

void CAutomation::save(void)
{
    char buffer[BUFFER_SIZE];
    size_t length=formatJson(buffer,BUFFER_SIZE);

    File f=FileSystem.open(AUTOMATION_FILE,"w+");

    if(f){
        f.write((const uint8_t*)buffer,length);
        f.close();
    }
}

void CAutomation::json(String& output)
{
    char buffer[BUFFER_SIZE];
    size_t length=formatJson(buffer,BUFFER_SIZE);
    buffer[length]='\0';
    output=String(buffer);
}


uint8_t  CAutomation::postBoilHopIndex(uint8_t hs,uint8_t hopIdx)
{
    uint8_t index=hopIdx+1;
    for(int i=0;i< hs;i++)
        index += _hopStandSessions[i].numberHops;

    return index;
}
