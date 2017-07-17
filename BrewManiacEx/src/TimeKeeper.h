#ifndef TimeKeeper_H
#define TimeKeeper_H

class TimeKeeperClass
{
public:
	TimeKeeperClass(void):_referenceSeconds(0),_referenceSystemTime(0){}
	void begin(const char* server1,const char* server2,const char* server3);
	void begin(bool useSaved=false);

	time_t getTimeSeconds(void); // get Epoch time
	const char *getDateTimeStr(void);

	void setInternetAccessibility(bool connected){ _online=connected; }
	void setCurrentTime(time_t current);
private:
	time_t _referenceSeconds;
	time_t _referenceSystemTime;
	bool _online;

	time_t _lastSaved;
	void saveTime(time_t t);
	time_t loadTime(void);
};

extern TimeKeeperClass TimeKeeper;

#endif
