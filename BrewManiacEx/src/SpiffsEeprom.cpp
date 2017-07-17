#include <FS.h>
#include "SpiffsEeprom.h"

#define EEPROM_FILENAME "/eeprom.bin"


SpiffsEEPROMClass SpiEEPROM;

void SpiffsEEPROMClass::begin(int size)
{
	_eeprom=(char*) malloc(size);

	if(!_eeprom) return;

	_size=size;

	if(SPIFFS.exists(EEPROM_FILENAME)){
		File f=SPIFFS.open(EEPROM_FILENAME,"r+");
		if(f){
//			Serial.printf("read EEPROM from file\n");
			f.readBytes(_eeprom,_size);
			f.close();
		}
	}else{
//		Serial.printf("EEPROM file not exit\n");
	}
}

void SpiffsEEPROMClass::commit(void){
		if(!_dirty) return;

//		Serial.printf("commit EEPROM\n");

		File f=SPIFFS.open(EEPROM_FILENAME,"w+");
		if(f){
//			Serial.printf("write EEPROM to file\n");
			f.write((const uint8_t*)_eeprom,_size);
			f.close();
			_dirty=false;
		}
}

byte SpiffsEEPROMClass::read(int address){
	if(address > _size) return 0;
	return (byte)_eeprom[address];
}

bool SpiffsEEPROMClass::write(int address,byte value){
		if(address > _size) return false;
		if(_eeprom[address] == value) return false;
		_eeprom[address] = value;
		_dirty=true;
		return true;
}
