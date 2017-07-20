#ifndef BrewLogger_H
#define BrewLogger_H
#include <FS.h>

#include "TimeKeeper.h"

#define INVALID_RECOVERY_TIME 2147483647

#define BREWING_TMPFILE "/brewing.tmp"

#define LOG_PATH "/log"
#define LOG_RECORD_FILE "/logrecord"
#define MAX_FILE_NUMBER 20
#define MAX_FILE_INDEX 9999

#define LogBufferSize 1024
//#define LogBufferSize 128


#define EventTag 0xF2
#define StageTag 0xF1
#define SetPointTag 0xF3

#define StartLogTag 0xFF
#define ResumeBrewTag 0xFE

typedef struct _FileInfo{
		int16_t index;
		uint32_t time;
} FileInfo;

class BrewLogger
{

public:
	BrewLogger(void){
		_started=false;
		_stage=101;
		_isFileOpen=false;
		_sensorNumber=0;
		_Indexloaded=false;
		_spChanged=false;
	}
	// Session related:
	bool checkRecovery(void){ return SPIFFS.exists(BREWING_TMPFILE);}
	void clearRecovery(void){ if(checkRecovery()) SPIFFS.remove(BREWING_TMPFILE);}

	void resumeSession(uint8_t *pStage,uint32_t *pTime)
	{
		_tmpFile=SPIFFS.open(BREWING_TMPFILE,"a+");
		size_t fsize= _tmpFile.size();
		size_t rsize;
		_savedLength=fsize;

		initProcessingResume();
		while( fsize > 0){
			rsize= _tmpFile.readBytes(_logBuffer,LogBufferSize);
			// process the content to get correct stage & time
			processSavedData(_logBuffer,rsize);

			fsize -= rsize;
		}
		endProcessingResume();
		// _sensorNumber, _tempLogPeriod,_starttime will derive from the saved data
		_logIndex=rsize;
		_savedLength -= rsize;  // data length in file exclude those in buffer
		// log a "new start" log
		DBG_PRINTF("resume, total _savedLength:%d, _logIndex:%d\n",_savedLength,_logIndex);
		_saveLog = true;
		_lastTempLog=0;
		_started = true;
		_isFileOpen=false;
		// add resume tag
		addResumeTag();
		// return value
		*pStage = _stage;
		*pTime= _time;
	}

	void startSession(uint8_t sensors, unsigned long period,bool fahrenheit, bool saved=true){
		if(_started) return; // alread start

		_saveLog=saved;

		if(_saveLog){
			_tmpFile=SPIFFS.open(BREWING_TMPFILE,"w");
			if(!_tmpFile){
				DBG_PRINTF("Error open temp file\n");
				return;
			}
		}
		_tempLogPeriod=period;
		_starttime= TimeKeeper.getTimeSeconds();
		_stage = 101;
		_logIndex = 0;

		_lastTempLog=0;
		_started = true;
		_savedLength=0;
		_sensorNumber=sensors;
		startLog(fahrenheit);
	}

	void abortSession(void){
		if(!_started) return;
//		if(_stage >8){ // boil finished. just end normally
//			endSession();
//		}
		_started=false;
		_tmpFile.close();
	}

	void endSession(void){
		if(!_started) return;
		_started=false;
		if(_saveLog){
			_tmpFile.close();
			int fidx=newLogFileIndex();
			char buf[32];
			createFilename(buf,fidx);
			SPIFFS.rename(BREWING_TMPFILE,buf);
		}
	}
	// tracking
	void setPoint(float sp){
		_spChanged=true;
		_lastSetPoint=sp;
	}

	void temperature(float temp){
		if(!_started) return;
		unsigned long miliseconds = millis();
		if((miliseconds -_lastTempLog) >= _tempLogPeriod){
			if(_spChanged){
				_spChanged=false;
				addSetPoint();
			}
			addTemperature(temp);
			_lastTempLog= miliseconds;
		}
	}

