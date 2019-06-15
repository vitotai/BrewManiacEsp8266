**BrewManiacEx Manual**



<p id="gdcalert1" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual0.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert2">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual0.jpg "image_tooltip")


<p style="text-align: right">
<strong>Version 0.4 draft v1</strong></p>


<p style="text-align: right">
<strong>July 5 2018</strong></p>


<p style="text-align: right">
</p>



# 0. Software Configuration

If you are using different configuration from mine, like different pin assignment or different IO Expander address, some of the compile options must be changed. Two files are involved: _config.h_ and _pins.h_.

The options in config.h can be configured in “platformio.ini” as described in the following table.


<table>
  <tr>
   <td>Option
   </td>
   <td>Description
   </td>
  </tr>
  <tr>
   <td><code>MultipleSensorEnabled</code>
   </td>
   <td>true: multiple sensors supported. Maximum to 5 sensors.
   </td>
  </tr>
  <tr>
   <td><code>SupportDistilling</code>
   </td>
   <td>Distilling support.
   </td>
  </tr>
  <tr>
   <td><code>SpargeHeaterSupportEnabled</code>
   </td>
   <td>Sparge Heater support.
   </td>
  </tr>
  <tr>
   <td><code>SecondaryHeaterSupportEnabled</code>
   </td>
   <td>Dual heater support.
   </td>
  </tr>
  <tr>
   <td><code>EnableLevelSensor</code>
   </td>
   <td>Level sensor support.
   </td>
  </tr>
  <tr>
   <td><code>UsePaddleInsteadOfPumpEnabled</code>
   </td>
   <td>Use paddle instead of PUMP. The “Stir Rest” setting will be in seconds instead of minutes for pump.
   </td>
  </tr>
</table>


pins.h contains the hardware configuration, include buttions, IO expander, and etc. You should know what you are using before changing them. Two of these options you might need to change are



*   pins.h \
`#define PUMP_INVERTED_LOGIC false \
` \
SSR(Solid State Relay) is recommended for heating control, and mechanical relay is usually good enough for PUMP control. Some relay modules on eBay use inverted logic. Change this option when necessary.
*   pins.h \
`#define PCF8574_ADDRESS 0x20` \
 \
Address of the IO Expander.

To suport level sensor, the following options should be changed to meet the hardware. \
`#define LevelSensorOnIoExpander true/false`


```
//Set to true if the sensor is connected to IO Expander
#define LevelSensorPin xxx 
```


   or `#define LevelSensorIOExpPin 32 `


```
// the PIN of level sensor
#define SensorNormalCloseOnNotFull true/false
// set to true if the sensor is "normal-close", closed when not full.
```


 \
 \
Address of the IO Expander

#define PCF8574_ADDRESS 0x20


# 


# **1.Sensor Configuration**

There are two configurations of BrewManiacEx: single sensor and multiple sensors. \
It is necessary to assign and identify the sensors before use for multi-sensor configuration. For single sensor configuration this is not necessary, you can however use just one sensor in multiple sensor configuration, but sensor setup is still necessary.

_For multi-sensor configuration, the sensor setting must be configured before all other functions._


# **2.Settings**


## 2.1 PID-PWM


<table>
  <tr>
   <td><strong>Setting</strong>
   </td>
   <td><strong>Values</strong>
   </td>
   <td><strong>Description</strong>
   </td>
  </tr>
  <tr>
   <td><strong>Constant kP</strong>
   </td>
   <td>-100~100
   </td>
   <td>PID parameters (can be auto tuned.)
   </td>
  </tr>
  <tr>
   <td><strong>Constant kI</strong> 
   </td>
   <td>-100~155
   </td>
   <td>PID parameters (can be auto tuned.)
   </td>
  </tr>
  <tr>
   <td><strong>Constant kD</strong>
   </td>
   <td>-100~100
   </td>
   <td>PID parameters (can be auto tuned.)
   </td>
  </tr>
  <tr>
   <td><strong>Sample Time</strong>
   </td>
   <td>1500~3500
   </td>
   <td>PID algorithm parameters
   </td>
  </tr>
  <tr>
   <td><strong>WindowSet</strong>
   </td>
   <td>4000~7500
   </td>
   <td>PID algorithm parameters. It should be greater than 2x Sample Time
   </td>
  </tr>
  <tr>
   <td><strong>Heat/PWM in Boil</strong>
   </td>
   <td>0~100
   </td>
   <td>The default PWM after Boil Point reached.
   </td>
  </tr>
  <tr>
   <td><strong>Sensor Calibration</strong>
   </td>
   <td>-5 ~ +5
   </td>
   <td>Calibration value of sensor(s)
   </td>
  </tr>
  <tr>
   <td><strong>PID Start</strong>
   </td>
   <td>1 ~ 3.5
   </td>
   <td>PID will be applied when the difference between the current and the set temperature is smaller than this value. For example, if set point is 65, and PID start is 1.5, then the PID will be applied when the temperature reaches 63.5. Before that, the heating is full-on.
   </td>
  </tr>
