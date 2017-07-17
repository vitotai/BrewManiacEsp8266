#ifndef AUTOMATION_H
#define AUTOMATION_H


#define MAXIMUM_HOP_NUMBER 10
#define MAXIMUM_STAGE_NUMBER 8


#define MAXIMUM_HSSESSION_NUMBER 5
#define MAXIMUM_HOP_IN_HSSESSION  5
#define MAXIMUM_POST_BOIL_HOP_NUMBER 10

#define AUTOMATION_FILE "/auto.json"


typedef struct _HopStandSession
{
    float startTemperature;
    float keepTemperature;
    uint8_t numberHops;
    uint8_t hopTime[MAXIMUM_HOP_IN_HSSESSION];
}HopStandSession;

class CAutomation
{
public:

    CAutomation(void){}

    bool load(void);
    void save(void);
    String json(void);

    float stageTemperature(uint8_t stage) { return _stageTemperatures[stage]; }
    void  setStageTemperature(uint8_t stage, float temp) { _stageTemperatures[stage] = temp; }
    uint8_t stageTime(uint8_t stage) { return _stageTimes[stage]; }
    void    setStageTime(uint8_t stage,uint8_t time){ _stageTimes[stage] = time;}

    uint8_t numberOfHops(void){ return _numberOfHops; }
    void   setNumberOfHops(uint8_t num) { _numberOfHops = num; }

    uint8_t boilTime(void) { return _boilTime; }
    void    setBoilTime(uint8_t time) { _boilTime = time; }

    uint8_t timeOfHop(uint8_t hop){ return _hopTimes[hop]; }
    void    setTimeOfHop(uint8_t hop, uint8_t time){ _hopTimes[hop] = time; }
    // hop stand
    uint8_t numberOfHopStandSession(void){return _numberOfHopStandSession;}
    void    setNumberOfHopStandSession(uint8_t num){_numberOfHopStandSession=num;}

    float  sessionStartTemperature(uint8_t hs){ return _hopStandSessions[hs].startTemperature; }
    void   setSessionStartTemperature(uint8_t hs,float temp){_hopStandSessions[hs].startTemperature = temp; }

    float  sessionKeepTemperature(uint8_t hs){ return _hopStandSessions[hs].keepTemperature;}
    void   setSessionKeepTemperature(uint8_t hs,float temp){_hopStandSessions[hs].keepTemperature=temp; }

    uint8_t  numberOfHopInSession(uint8_t hs){ return _hopStandSessions[hs].numberHops; }
    void     setNumberOfHopInSession(uint8_t hs,uint8_t num){ _hopStandSessions[hs].numberHops=num; }

    uint8_t  hopInSession(uint8_t hs,uint8_t hopIdx){ return _hopStandSessions[hs].hopTime[hopIdx];}
    void     setHopInSession(uint8_t hs,uint8_t hopIdx,uint8_t hop){ _hopStandSessions[hs].hopTime[hopIdx]=hop;}
    uint8_t  postBoilHopIndex(uint8_t hs,uint8_t hopIdx);

protected:
    uint8_t _boilTime;
    uint8_t _numberOfHops;
    uint8_t _hopTimes[MAXIMUM_HOP_NUMBER];
    uint8_t _stageTimes[MAXIMUM_STAGE_NUMBER];
    float   _stageTemperatures[MAXIMUM_STAGE_NUMBER];

    uint8_t  _numberOfHopStandSession;
    HopStandSession  _hopStandSessions[MAXIMUM_HSSESSION_NUMBER];

    size_t formatJson(char* buffer,size_t size);
};

extern CAutomation automation;

#endif
