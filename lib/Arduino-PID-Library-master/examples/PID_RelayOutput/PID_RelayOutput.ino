/********************************************************
 * PID RelayOutput Example
 * Same as basic example, except that this time, the output
 * is going to a digital pin which (we presume) is controlling
 * a relay.  the pid is designed to Output an analog value,
 * but the relay can only be On/Off.
 *
 *   to connect them together we use "time proportioning
 * control"  it's essentially a really slow version of PWM.
 * first we decide on a window size (5000mS say.) we then
 * set the pid to adjust its output between 0 and that window
 * size.  lastly, we add some logic that translates the PID
 * output into "Relay On Time" with the remainder of the
 * window being "Relay Off Time"
 ********************************************************/

#include <PID_v2.h>

#define PIN_INPUT 0
#define RELAY_PIN 6

// Specify the links and initial tuning parameters
double Kp = 2, Ki = 5, Kd = 1;
PID_v2 myPID(Kp, Ki, Kd, PID::Direct);

const int WindowSize = 5000;
unsigned long windowStartTime;

void setup() {
  windowStartTime = millis();

  // tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(0, WindowSize);

  // turn the PID on
  myPID.Start(analogRead(PIN_INPUT),  // input
              0,                      // current output
              100);                   // setpoint
}

void loop() {
  const double input = analogRead(PIN_INPUT);
  const double output = myPID.Run(input);

  /************************************************
   * turn the output pin on/off based on pid output
   ************************************************/
  while (millis() - windowStartTime > WindowSize) {
    // time to shift the Relay Window
    windowStartTime += WindowSize;
  }
  if (output < millis() - windowStartTime)
    digitalWrite(RELAY_PIN, HIGH);
  else
    digitalWrite(RELAY_PIN, LOW);
}