</table>


_Note1: for **multi-sensor configuration**, the “calibration value” should be the the number of sensors._

_Note2: PID is used to maintain the temperature of the mashing stages only. For the boiling stage, or when the setting point is greater or equal to boiling temperature, the heating is full-on until the boil temperature is reached. After reaching the boil temperature, the heating output is controlled by PWM._

_Note 3: If <code>SecondaryHeaterSupport </code>is “true”, there will be 3 set of P,I,D settings. The corresponding values will be used when heating elements configuration changes.</em>


## 2.2 Unit Parameters


<table>
  <tr>
   <td><strong>Setting</strong>
   </td>
   <td><strong>Values</strong>
   </td>
   <td><strong>Description</strong>
   </td>
  </tr>
  <tr>
   <td><strong>Temperature Unit</strong>
   </td>
   <td>°C/°F
   </td>
   <td>Celius or Fahrenheit
   </td>
  </tr>
  <tr>
   <td><strong>Disable Delay Start</strong>
   </td>
   <td>No/Yes
   </td>
   <td>Yes: Disable Delay Start
   </td>
  </tr>
  <tr>
   <td><strong>Boil Temperature</strong>
   </td>
   <td>80-120°C
<p>
176-248°F
   </td>
   <td>The temperature regarded as “Boiling”. Note: the set temperature must be greater or equal to this setting to be regarded as “boiling”. PWM is enabled once the measured temperature is greater than the set temperature AND the Boil temperature.
   </td>
  </tr>
  <tr>
   <td><strong>Pump Cycle</strong>
   </td>
   <td>5 ~ 15
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td><strong>Pump Rest</strong>
   </td>
   <td>0 ~ 5
   </td>
   <td>Set to 1-4 to enable auto pump rest. During mash stages, pump will run for “Pump Cycle” time and stop for “Pump Rest” time.
   </td>
  </tr>
  <tr>
   <td><strong>Pump PreMash</strong>
   </td>
   <td>OFF/ON
   </td>
   <td>Turn on pump before Mash-in.
   </td>
  </tr>
  <tr>
   <td><strong>Pump on Mash</strong>
   </td>
   <td>OFF/ON
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td><strong>Pump MashOut</strong>
   </td>
   <td>OFF/ON
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td><strong>Pump on Boil</strong>
   </td>
   <td>OFF/ON
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td><strong>Pump Stop Temperature</strong>
   </td>
   <td>80-120°C
<p>
176-248°F
   </td>
   <td>The temperature at which to stop the pump.
   </td>
  </tr>
  <tr>
   <td><strong>PID DoughtIn</strong>
   </td>
   <td>OFF/ON
   </td>
   <td>Specify if HEATING is no during dought-in.
   </td>
  </tr>
  <tr>
   <td><strong>PID MaltOut</strong>
   </td>
   <td>OFF/ON
   </td>
   <td>Specify if HEATING is no during removing malt pipe.
   </td>
  </tr>
  <tr>
   <td><strong>Skip Add</strong>
   </td>
   <td>No/Yes
   </td>
   <td>Skip asking “Add Malt”, go direct into first Mash step after “Mash-in” temperature reached.
   </td>
  </tr>
  <tr>
   <td><strong>Skip Remove</strong>
   </td>
   <td>No/Yes
   </td>
   <td>Skip asking “Remove Malt”, go direct into Boiling after mashout.
   </td>
  </tr>
  <tr>
   <td><strong>Skip Iodine</strong>
   </td>
   <td>No/Yes
   </td>
   <td>Skip iodine test, go directly to Mashout after last mash step.
   </td>
  </tr>
  <tr>
   <td><strong>Iodine Time</strong>
   </td>
   <td>0-120 
   </td>
   <td>The time to wait for iodine test.
   </td>
  </tr>
  <tr>
   <td><strong>Whirlpool</strong>
   </td>
   <td>Off/Cool/Hot
   </td>
   <td>The time to run Whirlpool.
