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
