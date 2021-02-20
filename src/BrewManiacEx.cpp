#include <Arduino.h>
#include <pgmspace.h>

#if ESP32
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#endif

#include <ArduinoOTA.h>
#include <FS.h>
#if ESP32
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <rom/spi_flash.h>
#else
#include <Hash.h>
#include <ESPAsyncTCP.h>
#endif

#include <ESPAsyncWebServer.h>
#if UseSoftwareSerial == true
#include <SoftwareSerial.h>
#endif
#include <Wire.h>
#include <EEPROM.h>
#include <ArduinoJson.h>

//#include <GDBStub.h>
#include "config.h"

#include "WiFiSetup.h"
#include "HttpUpdateHandler.h"
#include "BrewManiacWeb.h"
#include "ESPUpdateServer.h"

#include "TimeKeeper.h"
#if UseLittleFS
#include <LittleFS.h>
#endif

#if SerialDebug == true
#define DebugOut(a) DebugPort.print(a)
#define DBG_PRINTF(...) DebugPort.printf(__VA_ARGS__)
#else
#define DebugOut(a)
#define DBG_PRINTF(...)
#endif

#if EnableBrewLog
#include "BrewLogger.h"
#endif

extern bool brewmaniac_setup();
//extern void brewmaniac_ApPrompt(void);
extern void brewmaniac_loop();
//extern bool readSkipNetCfgButton(void);
extern void startBrewManiac(void);

extern const String getContentType(const String& filename);
#define ResponseAppleCNA true

/**************************************************************************************/
/**************************************************************************************/

#define WS_PATH 	"/ws"
#define SSE_PATH 	"/status.php"
#define LOGS_PATH 	"/logs.php"

#define CHART_DATA_PATH 	"/chart.php"
#define SETTING_PATH 		"/settings.php"
#define AUTOMATION_PATH 	"/automation.php"
#define BUTTON_PATH 		"/button.php"
#define UPDATE_AUTOMATION_PATH 	"/saveauto.php"
#define UPDATE_SETTING_PATH 	"/savesettings.php"

#define NETCFG_PATH 		"/netcfg.php"
#define DEFAULT_INDEX_FILE  "bm.htm"

#define MAX_CONFIG_LEN 1024
#define JSON_BUFFER_SIZE 1024
#define CONFIG_FILENAME 	"/network.cfg"

#define LS_PATH				"/list.php"
#define UPLOAD_PATH 		"/upfile.php"
#define RM_PATH 			"/rm.php"
#define RECIPE_PATH_Base 	"/R/"
#define RECIPE_PREFERNECE 	"/userpref.cfg"

#define SETTIME_PATH 	"/settime"
#define AUDIO_PATH 	"/audio"

#define AUDIO_FILE 	"/sounds.json"

#define WIFI_SCAN_PATH "/wifiscan"
#define WIFI_CONNECT_PATH "/wificon"
#define WIFI_DISC_PATH "/wifidisc"

#define RPC_TAG "rpc"
#define RPCID_TAG "id"

#define SCAN_SENSOR_CMD 	"scansensor"

#define MaxNameLength 32

char _gHostname[MaxNameLength];
char _gUsername[MaxNameLength];
char _gPassword[MaxNameLength];
bool _gSecuredAccess;

#if UseLittleFS
FS& FileSystem = LittleFS;
#else
FS& FileSystem =SPIFFS;
#endif

#if UseSoftwareSerial == true
SoftwareSerial wiSerial(SW_RX_PIN,SW_TX_PIN);
#else
#define wiSerial Serial
#endif

AsyncWebServer server(80);

BrewManiacWeb bmWeb;

#if EnableBrewLog
BrewLogger brewLogger;
#endif

typedef union _address{
                uint8_t bytes[4];  // IPv4 address
                uint32_t dword;
} IPV4Address;


extern const uint8_t* getEmbeddedFile(const char* filename,bool &gzip, unsigned int &size);

/* in ESP32/Arduino, 0.0.0.0 is INADDR_NONE
	for ESP8266,     0.0.0.0 is INADDR_ANY
	                255.255.255.255 is INADDR_NONE	                   
    there is NO isSet() for ESP32 framework.
    ESP8266 output "IP unset" while ESP32 outputs directly what it has.
*/

#if ESP32
#define IPAddress_String(ip) ip.toString()
#else
#define IPAddress_String(ip) ip.isSet()? ip.toString():String("0.0.0.0")
#endif


/**************************************************************************************/
/* common response.  */
/**************************************************************************************/
// it's kind of ugly, but might reduce the code size and simpler to modify
void requestSend(AsyncWebServerRequest *request,int code,const String& type="application/json",const String& content="{}"){
#if EnableCORS

	AsyncWebServerResponse *response;
	
	if(code == 200){
		response= request->beginResponse(200,type, content);
	}else{
		response= request->beginResponse(code);
	}
//	DBG_PRINTF("Access-Control-Allow-Origin\n");
	response->addHeader("Access-Control-Allow-Origin","*");
	request->send(response);

#else	
	if(code == 200){
		request->send(200,type, content);
	}else{
		request->send(code);
	}
#endif
}

void requestSend(AsyncWebServerRequest *request,AsyncWebServerResponse* response){
	#if EnableCORS
	response->addHeader("Access-Control-Allow-Origin","*");
	#endif
	request->send(response);
}

void requestSend(AsyncWebServerRequest *request,FS &fs, const String& path, const String& contentType=String() ){
	#if EnableCORS
	DBG_PRINTF("Send file %s.\n",path.c_str());

	AsyncWebServerResponse *response=request->beginResponse(fs,path,contentType);
	response->addHeader("Access-Control-Allow-Origin","*");
	request->send(response);

	#else
	request->send(fs,path,contentType);
	#endif
}

/**************************************************************************************/
/* Recipe Fle management interface */
/**************************************************************************************/
#define WRITE_MASK 0x2
#define READ_MASK 0x1
#define EXECUTE_MASK 0x4

class RecipeFileHandler:public AsyncWebHandler
{
	uint32_t _startTime;
	FILE  _tempFile;

	bool accessAllow(String& path,uint8_t mask){
		if(!path.startsWith("/")) path= String("/") + path;

		if(mask & WRITE_MASK) {
			if (path.startsWith(RECIPE_PATH_Base)) return true;
			if (path == RECIPE_PREFERNECE) return true;
			return false;
		}
		return true;
	}