<p>
Off: no whirlpool
<p>
Cool: whirlpool after cooling phase.
<p>
Hot: whirlpool before cooling phase.
   </td>
  </tr>
  <tr>
   <td><strong>Heat@Pump</strong>
   </td>
   <td>No/Yes
   </td>
   <td>If set to YES, heating will be off when PUMP is not running.
   </td>
  </tr>
</table>



## 2.3 Misc Setting

The setting will be available when corresponding options are turned on.


<table>
  <tr>
   <td><strong>Setting</strong>
   </td>
   <td><strong>Values</strong>
   </td>
   <td><strong>Description</strong>
   </td>
  </tr>
  <tr>
   <td><strong>Disable Delay Start</strong>
   </td>
   <td>No/Yes
   </td>
   <td>Yes: Disable Delay Start. The controller won’t delay start.
   </td>
  </tr>
  <tr>
   <td><strong>Buzz On Button</strong>
   </td>
   <td>OFF/ON
   </td>
   <td>Button pressing feedback.
   </td>
  </tr>
  <tr>
   <td><strong>Pump Prime Cycle</strong>
   </td>
   <td>0~10
   </td>
   <td>The number of pump priming cycle.
   </td>
  </tr>
  <tr>
   <td><strong>Pump Prime On Time</strong>
   </td>
   <td>250-10,000ms
   </td>
   <td>To prime the pump before automatic mode, the pump will be turned on and then off for multiple times. This setting defines the duration of ON. \
It increases in 250ms step. 
   </td>
  </tr>
  <tr>
   <td><strong>Pump Prime Off Time</strong>
   </td>
   <td>0-10,000 ms
   </td>
   <td>see above.
   </td>
  </tr>
  <tr>
   <td colspan="3" ><strong>Sparge Water heating Control *1</strong>
   </td>
  </tr>
  <tr>
   <td><strong>Control Sparge Heating</strong>
   </td>
   <td>No/Yes
   </td>
   <td>disable/enable the sparge water heating control
   </td>
  </tr>
  <tr>
   <td><strong>Temp. Ctrl</strong>
   </td>
   <td>No/Yes
   </td>
   <td>Whether or not to control the temperature of spage water heating
   </td>
  </tr>
  <tr>
   <td><strong>Sparge Sensor</strong>
   </td>
   <td>1-[max sensor id]
   </td>
   <td>the sensor id for sparge water
   </td>
  </tr>
  <tr>
   <td><strong>Sparge Temp</strong>
   </td>
   <td>75 ~ 80°C
   </td>
   <td>the desired temperature for sparge water
   </td>
  </tr>
  <tr>
   <td><strong>Temp.  Diff</strong>
   </td>
   <td>0.5 ~ 2.0°C
   </td>
   <td>the temperature range to keep
   </td>
  </tr>
  <tr>
   <td colspan="3" ><strong>Heating Element Usage</strong>
   </td>
  </tr>
  <tr>
   <td><strong>PreMash Heating Element</strong>
   </td>
   <td>Primary
<p>
Secondary
<p>
Both
   </td>
   <td>The elements used for PreMash stage, that is before dough-in.
   </td>
  </tr>
  <tr>
   <td><strong>Mashing Heating Element</strong>
   </td>
   <td>Primary
<p>
Secondary
<p>
Both
   </td>
   <td>The heating element used during mashing.
   </td>
  </tr>
  <tr>
   <td><strong>Boiling Heating Element</strong>
   </td>
   <td>Primary
<p>
Secondary
<p>
Both
   </td>
   <td>The heating element used during boiling.
   </td>
  </tr>
  <tr>
   <td><strong>Post Boil Heating Element</strong>
   </td>
   <td>Primary
<p>
Secondary
<p>
Both
   </td>
   <td>The heating element used for Hop Stand, if necesssary.
   </td>
  </tr>
  <tr>
   <td colspan="3" ><strong>Level Sensor</strong>
   </td>
  </tr>
  <tr>
   <td><strong>Enable LV Sen</strong>
   </td>
   <td>No/Yes
   </td>
   <td>Enable/disable the sensor
   </td>
  </tr>
  <tr>
   <td><strong>LV Trig. ms</strong>
   </td>
   <td>50 - 1000 ms
   </td>
   <td>Minimum triggering time of the sensor
   </td>
  </tr>
  <tr>
   <td><strong>Ext Pump Rest</strong>
   </td>
   <td>2-90 seconds
   </td>
   <td>Expended pump rest time.
   </td>
  </tr>
</table>


