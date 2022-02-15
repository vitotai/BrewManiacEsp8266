/********************************************************
 * PID Basic Example
 * Reading analog input 0 to control analog PWM output 3
 ********************************************************/

#include <PID_v2.h>

#define PIN_INPUT 0
#define PIN_OUTPUT 3

// Specify the links and initial tuning parameters
double Kp = 2, Ki = 5, Kd = 1;
PID_v2 myPID(Kp, Ki, Kd, PID::Direct);

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
