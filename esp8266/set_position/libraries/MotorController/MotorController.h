#ifndef MotorController_h
#define MotorController_h

#include <Arduino.h>

#define NUM_MOTORS 5

struct MotorConfig {
  int address;
  int shift;
};

class MotorController {
  public:
    MotorController(int sda_pin, int scl_pin, const MotorConfig* motors, int nMotors);
    void setup(); // Added setup method to separate hardware init from constructor if needed
    void moveStepper(int motorIndex, int steps);
    void update();
    void step(int addr, int shift, int steps);
    long getPosition(int motorIndex);
    void setPosition(int motorIndex, long position);
    void setZero(int motorIndex);

  private:
    int _scl;
    int _sda;
    MotorConfig _motors[NUM_MOTORS];
    int _nMotors;
    long currentPositions[NUM_MOTORS];
    long targetPositions[NUM_MOTORS];
};

#endif
