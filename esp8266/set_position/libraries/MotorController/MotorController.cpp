#include "MotorController.h"
#include <Wire.h>

# define GPIO 0x09
# define DELAY 2


MotorController::MotorController(int sda, int scl, const MotorConfig* motors, int nMotors) {
  _sda = sda;
  _scl = scl;
  _nMotors = nMotors;
  
  for(int i=0; i < nMotors && i < NUM_MOTORS; i++) {
    _motors[i] = motors[i];
    currentPositions[i] = 0;
    targetPositions[i] = 0;
  }
}

void MotorController::setup() {
  Wire.begin(_sda, _scl);
  
  // Initialize all unique addresses found in configuration
  for(int i=0; i < _nMotors; i++) {
    // Simple check to avoid re-initializing same address multiple times if possible, 
    // but re-writing config is usually harmless for these IO expanders.
    Wire.beginTransmission(_motors[i].address);
    Wire.write(0x00); // IODIR (Direction)
    Wire.write(0x00); // All outputs
    Wire.endTransmission();
  }
}

void MotorController::update() {
  for (int i = 0; i < _nMotors; i++) {
    if (currentPositions[i] != targetPositions[i]) {
      // Move one step towards target
      int dir = (targetPositions[i] > currentPositions[i]) ? 1 : -1;
      step(_motors[i].address, _motors[i].shift, dir);
      currentPositions[i] += dir;
    }
  }
}

// Move motor at address and shift location by steps
// positive steps = cw, negative = ccw
void MotorController::step(int addr, int shift, int steps) {  
  // Clockwise step sequence (half-step or full-step? 0x20/0x21 suggestions implied PCF8574 driving ULN2003)
  // Sequence: 1100, 0110, 0011, 1001 (AB, BC, CD, DA) - This is standard 4-step sequence
  static const int stepSequence[] = {
    0b1100,
    0b0110,
    0b0011,
    0b1001
  };
  
  // Wire.begin(_sda, _scl);
  int absSteps = abs(steps);
  
  for(int s = 0; s < absSteps; s++) {
    if(steps > 0) {
      // Clockwise
      for(int i = 0; i < 4; i++) {
        byte val = stepSequence[i];
        if(shift) {
          val = val << 4;
        }
        delay(DELAY);
        Wire.beginTransmission(addr);
        Wire.write(GPIO);
        Wire.write(val);
        Wire.endTransmission();
      }
    } else {
      // Counter-Clockwise
      for(int i = 3; i >= 0; i--) {
        byte val = stepSequence[i];
        if(shift) {
          val = val << 4;
        }

        delay(DELAY);
        Wire.beginTransmission(addr);
        Wire.write(GPIO);
        Wire.write(val);
        Wire.endTransmission();
      }
    }
  }
}

void MotorController::moveStepper(int motorIndex, int steps) {
  if (motorIndex < 0 || motorIndex >= _nMotors) return;
  // Blocking move for now, or just set target?
  // If update() is called in loop, we should set target. 
  // But existing code in handleMove calls moveStepper expecting action?
  // set_position.ino loop calls update(). 
  // Let's set target position.
  targetPositions[motorIndex] = currentPositions[motorIndex] + steps;
}

long MotorController::getPosition(int motorIndex) {
  if (motorIndex < 0 || motorIndex >= NUM_MOTORS) return 0;
  return currentPositions[motorIndex];
}

void MotorController::setPosition(int motorIndex, long position) {
  if (motorIndex < 0 || motorIndex >= NUM_MOTORS) return;
  currentPositions[motorIndex] = position;
  targetPositions[motorIndex] = position;
}

void MotorController::setZero(int motorIndex) {
  setPosition(motorIndex, 0);
}
