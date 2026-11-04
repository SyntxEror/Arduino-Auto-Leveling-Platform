Technical Documentation

1. System Overview

The Arduino Auto-Leveling Platform is designed to maintain a horizontal platform while its supporting structure is tilted or moved.

The system uses an MPU6500 IMU to measure tilt and an SG90 servo motor to adjust the platform in the opposite direction.

2. Hardware Components

Component| Purpose
Arduino UNO R3| Main controller
MPU6500 IMU| Measures platform orientation
SG90 Servo Motor| Adjusts platform position
External 5V Supply| Powers the servo
Mechanical Linkage| Transfers servo movement to the platform

3. Circuit Connections

MPU6500| Arduino UNO
SDA| A4
SCL| A5
GND| GND
VCC| According to breakout specifications

Servo| Connection
Signal| D9
VCC| External 5V supply
GND| Common ground

Important: The external servo supply and Arduino must share a common ground. Do not assume the Arduino's 5V pin can reliably power the servo.

4. Working Principle

The system operates through the following sequence:

1. The MPU6500 detects changes in the platform's tilt.
2. The Arduino reads and processes the sensor measurements.
3. The controller calculates the required correction using proportional control.
4. The servo rotates to compensate for the detected tilt.
5. This process repeats continuously to maintain the platform's horizontal position.

5. Control Algorithm

The current implementation uses a Proportional (P) controller.

The correction is proportional to the measured tilt error:

Correction = Kp × Error

Where:

- Kp: Proportional gain.
- Error: Difference between the desired level and measured tilt.
- Correction: Servo adjustment required to compensate for the tilt.

A deadband is also implemented to prevent unnecessary servo movement when the measured error is very small.

6. Software Configuration

The firmware includes configurable parameters for:

- Tilt measurement axis
- Correction direction
- Proportional gain (Kp)
- Deadband
- Servo centre position
- Servo movement limits
- Control loop interval

These parameters can be adjusted to improve stabilization and compensate for mechanical differences.

7. Simulation and Images

The "images/" directory contains visual references of the circuit and physical prototype.

A Wokwi circuit simulation can also be linked here:

Wokwi Simulation: https://wokwi.com/projects/476696343651801089

8. Current Development Status

The physical prototype has been assembled and tested.

The platform demonstrates automatic tilt compensation using sensor feedback and servo-based correction.

Further improvements may include:

- Reducing mechanical backlash.
- Improving stabilization accuracy.
- Testing different control parameters.
- Exploring more advanced control algorithms.
- Improving the mechanical design.

9. Future Development

Possible future developments include PID-based control, improved actuator precision, multi-axis stabilization, and more detailed performance testing.
