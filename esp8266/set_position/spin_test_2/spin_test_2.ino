#include <MotorController.h>
#include <Wire.h>

MotorConfig configs[] = {
  {0x20, 0}, 
  {0x20, 1},
  {0x22, 0},
  {0x22, 1},
  {0x24, 0}, 
  {0x24, 1}
};

MotorController m21(D2, D1, configs, 4);

void setup() {
  m21.setup();
}

void loop() {
  m21.step(0x20, 0, 1);
  m21.step(0x20, 1, -1);
  m21.step(0x22, 0, 1);
  m21.step(0x22, 1, -1);
  m21.step(0x24, 0, -1);
  m21.step(0x24, 1, 1);
}
