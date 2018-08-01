#ifndef SPIFFSEEPROM_H
#define SPIFFSEEPROM_H

class SpiffsEEPROMClass
{
	char* _eeprom;
	int  _size;
	bool _dirty;
public:
	SpiffsEEPROMClass(void):_size(0),_dirty(false){}

	void begin(int size);

	void commit(bool force=false);

	byte read(int address);

	bool write(int address,byte value);

	char* data(void){return _eeprom;}
	int   size(void){ return _size; }
};

extern SpiffsEEPROMClass SpiEEPROM;

#endif
