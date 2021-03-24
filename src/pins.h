#ifndef PINS_H
#define PINS_H
#if ESP32

// Buttons are INPUT
// byte btnReadPin(byte p){ return digitalRead(p);}
#define I2C_SDA 21
#define I2C_SCL 22
#define SensorPin 23

#define BuzzControlPin 18

#define PumpControlPin  16
#define HeatControlPin  17

//#define CoolControlPin  19
//#define SecondaryHeatControlPin  27


#define ButtonUpPin    32 // P1
#define ButtonDownPin  33 // p0 NODEMCU_PIN_D4
#define ButtonStartPin 25 //P3 NODEMCU_PIN_D5
#define ButtonEnterPin  26 //P2 NODEMCU_PIN_D6


void btnPrepareRead(void){}
#define btnReadPin digitalRead

// Heater, Pump, Buzz are OUTPUTs
inline void setHeaterOut(byte v)
{
	digitalWrite (HeatControlPin, v);
}

inline void setPumpOut(byte v)
{
	digitalWrite (PumpControlPin, v);
}

inline void setBuzzOut(byte v)
{
	digitalWrite (BuzzControlPin, v);
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

  	pinMode (ButtonUpPin,    INPUT_PULLUP);
  	pinMode (ButtonDownPin,    INPUT_PULLUP);
  	pinMode (ButtonStartPin, INPUT_PULLUP);
  	pinMode (ButtonEnterPin, INPUT_PULLUP);

	pinMode (HeatControlPin, OUTPUT);
	setHeaterOut(LOW);

	pinMode (PumpControlPin, OUTPUT);
	setPumpOut(readSetting(PS_PumpActuatorInverted)==0? LOW:HIGH);

	pinMode (BuzzControlPin, OUTPUT);
	setBuzzOut(LOW);

#if SpargeHeaterSupport == true
	pinMode (AuxHeatControlPin, OUTPUT);
	setAuxHeaterOut(LOW);
#endif

#if SecondaryHeaterSupport == true
	pinMode (AuxHeatControlPin, OUTPUT);
	setSecondaryHeaterOut(LOW);
#endif

#if EnableLevelSensor
	pinMode (LevelSensorPin, INPUT_PULLUP);
#endif
}

#if EnableLevelSensor
// close/connected/ground: not full
// open/disconnected/V+: full
#if SensorNormalCloseOnNotFull
bool isWaterLevelFull(void){
	return digitalRead(LevelSensorPin) != 0;
}

#else

bool isWaterLevelFull(void){
	return digitalRead(LevelSensorPin) == 0;
}

#endif //#if SensorNormalCloseOnNotFull
#endif // EnableLevelSensor

#else //ESP8266
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

#if EnableLevelSensor
#define LevelSensorOnIoExpander false

#if LevelSensorOnIoExpander
#define LevelSensorIOExpPin 32 //P5
#else
#define LevelSensorPin NODEMCU_PIN_D7
#endif

#define SensorNormalCloseOnNotFull true

#endif

#if PUMP_USE_EXT != true
#define PumpControlPin  NODEMCU_PIN_D5
#else
#define ExPumpControlPin  5
#endif

#if HEATER_USE_EXT != true
#define HeatControlPin  NODEMCU_PIN_D7
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
	digitalWrite (PumpControlPin, v);
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

#if SecondaryHeaterSupport == true
	pinMode (AuxHeatControlPin, OUTPUT);
	setSecondaryHeaterOut(LOW);
#endif

#if EnableLevelSensor
#if LevelSensorOnIoExpander != true
	pinMode (LevelSensorPin, INPUT_PULLUP);
#endif
#endif
}

#if EnableLevelSensor
// close/connected/ground: not full
// open/disconnected/V+: full
#if SensorNormalCloseOnNotFull
#if LevelSensorOnIoExpander
bool isWaterLevelFull(void){
	// _portvalue is read for button every loop cycle
	return (_portvalue & LevelSensorIOExpPin) !=0;
}
#else
bool isWaterLevelFull(void){
	return digitalRead(LevelSensorPin) != 0;
}
#endif

#else

#if LevelSensorOnIoExpander
bool isWaterLevelFull(void){
	// _portvalue is read for button every loop cycle
	return (_portvalue & LevelSensorIOExpPin) ==0;
}
#else
bool isWaterLevelFull(void){
	return digitalRead(LevelSensorPin) == 0;
}
#endif

#endif //#if SensorNormalCloseOnNotFull
#endif // EnableLevelSensor


#endif

#endif