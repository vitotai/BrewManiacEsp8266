#include <ArduinoOTA.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "HttpUpdateHandler.h"
#define RESET_WAITING_TIME 3000

#define SPIFFS_FORMAT_PATH     "/format-spiffs"
#define SPIFFS_FORMATTING_PATH "/exeformat-spiffs"


#if SerialDebug == true
#define DEBUGF(...) DebugPort.printf(__VA_ARGS__)
#define DDEBUGF(...) DebugPort.printf(__VA_ARGS__)
#else
#define DEBUGF(...)
#define DDEBUGF(...)
#endif

static const char indexhtml[] PROGMEM = R"END(
<!doctype html>
<html>
<head>
<meta charset=utf-8 name=viewport content="width=device-width, initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no">
<title>BrewManiac Update</title>
<style>.fwstat,.jsstat{display:none}body{background-color:#e5e6ea}</style>
<script>eval(function(p,a,c,k,e,r){e=function(c){return(c<a?'':e(parseInt(c/a)))+((c=c%a)>35?String.fromCharCode(c+29):c.toString(36))};if(!''.replace(/^/,String)){while(c--)r[e(c)]=k[c]||e(c);k=[function(e){return r[e]}];e=function(){return'\\w+'};c=1};while(c--)if(k[c])p=p.replace(new RegExp('\\b'+e(c)+'\\b','g'),k[c]);return p}('3 o(i){5.8(i).B.Y=\'1O\'}3 C(i){5.8(i).B.Y=\'1g\'}3 1h(a){6 b=1P.1i.Z;a=a.1j(/[\\[\\]]/g,"\\\\$&");6 c=K 1Q("[?&]"+a+"(=([^&#]*)|&|#|$)"),L=c.1R(b);7(!L)10 1k;7(!L[2])10\'\';10 1S(L[2].1j(/\\+/g," "))}3 q(a){6 b=K 1T();b.1U=3(){7(b.1V==4){7(b.1l==1W){a.s(b.1X)}9{7(D a["H"]!="E")a.H(b.1l);9 11("1Y 1m.")}}};b.1Z=3(){7(D a["M"]!="E")a.M();9 b.1n()},b.1n=3(){7(D a["H"]!="E")a.H(-1);9 11("21 1m.")};b.22(a.m,a.n,1o);7(D a["N"]!="E")b.23("24-25",a.N);7(D a["k"]!="E"){b.1p(a.k)}9 b.1p()}3 12(){6 a=5.1q("26");27(i=0;i<a.13;i++){a[i].28=1o}}3 O(p){14.w=p}O.29.15=3(a,b,h,f){o(14.w+"16");6 t=14;f=(f)?f:2a;q({m:"I",n:a,s:3(r){C(t.w+"16");k=P.Q(r);7(!f&&b==k.R){o(t.w+"2b")}9{h(k);o(t.w+"2c")}},H:3(){C(t.w+"16");o(t.w+"17")}})};3 18(a,b,c){y(3(){1i.2d()},a*1r);5.8(b).j=c+"<1s>2e 2f 2g 2h <1t 2i=1u>"+a+"</1t> 2j.</1s>";1v(3(){19=5.8("1u");19.j=""+(2k(19.j)-1)},1r)}3 S(){q({m:"I",n:l.2l,M:3(){5.8("J").1w(\'1x\',"...");y(S,F)},s:3(r){d=P.Q(r);7(d.1y){7(D d["1z"]!="E"){18(d.1z,"J","2m 2n.")}9{5.8("J").j=d.2o}}9{5.8("J").1w(\'1x\',"...");y(S,F)}}})}3 2p(){C("2q");o("J");12();q({m:"1A",n:l.2r,N:"1B/x-1C-1D-1E",k:"1F="+1a(T),s:3(){y(S,F)}})}6 u;6 z;3 U(){q({m:"I",n:l.2s,M:3(){y(U,F)},s:3(r){k=P.Q(r);7(k.1y){7(k.17!=0){11("17:"+k.2t)}9{z++;1b(z);7(z==u.v.13){q({m:"I",n:"/2u-2v",s:3(){18(20,"1G","2w 2x 2y.")}})}9{1c(z)}}}9{y(U,F)}}})}3 1c(i){k="V="+u.v[i].V+"&G="+1a(u.v[i].G);7(u.v[i].V!="2z")k+="&1H="+1a(u.v[i].1H);q({m:"1A",n:l.2A,N:"1B/x-1C-1D-1E",k:k,s:3(){y(U,F)}})}3 2B(){o("A");5.8("1G").j="2C..";12();z=0;1c(z);1b(0)}3 2D(){7(5.8("A").B.Y=="1g")o("A");9 C("A")}6 T="";3 1I(){6 e=(1h("1J")!=1k);5.8("2E").j=""+l.1K;6 f=K O("2F");f.15(l.T+"&1L="+l.1L,l.1K,3(d){5.8("2G").j=""+d.R;5.8("2H").Z=d.n;5.8("2I").j=d.2J;T=d.1F},e);5.8("2K").j=""+l.1d;6 g=K O("2L");6 h=(e)?"&1J=1":"";g.15(l.2M+l.1d+h,l.1d,3(d){5.8("2N").j=""+d.R;5.8("2O").Z=d.n;5.8("2P").j=""+d.v.13;C("A");6 c=5.W("1e");u=d;d.v.1M(3(a,i){6 b=5.W("1N");b.j="["+a.V+"] "+a.G;c.X(b)});5.8("A").X(c)},e)}3 1b(d){6 e=5.W("1e");u.v.1M(3(a,i){6 b=5.W("1N");7(i==d){b.j="[2Q]"+a.G;6 c=b;1v(3(){7(c.B.1f==0)c.B.1f=1;9 c.B.1f=0},2R)}9 7(i<d)b.j="[2S] "+a.G;9 b.j="[2T] "+a.G;e.X(b)});6 f=5.8("A");6 g=f.1q("1e")[0];f.2U(g);f.X(e)}6 l={};3 2V(){q({m:"I",n:"R.2W",s:3(r){l=P.Q(r);1I()}})}',62,183,'|||function||document|var|if|getElementById|else||||||||||innerHTML|data|ver_info||url|show||invoke||success||jsfiles|list|pre||setTimeout|processingIndex|filedetial|style|hide|typeof|undefined|3000|dst|fail|GET|fw_progress|new|results|timeout|mime|SoftwareUpdater|JSON|parse|version|fwprogress|fwurl|jsprogress|action|createElement|appendChild|display|href|return|alert|noSubmit|length|this|loadinfo|initial|error|countdown|ele|encodeURIComponent|updatelist|processFile|js|ul|opacity|none|getPara|location|replace|null|status|Error|onerror|true|send|getElementsByTagName|1000|h2|span|COUNTDOWN|setInterval|insertAdjacentHTML|beforeend|finished|refresh|POST|application|www|form|urlencoded|source|listdes|src|checkUpdate|forced|fw|opt|forEach|li|block|window|RegExp|exec|decodeURIComponent|XMLHttpRequest|onreadystatechange|readyState|200|responseText|Unexpected|ontimeout||Communication|open|setRequestHeader|Content|Type|input|for|disabled|prototype|false|latest|avail|reload|Wait|to|reconnect|in|id|seconds|parseInt|fwus|Update|Succeeded|result|updatefw|fw_avail|fwu|jsus|msg|system|reboot|All|file|updated|del|jsu|updatejs|Updating|showfilelist|fw_version|fw_|newversion|infolink|fsize|size|js_version|js_|jsurl|jsnewversion|jsinfolink|fileno|Processing|500|DONE|wait|removeChild|init|php'.split('|'),0,{}))</script>
</head>
<body onload=init()>
<H2>BrewManiac ESP8266 System update</H2>
<h3>Firmware version: <span id=fw_version>0.0</span></h3>
<div id=fw_initial class=fwstat>Getting firmware information..</div>
<div id=fw_error class=fwstat>Failed to get information. Check network connection or <a href=http://vito.tw/brewmaniac>check webside</a></div>
<div id=fw_latest class=fwstat>Latest version. No update available.</div>
<div id=fw_avail class=fwstat><h3>Latest version: <span id=newversion></span></h3>Size: <span id=fsize></span> bytes
<br />
<a href id=infolink target=_blank>Detail..</a> <form type=POST action=javascript:updatefw();><input type=submit value="update Now"></form>
</div>
<div id=fw_progress class=fwstat>Sart updating...</div>
<hr />
<h3>HTML/Javascript version: <span id=js_version>0.0</span></h3>
<div id=js_initial class=jsstat>Getting HTML/Javascript information..</div>
<div id=js_error class=jsstat>Failed to get information. Check network connection or <a href=http://vito.tw/brewmaniac>check webside</a></div>
<div id=js_latest class=jsstat>All files are up to date.</div>
<div id=js_avail class=jsstat><h3>Latest version: <span id=jsnewversion></span></h3>
<a href=javascript:showfilelist()><span id=fileno></span> files</a> to be updated. <a href id=jsinfolink target=_blank>Detail..</a> <form type=POST action=javascript:updatejs();><input type=submit value="update Now"></form>
<div id=filedetial><span id=listdes>Update list:</span></div>
</div>
</body>
</html>
)END";

