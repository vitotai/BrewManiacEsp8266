#if ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#endif
//needed for library
#include <DNSServer.h>
#include "config.h"
#include "WiFiSetup.h"
#include <ArduinoJson.h>

WiFiSetupClass WiFiSetup;

#if SerialDebug == true
#define DebugOut(a) DebugPort.print(a)
#define DBG_PRINTF(...) DebugPort.printf(__VA_ARGS__)
#define DBG_PRINTLN(a) DebugPort.println(a)
#else
#define DebugOut(a)
#define DBG_PRINTF(...)
#define DBG_PRINTLN(a) 
#endif

#if SerialDebug
#define wifi_info(a)	DBG_PRINTF("%s: %d ,SSID:%s pass:%s IP:%s, gw:%s\n",(a), WiFi.status(),WiFi.SSID().c_str(),WiFi.psk().c_str(),WiFi.localIP().toString().c_str(),WiFi.gatewayIP().toString().c_str())
#else
#define wifi_info(a)
#endif

void WiFiSetupClass::staConfig(bool apMode,IPAddress ip,IPAddress gw, IPAddress nm){
	_settingApMode = apMode;
	_ip=ip;
	_gw=gw;
	_nm=nm;
}

void WiFiSetupClass::enterApMode(void)
{
	WiFi.mode(WIFI_AP);
	_apMode=true;
}

void WiFiSetupClass::setupApService(void)
{
	dnsServer.reset(new DNSServer());
	dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
	dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());
	delay(500);
}


void WiFiSetupClass::begin(char const *ssid,const char *passwd)
{
	wifi_info("begin:");

	_apName=ssid;
	_apPassword=passwd;

	if( _settingApMode){
		DBG_PRINTF("\nAP mode\n");
		WiFi.mode(WIFI_AP);
		_apMode=true;
	}else{
		DBG_PRINTF("\nAP_STA mode\n");
		WiFi.mode(WIFI_AP_STA);
		_apMode=false;
		if(_ip !=INADDR_NONE){
			WiFi.config(_ip,_gw,_nm);
		}

		if(_targetSSID== emptyString) WiFi.begin();
		else WiFi.begin(_targetSSID.c_str(),_targetPass==emptyString? NULL:_targetPass.c_str());
	}
	WiFi.softAP(_apName, _apPassword);
	setupApService();
	DBG_PRINTF("\ncreate network:%s pass:%s\n",ssid, passwd);
	
}

void WiFiSetupClass::staNetwork(const String& ssid,const String& pass){
	_targetSSID=ssid;
	_targetPass=pass;
}

bool WiFiSetupClass::connect(const String& ssid,const String& passwd){
	DBG_PRINTF("Connect to %s pass:%s\n",ssid.c_str(), passwd.c_str());

	_targetSSID=ssid;
	_targetPass=passwd;

	_wifiState = WiFiStateChangeConnectPending;
	if(_apMode){
		_apMode =false;
	}
	return true;
}

bool WiFiSetupClass::disconnect(void){
	DBG_PRINTF("Disconnect Request\n");
	_settingApMode =true;
	_wifiState = WiFiStateDisconnectPending;
	return true;
}

bool WiFiSetupClass::isConnected(void){
	return WiFi.status() == WL_CONNECTED;
}

void WiFiSetupClass::onConnected(){
	if(_eventHandler){
		String netstat;
		status(netstat);
		_eventHandler(netstat.c_str());
	}
}

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


void WiFiSetupClass::status(String& output){

/*	String ret;
	ret  = String("{\"ap\":") + String(_settingApMode? 1:0) + String(",\"con\":") + String((WiFi.status() == WL_CONNECTED)? 1:0);

	if(!_settingApMode){
		ret += String(",\"ssid\":\"") + WiFi.SSID() 
			 + String("\",\"ip\":\"") + IPAddress_String(_ip)
			 + String("\",\"gw\":\"") + IPAddress_String(_gw)
			 + String("\",\"nm\":\"") + IPAddress_String(_nm)
			 + String("\"");
	}

	ret += String("}");
	DBG_PRINTF("Status:%s\n",ret.c_str());
	return ret;
*/

	StaticJsonDocument<256> json;
	json["ap"] =_settingApMode? 1:0;
	json["con"] = (WiFi.status() == WL_CONNECTED)? 1:0;

	if(!_settingApMode){
		json["ssid"] = WiFi.SSID();
		json["ip"] = IPAddress_String(_ip);
		json["gw"] =  IPAddress_String(_gw);
		json["nm"] =  IPAddress_String(_nm);
	}

	serializeJson(json,output);
}

