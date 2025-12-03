#include <MotorController.h>
#include <Wire.h>

// MotorConfig configs[] = {
//   {0x20, 0}, 
//   {0x20, 1}, 
//   {0x21, 0}, 
//   {0x21, 1}
// };

// MotorController m67(D2, D1, configs, 4);

void setup() {
  // m67.setup();
  Wire.begin(D2, D1);
  Wire.beginTransmission(0x20);
  Wire.write(0x00); // IODIR (Direction)
  Wire.write(0x00); // All outputs
  Wire.endTransmission();

  Wire.beginTransmission(0x20);
  Wire.write(0x09);
  Wire.write(0b11111111);
  Wire.endTransmission();
}

void loop() {
  int speed = 5;
  Wire.beginTransmission(0x20);
  Wire.write(0x09);
  Wire.write(0b1100);
  Wire.endTransmission();

  delay(speed);

  Wire.beginTransmission(0x20);
  Wire.write(0x09);
  Wire.write(0b0110);
  Wire.endTransmission();

  delay(speed);

  Wire.beginTransmission(0x20);
  Wire.write(0x09);
  Wire.write(0b0011);
  Wire.endTransmission();

  delay(speed);

  Wire.beginTransmission(0x20);
  Wire.write(0x09);
  Wire.write(0b1001);
  Wire.endTransmission();

  delay(speed);
  // m67.step(0x20, 0, 1);
  // m67.step(0x20, 1, -1);
  // m67.step(0x21, 0, -1);
  // m67.step(0x21, 1, 1);
}
