#ifndef PINS_H
#define PINS_H

#include <pcf8574_esp.h>

/*
#define NODEMCU_PIN_A0 17
#define NODEMCU_PIN_D0 16	Buzzer
#define NODEMCU_PIN_D1 5	 [I/O]SCL // I2C
#define NODEMCU_PIN_D2 4	 [I/O]SDA // I2C
#define NODEMCU_PIN_D3 0
#define NODEMCU_PIN_D4 2
#define NODEMCU_PIN_D5 14    Pump
#define NODEMCU_PIN_D6 12    Sensor
#define NODEMCU_PIN_D7 13    Heater
#define NODEMCU_PIN_D8 15    AuxHeater
*/

#define BUTTON_USE_EXT true

#define HEATER_USE_EXT false
#define PUMP_USE_EXT false
#define BUZZER_USE_EXT false

//#define PUMP_INVERTED_LOGIC true
// with all address pins grounded, PCF8574 is 0x20 while pCF8574A is 0x38
#define PCF8574_ADDRESS 0x20
//#define PCF8574_ADDRESS 0x38

//Using ESP8266 PINs as input seems to be a good idea

#define I2C_SCL NODEMCU_PIN_D1
#define I2C_SDA NODEMCU_PIN_D2
#define SensorPin    NODEMCU_PIN_D6


#define AuxHeatControlPin NODEMCU_PIN_D8

// the following pin are group into Output & Input
// Input is for button, while output is for heater,pump, and buzzer.

#if BUTTON_USE_EXT !=true
#define ButtonUpPin    NODEMCU_PIN_D3
#define ButtonDownPin   NODEMCU_PIN_D4
#define ButtonStartPin  NODEMCU_PIN_D5
#define ButtonEnterPin  NODEMCU_PIN_D7
#else

#define ButtonUpPin    2 // P1
#define ButtonDownPin  1 // p0 NODEMCU_PIN_D4
#define ButtonStartPin  8 //P3 NODEMCU_PIN_D5
#define ButtonEnterPin  4 //P2 NODEMCU_PIN_D6

#endif

#if PUMP_USE_EXT != true
#define PumpControlPin  NODEMCU_PIN_D5
#else
#define ExPumpControlPin  5
#endif

#if HEATER_USE_EXT != true
#define HeatControlPin  NODEMCU_PIN_D7
#else
#define ExHeatControlPin  7
#endif

#if BUZZER_USE_EXT != true
#define BuzzControlPin NODEMCU_PIN_D0
#else
#define ExBuzzControlPin 6
#endif


#if (BUTTON_USE_EXT == true) || (HEATER_USE_EXT == true) || (PUMP_USE_EXT == true) ||( BUZZER_USE_EXT == true)
PCF8574 pcf8574(PCF8574_ADDRESS,I2C_SDA, I2C_SCL);
#endif

// Buttons are INPUT
// byte btnReadPin(byte p){ return digitalRead(p);}

#if BUTTON_USE_EXT !=true

void btnPrepareRead(void){}
#define btnReadPin digitalRead

#else

byte _portvalue;
void btnPrepareRead(void)
{
	_portvalue=pcf8574.read8();
}

byte btnReadPin(byte pin)
{
	return (_portvalue & pin);
}

#endif

// Heater, Pump, Buzz are OUTPUTs
inline void setHeaterOut(byte v)
{
#if HEATER_USE_EXT != true
	digitalWrite (HeatControlPin, v);
#else
	pcf8574.write(ExHeatControlPin,v);
#endif
}

inline void setPumpOut(byte v)
{
#if PUMP_USE_EXT != true

#if PUMP_INVERTED_LOGIC
	digitalWrite (PumpControlPin, (v==LOW)? HIGH:LOW);
#else
	digitalWrite (PumpControlPin, v);
#endif

#else
	pcf8574.write(ExPumpControlPin,v);
#endif
}

inline void setBuzzOut(byte v)
{
#if BUZZER_USE_EXT != true
	digitalWrite (BuzzControlPin, v);
#else
	pcf8574.write(ExBuzzControlPin,v);
#endif
}

#if SpargeHeaterSupport == true
inline void setAuxHeaterOut(byte v)
{
	digitalWrite (AuxHeatControlPin, v);
}
#endif

#if SecondaryHeaterSupport == true
inline void setSecondaryHeaterOut(byte v)
{
	digitalWrite (AuxHeatControlPin, v);
}
#endif

void initIOPins(void)
{
#if (BUTTON_USE_EXT == true) || (HEATER_USE_EXT == true) || (PUMP_USE_EXT == true) ||( BUZZER_USE_EXT == true)
//	pcf8574.begin();
#endif

#if BUTTON_USE_EXT !=true
  	pinMode (ButtonUpPin,    INPUT_PULLUP);
  	pinMode (ButtonDownPin,    INPUT_PULLUP);
  	pinMode (ButtonStartPin, INPUT_PULLUP);
  	pinMode (ButtonEnterPin, INPUT_PULLUP);
#else
	_portvalue=0;
#endif

#if HEATER_USE_EXT != true
	pinMode (HeatControlPin, OUTPUT);
#endif
	setHeaterOut(LOW);

#if PUMP_USE_EXT != true
	pinMode (PumpControlPin, OUTPUT);
#endif
	setPumpOut(LOW);

#if BUZZER_USE_EXT != true
	pinMode (BuzzControlPin, OUTPUT);
#endif
	setBuzzOut(LOW);

#if SpargeHeaterSupport == true
	pinMode (AuxHeatControlPin, OUTPUT);
	setAuxHeaterOut(LOW);
#endif

}
#endif
