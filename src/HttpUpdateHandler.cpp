#include <ArduinoOTA.h>
#include <FS.h>
#if ESP32
#include <SPIFFS.h>
#include <AsyncTCP.h>
#else
#include <Hash.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "HttpUpdateHandler.h"
#include "BackupService.h"
#define RESET_WAITING_TIME 3000

#define SPIFFS_FORMAT_PATH     "/format-spiffs"
#define SPIFFS_FORMAT_EXE_PATH "/exeformat-spiffs"


extern FS& FileSystem;

#if SerialDebug == true
#define DBG_PRINTF(...) DebugPort.printf(__VA_ARGS__)
#else
#define DBG_PRINTF(...)
#endif

static const char spiffsformat_html[] PROGMEM = R"END(
<html>
<head>
<title>FS Format</title>
<script>eval(function(p,a,c,k,e,r){e=function(c){return c.toString(a)};if(!''.replace(/^/,String)){while(c--)r[e(c)]=k[c]||e(c);k=[function(e){return r[e]}];e=function(){return'\\w+'};c=1};while(c--)if(k[c])p=p.replace(new RegExp('\\b'+e(c)+'\\b','g'),k[c]);return p}('2 3(){4(5.6("0").7){1 8}9{a("b c 0 d e f g h.");1 i}};',19,19,'sure|return|function|makesure|if|document|getElementById|checked|true|else|alert|Please|make|you|know|what|will|happen|false'.split('|'),0,{}))</script>
</head>
<body>
By click the "Format" button. The File System will be formated.<br>
Wait for 60 seconds for formating the file system.
<form onsubmit="return makesure()" action="exeformat-spiffs">
<input type="checkbox" id="sure"> I know all files and data will be gone.<br>
<input type="submit" value="Format">
</form>
</body>
</html>
)END";

static const char spiffsformating_html[] PROGMEM = R"END(
<html>
<head>
<title>FS Format</title>
<script>/*<![CDATA[*/eval(function(p,a,c,k,e,r){e=function(c){return c.toString(a)};if(!''.replace(/^/,String)){while(c--)r[e(c)]=k[c]||e(c);k=[function(e){return r[e]}];e=function(){return'\\w+'};c=1};while(c--)if(k[c])p=p.replace(new RegExp('\\b'+e(c)+'\\b','g'),k[c]);return p}('2 7(){0 a=3.4();8(2(){0 c=9.d((3.4()-a)/5);e(c<6){0 b=f.g("h");b.i=""+(6-c)}j{1.k=1.l+"//"+1.m}},5)};',23,23,'var|location|function|Date|now|1000|60|count|setInterval|Math||||round|if|document|getElementById|sec|innerHTML|else|href|protocol|hostname'.split('|'),0,{}))/*]]>*/</script>
</head>
<body onload=count()>
Please Wait. Keep Calm and let the formatting carry on. Don't touch anything until the page reloaded.<br>
Formating... <span id="sec" style="font-size:32">60</span>
</body>
</html>
)END";


static const uint8_t fwupdate_htm_gz[] PROGMEM ={};

extern void getSystemInfo(String& json);

void HttpUpdateHandler::_sendVersion(AsyncWebServerRequest *request)
{
    String sysinfo=String();
    getSystemInfo(sysinfo);
	// construct customer parameter
	String param="{\"fw\":\"" + _fwVersion
			+"\",\"opt\":\"" + BUILD_OPTIONS
			+"\",\"fwurl\":\"" + _fwInfoUrl
			+"\",\"fwu\":\"/updatefw\",\"fwus\":\"/updatefwq\", \"system\":"+ sysinfo  +"}";
	request->send(200,"application/json",param);
}

void HttpUpdateHandler::_sendHtml(AsyncWebServerRequest *request, const char *html)
{
	AsyncWebServerResponse *response = request->beginResponse(String("text/html"),
  		strlen_P(html),
  		[=](uint8_t *buffer, size_t maxLen, size_t alreadySent) -> size_t {
    		if (strlen_P(html+alreadySent)>maxLen) {
      		// We have more to read than fits in maxLen Buffer
      		memcpy_P((char*)buffer, html+alreadySent, maxLen);
      		return maxLen;
    	}
    	// Ok, last chunk
    	memcpy_P((char*)buffer, html+alreadySent, strlen_P(html+alreadySent));
    	return strlen_P(html+alreadySent); // Return from here to end of indexhtml
 	 }
	);
//	if(gzip) response->addHeader("Content-Encoding", "gzip");
	request->send(response);
}



void HttpUpdateHandler::_firmwareUpdateStart(AsyncWebServerRequest *request)
{
	if(_state != US_Idle){
		request->send(403);
		return;
	}

	if(request->hasParam("source", true)){
  		AsyncWebParameter* p = request->getParam("source", true);
		_firmwareUpdateUrl= p->value();

		request->send(200);

		_state = US_FirmwareUpdatePending;
	}else{
		request->send(404);
	}

}


bool HttpUpdateHandler::canHandle(AsyncWebServerRequest *request)
{
    if(request->url().equals("/version.php")
//    	|| request->url().equals(_url)
    	|| request->url().equals(SPIFFS_FORMAT_PATH)
    	|| request->url().equals(SPIFFS_FORMAT_EXE_PATH)
    	|| request->url().equals("/system-reboot")){

        return true;
	}
	return false;
}


void HttpUpdateHandler::handleRequest(AsyncWebServerRequest *request)
{
	if(_user!=NULL && _pass!=NULL && !request->authenticate(_user, _pass))
		return request->requestAuthentication();

    if(request->url().equals("/version.php")){
	    _sendVersion(request);
/*    }else if(request->url().equals(_url)){
        // update
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", fwupdate_htm_gz, sizeof(fwupdate_htm_gz));
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);

*/		
   	}else if(request->url().equals(SPIFFS_FORMAT_PATH)) {
        if(request->hasArg("update")){
            request->send(200, "application/json","{}");
            _state = US_FormatForDownloadPending;
            DBG_PRINTF("Format before download\n");
        }else{
    		_sendHtml(request,spiffsformat_html);
        }
    }else if(request->url().equals(SPIFFS_FORMAT_EXE_PATH)){
		_sendHtml(request,spiffsformating_html);
		_state = US_FormatPending;
    }else if(request->url().equals("/system-reboot")){
    	if(_state == US_Idle){
    		request->send(200);
	    	_state = US_RestartInitiated;
	    }
	}
}

void HttpUpdateHandler::runUpdate(void)
{
	if(_state == US_RestartInitiated){
		_resetInitiatedTimer= millis();
		_state = US_RestartPending;
	}else if(_state == US_RestartPending){
		if((millis() - _resetInitiatedTimer) > RESET_WAITING_TIME)
		 	ESP.restart();
	}else if(_state ==US_FormatPending || _state == US_FormatForDownloadPending){
        // todo: STOP normal web service
        UpdaterState oldstate=_state;
		_state= US_Formating;
		#if !ESP32
        BackupService.backup();
		#endif
		DBG_PRINTF("Start Formating FS\n");
		#if ESP32
		SPIFFS.format();
		#else
		FileSystem.format();
		#endif
		DBG_PRINTF("End Formating FS\n");
		#if !ESP32
        BackupService.restore();
		#endif

        if(oldstate == US_FormatPending)
    		_state = US_RestartInitiated;
        else
            _state = US_FormatForDownloadFinished;
	}
}