The advantage of using sparge water heating control is that the sparge heater and main heater will run **_exclusively_** so that they can share one 120V/20A GFI socket(loop) (American system)

The heating of sparge water can be temperature controlled if required. If temperature control is not applied, the heating is ON when the main heating is OFF, in this case the temperature of the sparge water should be controlled manually or by another controller. The temperature control options are only available in multi-sensor configuration.

You don’t need to specify all P.I.D parameters for all usage of heating element. For example, if BOTH is used only for boiling, P.I.D. is not applied in boiling stage so it is not necessary to supply the setting values. (However, it might be used in manual mode.)


## 2.4 Sensor Setting

**_Connect all the sensors, and run sensor setting before all other actions._**

The maximum number of sensors supported is 5.

There are two steps in sensor setting, **identifying sensors** and assigning **sensor usage**. In the first step, the sensor is identified and assigned to a _number_ as its ID. In the second step, the primary sensor for temperature control and the auxiliary sensor reading to be displayed are assigned for each step, including Pre-Mash, Mashing, Boiling, Cooling, Manual mode, and Main screen.

**Step 1: Assign sensors**



<p id="gdcalert2" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual1.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert3">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual1.jpg "image_tooltip")


Assign the sensor to the number displayed, #1 in this picture. Use up/down to change sensors. The last 8 digits of the sensor address and current temperature reading is displayed. _To get updated temperature reading, use UP/DOWN to change to other sensors and get lastest reading._

**Step 2: Assign sensor usage**

Only two readings can be displayed on the 20x4 LCD, the **_primary_** one is used for temperature control while the **_auxiliary_** one is for display only. The primary and auxiliary sensors can be set for the following stages: Pre-Mash, Mashing, Boiling, Cooling, Manual Mode, and Main screen. The reading of primary sensor is always the number at **_TOP_** or **_LEFT_**.



<p id="gdcalert3" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual2.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert4">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual2.jpg "image_tooltip")




<p id="gdcalert4" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual3.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert5">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual3.jpg "image_tooltip")


**_Note: if  temperature controlled sparge water heating control is applied, the secondary reading is always the sensor assigned to the sparge water temperature during Mashing._**


# **3. Manual Mode and PID autotune**


## 3.1 Manual Mode

In manual mode, the heating system the and pump are controlled manually.

The time starts running when the temperature reaches the set point, and it will be reset when the set temperature is changed 1 degree away.


<table>
  <tr>
   <td><strong>Buttons</strong>
   </td>
   <td><strong>Functions</strong>
   </td>
  </tr>
  <tr>
   <td>Up
   </td>
   <td>Increse setting temperature. Long press to speed up.
   </td>
  </tr>
  <tr>
   <td>Down
   </td>
   <td>Decrease setting temperature. Long press to speed up.
   </td>
  </tr>
  <tr>
   <td>Heat(Start)
   </td>
   <td>Toggle heating element. In dual element mode, it controls primary heating element.
   </td>
  </tr>
  <tr>
   <td>Pump(Enter)
   </td>
   <td>Toggle pump.
   </td>
  </tr>
  <tr>
   <td>Heat + Pump
   </td>
   <td>Enter countdown mode to input count down time. After input of countdown time, press Heat + Pump again to end editing countdown timer. The countdown will begin when the set temperature is reached. After the countdown finishes, there will be a buzzing sound and the time starts to count “up”.
   </td>
  </tr>
  <tr>
   <td>Down + Heat
   </td>
   <td>Toggle sparge heating, if supported.
   </td>
  </tr>
  <tr>
   <td>Up + Heat
   </td>
   <td>Toggle Secondary heating, if supported.
   </td>
  </tr>
  <tr>
   <td>Long press Heat
   </td>
   <td>Enter PID autotune. The P.I.D tuned will be the configuration of heating elements used.
   </td>
  </tr>
  <tr>
   <td>Long press Pump
   </td>
   <td>Enable Pump rest.
   </td>
  </tr>
</table>



## 3.2 PID AutoTune

The PID autoTune function is used to derive the kP, kI, and kP values. To run PID AutoTune:

1. Add water

2. Enter Manual Mode, set the desired temperature to the temperature you usually mash at.

3. After the set point is reached, **LONG PRESS the “Heat” button**, and answer “Yes” to enter PID AutoTune. \


<p id="gdcalert5" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual4.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert6">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual4.png "image_tooltip")


4. The PID AutoTune function will run to get the optimal parameters which can keep the temperature steady at current temperature. \


