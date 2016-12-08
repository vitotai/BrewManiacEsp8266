# BrewManiac
an Open ArdBir alternative.

BrewManiac is an ARDUINO sketch that minics Open ArdBir. 
BrewManiac is crated from scratch but its interface and behavior is almost the same as Open ArdBir.
I wanted to add wireless capability to Open ArdBir but ended up learning that writing a new sketch might be simpler.
The main difference between Open ArdBir and BrewManiac is the program structure. 
BrewManiac is event-driven while Open Ardbir is mor like a series of functoins.

Current implementation supports HM10, a BLE transceiver.

Usage:
1.download necessary library, including
EEPROM
OneWire
PID_v1
SoftwareSerial
LiquidCrystal
Wire (for I2C LCD)
LiquidCrystal_I2C (for I2C LCD)

2.downlad the code and open the main sketch file to modify the hardware setting.

(to Be Continue...)
