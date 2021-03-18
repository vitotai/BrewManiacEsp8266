#ifndef HttpUpdateHandler_H
#define HttpUpdateHandler_H
#if ESP32
#include <HTTPClient.h>
#else
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPClient.h>
#endif
#define QueueSize 4
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
	US_Formating,
	US_FormatForDownloadPending,
	US_FormatForDownloadFinished
}UpdaterState;

class HttpUpdateHandler: public AsyncWebHandler
{
	String _fwVersion;
	String _fwInfoUrl;

	UpdaterState _state;


	String _firmwareUpdateUrl;

	void _sendHtml(AsyncWebServerRequest *request,const char* html);
	void _sendVersion(AsyncWebServerRequest *request);

	void _firmwareUpdateStart(AsyncWebServerRequest *request);
	void _firmwareUpdateStatus(AsyncWebServerRequest *request);

	unsigned long _resetInitiatedTimer;
	String _url;
	char *_user;
	char *_pass;
public:
	public:
	HttpUpdateHandler(String fwUrl):_fwInfoUrl(fwUrl),_state(US_Idle){_user=_pass=NULL;}
	void setUrl(String url){_url = url;}
	void setVersion(String firmware){ _fwVersion=firmware;}

	void setCredential(char* user, char* pass){_user=user; _pass=pass;}

    bool canHandle(AsyncWebServerRequest *request);
    void handleRequest(AsyncWebServerRequest *request);
    void runUpdate(void);
	virtual bool isRequestHandlerTrivial() override final {return false;}
};
#endif
