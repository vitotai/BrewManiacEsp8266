
//********************************************************
//* for update temperature and time on screen
//********************************************************
#define COUNTING_PAUSE 0
#define COUNTING_UP 1
#define COUNTING_DOWN 2

#define TemperatureMainScreenPosition 0
#define TemperatureManualModePosition 1
#define TemperaturePausePosition 2
#define TemperatureAutoModePosition TemperatureManualModePosition

#define RunningTimeShowCycle 700
#define RunningTimeHideCycle 300

float _lastShownTemperature=0;
float _lastShownAuxTemperature=0;

#if MaximumNumberOfSensors > 1


const byte TemperatureDisplayPos[] PROGMEM=
{
	2,1,  // Idle/Main screen
	1,1,  // Auto mode & Manual mode
	3,0   // Paused mode
};

const byte AuxTemperatureDisplayPos[] PROGMEM=
{
	10,1,  // Idle/Main screen
	1,2,  // Auto mode & Manual mode
	10,0   // Paused mode
};

#else
const byte TemperatureDisplayPos[] PROGMEM=
{
	6,1,  // Idle/Main screen
	1,1,  // Auto mode & Manual mode
	7,0   // Paused mode
};
#endif

const byte RunningTimeDisplayPosition[] PROGMEM=
{
	11,2,
	6,2,
};

#define RunningTimeNormalPosition 0
#define RunningTimePausePosition 1
#define RunningTimeDelayInputPosition 1


byte _uiTpDisplayRow;
byte _uiTpDisplayCol;
boolean _uiDisplayTemperature;

#if MaximumNumberOfSensors > 1
byte _uiAuxTpDisplayRow;
byte _uiAuxTpDisplayCol;
#endif

byte _countingTimeDirection;
unsigned long _countingTimeRef;
unsigned long _countingTimeDisplay;
byte _runningTimeRow;
byte _runningTimeCol;

boolean _runningTimeHidden;

boolean _runningTimeBlinking;
boolean _runningTimeBlinkShow;

unsigned long _runningTimeBlinkTime;


int uiGetDisplayTime(void){
	return (int)_countingTimeDisplay;
}

bool uiIsTimerRunning(void)
{
	return _countingTimeDirection != COUNTING_PAUSE;
}

bool uiIsTimerRunningUp(void)
{
	return _countingTimeDirection == COUNTING_UP;
}


void uiTempDisplayHide(void)
{
	_uiDisplayTemperature=false;
}


void uiTempDisplaySetPosition(byte index)
{
   	_uiDisplayTemperature=true;
	_lastShownTemperature=INVALID_TEMP_C; // for redraw

   _uiTpDisplayRow=pgm_read_byte_near(& TemperatureDisplayPos[index*2 +1]);
   _uiTpDisplayCol=pgm_read_byte_near(& TemperatureDisplayPos[index*2]);

 #if MaximumNumberOfSensors > 1
	_lastShownAuxTemperature=INVALID_TEMP_C; // for redraw

   _uiAuxTpDisplayRow=pgm_read_byte_near(& AuxTemperatureDisplayPos[index*2 +1]);
   _uiAuxTpDisplayCol=pgm_read_byte_near(& AuxTemperatureDisplayPos[index*2]);
 #endif
}

//****************************
// running time

void uiRunningTimeStop(void)
{
	_countingTimeDirection = COUNTING_PAUSE;
}

void uiRunningTimePrint(unsigned long timeInSeconds)
{
	char buffer[10]; // using 10 bytes wan't too bad.
	unsigned long hour = timeInSeconds / (60*60);
	int dec=(hour/10);

	buffer[0]= (char) (dec<10)?(dec + '0'):(dec + 'A');
	buffer[1]= (char)((hour%10) + '0');
	buffer[2]=':';

	unsigned long minute =(timeInSeconds - hour * 60*60)/60;

	buffer[3]= (char)((minute/10) + '0');
	buffer[4]= (char)((minute%10) + '0');
	buffer[5]=':';

	unsigned long seconds=timeInSeconds - hour*60*60 - minute*60;

	buffer[6]=(char)((seconds/10) + '0');
	buffer[7]=(char)((seconds%10) + '0');
	buffer[8]=0;

	uiLcdPrint(_runningTimeCol,_runningTimeRow,buffer);
}

void uiRunningTimeHide(boolean hide)
{
	_runningTimeHidden = hide;
	if(! hide)
	{
		uiRunningTimePrint(_countingTimeDisplay);
	}
}

