/*
  Auto-leveling platform - MPU6500 version

  Based on the Science Buddies auto-leveling platform activity.
  The ADXL335 (analog, pin A0) is replaced by an MPU6500 (digital, I2C).
  Paste this into a new Arduino IDE sketch.

  WIRING
    MPU6500 VCC -> 3.3V or 5V (check what your breakout board supports)
    MPU6500 GND -> GND
    MPU6500 SDA -> A4
    MPU6500 SCL -> A5
    MPU6500 AD0 -> GND  (address 0x68; connect to VCC for 0x69)
    Servo signal -> D9
    Servo power  -> external 5V supply, with its GND joined to Arduino GND
    Nothing is connected to A0 any more.

  HOW IT DIFFERS FROM THE ADXL335 CODE
    - The sensor is read over I2C and converted to a tilt angle in degrees.
    - "Level" is 0 degrees (not an ADC value like 340). By default the setpoint
      is measured automatically at power-up: hold the base level for ~3 seconds.
    - The loop runs at a fixed rate (LOOP_MS), which replaces the delay() experiment.
    - Deadband, step size and Kp are all variables at the top of the file.

  SERIAL MONITOR: use 115200 baud.
*/

#include <Wire.h>
#include <Servo.h>

// ====================== USER SETTINGS ======================

// Controller type: 0 = on/off controller, 1 = proportional controller
const int CONTROL_MODE = 1;

// Print values to the Serial Monitor. Set to false for the best speed.
const bool DEBUG = true;

// MPU6500 I2C address (0x68 if AD0 is on GND, 0x69 if AD0 is on VCC)
const uint8_t MPU_ADDR = 0x68;

// Accelerometer axis that changes when the platform tilts: 'X' or 'Y'
const char TILT_AXIS = 'X';

// +1 or -1. If the platform runs away to one end instead of levelling, flip it.
const int DIRECTION = 1;

// Setpoint (level reference, in degrees)
// true  = measure it at power-up (hold the base level while it calibrates)
// false = use MANUAL_SETPOINT_DEG, read from the Serial Monitor like in the lab
const bool AUTO_CALIBRATE = true;
const float MANUAL_SETPOINT_DEG = 0.0;

// Deadband: the servo holds still while the tilt is within +/- this many degrees
// of the setpoint. Set to 0 to see the lab's "exact match" behaviour.
const float DEADBAND_DEG = 1.0;

// On/off controller: how far the servo moves each loop (degrees)
const float ONOFF_STEP_DEG = 1.0;

// Proportional controller gain. Lower it if the servo oscillates,
// then raise it in small steps for a faster response.
const float Kp = 0.1;

// Time between control updates (milliseconds). Servos update at about 50 Hz,
// so 20 ms is a good start. Raise it if the servo overshoots or buzzes.
const unsigned long LOOP_MS = 20;

// Servo settings (adjust to match your servo and linkage)
const int SERVO_PIN = 9;
const int SERVO_CENTER_US = 1700;   // pulse width when the platform is level
const int SERVO_MIN_US = 1000;      // lowest safe pulse width
const int SERVO_MAX_US = 2400;      // highest safe pulse width
const float US_PER_DEG = 10.0;      // roughly 10 us per degree for a typical SG90

// ===========================================================

// MPU6500 registers
const uint8_t REG_WHO_AM_I      = 0x75;
const uint8_t REG_PWR_MGMT_1    = 0x6B;
const uint8_t REG_ACCEL_CONFIG  = 0x1C;
const uint8_t REG_ACCEL_CONFIG2 = 0x1D;
const uint8_t REG_ACCEL_XOUT_H  = 0x3B;

// +/-2 g range gives 16384 counts per g
const float ACCEL_LSB_PER_G = 16384.0;

Servo servo1;

float setpointDeg = 0.0;     // tilt reading that counts as "level"
float servoCmdDeg = 0.0;     // servo command, degrees away from center
unsigned long lastLoop = 0;

// ---------- MPU6500 helpers ----------

void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, (uint8_t)1);
  return Wire.read();
}