	void temperatures(float* temps){
		if(!_started) return;
		unsigned long miliseconds = millis();
		if((miliseconds -_lastTempLog) >= _tempLogPeriod){
			if(_spChanged){
				_spChanged=false;
				addSetPoint();
			}
			for(byte i=0;i< _sensorNumber;i++)
				addTemperature(temps[i]);

			_lastTempLog= miliseconds;
		}
	}

	void event(byte event){
		if(!_started) return;
		addEvent( event);
	}

	void stage(byte s){
		if(!_started) return;
		if(_stage==s) return;
		_stage=s;
		addStage(s);
	}

	size_t beginCopyAfter(int last)
	{
		_readStart = last;
		//DBG_PRINTF("beginCopyAfter:%d, _logIndex=%ld, saved=%ld last >= (_logIndex +_savedLength)=%c\n",last,_logIndex,_savedLength, (last >= (_logIndex +_savedLength))? 'Y':'N' );
		if(last >= (_logIndex +_savedLength)) return 0;
		return ( _logIndex+_savedLength - last);
	}

	size_t read(uint8_t *buffer, size_t maxLen, size_t index)
	{
		size_t sizeRead;
		size_t rindex= index + _readStart;
		//DBG_PRINTF("read index:%d, rindex=%ld\n",index,rindex);

		if(rindex > (_savedLength +_logIndex)) return 0;

		if( rindex < _savedLength){
			//DBG_PRINTF("read from file\n");
			// read from file
			if(!_isFileOpen){
				_file=SPIFFS.open(BREWING_TMPFILE,"r");
				if(!_file){
					DBG_PRINTF("error open file\n");
					return 0;
				}
				_isFileOpen=true;
				//DBG_PRINTF("file opened.\n");
				if(rindex !=0) _file.seek(rindex,SeekSet);
			}
			sizeRead = _savedLength - rindex;
			if(sizeRead > maxLen) sizeRead=maxLen;
			sizeRead=_file.read(buffer,sizeRead);

			if((sizeRead+ rindex) >= _savedLength ){
				_file.close();
				_isFileOpen=false;
//				DBG_PRINTF("file closed.\n");
			}
		}else{
			//DBG_PRINTF("read from buffer\n");
			// read from buffer
			rindex -=  _savedLength;
			// rindex should be smaller than _logIndex
			sizeRead = _logIndex - rindex;
			if(sizeRead > maxLen) sizeRead=maxLen;
			memcpy(buffer,_logBuffer+rindex,sizeRead);
		}
		//DBG_PRINTF("sizeRead:%ld\n",sizeRead);
		return sizeRead;
	}

	FileInfo* getLogFileInfo(void)
	{
		loadIdxFile();
		return _fileInfo;
	}

	void createFilename(char *buf,int idx)
	{
		sprintf(buf,"%s/%04d",LOG_PATH,idx);
	}


private:
	uint8_t _sensorNumber;
	size_t  _tempLogPeriod;
	size_t _starttime;
	size_t _lastTempLog;
	uint32_t _time;

	byte _stage;
	bool _started;

	int _logIndex;
	int _savedLength;
	int _readStart;

	char _logBuffer[LogBufferSize];
	File _file;
	bool _isFileOpen;
 	FileInfo _fileInfo[MAX_FILE_NUMBER];
	bool _Indexloaded;

	float _lastSetPoint;
	bool _spChanged;

	File    _tmpFile;
	//
	bool _saveLog;

	void startLog(bool fahrenheit)
	{
		char *ptr=_logBuffer;
		// F0FF  peroid   4 bytes
		// Start system time 4bytes
		*ptr++ = StartLogTag;
		*ptr++ = _sensorNumber | (fahrenheit? 0xF0:0xE0) ;

		int period = _tempLogPeriod/1000;
		*ptr++ = (char) (period >> 8);
		*ptr++ = (char) (period & 0xFF);
		*ptr++ = (char) (_starttime >> 24);
		*ptr++ = (char) (_starttime >> 16);
		*ptr++ = (char) (_starttime >> 8);
		*ptr++ = (char) (_starttime & 0xFF);
		_logIndex=8;
		_isFileOpen=false;
		_savedLength=0;

		writeToFile(_logBuffer,_logIndex);

		//DBG_PRINTF("*startLog*\n");
	}

