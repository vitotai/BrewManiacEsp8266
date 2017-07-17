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

	void commit(void);

	byte read(int address);

	bool write(int address,byte value);
};

extern SpiffsEEPROMClass SpiEEPROM;

#endif