	void listDirectory(const String& path,String& json){
		#if ESP32
		File dir = FileSystem.open(path);
		#else
		Dir dir = FileSystem.openDir(path);
		#endif

		json=String("[");
		bool comma=false;
		#if !UseLittleFS
		uint16_t len=path.length();
		#endif

		#if defined(ESP32)
  		File entry = dir.openNextFile();

  		while(entry){
			  String file=entry.name();
		#else

		while (dir.next()) {
			String file=dir.fileName();
		#endif
    		DBG_PRINTF("LS File:%s\n",file.c_str());
    		if(comma) json = json + String(",");
    		else comma=true;
			#if UseLittleFS
    		json += String("\"") + file + String("\"");
			#else
    		json += String("\"") + file.substring(len) + String("\"");
			#endif

			#if defined(ESP32)
    		entry = dir.openNextFile();
			#endif

		}
		json += String("]");
	}
public:
	RecipeFileHandler(){}

	void handleRequest(AsyncWebServerRequest *request){
		if(request->url() == RM_PATH){
			if( request->hasParam("file",true)){
				String file=request->getParam("file", true)->value();
				DBG_PRINTF("RM request:%s\n",file.c_str());

				if(accessAllow(file,WRITE_MASK)){
					DBG_PRINTF("RM executed:%s\n",file.c_str());
					FileSystem.remove(file.c_str());
					requestSend(request,200);
				}else{
					DBG_PRINTF("RM not allowed:%s\n",file.c_str());
					requestSend(request,400);
				}
			}else{
				DBG_PRINTF("miss file in req.");
				requestSend(request,400);
			}
		}else if(request->url() == UPLOAD_PATH){
			if( request->hasParam("file",true,true)){
				String file=request->getParam("file", true, true)->value();
				if(accessAllow(file,WRITE_MASK)){
					if( FileSystem.exists(file)){
						DBG_PRINTF("File UL success:%s\n",file.c_str());
						requestSend(request,200);
					}else{
						DBG_PRINTF("File UL Failed:%s\n",file.c_str());
						requestSend(request,500);
					}
				}else{
					requestSend(request,401);
					DBG_PRINTF("File UL not allowed:%s\n",file.c_str());
				}
			}else{
				DBG_PRINTF("miss file in req.");
				requestSend(request,404);
			}
		}else if(request->url() == LS_PATH){
			if(request->hasParam("dir",true)){
				String file=request->getParam("dir", true)->value();
				DBG_PRINTF("LS request:%s\n",file.c_str());
				if(accessAllow(file,EXECUTE_MASK | READ_MASK)){
					String list;
					listDirectory(file,list);
					requestSend(request,200, "application/json",list);
				}
				else
					requestSend(request,401);
			}else
				requestSend(request,400);

		}else{
			// just return the file WITHOUT CACHE!
			DBG_PRINTF("requestin recipe file:%s",request->url().c_str());
			requestSend(request,FileSystem,request->url());
		}
	}
    virtual void handleUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
	{
		if(!index){
			String file=filename;
			DBG_PRINTF("upload: %s\n", filename.c_str());
			if(accessAllow(file,WRITE_MASK)){
	        	request->_tempFile = FileSystem.open(file, "w");
    	    	_startTime = millis();
      		}
      	}
      	if(request->_tempFile){
        	if(len){
          	request->_tempFile.write(data,len);
        	}
        	if(final){
          		request->_tempFile.close();
				#if SerialDebug
          		uint32_t uploadTime = millis() - _startTime;
          		DBG_PRINTF("upload: %s, %u B, %u ms\n", filename.c_str(), index+len, uploadTime);
				#endif
       		}
      	}
    }

	bool canHandle(AsyncWebServerRequest *request){
	 	if(request->url() == RM_PATH 
		 || request->url() ==UPLOAD_PATH 
		 || request->url() ==LS_PATH){
			return true;
		}else if(request->url() == RECIPE_PREFERNECE){
			  return FileSystem.exists(request->url());
	 	}else if(request->url().startsWith(RECIPE_PATH_Base)){
		 	return FileSystem.exists(request->url());
		}
	 	return false;
	}
	virtual bool isRequestHandlerTrivial() override final {return false;}

};

RecipeFileHandler recipeFileHandler;
/**************************************************************************************/
/* Temperature logging interface */
/**************************************************************************************/


class TemperatureLogHandler:public AsyncWebHandler
{
public:

	void handleRequest(AsyncWebServerRequest *request){
		if( request->url() == LOGS_PATH){
			if(request->hasParam("dl")){
				int index=request->getParam("dl")->value().toInt();
				//DBG_PRINTF("Get log index:%d\n",index);
				char buf[40];
				brewLogger.createFilename(buf,index);
				if(FileSystem.exists(buf)){
					requestSend(request,FileSystem,buf,"application/octet-stream");
				}else{
					requestSend(request,404);
				}
			}else{
				// list
				FileInfo* list=brewLogger.getLogFileInfo();
				String json=String("[");
				bool comma=false;
				for(int i=0;i<MAX_FILE_NUMBER;i++){
					if(list[i].index >=0){
						if(comma) json +=",";
						else comma=true;

						json += "{\"f\":" +String(list[i].index) +",\"t\":"
							+String(list[i].time) +"}";
					}else{
						break;
					}
				}
				json += "]";
				requestSend(request,200, "application/json;",json);
			}
			return;
		}
		int offset;
		if(request->hasParam("offset")){
			offset=request->getParam("offset")->value().toInt();
			//DBG_PRINTF("offset= %d\n",offset);
		}else{
			offset=0;
		}
		size_t size=brewLogger.beginCopyAfter(offset);
		if(size >0){
			AsyncWebServerResponse *response=request->beginResponse("application/octet-stream", size, [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
				return brewLogger.read(buffer, maxLen,index);
			});
			requestSend(request,response);
		}else{
			requestSend(request,204);
		}
	}
	TemperatureLogHandler(){}
	bool canHandle(AsyncWebServerRequest *request){
	 	if(request->url() == CHART_DATA_PATH || request->url() ==LOGS_PATH) return true;
	 	return false;
	}
	virtual bool isRequestHandlerTrivial() override final {return false;}

};


TemperatureLogHandler logHandler;

/**************************************************************************************/
/* network configuration */
/**************************************************************************************/


