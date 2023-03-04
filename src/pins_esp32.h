#ifndef PINS_H
#define PINS_H
// Buttons are INPUT
// byte btnReadPin(byte p){ return digitalRead(p);}
#define I2C_SDA 21
#define I2C_SCL 22
#define SensorPin 23

#define BuzzControlPin 18

#define PumpControlPin  16
#define HeatControlPin  17



#define ButtonUpPin    32 // P1
#define ButtonDownPin  33 // p0 NODEMCU_PIN_D4
#define ButtonStartPin 25 //P3 NODEMCU_PIN_D5
#define ButtonEnterPin  26 //P2 NODEMCU_PIN_D6

#if EnableLevelSensor
#define LevelSensorPin 19
#if USE_MAX6675
#error "conflict setting  Max6675 and LevelSensorPin"
#endif

#endif

#define SpargeHeatControlPin  27
#define SecondaryHeatControlPin  28

#if USE_MAX6675
#if MultipleSensorEnabled
#error "Conflict setting with Max6675 and Multiple sensor"
#endif

#undef SensorPin
#undef  BuzzControlPin
#define BuzzControlPin 4

#define SPI_SCK  18
#define SPI_MISO  19
#define SPI_CS  23

#endif

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
inline void setSpargeHeaterOut(byte v)
{
	digitalWrite (SpargeHeatControlPin, v);
}
#endif

#if SecondaryHeaterSupport == true
inline void setSecondaryHeaterOut(byte v)
{
	digitalWrite (SecondaryHeatControlPin, v);
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
	pinMode (SpargeHeatControlPin, OUTPUT);
	setSpargeHeaterOut(LOW);
#endif

#if SecondaryHeaterSupport == true
	pinMode (SecondaryHeatControlPin, OUTPUT);
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


#endif