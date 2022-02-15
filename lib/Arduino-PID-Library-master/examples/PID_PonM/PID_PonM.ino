/********************************************************
 * PID Proportional on measurement Example
 * Setting the PID to use Proportional on measurement will
 * make the output move more smoothly when the setpoint
 * is changed.  In addition, it can eliminate overshoot
 * in certain processes like sous-vides.
 ********************************************************/

#include <PID_v2.h>

#define PIN_INPUT 0
#define PIN_OUTPUT 3

// Specify the links and initial tuning parameters
PID_v2 myPID(2, 5, 1, PID::Direct, PID::P_On::Measurement);

void setup() {
  myPID.Start(analogRead(PIN_INPUT),  // input
              0,                      // current output
              100);                   // setpoint
}

void loop() {
  const double input = analogRead(PIN_INPUT);
  const double output = myPID.Run(input);
  analogWrite(PIN_OUTPUT, output);
}
