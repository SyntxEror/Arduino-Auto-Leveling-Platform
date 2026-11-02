Arduino Auto-Leveling Platform

An Arduino-based self-leveling platform designed to maintain a horizontal surface while its supporting handle is tilted.

Project Overview

This project uses an Arduino UNO R3, an MPU6500 inertial measurement unit (IMU), and an SG90 servo motor to detect and compensate for platform tilt.

Hardware Components

- Arduino UNO R3
- MPU6500 accelerometer and gyroscope
- SG90 servo motor
- External 5V servo power supply
- Mechanical platform and linkage

Working Principle

1. The MPU6500 measures the platform's tilt.
2. The Arduino processes the sensor readings.
3. A proportional control algorithm calculates the correction.
4. The servo adjusts the platform to compensate for the movement.

Pin Connections

Component| Arduino connection
MPU6500 SDA| A4
MPU6500 SCL| A5
MPU6500 GND| GND
MPU6500 VCC| According to breakout specifications
Servo signal| D9

The servo uses an external 5V supply with a common ground.

Software

- Arduino IDE
- Arduino C/C++
- Wire library
- Servo library

Repository Structure

Arduino-Auto-Leveling-Platform/
├── README.md
├── LICENSE
├── src/
│   └── Auto-Leveling.ino
├── docs/
└── images/

Current Status

Working physical prototype. Improvements to stabilization, mechanical precision, and control behaviour remain possible.

Future Improvements

- Improved servo precision
- Mechanical optimization
- Advanced control algorithms
- Performance testing and documentation

License

MIT License. See "LICENSE" (LICENSE).