bool WiFiSetupClass::stayConnected(void)
{
	dnsServer->processNextRequest();
	if(! _apMode){
		if(_wifiState==WiFiStateChangeConnectPending){
			DBG_PRINTF("Change Connect\n");
			if(WiFi.getMode() == WIFI_AP){
				WiFi.mode(WIFI_AP_STA);
			}
			//if(WiFi.status() == WL_CONNECTED){
			WiFi.disconnect();
			//DBG_PRINTF("Disconnect\n");
			//}
			if(_ip != INADDR_NONE){
				WiFi.config(_ip,_gw,_nm);
			}
			WiFi.begin(_targetSSID.c_str(),_targetPass==emptyString? NULL:_targetPass.c_str());
			_time=millis();
			_reconnect =0;
			_wifiState = WiFiStateConnecting;

			wifi_info("**try:");

		}else if(_wifiState==WiFiStateDisconnectPending){
			WiFi.disconnect();
			WiFi.mode(WIFI_OFF);
			DBG_PRINTF("Enter AP Mode\n");
    		_apMode=true;
			WiFi.mode(WIFI_AP);	
			_wifiState =WiFiStateDisconnected;
			return true;
			
		}else if(WiFi.status() != WL_CONNECTED){
 			if(_wifiState==WiFiStateConnected)
 			{
				wifi_info("**disc:");

				_time=millis();
				DBG_PRINTF("Lost Network. auto reconnect %d\n",_autoReconnect);
				if(_autoReconnect){
					_wifiState = WiFiStateWaitToConnect;
				}else{
					_wifiState = WiFiStateDisconnected;
				}
				return true;
			}
			else if(_wifiState==WiFiStateWaitToConnect)
			{
				if((millis() - _time) > TIME_WAIT_TO_CONNECT)
				{
					WiFi.begin();
					_time=millis();
					_wifiState = WiFiStateConnecting;
					DBG_PRINTF("Reconnect...\n");
				}
			}
			else if(_wifiState==WiFiStateConnecting)
			{
				if((millis() - _time) > TIME_RECONNECT_TIMEOUT){
					_time=millis();
					_wifiState = WiFiStateWaitToConnect;
					_reconnect++;
					DBG_PRINTF("Reconnect fail\n");

					if(_switchToAp){
						if(_maxReconnect !=0 && _reconnect>=_maxReconnect){
							DBG_PRINTF("Fail to reconnect. Setup AP mode.\n");
 							enterApMode();
 							return true;
						}
					}
				}
			}
			// WiFiStateDisconnected else do nothing.
 		}
 		else
 		{
 			byte oldState=_wifiState;
 			_wifiState=WiFiStateConnected;
 			_reconnect=0;
 			if(oldState != _wifiState){
				onConnected();
				return true;
			}
  		}
	}
	
	if(_wifiScanState == WiFiScanStatePending){
		String nets;
		scanWifi(nets);
		_wifiScanState = WiFiScanStateNone;
		if(_eventHandler) _eventHandler(nets.c_str());
	}

	return false;
}

bool WiFiSetupClass::requestScanWifi(void) {
	if(_wifiScanState == WiFiScanStateNone){
		_wifiScanState = WiFiScanStatePending;
		return true;
	}
	return false;
}

void WiFiSetupClass::scanWifi(String& output) {
	
	output="{\"list\":[";
	
	DBG_PRINTF("Scan Networks...\n");
	int n = WiFi.scanNetworks();
    DBG_PRINTF("Scan done");
    if (n == 0) {
    	DBG_PRINTF("No networks found");
    } else {
      	//sort networks by RSSI
      	int indices[n];
      	for (int i = 0; i < n; i++) {
        	indices[i] = i;
    	}
      	// bubble sort
      	for (int i = 0; i < n; i++) {
        	for (int j = i + 1; j < n; j++) {
          		if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            		std::swap(indices[i], indices[j]);
          		}	
        	}
      	}

	    // remove duplicates ( must be RSSI sorted )
        String cssid;
        for (int i = 0; i < n; i++) {
        	if (indices[i] == -1) continue;
          	cssid = WiFi.SSID(indices[i]);
          	for (int j = i + 1; j < n; j++) {
            	if (cssid == WiFi.SSID(indices[j])) {
              		DBG_PRINTF("DUP AP: ");
					DBG_PRINTF(WiFi.SSID(indices[j]).c_str());
              		indices[j] = -1; // set dup aps to index -1
            	}
          	}
        }
		
      	//display networks in page
		bool comma=false; // i==0 might not the "first", might be duplicated.
      	for (int i = 0; i < n; i++) {
        	if (indices[i] == -1) continue; // skip dups
        	DBG_PRINTLN(WiFi.SSID(indices[i]));
	        DBG_PRINTLN(WiFi.RSSI(indices[i]));
        	//int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));
			String item=String("{\"ssid\":\"") + WiFi.SSID(indices[i]) + 
			String("\",\"rssi\":") + WiFi.RSSI(indices[i]) +
			String(",\"enc\":") + 
			#if ESP32			
			String((WiFi.encryptionType(indices[i]) != WIFI_AUTH_OPEN)? "1":"0")
			#else
			String((WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE)? "1":"0")
			#endif
			+ String("}");
			if(comma){
				output += ",";	
			}else{
				comma=true;
			}
			output += item;
      	}
    }
	output += "]}";
	DBG_PRINTF("scan result:%s\n",output.c_str());
}