	IPAddress scanIP(const char *str)
	{
    	// DBG_PRINTF("Scan IP length=%d :\"%s\"\n",len,buffer);
    	// this doesn't work. the last byte always 0: ip.fromString(buffer);

    	int Parts[4] = {0,0,0,0};
    	int Part = 0;
		char* ptr=(char*)str;
    	for ( ; *ptr; ptr++)
    	{
	    char c = *ptr;
	    if ( c == '.' )
	    {
		    Part++;
		    continue;
	    }
	    Parts[Part] *= 10;
	    Parts[Part] += c - '0';
    	}

    	IPAddress sip( Parts[0], Parts[1], Parts[2], Parts[3] );
    	return sip;
	}

void requestRestart(bool disc);


class NetworkConfig:public AsyncWebHandler
{
public:
	void configString(String &str){

		const size_t capacity = JSON_OBJECT_SIZE(16);
		DynamicJsonDocument doc(capacity);

		doc["host"] = _gHostname;
		doc["user"] =_gUsername;
		doc["pass"] =_gPassword;
		doc["secured"] = _gSecuredAccess? 1:0;
		doc["ip"] = IPAddress_String(WiFiSetup.staIp());
		doc["gw"] = IPAddress_String(WiFiSetup.staGateway());
		doc["nm"] = IPAddress_String(WiFiSetup.staNetmask());
		doc["ap"] = WiFiSetup.isApMode()? 1:0;
		doc["ssid"] =WiFiSetup.staSsid();
		doc["stapass"] = WiFiSetup.staPass();
		serializeJson(doc, str);
		DBG_PRINTF("sav netcfg:%s",str.c_str());
	}

	bool saveConfig(void)
	{

		File config=FileSystem.open(CONFIG_FILENAME,"w+");
  		if(!config){
  				return false;
  		}
		String configStr;
		configString(configStr);
		config.print(configStr);
  		config.close();
		return true;
	}
	NetworkConfig(){}

