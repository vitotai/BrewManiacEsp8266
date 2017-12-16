## BrewManiacExManual_0.1.pdf

The user manual.


## circuit.png

The reference circuit diagram.

## brewmaniac_simple.jpg

A simplfied setup withoug custom PCB. It is my initial platform, and I used it to brew at least twice, not couting testing.
* Supply 5V to Nodemcu. Use at least 1A ~ 1.5A power supply.
* Relay module.
    * Supply 5V. 
    * Sometimes you might get negative relay module. Add `-DPUMP_INVERTED_LOGIC=true` to build_flags in platformio.ini if you have that. 
 * PCF8574 module
    * Ground all adress section jumpers.
    * Check the lable on the IC. You might get PCF8574 or PCF8574A. Change line 27 in pins.h to
     ` #define PCF8574_ADDRESS 0x38` (As the commented line28)
    * You can supply 3.3V or 5v to PCF8574. There are issues for both, described later.
 * Keypad
    * Connect GND, and the PINs next to GND to P0 of PCF8574, and the follwoinng PINs toP1, P2, P3. 
 * I2C LCD
    * The PCF8574 module has a socket for I2C bus. Therefore, you can connect I2C to PCF8574.
    * You can supply 3.3V or 5v.
        * The LCD might not work well under 3.3v power supply.
 * DS18B20 temperature sensors
    * Use 4.7k resistor. DS18B20 doesn't work well without pullup on ESP8266.
    * 3.3v/5v see the description following.
 * Most SSR seems to accept 3V~ signal and draw small current. Just make sure the specifiction of your SSR. 
 * Buzzer should have no issue.

### 3.3v or 5v
PCF8574 module, LCD, and DS18B20 require power supply. The power supply to Nodemcu is 5v and the small regulator on Nodemcu provides 3.3v for our convience. I've tried providing 3.3v and 5v to all the components. Both configurations worked. However, there are some concerns. 
 * Using 5v
    * Good: the power comes from power supply, mot the small regulator of Nodemcu
    * Bad: The IO of ESP8266 is 3.3v. It works but out of speicication.
 * Using 3.3v
    * Good: no 5V siginal to ESP8266 IO
    * Bad#1: The 3.3v is supplied by the small regulator of Nodemcu.
    * Bad#2: I couldn't get good constrast of LCD at 3.3v
