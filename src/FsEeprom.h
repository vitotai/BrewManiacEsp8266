#ifndef FSEEPROM_H
#define FSEEPROM_H

class FsEEPROMClass
{
	char* _eeprom;
	int  _size;
	bool _dirty;
public:
	FsEEPROMClass(void):_size(0),_dirty(false){}

	void begin(int size);

	void commit(bool force=false);

	byte read(int address);

	bool write(int address,byte value);

	char* data(void){return _eeprom;}
	int   size(void){ return _size; }
};

extern FsEEPROMClass FsEEPROM;

#endif