<p id="gdcalert6" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual5.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert7">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual5.png "image_tooltip")


5. When the process is finished, the parameters are stored automatically. The controller will return to  Manual mode. You can stop the auto tune anytime before it finishes. \


<p id="gdcalert7" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual6.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert8">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual6.png "image_tooltip")


Note:

The PID AutoTune function derives the optimal parameters by kicking the target either way and seeing how far and how quickly it goes. Therefore, the parameters derived are best for the conditions in which  it runs. To get the best result, use the same volume of water as your brew size, and put your device in the place where you usually brew. Running the PUMP is also recommended if the pump is on during mashing, which is usually true.


# 4. Brew with Automatic mode

Automation is the heart of BrewManiacEx controller. The brew process starts from heating strike water to optional whirlpool and cooling. It prompts for user interactions, including:

-Malt In

-Malt Out

-Iodine test

-Hop throwing


## 4.1 Setup Automation

Before starting automation, the mash schedule, boil time, and hop time need to be specified. To edit the automation procedure, enter “Setup” then select “Automation”.



<p id="gdcalert8" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual7.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert9">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual7.png "image_tooltip")


The first item to edit  is the temperature of “Mash In”, or the temperature of the strike water.



<p id="gdcalert9" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual8.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert10">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual8.png "image_tooltip")


Then, specify the first mash step temperature:



<p id="gdcalert10" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual9.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert11">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual9.png "image_tooltip")


and, the rest time.



<p id="gdcalert11" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual10.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert12">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual10.png "image_tooltip")


After entering the time by **Up/Down** buttons, press “**Done**”(Start button) if this is the final mash step(rest), or “**More**”(Enter button) if more rests are needed.

Repeat the input of temperature and time until all steps are input or the maximum 6 steps are input. 



<p id="gdcalert12" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual11.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert13">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual11.png "image_tooltip")


Then mashout temp and time:



<p id="gdcalert13" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual12.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert14">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual12.png "image_tooltip")


Input number of hops, **Boil Time, **and the time of each hop.



<p id="gdcalert14" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual13.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert15">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual13.png "image_tooltip")




<p id="gdcalert15" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual14.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert16">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual14.png "image_tooltip")




<p id="gdcalert16" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual15.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert17">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual15.png "image_tooltip")


The automation settings are saved. Therefore, if the same recipe is brewed, it is not necessary to setup again.

HopStand editing:



<p id="gdcalert17" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual16.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert18">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual16.jpg "image_tooltip")


A HopStand Session is a span of temperature range. Maximum number of sessions is 5.



<p id="gdcalert18" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual17.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert19">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual17.jpg "image_tooltip")


The “start” of hop stand. For FlameOut(KnockOff) hopping, set this number to greater or equal to boiling point. After boiling, when the temperature drops to this value, the controller will alarm to prompt for hop dropping and start counting time.



<p id="gdcalert19" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual18.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert20">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual18.jpg "image_tooltip")


The minimu temperature of this hopstand session. If the temperature drops below this setting before the specified hop time expires, the heating will be kicked start. If using of heating element is not desired, set this value to a lower value. However, the “start” temperature of next session should be lower than previous “keep” temperature. If precise and specific temperature is desired, set the start and keep temperature to exact the same value.



<p id="gdcalert20" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual19.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert21">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual19.jpg "image_tooltip")


Set the time of PostBoilHop #01. The first hop in the hopstand session defines the time of the hop stand. Press “Done” if no more hop, “More” else.

Repeat editing HopStand Session and PostBoilHop time until finished.


## 4.2 Running Automation

The automation will run through the brew process that is specified by the automation setup. When user interaction is needed, it will alert the user.



<p id="gdcalert21" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual20.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert22">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual20.jpg "image_tooltip")


During automated processes, like mashing and boil, additional control is possible. For example, the mash step can be skipped, and the pump can be controlled manually during mashing and boiling.



Automatic Brewing Screens:



*   Delay Start



<p id="gdcalert22" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual21.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert23">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual21.jpg "image_tooltip")


If “Disable Delay Start” is set to “NO”, you will have the option to delay-start the brew. 



*   Edit Delay Start Time



<p id="gdcalert23" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual22.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert24">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual22.jpg "image_tooltip")


Use Up/Down to change the delay time. 15min a step.



*   Delay Start Waiting



