# Arduino PID controller library, version 2

This is an update/refactor of https://github.com/br3ttb/Arduino-PID-Library,
since the original library seems pretty much abandoned.

For an explanation of many of the implementation details see the excellent
series of posts by the original library author:
http://brettbeauregard.com/blog/2011/04/improving-the-beginners-pid-introduction/

## Usage

```c++
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
```

I've added a few things, but code written with the old library should still work
after changing `#include` line.

## Authors

* Brett Beauregard <br3ttb@gmail.com> brettbeauregard.com
* Max Ignatenko