	char *allocByte(byte size)
	{
		if((_logIndex+size) >= LogBufferSize){
			DBG_PRINTF("buffer full!\n");
			if(_saveLog) {
				_savedLength += _logIndex;
				_logIndex =0;
			}else{
				return NULL;
			}
		}
//		if((_logIndex+size) >= LogBufferSize) return NULL;
		char *ptr=_logBuffer + _logIndex;
		_logIndex += size;
		return ptr;
	}

	void addSetPoint(void){
		char *ptr = allocByte(4);
		if(ptr == NULL) return;
		*ptr = SetPointTag;
		*(ptr+1)=0;

		int spi=(int) (_lastSetPoint * 100.0);
		spi = spi | 0x8000;
		*(ptr+2) =(char) (spi >> 8);
		*(ptr+3) =(char)(spi & 0xFF);
		writeToFile(ptr,4);
	}

	void addTemperature(float temp){
		char *ptr = allocByte(2);
		if(ptr == NULL) return;
		int temp_int=(int)(temp * 100.0);
		// assume temp is smaller than 300, -> maximum temp *100= 30000 < 32767
		//DBG_PRINTF("add temperature:%d\n",temp_int);

		if(temp_int > 25000){
			*ptr = 0x7F;
			*(ptr+1) = 0xFF;
		}else{
			*ptr = (char)((temp_int >> 8) & 0x7F);
			*(ptr+1) =(char)(temp_int & 0xFF);
		}

		writeToFile(ptr,2);
	}

	void addEvent(byte event){
		char *ptr = allocByte(2);
		if(ptr == NULL) return;
//		DBG_PRINTF("**add event:%d **\n",event);
		*ptr = EventTag;
		*(ptr+1)= event;
		writeToFile(ptr,2);
	}

	void addStage(byte s){
		char *ptr = allocByte(2);
		if(ptr == NULL) return;
//		DBG_PRINTF("**add stage:%d **\n",s);
		*ptr = StageTag;
		*(ptr+1)= s;

		writeToFile(ptr,2);
	}

	void addResumeTag(void)
	{
		char *ptr = allocByte(2);
		if(ptr == NULL) return;
		*ptr = ResumeBrewTag;
		size_t rtime= TimeKeeper.getTimeSeconds();
		 *(ptr+1)=(uint8_t)((rtime - _starttime)/60);
		 writeToFile(ptr,2);
	}

	size_t _tempCount;
	size_t _timerStart;
	size_t _paused;
	size_t _pauseSum;
	bool   _timeRunning;

	void initProcessingResume(void)
	{
		_tempCount=0;
		_timeRunning=false;
		_pauseSum=0;
	}

	void endProcessingResume(void)
	{
		if(!_timeRunning){
			_time = INVALID_RECOVERY_TIME;
			DBG_PRINTF("timer not started\n");
		}else{
			int elapse=_tempCount - _timerStart - _pauseSum;
			int ticks = elapse / _sensorNumber;

			_time=ticks * ((_tempLogPeriod/1000) / 60);
			DBG_PRINTF("timer: %d, ticks:%d \n",_time,ticks);

		}
	}

