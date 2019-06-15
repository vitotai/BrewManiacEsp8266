# Appendix

**A1.Q&A**

**Q1: PID AutoTune.**

A1: Check the detail at this page:

http://brettbeauregard.com/blog/2012/01/arduino-pid-autotune-library/

**Q2: First Wort Gravity?**

A2: The first wort gravity is calculated based on 80% yield of grain, so it’s a approximate value.

Brix of First Wort  =  ( grain in kg) * 0.8  / [ ( grain in kg) * 0.8 +  (water in L)]

Convertion from Brix to Gravity:

SG = 1 + (Brix/(258.6-((Brix/258.2)*227.1)))

**Q3: Zero crossing relay and  heater switch.**

A3:

  Zero crossing relays switch ON/OFF when the voltage crosses zero, which introduces a lag or delay. If both the main heater and sparger heater are controlled by zero crossing relays, it should be fine because they will be turned on and off at the same time when the voltage crosses zero. However, if one of the relay isn’t zero crossing and switches immediately, it might result in overload of power when the turned-off zero crossing relay "waits" for the voltage to cross zero and the non zero crossing relay turns on immediately.

  A minimum 10ms delay between heater switch is introduced to solve this issue. The delay might sometimes extends over 100ms because of the limitation of software structure.

**Q4: Time in brew logs.**

A4: The real clock time will be recorded in the brew log as long as the time is known when the brew starts. BrewManiacEx will try to get time from the internet, NTP servers, at startup. If that fails, it can still get time from the computer or phone that connects to it. Therefore, connect to BrewManaicEx before the brew starts so that it can get the time of your computer or phone if it doesn’t have internet access.