static const char spiffsformat_html[] PROGMEM = R"END(
<html>
<head>
<title>SPIFFS Format</title>
<script>eval(function(p,a,c,k,e,r){e=function(c){return c.toString(a)};if(!''.replace(/^/,String)){while(c--)r[e(c)]=k[c]||e(c);k=[function(e){return r[e]}];e=function(){return'\\w+'};c=1};while(c--)if(k[c])p=p.replace(new RegExp('\\b'+e(c)+'\\b','g'),k[c]);return p}('2 3(){4(5.6("0").7){1 8}9{a("b c 0 d e f g h.");1 i}};',19,19,'sure|return|function|makesure|if|document|getElementById|checked|true|else|alert|Please|make|you|know|what|will|happen|false'.split('|'),0,{}))</script>
</head>
<body>
By click the "Format" button. The SPIFFS will be formated.<br>
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
<title>SPIFFS Format</title>
<script>/*<![CDATA[*/eval(function(p,a,c,k,e,r){e=function(c){return c.toString(a)};if(!''.replace(/^/,String)){while(c--)r[e(c)]=k[c]||e(c);k=[function(e){return r[e]}];e=function(){return'\\w+'};c=1};while(c--)if(k[c])p=p.replace(new RegExp('\\b'+e(c)+'\\b','g'),k[c]);return p}('2 7(){0 a=3.4();8(2(){0 c=9.d((3.4()-a)/5);e(c<6){0 b=f.g("h");b.i=""+(6-c)}j{1.k=1.l+"//"+1.m}},5)};',23,23,'var|location|function|Date|now|1000|60|count|setInterval|Math||||round|if|document|getElementById|sec|innerHTML|else|href|protocol|hostname'.split('|'),0,{}))/*]]>*/</script>
</head>
<body onload=count()>
Please Wait. Keep Calm and let the formatting carry on. Don't touch anything until the page reloaded.<br>
Formating... <span id="sec" style="font-size:32">60</span>
</body>
</html>
)END";

