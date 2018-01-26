#ifndef HttpUpdateHandler_H
#define HttpUpdateHandler_H

#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>
#define QueueSize 4

class HttpFileDownloader
{
 	HTTPClient _http;
 	bool _finished;
 	int _error;
 	String _errorMsg;

 	String _url;
 	String _filename;
 	uint8_t *_buff;
public:
	HttpFileDownloader(void):_errorMsg(""){}
	void begin(String url,String filename){_finished=false; _url=url; _filename=filename;}
	void download(void);
	bool isFinished(void){return _finished;}
	bool isSuccess(void){ return (_error ==0);}
	int  errorCode(void){ return _error;}
	String errorMessage(void) {return _errorMsg;}
};

typedef enum _UpdaterState
{
	US_Idle,
	US_FirmwareUpdatePending,
	US_FirmwareUpdating,
	US_FileDownloadPending,
	US_FileDownloading,
	US_RestartInitiated,
	US_RestartPending,
	US_FormatPending,
	US_Formating
}UpdaterState;

class HttpUpdateHandler: public AsyncWebHandler
{
	UpdaterState _state;

	String _fwVersion;
	String _jsVersion;
	String _fwInfoUrl;
	String _jsInfoUrl;

	String _firmwareUpdateUrl;
	t_httpUpdate_return _updateReturn;

	void _sendHtml(AsyncWebServerRequest *request,const char* html);
	void _sendVersion(AsyncWebServerRequest *request);

	void _scriptUpdateStart(AsyncWebServerRequest *request);
	void _scriptUpdateStatus(AsyncWebServerRequest *request);
	void _firmwareUpdateStart(AsyncWebServerRequest *request);
	void _firmwareUpdateStatus(AsyncWebServerRequest *request);

	HttpFileDownloader _fileDownloader;
	unsigned long _resetInitiatedTimer;
	String _url;
	char *_user;
	char *_pass;
public:
	public:
	HttpUpdateHandler(String fwUrl,String jsUrl):_fwInfoUrl(fwUrl),_jsInfoUrl(jsUrl),_state(US_Idle){_user=_pass=NULL;}
	void setUrl(String url){_url = url;}
	void setVersion(String firmware,String js){_jsVersion=js; _fwVersion=firmware;}

	void setCredential(char* user, char* pass){_user=user; _pass=pass;}

    bool canHandle(AsyncWebServerRequest *request);
    void handleRequest(AsyncWebServerRequest *request);
    void runUpdate(void);
	virtual bool isRequestHandlerTrivial() override final {return false;}
};

#endif