void uiRunningTimeSetPosition(byte pos)
{
   _runningTimeRow=pgm_read_byte_near(& RunningTimeDisplayPosition[pos*2 +1]);
   _runningTimeCol=pgm_read_byte_near(& RunningTimeDisplayPosition[pos*2]);
}

void uiRunningTimeShowInitial(unsigned long initialValue)
{
	_countingTimeDirection = COUNTING_PAUSE;
	_countingTimeDisplay = initialValue;
	_runningTimeHidden= false;
	uiRunningTimePrint(initialValue);
}

void uiRunningTimeStart(void)
{
	// use reference to note time.
	_countingTimeRef=gCurrentTimeInSec;
	_countingTimeDirection = COUNTING_UP;
}

void uiRunningTimeStartFrom(unsigned long start)
{
	// use reference to note time.
	_countingTimeRef=gCurrentTimeInSec - start;
	_countingTimeDirection = COUNTING_UP;
}


void uiRunningTimeStartCountDown(unsigned long seconds)
{
	_countingTimeRef=gCurrentTimeInSec + seconds;
	_countingTimeDirection = COUNTING_DOWN;
}


void uiRunningTimeBlank(void)
{
	uiLcdClear(_runningTimeCol,_runningTimeRow,8);
}

void uiRunningTimeBlink(boolean blink)
{
	if(blink && !_runningTimeBlinking)
	{
		_runningTimeBlinkTime = gCurrentTimeInMS;
	}
	_runningTimeBlinking = blink;

	if(! _runningTimeBlinking && ! _runningTimeBlinkShow)
		uiRunningTimePrint(_countingTimeDisplay);
}

void uiPrintTemperature(byte col, byte row, float displayTemp)
{
	char buffer[8];
	byte indent;

    if(IS_TEMP_INVALID(displayTemp))
    {
        indent = 2;
        for(byte i=0;i<4;i++) buffer[i]='-';
        buffer[4]='\0';
    }
    else
    {
       	byte digitNum=sprintFloat(buffer,displayTemp,2);
		buffer[digitNum]='\0';
        indent = 6 - digitNum;
	}
	uiLcdClear(col,row,6);
	uiLcdPrint(col + indent, row,buffer);
}

void uiDisplayTemperatureAndRunningTime(void)
{
    if(_uiDisplayTemperature && (
    (_lastShownTemperature != gCurrentTemperature)
#if MaximumNumberOfSensors > 1
	|| (_lastShownAuxTemperature != gAuxTemperature)
#endif
    ))
    {
    	_lastShownTemperature = gCurrentTemperature;
#if MaximumNumberOfSensors > 1
			_lastShownAuxTemperature = gAuxTemperature;
#endif

	    uiPrintTemperature(_uiTpDisplayCol,_uiTpDisplayRow,gCurrentTemperature);
		uiLcdDrawSymbol(_uiTpDisplayCol +6,_uiTpDisplayRow,LcdCharDegree);
#if MaximumNumberOfSensors > 1
	    uiPrintTemperature(_uiAuxTpDisplayCol,_uiAuxTpDisplayRow,gAuxTemperature);
		uiLcdDrawSymbol(_uiAuxTpDisplayCol +6,_uiTpDisplayRow,LcdCharDegree);
#endif

    }
			if(_runningTimeBlinking)
			{
				if(_runningTimeBlinkShow)
				{
					if((gCurrentTimeInMS - _runningTimeBlinkTime) > RunningTimeShowCycle)
					{
						_runningTimeBlinkTime=gCurrentTimeInMS;
						_runningTimeBlinkShow = false;
						//hide
						uiRunningTimeBlank();
					}
				}
				else
				{
					if((gCurrentTimeInMS - _runningTimeBlinkTime) > RunningTimeHideCycle)
					{
						_runningTimeBlinkTime=gCurrentTimeInMS;
						_runningTimeBlinkShow = true;
						// shown
    					uiRunningTimePrint(_countingTimeDisplay);
					}
				}

			}

    if(_countingTimeDirection != COUNTING_PAUSE)
    {
    	unsigned long count;
    	if (_countingTimeDirection == COUNTING_UP)
    	{
    		count=gCurrentTimeInSec - _countingTimeRef;
    	}
    	else
    	{
    		// counting down
    		count=_countingTimeRef - gCurrentTimeInSec;
    		if(count > 86400000L)
    		{
    			count =0;
    			_countingTimeDirection = COUNTING_PAUSE;
    		}
    	}

	 	if(! _runningTimeHidden)
	 	{
	 		if(count != _countingTimeDisplay)
    		{
    			_countingTimeDisplay=count;
    			uiRunningTimePrint(_countingTimeDisplay);
    		}

    	}
    }
}