void HttpUpdateHandler::_sendVersion(AsyncWebServerRequest *request)
{
	// construct customer parameter
	String param="{\"fw\":\"" + _fwVersion
			+"\",\"opt\":\"" + BUILD_OPTIONS
			+"\",\"fwurl\":\"" + _fwInfoUrl
			+"\",\"fwu\":\"/updatefw\",\"fwus\":\"/updatefwq\" ,\"js\":\"" + _jsVersion
			+ "\",\"jsurl\":\"" + _jsInfoUrl
			+"\",\"jsu\":\"/updatejs\",\"jsus\":\"/updatejsq\"}";
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
//	response->addHeader("Server", "MyServerString");
	request->send(response);
}

void HttpUpdateHandler::_firmwareUpdateStatus(AsyncWebServerRequest *request)
{
	if(_state == US_Idle){
		if(_updateReturn==HTTP_UPDATE_FAILED) {
        	DEBUGF("[update] Update failed.");
	      	request->send(200, "application/json", "{\"finished\":1, \"result\":\"Error:"+ ESPhttpUpdate.getLastErrorString() +"\"}");
        }else if(_updateReturn == HTTP_UPDATE_NO_UPDATES){
		    DEBUGF("[update] Update no Update.");
	      	request->send(200, "application/json", "{\"finished\":1, \"result\":\"Nothing to Update.\"}");
		}else if(_updateReturn == HTTP_UPDATE_OK){
			DEBUGF("[update] Update ok.");
	      	request->send(200, "application/json", "{\"finished\":1, \"refresh\":15, \"result\":\"OK\"}");
	      	// start timer to reset
	      	_state=US_RestartInitiated;
		}else{
			request->send(200, "application/json", "{\"finished\":1, \"result\":\"unknown error\"}");
		}
	}else if (_state == US_FirmwareUpdatePending || _state == US_FirmwareUpdating){
		DEBUGF("[RSP] Updating...\n");
		request->send(200, "application/json",R"({"finished":0})");
	}else{
		request->send(500);
	}
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

void HttpUpdateHandler::_scriptUpdateStart(AsyncWebServerRequest *request)
{
	if(_state != US_Idle){
		request->send(403);
		return;
	}

	if(!request->hasParam("action", true) || !request->hasParam("dst", true)){
		request->send(403);
		return;
	}
	//else
	AsyncWebParameter* p = request->getParam("action", true);
	AsyncWebParameter* d = request->getParam("dst", true);

	String filename=d->value();
	if(!filename.startsWith("/")) filename = "/"+filename;

	if(p->value().equals("new")){
	  	if(!request->hasParam("src", true)){
		  		request->send(403);
	  			return;
	  	}
		String src=request->getParam("src",true)->value();
	  	request->send(200);
	  	_fileDownloader.begin(src,filename);

	  	_state=US_FileDownloadPending;

	}else if(p->value().equals("del")){
		SPIFFS.remove(filename);
	  	request->send(200);
	}
}

void HttpUpdateHandler::_scriptUpdateStatus(AsyncWebServerRequest *request)
{
	DEBUGF("JS update status, state=%d\n",_state);
	if(_state == US_Idle){
		//finished
		if(_fileDownloader.isSuccess()){
			request->send(200, "application/json","{\"finished\":1,\"error\":0}");
		}else{
			String output="{\"finished\":1,\"error\":"+ String(_fileDownloader.errorCode())
						+",\"msg\":\"" + _fileDownloader.errorMessage() + "\"}";
			request->send(200, "application/json",output);
		}
	}else if(_state == US_FileDownloadPending || _state == US_FileDownloading){
		DEBUGF("Query download Status...\n");
		request->send(200, "application/json",R"({"finished":0})");
	}else {
		request->send(500);
	}
}


bool HttpUpdateHandler::canHandle(AsyncWebServerRequest *request)
{
    if(request->url().equals("/version.php")
    	|| request->url().equals(_url)
    	|| request->url().equals(SPIFFS_FORMAT_PATH)
    	|| request->url().equals(SPIFFS_FORMATTING_PATH)
    	|| request->url().equals("/updatefw")
    	|| request->url().equals("/updatefwq")
    	|| request->url().equals("/updatejs")
    	|| request->url().equals("/updatejsq")
    	|| request->url().equals("/system-reboot"))

        return true;
	return false;
}


void HttpUpdateHandler::handleRequest(AsyncWebServerRequest *request)
{
	if(_user!=NULL && _pass!=NULL && !request->authenticate(_user, _pass))
		return request->requestAuthentication();

    if(request->url().equals("/version.php")){
	    _sendVersion(request);
    }else if(request->url().equals(_url)){
    	_sendHtml(request,indexhtml);
   	}else if(request->url().equals(SPIFFS_FORMAT_PATH)) {
		_sendHtml(request,spiffsformat_html);
    }else if(request->url().equals(SPIFFS_FORMATTING_PATH)){
		_sendHtml(request,spiffsformating_html);
		_state = US_FormatPending;
    }else if(request->url().equals("/updatefw")){
    	_firmwareUpdateStart(request);
    }else if(request->url().equals("/updatefwq")){
	    _firmwareUpdateStatus(request);
    }else if(request->url().equals("/updatejs")){
    	_scriptUpdateStart(request);
    }else if(request->url().equals("/updatejsq")){
    	_scriptUpdateStatus(request);
    }else if(request->url().equals("/system-reboot")){
    	if(_state == US_Idle){
    		request->send(200);
	    	_state = US_RestartInitiated;
	    }
	}
}

void HttpUpdateHandler::runUpdate(void)
{
	if(_state ==US_FirmwareUpdatePending){
		_state = US_FirmwareUpdating;
		DEBUGF("Start http update:%s\n",_firmwareUpdateUrl.c_str());
		ESPhttpUpdate.rebootOnUpdate(false);
		_updateReturn = ESPhttpUpdate.update(_firmwareUpdateUrl, _fwVersion);
		DEBUGF("End of http update\n");

		_state = US_Idle;
	}else if(_state ==US_FileDownloadPending){
		_state= US_FileDownloading;
		DEBUGF("Start file download\n");
		_fileDownloader.download();
		DEBUGF("End file download\n");
		_state = US_Idle;
	}else if(_state == US_RestartInitiated){
		_resetInitiatedTimer= millis();
		_state = US_RestartPending;
	}else if(_state == US_RestartPending){
		if((millis() - _resetInitiatedTimer) > RESET_WAITING_TIME)
		 	ESP.restart();
	}else if(_state ==US_FormatPending){
		_state= US_Formating;
		DEBUGF("Start Formating SPIFFS\n");
		SPIFFS.format();
		DEBUGF("End Formating SPIFFS\n");
		_state = US_RestartInitiated;
	}

}


void HttpFileDownloader::download(void)
{
 	// configure server and url

    File f=SPIFFS.open(_filename,"w+");
    if(!f){
    	DEBUGF("file open failed\n");
		_error = -1;
		_errorMsg = "Error open filed!";

    	_finished=true;
    	return;
    }
	DEBUGF("Download %s\n",_filename.c_str());

	_buff =(uint8_t*) malloc(FLASH_SECTOR_SIZE);
	if(!_buff){
		DEBUGF("error allocate memory\n");
		_error = -1;
		_errorMsg = "Not enough resource.";

		_finished=true;
		return;
	}

    _http.begin(_url);

    DEBUGF("[HTTP] GET %s...\n",_url.c_str());
    // start connection and send HTTP header
    int httpCode = _http.GET();
    if(httpCode > 0) {
    	// HTTP header has been send and Server response header has been handled
        DEBUGF("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if(httpCode == HTTP_CODE_OK) {

            // get lenght of document (is -1 when Server sends no Content-Length header)
            int len = _http.getSize();
            int wlen=0;
			DEBUGF("[HTTP] Content Length: %d\n", len);
            // create buffer for read
            //uint8_t buff[512] = { 0 };

            // get tcp stream
            WiFiClient * stream = _http.getStreamPtr();
            // read all data from server

            uint8_t *ptr =_buff;
            size_t space=FLASH_SECTOR_SIZE;
            while(_http.connected() && (len > 0 || len == -1)) {
                // get available data size
            	size_t size = stream->available();

                if(size) {
                	int c = stream->readBytes(ptr, ((size > space) ? space : size));
					DDEBUGF("read %d bytes\n",c);
					ptr += c;
					space -= c;
                    if(len > 0) len -= c;

                    if(space == 0) {
	                    DDEBUGF("write %d bytes\n",FLASH_SECTOR_SIZE);
                        ESP.wdtDisable();
                        f.write(_buff,FLASH_SECTOR_SIZE);
                        ESP.wdtEnable(10);
                        ptr =_buff;
                        space=FLASH_SECTOR_SIZE;
                        wlen += space;
                    }
                }
                delay(1);
            }
            if(space < FLASH_SECTOR_SIZE){
				DDEBUGF("write %d bytes\n",FLASH_SECTOR_SIZE-space);
                ESP.wdtDisable();
                f.write(_buff,FLASH_SECTOR_SIZE - space);
                wlen += FLASH_SECTOR_SIZE-space;
                ESP.wdtEnable(10);
            }

			if(len == 0 || len == -1){
				_error = 0;
			}else{
				_error = -1;
				_errorMsg = "Connection broken!";
			}

            DEBUGF("[HTTP] connection closed. written %d.\n",wlen);

    	} else {
    		// httpCode != OKs
    		DEBUGF("[HTTP] Response:%d\n",httpCode);
    		_error = httpCode;
    		_errorMsg="HTTP code:" + String(_error);
    	}
    } else {
    	// httpCode <=0
        DEBUGF("[HTTP] GET... failed, error: %s\n", _http.errorToString(httpCode).c_str());
        _error=httpCode;
        _errorMsg=_http.errorToString(httpCode);
    }
	f.close();
	free(_buff);

    _http.end();
    _finished=true;
}
