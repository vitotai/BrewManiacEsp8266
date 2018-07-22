#ifndef WiFiSetup_H
#define WiFiSetup_H

#include <DNSServer.h>

#define WiFiStateConnected 0
#define WiFiStateWaitToConnect 1
#define WiFiStateConnecting 2
#define WiFiStateDisconnected 3
#define WiFiStateDisconnectPending 4
#define WiFiStateChangeConnectPending 5

#define WiFiScanStateNone 0
#define WiFiScanStatePending 1
#define WiFiScanStateScanning 2


#define TIME_WAIT_TO_CONNECT 20000
#define TIME_RECONNECT_TIMEOUT 20000
#define    DNS_PORT  53

class WiFiSetupClass
{
public:
	WiFiSetupClass():_wifiState(WiFiStateConnected),_wifiScanState(WiFiScanStateNone),_apMode(false),_switchToAp(false),_autoReconnect(true),
		 _maxReconnect(0),_eventHandler(NULL),_targetSSID(NULL),_targetPass(NULL){}

	void begin(char const *ssid,const char *passwd=NULL);
	void onEvent(std::function<void(const char*)> handler){ _eventHandler = handler;}

	bool stayConnected(void);
	bool isApMode(void) {return _apMode;}

	void setMaxReconnect(unsigned int reconnect){_maxReconnect=reconnect;}
	void setSwitchToApWhenDisconnected(bool toAp){  _switchToAp= toAp; }
	void setAutoReconnect(bool reconnect){ _autoReconnect=reconnect; }

	String scanWifi(void);
	bool requestScanWifi(void);
	bool connect(char const *ssid,const char *passwd);
	bool disconnect(void);

	bool isConnected(void);

private:
	byte _wifiState;
	byte _wifiScanState;
	bool _apMode;
	bool _switchToAp;
	bool _autoReconnect;

	unsigned int _maxReconnect;
	unsigned int _reconnect;

	unsigned long _time;
	std::function<void(const char*)> _eventHandler;
	
	std::unique_ptr<DNSServer>        dnsServer;

	const char *_apName;
	const char *_apPassword;

	const char *_targetSSID;
	const char *_targetPass;

	void setupApService(void);
	void enterApMode();
};

extern WiFiSetupClass WiFiSetup;
#endif