<p id="gdcalert24" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual23.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert25">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual23.jpg "image_tooltip")


The brew process will begin after the time counts to zero. 

Go: start immediately.

Quit: quit the brew.





*   Mashing



<p id="gdcalert25" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual24.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert26">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual24.jpg "image_tooltip")


Up/Down: adjust the setting temperature.

PmPus (Pump & Pause): Press to toggle pump.


                  Long press to Pause the brew. 

STP: long press to extend or skip this rest step.

Multi-sensor Mashing Screen



<p id="gdcalert26" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual25.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert27">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual25.jpg "image_tooltip")


The upper reading is the main temperature reading which is under PID control.

The lower reading is AUXiliary temperature reading.



*   Paused



<p id="gdcalert27" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual26.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert28">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual26.jpg "image_tooltip")


Paused mash step. The brew is paused until “Exit” pressed. The timer is stopped, as well as the pump and temperature control(heating).



*   Modify Mash Step



<p id="gdcalert28" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual27.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert29">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual27.jpg "image_tooltip")


The mash step can be modified by **LONG PRESS** the “**STP**” button.

_Extend_: toggle rest extension. The extension status is displayed as “+” sign or nothing after the “Mash 1/3" title line. 

_Skip_: Skip this rest step.

_Back_: back to mashing screen.



*   Extended Mashing



<p id="gdcalert29" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual28.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert30">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual28.jpg "image_tooltip")


The duration of mash rest can be extended when necessary. It is useful under certain circumstances, like when decoction is used or conversion isn’t finished yet. When extended status is set, a “+”(plus sign) will be shown. When time runs out, the timer starts to **_count UP_**, and the “+”(plus sign) turns into “*”(star sign). 



<p id="gdcalert30" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual29.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert31">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual29.jpg "image_tooltip")


Press “STP” to finsih current rest step.



*   Iodine Test

If enabled (skip iodine set to No), the controller will prompt for “Iodine Test” after last mash step finished and before Mashout start.



<p id="gdcalert31" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual30.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert32">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual30.jpg "image_tooltip")


If the Iodine test reveals the conversion is finished, press “M.Out” to go to mashout. If the conversionis not finished, press “Ext”. The last mashing step will be extended and run until “STP” is pressed.



*   Before Boiling



<p id="gdcalert32" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual31.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert33">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual31.jpg "image_tooltip")


Up/Down: adjust target temperature.

Pmp: toogle pump.



*   Boiling



<p id="gdcalert33" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual32.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert34">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual32.jpg "image_tooltip")


Up/Down: adjust target temperature or PWM.

Pause: pause timer. The timer will paused, but the heating control remain active. Use this to extend boil time when needed.

Pmp: toogle pump.

Note: Only when the temperature is higher than **_Target Temperature and the Boil Temperature _**will the heating be controlled by PWM, the PWM value will be shown.

Boiling Screen for Multi-sensor



<p id="gdcalert34" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual33.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert35">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual33.jpg "image_tooltip")




<p id="gdcalert35" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual34.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert36">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual34.jpg "image_tooltip")


The PWM duty cycle is displayed before the timer. When it is 100%, it is displayed as “HH%”.



*   HopStand

If hopstand is specified, the controller will prompt “chilling” to drop temperature for hopstand. (“Cooling” is for final cooling.)



<p id="gdcalert36" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual35.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert37">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual35.jpg "image_tooltip")


Press OK, and start chilling



<p id="gdcalert37" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual36.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert38">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual36.jpg "image_tooltip")


The temperature can be adjusted. The chilling can be skipped by **_LONG PRESS_** “Skip”, and the controller will go to HopStand directly without checking the temperature.



<p id="gdcalert38" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual37.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert39">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual37.jpg "image_tooltip")


When temperature reaches the “start” temperature, the controller prompts for hop adding and start counting time. When time is up, the controller prompt for chilling again if another hopstand follows.


## 4.3 Automation Resumption

If a brew is not ended “normally” by pressing the “END” button after boiling finishes, the brew is considered “unfinished”, and BrewManiacEx will ask for resumption when entering automation.

The system will try to resume from last step.


# 5. Web Interface


## 5.0 Initial Setup

Since v0.4.2, BrewManiacEx **_always_** creates a WiFi network and serves as an AP. The SSID is the same as hostname, and the minimum length of password is 8. The network BrewManaicEx created is always password protected.

Connect your computer/phone/tablet to the network, “bm” by default, to setup WiFi connection if not yet done.

