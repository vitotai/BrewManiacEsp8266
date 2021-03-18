# BrewManiacEx
![BrweManiacEx Architecture](./docs/bmex_arch.jpg)

BrewManiacEx is a brew controller based on ESP8266. It was started by mimicking famous Arduino-based Open ArdBir, so most of Open ArdBir functions are included.

 * Manual and Automation brewing process control
 * PID and PWM heat control
 * Automatic PUMP control

The most important feature that BrewManiacEx differs from Open ArdBir is

 * WiFi Enabled, Web-based interface
You can view, change the settings, monitor and control the brew on your phones, tablets, and computers.

Additionally,

 * Multiple sensors support
Up to 5 sensors are supported. Different sensors can be used in different stages.
 * PID autotune
Run the PID Autotune to get the best PID parameters for your system instead of guessing.
 * Sparge water Heating control
Run the sparge water heating when the main heating element is idle during mashing to enable using only one 15A/20A GFCI socket.
 * Temperature logging chart
Watching the temperature chart during brew and after brew
 * [Untested/uncompleted] Distilling
PWM controlled heating instead of PID controlled. Customized 4 stages: pre-heat, head, heart, and tail, which are defined by temperature ranges.

## NOTE:
Flash layout and File system are changed in 0.4.9. All data and setting will be lost after upgrading. Please backup before upgrading.
The major change in 0.4.9 is adapting of LitteFS, whihch is quiet new to ESP8266/Arduino framework. Although the performance is better,
the stability is in doubt. Please be aware.

## Update History
 * 0.5
   * Rollback framework to 2.2.0. (Newer framework seems to have issues with my new WiFi6 AP.)
   * New web page theme by bootstrap
   * Sensor management from web pages
   * Customizable Recipe Source
   * embedded HTML/Javascript
   * (Partial) Russian LCD text 
   * Bugs fix
      * Heating is turned on during Hopstand
      * Time isn't synchronized
      * Log saving failure for LittleFS.
 * 0.4.9 (pre-release)
   * Update framework to 2.3.3
   * Change File System to LittleFS
   * Display critical system information on WiFi setup page, system update page, and about page.
   * Fix HTML/JS version error.
   * Change Flash Layout to 2M program
   * 16M flash layout support.

 * 0.4.8
    * Only HTML/JS has 0.4.8.
    * Change Charting library to dygraph.
    * re-orgnize HTML/JS

 * 0.4.7
    * Update framework to 2.2.0
    * Update ArduinoJson to v6

 * 0.4.6
    * multi sensor calibration error fixed
    * enable dragging on mobile platform
    * Heat@Pump option to stop heating if pump is not running.

 * 0.4.5
    * new web fontend
    * LCD mirror on web pages

 * 0.4.2 
    * AP only mode fixes
    * HopStand support for exported beerXML from BeerSmith and Brewersfriend.com
    * Settings survive through formatting file system
    * Low pass filtered applied to temperature sensor readings.
    * (untested, unverified, not enabled by default) Level sensor(float ball) support for Kettle-RIMS

 * 0.4 
    * update ESPAsyncTCP and ESPAsyncWebServer library
    * Websocket instead of Server Side Event(EventSource)
    * Continuously Up/Down button action
    * Bug fixed, no temperature chart in Manual and Distilling mode.
    * Bug fixed, setting temperature udpate when changed
    * Bug fixed, No PUMP control in distilling auto mode

 * 0.3.3 [12/06/2017]
    * bugs fixed.
    * Moveable control panel, progress table, and temperature chart.