// Reads the tilt angle in degrees. Returns false if the read failed.
bool readTilt(float &tilt) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(REG_ACCEL_XOUT_H);
  Wire.endTransmission(false);

  // X and Y accelerometer values (4 bytes)
  if (Wire.requestFrom(MPU_ADDR, (uint8_t)4) < 4) {
    return false;
  }

  // Read each byte on its own line so the order is guaranteed
  uint8_t xHigh = Wire.read();
  uint8_t xLow  = Wire.read();
  uint8_t yHigh = Wire.read();
  uint8_t yLow  = Wire.read();

  int16_t rawX = (int16_t)((xHigh << 8) | xLow);
  int16_t rawY = (int16_t)((yHigh << 8) | yLow);

  int16_t raw = (TILT_AXIS == 'X') ? rawX : rawY;

  // Convert to g, then to a tilt angle (single-axis tilt)
  float g = raw / ACCEL_LSB_PER_G;
  g = constrain(g, -1.0, 1.0);
  tilt = asin(g) * 57.29578;   // radians to degrees
  return true;
}

// Averages many readings while the platform is held level
float calibrateSetpoint() {
  float sum = 0.0;
  int count = 0;
  for (int i = 0; i < 200; i++) {
    float t;
    if (readTilt(t)) {
      sum += t;
      count++;
    }
    delay(5);
  }
  if (count == 0) {
    return 0.0;
  }
  return sum / count;
}

// ---------- Servo helper ----------

void writeServo() {
  float us = SERVO_CENTER_US + servoCmdDeg * US_PER_DEG;
  us = constrain(us, SERVO_MIN_US, SERVO_MAX_US);

  // Keep the stored command inside the real range so it cannot "wind up"
  servoCmdDeg = (us - SERVO_CENTER_US) / US_PER_DEG;

  servo1.writeMicroseconds((int)us);
}

// ---------- Main program ----------

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);

  // Make sure the MPU6500 answers on the I2C bus
  Wire.beginTransmission(MPU_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println(F("MPU6500 not found. Check wiring and the I2C address."));
    while (true) {
      // stop here; the servo is not attached, so it will not move
    }
  }

  writeRegister(REG_PWR_MGMT_1, 0x00);     // wake the sensor up
  delay(100);
  writeRegister(REG_ACCEL_CONFIG, 0x00);   // +/-2 g range
  writeRegister(REG_ACCEL_CONFIG2, 0x03);  // low-pass filter at about 44 Hz

  Serial.print(F("WHO_AM_I: 0x"));
  Serial.println(readRegister(REG_WHO_AM_I), HEX);

  // Attach the servo and move it to center
  servo1.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);
  writeServo();
  delay(500);

  if (AUTO_CALIBRATE) {
    Serial.println(F("Hold the base level. Calibrating in 3 seconds..."));
    delay(3000);
    setpointDeg = calibrateSetpoint();
  } else {
    setpointDeg = MANUAL_SETPOINT_DEG;
  }

  Serial.print(F("Setpoint (deg): "));
  Serial.println(setpointDeg, 2);
  Serial.println(F("Running."));
}

void loop() {
  // Run the controller at a fixed rate
  unsigned long now = millis();
  if (now - lastLoop < LOOP_MS) {
    return;
  }
  lastLoop = now;

  // Read the sensor
  float tilt;
  if (!readTilt(tilt)) {
    return;   // skip this loop if the read failed
  }

  // Error = how far the platform is from level
  float error = tilt - setpointDeg;

  // Deadband: ignore small errors
  if (fabs(error) <= DEADBAND_DEG) {
    error = 0.0;
  }

  // Work out how much to move the servo this loop
  float change = 0.0;
  if (CONTROL_MODE == 0) {
    // On/off controller: fixed step toward level
    if (error < 0.0) {
      change = -ONOFF_STEP_DEG;
    } else if (error > 0.0) {
      change = ONOFF_STEP_DEG;
    }
  } else {
    // Proportional controller: bigger error, bigger step
    change = Kp * error;
  }

  servoCmdDeg = servoCmdDeg + DIRECTION * change;
  writeServo();

  if (DEBUG) {
    Serial.print(F("Tilt: "));
    Serial.print(tilt, 1);
    Serial.print(F(" | Error: "));
    Serial.print(error, 1);
    Serial.print(F(" | Servo cmd: "));
    Serial.println(servoCmdDeg, 1);
  }
}