If the html/javascript files are not uploaded and BrewManiacEx is not connected to a network, a page of network selection will show up. If it doesn’t show up automatically, go to [http://192.168.4.1/](http://192.168.4.1/)

Connect BrewManacEx to the internet, so that update page can download the html/javascript files from the internet. You will have to connect to the network that BrewManaicEx connects to, so that the update page will work.



<p id="gdcalert39" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual38.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert40">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual38.jpg "image_tooltip")


If the html/javascript files are already installed, the network setting can be accessed by “Network” tab.



<p id="gdcalert40" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual39.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert41">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual39.jpg "image_tooltip")


The way to change network is the same as initial setup.

To set AP Only mode, check the box and **press OK**.

To change or assign network, fill the information and** press Save.**



<p id="gdcalert41" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual40.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert42">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual40.jpg "image_tooltip")



## 5.1 Serving page



<p id="gdcalert42" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual41.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert43">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual41.jpg "image_tooltip")


~~BrewManiacEx will try to connect to the previously connected network at startup. If the connection is successful, the IP address will be shown. If it can’t connect to the network, it will stay at the “Setup Network..” screen and setup a network named “bm”. Using a computer or phone to connect to that “bm” network, and specify the WiFi network and optional password. If BrewManiacEx connects the network successfully, it will show the IP address on LCD.~~

~~The web interface can be accessed by browsers that support HTML5 and SSE(ServerSideEvent), like **_Chrome_** and **_Safari_**. Microsoft IE and Edge are **not** compatible due to lack of SSE support at the time of writing.~~

~~The hostname which by default is ‘bm’, can be used if mDNS is supported by your phone or computer, which is true for Apple’s products. The serving page can be accesse~~d from

[http://bm.local](http://bm.local)    

or

<span style="text-decoration:underline;">http://**_[IP Address]_**</span>


## 5.2 Main/Panel

This tab is used to control and watch the status of BrewManiacEx. There is a “cover” above the buttons to prevent fat fingers.

**_NOTE: it is not recommended to control the brew remotely._**


## 5.3 Settings



<p id="gdcalert43" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual42.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert44">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual42.png "image_tooltip")





## 5.4 Automation



<p id="gdcalert44" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual43.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert45">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual43.png "image_tooltip")



## 5.5 Recipes

Recipes are created by the import of BeerXML files. The imported BeerXML should have the following information so that it can be used and translated into automation settings:



*   Boil time
*   Mash profile

The following fields are also referenced if available



*   Hop and other additions schedules
    *   The information will be translated into time of “Hop#1”, “Hop #2”, etc.
*   Water amount of first mash step
    *   If the mash-in temperature is set to “calculated”, this field is used in the formula to derive mash-in temperature.

Other fields are display only.




### 5.5.1 Recipe Options



<p id="gdcalert45" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual44.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert46">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual44.png "image_tooltip")




*   Mash-in temperature will be inserted automatically based on the settings. The formula to calculate mash-in temperature is based on[ Palmer’s formula](http://howtobrew.com/book/section-3/the-methods-of-mashing/calculations-for-boiling-water-additions).
*   Temp = R * [Grain weight] / [Water amount] * ( [First rest temp] – [Grain temp] ) + [First rest temp] + [Equipment Adjustment]
*   R: is the heat capacity coefficient of grain.
*   General speaking, the “Equipment Adjust” should be a minus value because the kettle is at the “mash-in” temperature when doughing in.
*   A mash-out is necessary for BrewManiacEx. If the last mash step in the BeerXML recipe is in the mash-out range( >75°C), it will be regarded as the Mashout step. Otherwise a ”Default Mash Out” will be inserted automatically.




### 5.5.2 Import



<p id="gdcalert46" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual45.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert47">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual45.jpg "image_tooltip")


Used to import the beer.xml file



*   Your BeerXML file may contain more than one recipe, if so you can select your desired recipe.
*   You can “Save” or “Brew” your recipe only if it specifies at least the Boil Time and Mash Profile.
*   The saved name can’t contain special characters and spaces. The maximum length is 28.
*   Caution: Using the same recipe name as an existing saved recipe will overwrite the existing saved recipe.
*   You can “Brew” a recipe without having used “Save”. Pressing “Brew” button will set the automation settings.
*   The “Mash-In” temperature is derived from the settings at the time it is shown. The “Mash-out” is set to a default if it is not present in the imported BeerXML file. **The original BeerXML is saved**, and you can have different Mash-In and Mash-out(if not present in the BeerXML) if you change the options before “viewing” a recipe


### 5.5.3 Saved recipe view



<p id="gdcalert47" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual46.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert48">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual46.jpg "image_tooltip")




*   You can “Delete” and “Brew” the saved recipes.
*   You can change the options to get a different Mash-In temperature just before you “Brew” it.




## 5.6 Calculators

For convenience when brewing a series of calculators are provided as follows



<p id="gdcalert48" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual47.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert49">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual47.jpg "image_tooltip")



## 5.7 Brew Logs



<p id="gdcalert49" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual48.jpg). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert50">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual48.jpg "image_tooltip")



## 5.8 Network



<p id="gdcalert50" ><span style="color: red; font-weight: bold">>>>>>  gd2md-html alert: inline image link here (to images/BrewManiacEx-Manual49.png). Store image on your image server and adjust path/filename if necessary. </span><br>(<a href="#">Back to top</a>)(<a href="#gdcalert51">Next alert</a>)<br><span style="color: red; font-weight: bold">>>>>> </span></p>


![alt_text](images/BrewManiacEx-Manual49.png "image_tooltip")


The serving page can be password-protected if “Always need password” option is checked. It is not protected by default.

The username, password, and hostname can be changed in “Network Settings” page. To change the setting in “Network Settings” page, the correct username and password must be provided. **_The change of network settings will be effective after next power on._**

Default values:	Host Name: bm


        UserName: brewmaniac


        Password: rdwhahb!  


        Password before 0.4.2: bdwhahb


## 5.9 Update page


### 5.9.1 Firmware update

**_!!BE CAREFUL!! Uploading the wrong image might brick your controller._**

The firmware can be updated by uploading new *.bin file from browser at this url

http://bm.local:8008/systemupdate


### 5.9.2 Web file update

**_!!BE CAREFUL!! Messing up the files might result in misbehavior of BrewManiacEx._**

To access the SPIFFS file sytem, use the url,

http://bm.local:8008/filemanager

Take care not to mess up the files.

Be Warned! Dangerous!!!

Dangerous!!! Don’t read the following content

_There is a way to format the file system by which all the files and logs will be gone. The url is at http://bm.local/format-spiffs_

Dangerous!!! Don’t read the content brefore


### 5.9.3 Automatic OTA Update

_Functionality currently unstable _

By accessing the url below, BrewManiacEx will check the availability of new firmware and new web page files.

http://bm.local/update

If BrewManiacEx can’t find the files, the update page will be present when main page is accessed. The data files can be downloaded from the network instead of using SPIFFS tool.


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

  Zero crossing relays switch ON/OFF when the voltage crosses zero, which introduces a lag or delay. If both the main heater and sparger heater are controlled by zero crossing relays, it should be fine because they will be turned on and off at the same time when the voltage crosses zero. However, if one of the relay isn’t zero crossing and switches immediately, it might result in overload of power when the turned-off zero crossing relay “waits” for the voltage to cross zero and the non zero crossing relay turns on immediately.

  A minimum 10ms delay between heater switch is introduced to solve this issue. The delay might sometimes extends over 100ms because of the limitation of software structure.

**Q4: Time in brew logs.**

A4: The real clock time will be recorded in the brew log as long as the time is known when the brew starts. BrewManiacEx will try to get time from the internet, NTP servers, at startup. If that fails, it can still get time from the computer or phone that connects to it. Therefore, connect to BrewManaicEx before the brew starts so that it can get the time of your computer or phone if it doesn’t have internet access.



Revision:


<table>
  <tr>
   <td>Dec  7, 2016
   </td>
   <td>0.1 draft V1
   </td>
   <td>Vito Tai
   </td>
  </tr>
  <tr>
   <td>Jan 19, 2017
   </td>
   <td>0.1 draft V2
   </td>
   <td>Iain Hay
   </td>
  </tr>
  <tr>
   <td>Mar 8, 2017
   </td>
   <td>0.2 draft V1
   </td>
   <td>Iain Hay
   </td>
  </tr>
  <tr>
   <td>Mar 14, 2017
   </td>
   <td>0.2 draft V2
   </td>
   <td>Iain Hay
   </td>
  </tr>
  <tr>
   <td>July 5, 2017
   </td>
   <td>0.3 drat V1
   </td>
   <td>Vito Tai
   </td>
  </tr>
</table>



<!-- Docs to Markdown version 1.0β17 -->
