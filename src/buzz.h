/**********************************************************************
 BrewManiac
 created by Vito Tai
 Copyright (C) 2015 Vito Tai

 This soft ware is provided as-is. Use at your own risks.
 You are free to modify and distribute this software without removing
 this statement.
 BrewManiac by Vito Tai is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
***********************************************************************/

#ifndef BUZZ_H
#define BUZZ_H

#define BUZZER_TIME_TOLERANCE 5

typedef byte SoundId;


// for one shot sound
#define SoundIdTemperatureReached 0
#define SoundIdCountDown 1
#define SoundIdAddHop 2
#define SoundIdWaitUserInteraction 3
#define SoundIdBrewEnd 4

#define SoundIdMashInTempReached SoundIdTemperatureReached
#define SoundIdBoil SoundIdTemperatureReached
#define SoundIdDelayTimeout SoundIdTemperatureReached
#define SoundIdIodineTest  SoundIdTemperatureReached
#define SoundIdAutoTuneFinished SoundIdAddHop
#define SoundIdUserInteractiveNeeded 0
#define SoundIdConfirmUser 0


#define PumpRestSoundId    SoundIdTemperatureReached
#define PumpRestEndSoundId  SoundIdTemperatureReached

#define SoundIdWarnning SoundIdTemperatureReached

const byte _soundIdTemperatureReached[] PROGMEM ={5,10,3,10,3,10};
const byte _soundIdCountDown[] PROGMEM ={9,20,20,20,20,20,20,20,20,40}; // 5 seconds
//HOP_ALTERTING_TIME
const byte _soundIdAddHop[] PROGMEM ={19,30,10,30,10,30,10,30,10,30,10,30,10,30,10,30,10,30,10,40}; // 10 seconds
const byte _soundIdWaitUserInteraction[] PROGMEM ={2,10,30};
const byte _soundIdBrewEnd[] PROGMEM ={2,37,7};
const byte * const _sounds [] PROGMEM ={
_soundIdTemperatureReached,
_soundIdCountDown,
_soundIdAddHop,
_soundIdWaitUserInteraction,
_soundIdBrewEnd};

// a hit & run function
// buzzer thread will handle the pattern & time
byte _numberofNtesToPlay;
boolean _repeat;
boolean _playing;
boolean _buzzing;
unsigned long _buzzingTime;
word _currentPeriod;
byte* _ptrCurrentNote;

byte* _currentSound;

void buzzStartPlay(SoundId id, boolean repeat)
{
	#ifdef ESP8266
	_currentSound=(byte *)pgm_read_dword(&(_sounds[id]));
	#else
	_currentSound=(byte *)pgm_read_word(&(_sounds[id]));
	#endif

	_numberofNtesToPlay = pgm_read_byte(_currentSound);

	_ptrCurrentNote =_currentSound;

	_ptrCurrentNote ++;
	_currentPeriod = (word)pgm_read_byte(_ptrCurrentNote) *25;

	_repeat = repeat;
	_playing=true;

	_buzzingTime = millis();
	setBuzzOut(HIGH);
	_buzzing=true;
}

void buzzPlaySound(SoundId id)
{
	buzzStartPlay(id,false);
}

// play the sound until mute() is called
void buzzPlaySoundRepeat(SoundId id)
{
	buzzStartPlay(id,true);
}

void buzzMute(void)
{
	setBuzzOut(LOW);
	_playing=false;

}

void buzzOn(void)
{
	setBuzzOut(HIGH);
}

void buzzThread(void)
{
	if(!_playing) return;

	if((gCurrentTimeInMS-_buzzingTime) >= (_currentPeriod + BUZZER_TIME_TOLERANCE))
	{
		_numberofNtesToPlay --;
		if(_numberofNtesToPlay ==0)
		{
			if(_repeat)
			{
				_ptrCurrentNote =_currentSound;
				_numberofNtesToPlay = pgm_read_byte(_currentSound);
			}
			else
			{
				// finished, stop
				buzzMute();
				return;
			}
		}
		_buzzingTime = millis();
		//else
		_ptrCurrentNote ++;
		_currentPeriod = (word)pgm_read_byte(_ptrCurrentNote) *25;

		if(_buzzing)
		{
			setBuzzOut(LOW);
		}
		else
		{
			setBuzzOut(HIGH);
		}
		_buzzing = ! _buzzing;
	}
}

#endif