	void handleRequest(AsyncWebServerRequest *request){
		if(request->url() == NETCFG_PATH) handleNetCfg(request);
		else if(request->url() == WIFI_DISC_PATH) handleNetworkDisconnect(request);
	}
	void handleNetworkDisconnect(AsyncWebServerRequest *request){
		WiFiSetup.disconnect();
		requestSend(request,200);
		saveConfig();
	}
	void handleNetCfg(AsyncWebServerRequest *request){
		if(request->method() == HTTP_POST){
			String data=request->getParam("data", true, false)->value();
			DBG_PRINTF("netcfg:\"%s\"\n",data.c_str());

			#if ARDUINOJSON_VERSION_MAJOR == 6

			DynamicJsonDocument root(1024);
			auto jsonerror=deserializeJson(root,data);

			if(jsonerror){
			#else
			DynamicJsonBuffer jsonBuffer(JSON_BUFFER_SIZE);
			JsonObject& root = jsonBuffer.parseObject(data.c_str());
			if (!root.success()){
			#endif
				DBG_PRINTF("Invalid JSON string\n");
				requestSend(request,404);
				return;
			}
			if(!root.containsKey("user") || !root.containsKey("pass")
			 || (strcmp(_gUsername,root["user"]) !=0)
			 || (strcmp(_gPassword,root["pass"]) !=0) ){
				//DBG_PRINTF("expected user:%s pass:%s\n",_gUsername,_gPassword);
			 	requestSend(request,400);
			 	return;
			}
			if(root.containsKey("disconnect")){
				requestRestart(true);
  				requestSend(request,200);
  				return;
			}

			if(root.containsKey("host")){
				strcpy(_gHostname,root["host"]);
			}

			if(root.containsKey("nuser")){
				strcpy(_gUsername,root["nuser"]);
			}

			if(root.containsKey("npass")){
				strcpy(_gPassword,root["npass"]);
			}

			if(root.containsKey("secured")){
				byte value=root["secured"];
				_gSecuredAccess =( 0 != value);
			}

			if(saveConfig()){	
				requestSend(request,200);
			}else{
				requestSend(request,500);
			}

		}else if(request->method() == HTTP_GET){
			if(FileSystem.exists(CONFIG_FILENAME)){
				requestSend(request,FileSystem,CONFIG_FILENAME, "application/json");
			}else{
				String rsp;
				configString(rsp);
				requestSend(request,200, "application/json",rsp);
			}
		}
	}

	bool canHandle(AsyncWebServerRequest *request){
	 	if(request->url() == NETCFG_PATH) return true;
		else if(request->url() == WIFI_DISC_PATH) return true;

	 	return false;
	}

	void loadSetting(void){
		// try open configuration
		char configBuf[MAX_CONFIG_LEN];
		File fh=FileSystem.open(CONFIG_FILENAME,"r+");
		bool fileError=true;
		if(fh){
			size_t len=fh.readBytes(configBuf,MAX_CONFIG_LEN);
			configBuf[len]='\0';
			DBG_PRINTF("read %d bytes:%s\n",len,configBuf);
			fh.close();
			fileError =false;
		}
		else{
			DBG_PRINTF("read failed\n");
		}
		#if ARDUINOJSON_VERSION_MAJOR == 6
		DynamicJsonDocument root(2048);
		auto error=deserializeJson(root,configBuf);
		
		if(error 
				|| fileError 
				|| !root.containsKey("host")
				|| !root.containsKey("user")
				|| !root.containsKey("pass")){

		#else
		DynamicJsonBuffer jsonBuffer(JSON_BUFFER_SIZE);
		JsonObject& root = jsonBuffer.parseObject(configBuf);

		if(!fh
				|| !root.success()
				|| !root.containsKey("host")
				|| !root.containsKey("user")
				|| !root.containsKey("pass")){

		#endif

  			strcpy(_gHostname,Default_HOSTNAME);
  			strcpy(_gUsername,Default_USERNAME);
  			strcpy(_gPassword,Default_PASSWORD);
			_gSecuredAccess=false;

			WiFiSetup.staConfig(false);
			DBG_PRINTF("loading cfg error:%s\\n",configBuf);

		}else{
  			strcpy(_gHostname,root["host"].as<String>().c_str());
  			strcpy(_gUsername,root["user"].as<String>().c_str());
  			strcpy(_gPassword,root["pass"].as<String>().c_str());
  			_gSecuredAccess=(root.containsKey("secured"))? (bool)(root["secured"]):false;

			if(root.containsKey("ap")){
				bool apmode=root["ap"];
				if(apmode) WiFiSetup.staConfig(true);
				else{
					IPAddress ip=root.containsKey("ip")? scanIP(root["ip"].as<String>().c_str()):INADDR_NONE;
					IPAddress gw=root.containsKey("gw")? scanIP(root["gw"].as<String>().c_str()):INADDR_NONE;
					IPAddress nm=root.containsKey("nm")? scanIP(root["nm"].as<String>().c_str()):INADDR_NONE;
					WiFiSetup.staConfig(false,ip,gw,nm);
				}
			}else{
				WiFiSetup.staConfig(false);
			}
			if(root.containsKey("ssid")){
				WiFiSetup.staNetwork(root["ssid"],root.containsKey("stapass")? root["stapass"]:emptyString);
			}
  		}

	}
		virtual bool isRequestHandlerTrivial() override final {return false;}

};

NetworkConfig networkConfig;

/**************************************************************************************/
/* BrewManiac interface */
/**************************************************************************************/

class FileReader
{
    File _file;
    size_t _offset;
    size_t _size;
    size_t _filesize;
    size_t _end;
public:
    FileReader(void){}
    bool prepare(String filename,int start, int end){
		_file = FileSystem.open(filename, "r+");

		if(!_file) return false;
		_filesize=_file.size();
		if(end <0){
		    _size=_filesize-start +1;
		    _end = _filesize -1;
    	}else{
    	    _size = end - start +1;
    	    _end = end;
    	}
        return true;
    }

    size_t size(void){return _size;}
    size_t end(void){return _end; }
    size_t filesize(void){return _filesize; }
    size_t read(uint8_t *buffer, size_t maxLen, size_t index){
        _file.seek(_offset + index, SeekSet);
        return _file.read(buffer,maxLen);
    }
    void finished(void){
        _file.close();
    }
} fileReader;

class BmwHandler: public AsyncWebHandler
{
protected:
    void decodeRange(String range,int& start, int& end)
    {
        String startStr=range.substring(range.indexOf('=')+1);

        start = startStr.toInt();
        int endIndex =range.indexOf('-');

        if(endIndex <0){
            end = -1;
        }else{
            int pend=range.substring(endIndex + 1 ).toInt();
            if(pend  > 0) end = pend;
        }
    }
public:
	BmwHandler(void){}
	virtual bool isRequestHandlerTrivial() override final {return false;}

	void handleRequest(AsyncWebServerRequest *request){

		if(_gSecuredAccess && !request->authenticate(_gUsername, _gPassword))
	        return request->requestAuthentication();
        if(request->method() == HTTP_GET && request->url() == SETTIME_PATH ){
			if(request->hasParam("time")){
  				AsyncWebParameter* tvalue = request->getParam("time");
  				DBG_PRINTF("Set Time:%ld, current:%ld\n",tvalue->value().toInt(),TimeKeeper.getTimeSeconds());
	 			TimeKeeper.setCurrentTime(tvalue->value().toInt());
	 			requestSend(request,200);
	 		}
	 		else requestSend(request,400);
	 	}else if(request->method() == HTTP_GET && request->url() == SETTING_PATH ){ // deprecated.
	 		String setting;
	 		bmWeb.getSettings(setting);
	 		String json= "{\"code\":0,\"result\":\"OK\", \"data\":"+ setting + "}";
	 		requestSend(request,200, "application/json", json);

			//piggyback the time from browser
			if(request->hasParam("time")){
  				AsyncWebParameter* tvalue = request->getParam("time");
  				DBG_PRINTF("Set Time:%ld, current:%ld\n",tvalue->value().toInt(),TimeKeeper.getTimeSeconds());
	 			TimeKeeper.setCurrentTime(tvalue->value().toInt());
	 		}

	 	}else if(request->method() == HTTP_GET && request->url() == AUTOMATION_PATH){ // deprecated.
	 		String json;
	 		String autojson;

	 		bmWeb.getAutomation(autojson);
	 		json = "{\"code\":0,\"result\":\"OK\", \"data\":";
            json += autojson;
            json += "}";
	 		requestSend(request,200, "application/json", json);
	 	}else if(request->method() == HTTP_GET && request->url() == BUTTON_PATH){
			if(request->hasParam("code")){
				AsyncWebParameter* p = request->getParam("code");
				byte code=p->value().toInt();
	 			bmWeb.sendButton(code & 0xF, (code & 0xF0)!=0);
	 			requestSend(request,200);
	 		}else{
	 			requestSend(request,400);
	 		}
	 	}else if(request->method() == HTTP_POST && request->url() == UPDATE_AUTOMATION_PATH){
	 		String data=request->getParam("data", true, false)->value();
//	 		DebugOut("saveauto.php:\n");
//	 		DebugOut(data.c_str());
	 		if(bmWeb.updateAutomation(data)){
	 			requestSend(request,200);
	 		}else{
	 			requestSend(request,400);
	 		}

	 	}else if(request->method() == HTTP_POST && request->url() == UPDATE_SETTING_PATH){
	 		String data=request->getParam("data", true, false)->value();
	 		DebugOut("savesettings.php:\n");
	 		DebugOut(data.c_str());
	 		if(bmWeb.updateSettings(data)){
	 			requestSend(request,200);
	 		}else{
	 			requestSend(request,400);
	 		}
	 	}else if(request->method() == HTTP_GET){

	 	    if(request->url() == AUDIO_PATH){
	 	        // no cache.
    	 		 requestSend(request,FileSystem,AUDIO_FILE);
    	 		 return;
	 		}
			// file retrievl.

		 	AsyncWebServerResponse *response;

			String path=request->url();
	 		if(path.endsWith("/")) path += String(DEFAULT_INDEX_FILE);
			
			DBG_PRINTF("checking#1 file:%s\n",path.c_str());
  			
			  if(path.endsWith(".js")){

	 			String pathWithJgz = path.substring(0,path.lastIndexOf('.')) + ".jgz";
				//DBG_PRINTF("checking with:%s\n",pathWithJgz.c_str());
  			  	if(FileSystem.exists(pathWithJgz)){
  			  		//DBG_PRINTF("response with:%s\n",pathWithJgz.c_str());
		 			response = request->beginResponse(FileSystem, pathWithJgz,"application/javascript");
					response->addHeader("Content-Encoding", "gzip");
					response->addHeader("Cache-Control","max-age=2592000");
					requestSend(request,response);
					return;
				}
  			}else{
  				//DBG_PRINTF("non js file:\"%s\"\n",path.c_str());
  			}
			
	 		if(path.endsWith(".m4a") || path.endsWith(".mp3") || path.endsWith(".ogg")){

    	 		int start=0;
    	 		int end = -1;
    	 		if(request->hasHeader("range")){
                    AsyncWebHeader* h = request->getHeader("range");
                    DBG_PRINTF("Range: %s\n", h->value().c_str());
                    decodeRange(h->value(),start,end);
                    DBG_PRINTF("decode: %d - %d\n", start, end);
                }
			    const char *mime=NULL;
			    if(path.endsWith(".m4a")) mime = "audio/mp4";
			    else if(path.endsWith(".mp3")) mime="audio/mepg";
			    else /*if(path.endsWith(".ogg"))*/ mime = "audio/ogg";

                if(start ==0 && end == -1){
    	 			response = request->beginResponse(FileSystem, path,mime);
	    		    response->addHeader("Accept-Ranges","bytes");
		    	    response->addHeader("Cache-Control","max-age=2592000");
			        requestSend(request,response);
			    }else{
			        if(! fileReader.prepare(path,start,end)) {
			            requestSend(request,500);
			            return;
			        }

                    AsyncWebServerResponse *response = request->beginResponse(mime,fileReader.size() , [](uint8_t *buffer, size_t maxLen, size_t index) -> size_t {
                              return fileReader.read(buffer, maxLen, index);
                    });
                    char buff[128];
                    sprintf(buff,"bytes %d-%d/%d",start,fileReader.end(),fileReader.filesize());
                    //response->addHeader("Content-Range",buff);
			
	    		    response->addHeader("Accept-Ranges","bytes");
		    	    response->addHeader("Cache-Control","max-age=2592000");
		    	    response->setCode(206);

		    	    requestSend(request,response);
			    }
	 		}else{
    	 		String pathWithGz = path + String(".gz");
				DBG_PRINTF("checking file:%s, gz:%s\n",path.c_str(),pathWithGz.c_str());
  	    		if(FileSystem.exists(pathWithGz)){
			    	// AsyncFileResonse will add "content-disposion" header, result in "download" of Safari, instead of "render" 
	 	    	  	// response = request->beginResponse(FileSystem, pathWithGz,"application/x-gzip");
			      	// response->addHeader("Content-Encoding", "gzip");
				  	File file=FileSystem.open(pathWithGz,"r");
			 	  	if(!file){
						requestSend(request,500);
						return;
					}
					response = request->beginResponse(file, path,getContentType(path));
				    response->addHeader("Cache-Control","max-age=2592000");
				    requestSend(request,response);

  			    }else if(FileSystem.exists(path)){
	 			    response = request->beginResponse(FileSystem, path);
				    response->addHeader("Cache-Control","max-age=2592000");
				    requestSend(request,response);
			    }else{
					DBG_PRINTF("embedded file\n");
					bool gzip;
					uint32_t size;
					const uint8_t* file=getEmbeddedFile(path.c_str(),gzip,size);
					if(file){
						response = request->beginResponse_P(200, "text/html", file, size);
						if(gzip){
    	           			response->addHeader("Content-Encoding", "gzip");
				    		response->addHeader("Cache-Control","max-age=2592000");
						    requestSend(request,response);
						}
					}else{
						// error.
						requestSend(request,500);
						return;
					}
				}
			}
		}
	 }

	bool canHandle(AsyncWebServerRequest *request){
	 	if(request->method() == HTTP_GET){
	 		if(request->url() == SETTIME_PATH 
			 	|| request->url() == SETTING_PATH 
				|| request->url() == AUTOMATION_PATH
	 		    || request->url() == BUTTON_PATH){
	 			return true;
	 		}else{
				DBG_PRINTF("BmwHandler canHandle file:%s\n",request->url().c_str());

	 		    if(request->url() == AUDIO_PATH){
    	 		    return FileSystem.exists(AUDIO_FILE);
	 		    }

				// get file
				if(request->url().endsWith(".m4a") || request->url().endsWith(".mp3") || request->url().endsWith(".ogg")){
					request->addInterestingHeader("range");
				}
				
				String path=request->url();
	 			if(path.endsWith("/")) path +=DEFAULT_INDEX_FILE;
	 			//DBG_PRINTF("request:%s\n",path.c_str());
				//if(fileExists(path)) return true;
				
				bool dum;
	    		unsigned int dum2;
			    if(getEmbeddedFile(path.c_str(),dum,dum2)) return true;

				if(FileSystem.exists(path)) return true;
				// special handling of jgz->js
  				if(path.endsWith(".js")){
	 				String pathWithJgz = path.substring(0,path.lastIndexOf('.')) + ".jgz";
					//DBG_PRINTF("checking with:%s\n",pathWithJgz.c_str());
  			  		if(FileSystem.exists(pathWithJgz)) return true;
  			  	}
				// pre compressed file
  				String pathWithGz = path + ".gz";
  				if(FileSystem.exists(pathWithGz)) return true;

	 		}
	 	}else if(request->method() == HTTP_POST){
	 		if(request->url() == UPDATE_AUTOMATION_PATH 
			 	|| request->url() == UPDATE_SETTING_PATH){
	 			return true;
			}
	 	}
	 	return false;
	 } // end of bool canHandle(...)
};

BmwHandler bmwHandler;

/**************************************************************************************/
/* server push  */
/**************************************************************************************/
#ifndef WL_MAC_ADDR_LENGTH
#define WL_MAC_ADDR_LENGTH 6
#endif
#define ESPAsyncTCP_issue77_Workaround 1

void getSystemInfo(String& json){
	#if ESP32
	json += String("{\"fid\":") + String(g_rom_flashchip.device_id)
			+ String(",\"rsize\":") + String(g_rom_flashchip.chip_size)
			+ String(",\"ssize\":") + String(SPIFFS.totalBytes());
	#else
	json += String("{\"fid\":") + String(ESP.getFlashChipId())
			+ String(",\"rsize\":") + String(ESP.getFlashChipRealSize())
			+ String(",\"ssize\":") + String(ESP.getFlashChipSize());
	FSInfo fs_info;
	FileSystem.info(fs_info);
	json +=  String(", \"fs\":") + String(fs_info.totalBytes);
	#endif

	json += String(",\"buildtime\":\"") +String(__DATE__) +String(" ")+String(__TIME__) +String("\"");

	uint8_t mac[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(mac);
	json +=String(", \"mac\":\"");

	#define toHex(a)  ((char)(((a)>9)? ('A'+((a)-10)):('0' +(a))))

	for(int i=0;i<WL_MAC_ADDR_LENGTH;i++){

		json += String(toHex(mac[i] >>4)) + String(toHex(mac[i] & 0x0F)); 
	}
	json +="\"}";
}

// version
void getVersionInfo(String& json)
{
	json += String("{\"firmware\":{\"v\":\"") + String(BME8266_VERSION);
	json += String("\",\"sensors\":") + String(MaximumNumberOfSensors);
	#if SupportDistilling
	json += String(",\"distill\":1");	
	#endif
	#if UsePaddleInsteadOfPump
	json += String(",\"paddle\":1");
	#else
	json += String(",\"paddle\":0");
	#endif

	json += String(",\"system\":");

	getSystemInfo(json);

	json +=" }}";
}

void greeting(std::function<void(const String&,const char*)> sendFunc){
	// version information
	String version;
	getVersionInfo(version);
	sendFunc(version,NULL);

	// send setting, automation, and network config
	String settings;
	bmWeb.getSettings(settings);
	sendFunc(settings,"setting");
	String automation;
	bmWeb.getAutomation(automation);
	sendFunc(automation,"auto");

	char buf[256];
	String netstat;
	WiFiSetup.status(netstat);
	sprintf(buf,"{\"host\":\"%s\",\"secured\":%d,\"wifi\":%s}",_gHostname,_gSecuredAccess? 1:0,netstat.c_str());
	
	sendFunc(buf,"netcfg");
	sprintf(buf,"{\"time\":%ld}",TimeKeeper.getTimeSeconds());
    sendFunc(String(buf),"timesync");
	String status;
	bmWeb.getCurrentStatus(status,true);
	sendFunc(status,NULL);	
}

#if UseWebSocket == true
AsyncWebSocket ws(WS_PATH);

extern void printSensorAddress(char *buf, byte *addr);


void wifiConnect(DynamicJsonDocument& root){

	if(root.containsKey("ap")){
			// AP only mode
			WiFiSetup.disconnect();
			// save to config
	}else if(root.containsKey("nw")){
		String ssid=root["nw"];
		if(root.containsKey("ip") && root.containsKey("gw") && root.containsKey("nm")){
			DBG_PRINTF("static IP\n");
			WiFiSetup.staConfig(false,scanIP(root["ip"].as<String>().c_str()),
							scanIP(root["gw"].as<String>().c_str()),
							scanIP(root["nm"].as<String>().c_str()));
				// save to config
		}else{
			WiFiSetup.staConfig(false,IPAddress(0,0,0,0),IPAddress(0,0,0,0),IPAddress(0,0,0,0));
		}
		WiFiSetup.connect(ssid,root.containsKey("pass")? root["pass"]:emptyString);
	}
	networkConfig.saveConfig();
}

void processRemoteCommand(AsyncWebSocketClient * client, uint8_t *data, size_t len)
{
	char buf[128];
	int i;
	for(i=0;i< (int)len && i<127;i++){
		buf[i]=data[i];
	}
	buf[i]='\0';
	
	DBG_PRINTF("processRemoteCommand:\"%s\"\n",buf);

#if ARDUINOJSON_VERSION_MAJOR == 6
	DynamicJsonDocument root(1024);
	auto jsonerror=deserializeJson(root,buf);
	if(!jsonerror)

#else

	StaticJsonBuffer<128> jsonBuffer;
	JsonObject& root = jsonBuffer.parseObject(buf);

	if (root.success())
#endif
	{
		if(root.containsKey("btn") ){
			int code = root["btn"];
			bmWeb.sendButton(code & 0xF, (code & 0xF0)!=0);
		}else if(root.containsKey("btnx")){
			int code = root["btnx"];
			bmWeb.sendButton(code,false);
		}else if(root.containsKey("wificmd")){
			String cmd=root["wificmd"];
			if(cmd == "scan"){
				WiFiSetup.requestScanWifi();
			}else if (cmd =="con"){
				wifiConnect(root);
			}
		}else if(root.containsKey(RPC_TAG)
				&& root.containsKey(RPCID_TAG)){			
			String rpcFunc=root[RPC_TAG];
			String rpcID=root[RPCID_TAG];

#if	MaximumNumberOfSensors	> 1
#if ARDUINOJSON_VERSION_MAJOR < 6
#error "ArduinoJson v6 is required"
#endif
		// not finished
			if(rpcFunc == SCAN_SENSOR_CMD){
				byte addr[MaximumNumberOfSensors][8];
		 		byte num=bmWeb.scanSensors(MaximumNumberOfSensors,addr);

				StaticJsonDocument<1024> json;
				json["ok"] = 1;
				json["id"] = rpcID;
				JsonObject ret=json.createNestedObject("ret");
				ret["num"] = num;

				char hex[20];
				if(num > 0){
					JsonArray addrs=ret.createNestedArray("sensors");
					for(int i=0;i<num;i++){
						printSensorAddress(hex,addr[i]);
						addrs.add(String("0x") + String(hex));
					}
				}
				String msg;
				serializeJson(json,msg);
				client->text(String("rpc:")+ msg);
				DBG_PRINTF("cmd rsp:%s\n",msg.c_str());
			}
#endif
		}
	}
}

void wsMessageOnConnect(AsyncWebSocketClient * client)
{
	greeting([=](const String& msg,const char* event){
		if(event==NULL) client->text(msg);
		else client->text(String(event)+":" + msg);
	});
}

#if ESPAsyncTCP_issue77_Workaround
AsyncWebSocketClient * _lastWsClient=NULL;

void wsHello()
{
	if(!_lastWsClient) return;
	
	wsMessageOnConnect(_lastWsClient);

	_lastWsClient=NULL;
}

#endif

void onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
	if(type == WS_EVT_CONNECT){
    	DBG_PRINTF("ws[%s][%u] connect\n", server->url(), client->id());
		client->ping();
#if ESPAsyncTCP_issue77_Workaround
		_lastWsClient=client;
#else		
		wsMessageOnConnect(client);
#endif
  	} else if(type == WS_EVT_DISCONNECT){
    	DBG_PRINTF("ws[%s] disconnect: %u\n", server->url(),(unsigned) client->id());
  	} else if(type == WS_EVT_ERROR){
    	DBG_PRINTF("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  	} else if(type == WS_EVT_PONG){
    	DBG_PRINTF("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  	} else if(type == WS_EVT_DATA){
    	AwsFrameInfo * info = (AwsFrameInfo*)arg;
    	String msg = "";
    	if(info->final && info->index == 0 && info->len == len){
      		//the whole message is in a single frame and we got all of it's data
      		DBG_PRINTF("ws[%s][%u] %s-message[%llu]\n", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
			
			processRemoteCommand(client,data,info->len);

		} else {
      		//message is comprised of multiple frames or the frame is split into multiple packets
#if 0  // for current application, the data should not be segmented
      		if(info->index == 0){
        		if(info->num == 0)
          		DBG_PRINTF("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        		DBG_PRINTF("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
      		}

      		DBG_PRINTF("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);

	        for(size_t i=0; i < info->len; i++) {
    	    	//msg += (char) data[i];
        	}

      		//DBG_PRINTF("%s\n",msg.c_str());

			if((info->index + len) == info->len){
				DBG_PRINTF("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
        		if(info->final){
        			DBG_PRINTF("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
        		}
      		}
#endif
      	}
    }
}

#endif // #if UseWebSocket == true


#if UseServerSideEvent == true
AsyncEventSource sse(SSE_PATH);

void sseConnect(AsyncEventSourceClient *client)
{
	greeting([=](const String& msg,const char* event){
		client->send(msg.c_str(),event);
	});
}
/*
void sseConnect(AsyncEventSourceClient *client)
{
	String version;
	getVersionInfo(version);
	client->send(version.c_str());
	String json;
	// send setting, automation, and network config
	bmWeb.getSettings(json);
	client->send(json.c_str(),"setting");
	bmWeb.getAutomation(json);
	client->send(json.c_str(),"auto");

	char buf[128];
	sprintf(buf,"{\"host\":\"%s\",\"secured\":%d}",_gHostname,_gSecuredAccess? 1:0);

	client->send(buf,"netcfg");
	sprintf(buf,"{\"time\":%ld}",TimeKeeper.getTimeSeconds());

    client->send(buf,"timesync");

	bmWeb.getCurrentStatus(json,true);
	client->send(json.c_str());

}
*/


#if ESPAsyncTCP_issue77_Workaround
// temp workaround. not a solution
// method: not sending data in onConnect() but in main loop.
// potential issue: two or more clients connect at the same time, before the mainloop can handle it
// (the poential issue can be solved, but mutual access issue should be addressed.)
//
AsyncEventSourceClient *_newClient=NULL;

void sseHello(void)
{
	if(!_newClient) return;
	sseConnect(_newClient);
	_newClient = NULL;
}

void sseDelayConnect(AsyncEventSourceClient *client)
{
	_newClient=client;
}

#endif //#if ESPAsyncTCP_issue77_Workaround

#endif //#if UseServerSideEvent == true


void broadcastMessage(String msg)
{
#if UseWebSocket == true
	ws.textAll(msg);
#endif

#if UseServerSideEvent == true
	sse.send(msg.c_str());
#endif
}

void broadcastMessage(const char* msg, const char* event=NULL)
{
#if UseWebSocket == true
	if(event==NULL) ws.textAll(msg);
	ws.textAll(String(event)+":" + msg);
#endif

#if UseServerSideEvent == true
	sse.send(msg,event);
#endif
}

#if ESPAsyncTCP_issue77_Workaround
void sayHello()
{
#if UseWebSocket == true
	wsHello();
#endif

#if UseServerSideEvent == true
	sseHello();
#endif
}
#endif

void wiFiEvent(const char* msg){
	broadcastMessage(msg,"wifi");
}
/**************************************************************************************/
/* callback from BM */
/**************************************************************************************/

void bmwEventHandler(BrewManiacWeb* bmw, BmwEventType event)
{
	if(event==BmwEventAutomationChanged){
		// request reload automation
		//broadcastMessage("{\"update\":\"recipe\"}");

    	String json;
    	bmWeb.getAutomation(json);
	    broadcastMessage(json.c_str(),"auto");

	}else if(event==BmwEventSettingChanged){
		// request reload setting
//		broadcastMessage("{\"update\":\"setting\"}");

    	String json;
	    // send setting, automation, and network config
	    bmWeb.getSettings(json);
	    broadcastMessage(json.c_str(),"setting");

	}else if(event==BmwEventStatusUpdate || event==BmwEventButtonLabel){
		String json;
//		if( event==BmwEventButtonLabel) DebugOut("Buttons\n");
		bmw->getCurrentStatus(json);
		broadcastMessage(json);

	}else if(event==BmwEventBrewEvent){
		String json;
		bmw->getLastEvent(json);
		broadcastMessage(json);
	}else if(event==BmwEventPwmChanged){
		String json;
		bmw->getSettingPwm(json);
		broadcastMessage(json);

	}else if(event==BmwEventSettingTemperatureChanged){
		String json;
		bmw->getSettingTemperature(json);
		broadcastMessage(json);
	}
}
/**************************************************************************************/
/* CNA, ap mode  */
/**************************************************************************************/

#if ResponseAppleCNA == true

class AppleCNAHandler: public AsyncWebHandler
{
public:
	AppleCNAHandler(){}
	void handleRequest(AsyncWebServerRequest *request){
		request->send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
	}
	bool canHandle(AsyncWebServerRequest *request){
		String host=request->host();
		//DBG_PRINTF("Request host:");
		//DBG_PRINTF(host.c_str());
		//DBG_PRINTF("\n");
  		if(host.indexOf(String("apple")) >=0
  		|| host.indexOf(String("itools")) >=0
  		|| host.indexOf(String("ibook")) >=0
  		|| host.indexOf(String("airport")) >=0
  		|| host.indexOf(String("thinkdifferent")) >=0
  		|| host.indexOf(String("akamai")) >=0 ){
  			return true;
  		}
  		return false;
	}
};

AppleCNAHandler appleCNAHandler;

#endif //#if ResponseAppleCNA == true

HttpUpdateHandler httpUpdateHandler(FIRMWARE_UPDATE_URL);
/*
bool testFileSystem(void)
{
	File vf=FileSystem.open("/BME_TestWrite.t","w+");
	if(!vf){
  		DebugOut("Failed to open file for test\n");
		return false;
	}
	const char *str="test string\n";
	vf.print(str);
	vf.close();
	DebugOut("Close file writing\n");

	File rf=FileSystem.open("/BME_TestWrite.t","r");
	if(!rf){
  		DebugOut("Failed to open file for test for reading\n");
		return false;
	}
//	String c=rf.readString();
	String c=rf.readStringUntil('\n');
	rf.close();

	DebugOut("Reading back data:");
	DebugOut(c.c_str());
	return true;
}
*/
#define PROFILING false
#if PROFILING == true
unsigned long _profileMaximumLoop=0;
unsigned long _profileLoopBegin;
#endif

void displayIP(bool apmode){
	IPV4Address ip;
	if(apmode){
	  	DebugOut("AP IP: ");
  		DebugOut(WiFi.softAPIP());

		ip.dword = WiFi.softAPIP();
		bmWeb.setIp(ip.bytes,true);
	}else{
	  	DebugOut("local IP: ");
  		DebugOut(WiFi.localIP());

		ip.dword = WiFi.localIP();
		bmWeb.setIp(ip.bytes);
	}
}


/**************************************************************************************/
/* Main procedure */
/**************************************************************************************/

void setup(void){
	//0. initilze debug port
	#if SerialDebug == true
  	DebugPort.begin(115200);
  	DebugOut("Start..\n");
  	DebugPort.setDebugOutput(true);
  	#endif

	#if SwapSerial == true && UseSoftwareSerial != true
	Serial.swap();
	#endif

	//1.Initialize file system
	//start SPI Filesystem
	#if ESP32
	if(!SPIFFS.begin(true)){
	#else
  	if(!FileSystem.begin()){
	#endif
  		// TO DO: what to do?
  		DebugOut("File System begin() failed\n");
  	}else{
  		DebugOut("File System begin() Success\n");
  	}

	//1b. load nsetwork conf
	networkConfig.loadSetting();

	DBG_PRINTF("hostname:%s, user:%s, pass:%s, secured:%d\n",_gHostname,_gUsername,_gPassword,_gSecuredAccess);

	// 2. start brewmaniac part, so that LCD will be ON.
	if(brewmaniac_setup()){
		DBG_PRINTF("recovery mode!\n");
		WiFiSetup.staConfig(true);
	}


	//3. Start WiFi
	WiFiSetup.onEvent(wiFiEvent);
	WiFiSetup.begin((const char*)_gHostname,(const char*)_gPassword);

  	DebugOut("Connected! IP address: ");
  	DebugOut(WiFi.localIP());
	#if ESP32
	if (!MDNS.begin(_gHostname)) {
	#else
	if (!MDNS.begin(_gHostname,WiFi.localIP())) {
	#endif
		DebugOut("Error setting mDNS responder");
	}
	// TODO: SSDP responder
	if(WiFiSetup.isApMode())
		TimeKeeper.begin(false);
	else
		TimeKeeper.begin("time.nist.gov","time.windows.com","de.pool.ntp.org");

	//4. check version

	//5.1 HTTP Update page
	httpUpdateHandler.setUrl(ONLINE_UPDATE_PATH);

	httpUpdateHandler.setCredential(_gUsername,_gPassword);
	httpUpdateHandler.setVersion(BME8266_VERSION);
	server.addHandler(&httpUpdateHandler);


		//5.2 Normal serving pages
		//5.2.1 status report through SSE
#if UseWebSocket == true
		ws.onEvent(onWsEvent);
  		server.addHandler(&ws);
#endif

#if	UseServerSideEvent == true
#if ESPAsyncTCP_issue77_Workaround
  		sse.onConnect(sseDelayConnect);
#else
  		sse.onConnect(sseConnect);
#endif
  		server.addHandler(&sse);
#endif
		server.addHandler(&networkConfig);
		// this must take priority over bmwHandler
		server.addHandler(&recipeFileHandler);

		server.addHandler(&bmwHandler);

#if ResponseAppleCNA == true
	server.addHandler(&appleCNAHandler);
#endif

	server.addHandler(&logHandler);
	//5.2.2 FileSystem is part of the serving pages
	//securedAccess need additional check
	// server.serveStatic("/", FileSystem, "/","public, max-age=259200"); // 3 days

#if ESP32
	server.on("/system",[](AsyncWebServerRequest *request){
		request->send(200,"","totalBytes:" +String(SPIFFS.totalBytes()) +
		" usedBytes:" + String(SPIFFS.usedBytes()) +
		" heap:"+String(ESP.getFreeHeap()));
		//testSPIFFS();
	});

#else
	server.on("/system",[](AsyncWebServerRequest *request){
		FSInfo fs_info;
		FileSystem.info(fs_info);
		request->send(200,"","totalBytes:" +String(fs_info.totalBytes) +
		" usedBytes:" + String(fs_info.usedBytes)+" blockSize:" + String(fs_info.blockSize)
		+" pageSize:" + String(fs_info.pageSize)
		+" heap:"+String(ESP.getFreeHeap()));
	});
#endif
#if PROFILING == true
	server.on("/profile",[](AsyncWebServerRequest *request){
		request->send(200,"","max loop time:" +String(_profileMaximumLoop));
	});
#endif
	// 404 NOT found.
  	//called when the url is not defined here
	server.onNotFound([](AsyncWebServerRequest *request){
		requestSend(request,404);
	});

	//6. start Web server
	server.begin();
	DebugOut("HTTP server started\n");

	MDNS.addService("http", "tcp", 80);

	// 7. try to connnect Arduino
  	bmWeb.onEvent(bmwEventHandler);

	// 8. start WEB update pages.
	ESPUpdateServer_setup(_gUsername,_gPassword);

	// 9. display IP
	startBrewManiac();
	displayIP(WiFiSetup.isApMode());

	DebugOut("End Setup\n");
}

#define SystemStateOperating 0
#define SystemStateRestartPending 1
#define SystemStateWaitRestart 2

#define TIME_RESTART_TIMEOUT 3000

bool _disconnectBeforeRestart;
static unsigned long _time;
byte _systemState=SystemStateOperating;
void requestRestart(bool disc)
{
	_disconnectBeforeRestart=disc;
	_systemState =SystemStateRestartPending;
}

#define IS_RESTARTING (_systemState!=SystemStateOperating)


void loop(void){
#if PROFILING == true
	_profileLoopBegin = millis();
#endif

	ESPUpdateServer_loop();
  	bmWeb.loop();
	
	#if !ESP32
	MDNS.update();
	#endif

  	brewmaniac_loop();

	if(WiFiSetup.stayConnected()){
		if(WiFiSetup.isApMode()){
			TimeKeeper.setInternetAccessibility(false);
			displayIP(true);
		}else{
			if(WiFi.status() != WL_CONNECTED){
				uint8_t nullip[]={0,0,0,0};
				bmWeb.setIp(nullip);
			}else{
				displayIP(false);
			}
		}
	}

  	httpUpdateHandler.runUpdate();

  	if(_systemState ==SystemStateRestartPending){
	  	_time=millis();
	  	_systemState =SystemStateWaitRestart;
  	}else if(_systemState ==SystemStateWaitRestart){
  		if((millis() - _time) > TIME_RESTART_TIMEOUT){
  			if(_disconnectBeforeRestart){
  				WiFi.disconnect();
  				WiFiSetup.setAutoReconnect(false);
  				delay(1000);
  			}
//  			ESP.restart();
  		}
  	}

#if ESPAsyncTCP_issue77_Workaround
	sayHello();
#endif

#if PROFILING == true
	unsigned long thisloop = millis() - _profileLoopBegin;
	if(thisloop > _profileMaximumLoop) _profileMaximumLoop = thisloop;
#endif

}
//end of file
