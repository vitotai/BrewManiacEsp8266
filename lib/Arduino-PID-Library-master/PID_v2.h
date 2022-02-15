#ifndef PID_v2_h
#define PID_v2_h

class PID {
 public:
  // Constants used in some of the functions below
  enum Mode { Manual = 0, Automatic = 1 };
  enum Direction { Direct = 0, Reverse = 1 };
  enum class P_On { Measurement = 0, Error = 1 };

  // commonly used functions
  // **************************************************************************
  // Constructor. Links the PID to the Input, Output, and Setpoint. Initial
  // tuning parameters are also set here. (overload for specifying proportional
  // mode)
  PID(double *Input, double *Output, double *Setpoint, double Kp, double Ki,
      double Kd, P_On, Direction);

  // Constructor. Links the PID to the Input, Output, and Setpoint. Initial
  // tuning parameters are also set here
  PID(double *Input, double *Output, double *Setpoint, double Kp, double Ki,
      double Kd, Direction);

  // Sets PID to either Manual (0) or Auto (non-0)
  void SetMode(Mode Mode);

  // Performs the PID calculation. It should be called every time loop()
  // cycles. ON/OFF and calculation frequency can be set using SetMode and
  // SetSampleTime respectively
  bool Compute();

  // Clamps the output to a specific range. 0-255 by default, but it's likely
  // the user will want to change this depending on the application
  void SetOutputLimits(double, double);

  // available but not commonly used functions
  // ********************************************************
  // While most users will set the tunings once in the constructor, this
  // function gives the user the option of changing tunings during runtime for
  // Adaptive control
  void SetTunings(double, double, double);
  // Overload for specifying proportional mode
  void SetTunings(double, double, double, P_On);

  // Sets the Direction, or "Action" of the controller. DIRECT means the output
  // will increase when error is positive. REVERSE means the opposite. It's very
  // unlikely that this will be needed once it is set in the constructor.
  void SetControllerDirection(Direction);

  // Sets the frequency, in Milliseconds, with which the PID calculation is
  // performed. Default is 100.
  void SetSampleTime(int);

  // Display functions
  // ****************************************************************
  // These functions query the pid for interal values.they were created mainly
  // for the pid front-end, where it's important to know what is actually inside
  // the PID.
  double GetKp() const;
  double GetKi() const;
  double GetKd() const;
  Mode GetMode() const;
  Direction GetDirection() const;
  double GetLastP() const;
  double GetLastI() const;
  double GetLastD() const;

 private:
  void Initialize();

  double dispKp;  // * we'll hold on to the tuning parameters in user-entered
  double dispKi;  //   format for display purposes
  double dispKd;  //

  double kp;  // * (P)roportional Tuning Parameter
  double ki;  // * (I)ntegral Tuning Parameter
  double kd;  // * (D)erivative Tuning Parameter

  Direction controllerDirection;
  P_On pOn;

  // Pointers to the Input, Output, and Setpoint variables. This creates a hard
  // link between the variables and the PID, freeing the user from having to
  // constantly tell us what these values are.  with pointers we'll just know.
  double *myInput, *myOutput, *mySetpoint;

  unsigned long lastTime;
  double outputSum, lastInput;
  double lastP, lastD;

  unsigned long SampleTime;
  double outMin, outMax;
  bool inAuto, pOnE;
};

class PID_v2 : public PID {
 public:
  PID_v2(double Kp, double Ki, double Kd, Direction dir, P_On POn = P_On::Error)
      : PID(&this->input, &this->output, &this->setpoint, Kp, Ki, Kd, POn,
            dir) {}

  void Setpoint(double v) { this->setpoint = v; }
  double GetSetpoint() const { return this->setpoint; }

  // Initializes and enables PID controller. Current input and output values
  // are needed for a smooth transision to automatic control.
  void Start(double input_, double currentOutput, double setpoint_) {
    this->input = input_;
    this->output = currentOutput;
    this->setpoint = setpoint_;
    this->SetMode(Mode::Automatic);
  }

  // Recompute the values and return the output value. Note that computation
  // (and therefore, change of output value) will happen only once every sample
  // time, set by SetSampleTime method.
  double Run(double input_) {
    this->input = input_;
    this->Compute();
    return this->output;
  }

 private:
  double input, output, setpoint;
};

#ifndef PID_v2_SKIP_COMPAT_WITH_v1
const PID::Mode AUTOMATIC = PID::Automatic;
const PID::Mode MANUAL = PID::Manual;
const PID::Direction DIRECT = PID::Direct;
const PID::Direction REVERSE = PID::Reverse;
const PID::P_On P_ON_M = PID::P_On::Measurement;
const PID::P_On P_ON_E = PID::P_On::Error;
#endif

#endif  // PID_v2_h