	void processSavedData(char* data,size_t len)
	{
		char* ptr=data;

		while(ptr < (data +len)){

			uint8_t b1= *ptr++;
			uint8_t b2= *ptr++;

			if(b1 == StartLogTag){
				_sensorNumber = b2 & 0x07;
				_tempLogPeriod = ((*ptr << 8) | *(ptr+1)) * 1000;
				ptr +=2;
				_starttime = (*ptr <<24) | ( *(ptr+1) << 16) | ( *(ptr+2) << 8) | *(ptr+3);
				ptr +=4;

				DBG_PRINTF("Resume: sensor:%d, period:%d, start:%d\n",_sensorNumber,_tempLogPeriod,_starttime);
			}else if(b1 == StageTag){
				_stage = b2;
				DBG_PRINTF("%ld - stage %d\n",ptr - data,_stage);

				if(_stage == 12){ //StageHopStand
				    if(! _timeRunning){
        				_timeRunning=true;
        				_timerStart= _tempCount;
        				DBG_PRINTF("hopstand :%ld\n",_timerStart);
        			}else{
        			    _pauseSum += _tempCount - _paused;
        			    DBG_PRINTF("hopstandchill end:%ld\n",_pauseSum);
        			}
				}else if(_stage == 11){ //StageHopStandChill
				    if(_timeRunning){
    				    _paused = _tempCount;
                    }
                    DBG_PRINTF("hopstandchill :%ld\n",_timerStart);
				}else{
    				_timeRunning=false;
	    			_pauseSum=0;
				}

			}else if(b1 == EventTag){
				if(b2 == 1){ //RemoteEventTemperatureReached
					_timerStart= _tempCount;
					_timeRunning=true;
				} else if(b2 == 5){ // pause
					_paused = _tempCount;
					//DBG_PRINTF("paused @ %d\n",_paused);
				} else if(b2 == 6){ // resume
					_pauseSum += _tempCount - _paused;
					//DBG_PRINTF("resume @ %d. sum=Td\n",_tempCount,_pauseSum);
				}else if (b2 == 10){ //RemoteEventBoilFinished
    				_timeRunning=false;
	    			_pauseSum=0;
				}
			}else if(!(b1 & 0x80)){
				// temperature reading
				_tempCount ++;
			}
		}

	}

	void writeToFile(char* buf,int len)
	{
		if(!_saveLog) return;
		_tmpFile.write((const uint8_t*)buf,len);
		_tmpFile.flush();
	}


	void loadIdxFile(void)
	{
		if(_Indexloaded) return;
		// load index
		File idxFile= SPIFFS.open(LOG_RECORD_FILE,"r+");
		if(idxFile){
			idxFile.readBytes((char*)&_fileInfo,sizeof(_fileInfo));
			idxFile.close();
			//DBG_PRINTF("Load index from file\n");
		}else{
			for(uint8_t i=0;i<MAX_FILE_NUMBER;i++){
				_fileInfo[i].index = -1;
			}
			//DBG_PRINTF("initialize file index\n");
		}
		_Indexloaded=true;
	}

	void saveIdxFile(void)
	{
		File idxFile= SPIFFS.open(LOG_RECORD_FILE,"w+");
		if(idxFile){
			idxFile.write((uint8_t*)&_fileInfo,sizeof(_fileInfo));
			idxFile.close();
			//DBG_PRINTF("save file index\n");
		}
	}

	int newLogFileIndex(void)
	{
		loadIdxFile();
		// find empty or latest one
		uint8_t i;
		for(i=0;i<MAX_FILE_NUMBER;i++){
			if(_fileInfo[i].index == -1){
				break;
			}
		}
		int16_t nidx=0;
		if( i < MAX_FILE_NUMBER){
			// find empty slot
			if(i==0) nidx=1; // start from 1
			else nidx=_fileInfo[i-1].index +1;
			//DBG_PRINTF("File index has empty slot %d, idx:%d\n",i,nidx);
		}else{
			// no empty slot
			char buf[32];
			createFilename(buf,_fileInfo[0].index);
			if(SPIFFS.exists(buf)){
				SPIFFS.remove(buf);
			}
			for(i=1;i<MAX_FILE_NUMBER;i++){
				_fileInfo[i-1].index=_fileInfo[i].index;
				_fileInfo[i-1].time=_fileInfo[i].time;
			}
			i=MAX_FILE_NUMBER-1;
			nidx=_fileInfo[i-1].index + 1;

			//DBG_PRINTF("File index DON'T have empty slot %d, idx:%d\n",i,nidx);
		}
		if(nidx > MAX_FILE_INDEX) nidx =1;
		_fileInfo[i].index = nidx;
		_fileInfo[i].time = _starttime;
		saveIdxFile();
		return nidx;
	}

};

extern BrewLogger brewLogger;
#endif
