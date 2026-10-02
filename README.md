# Self-Balancing Robot

This is the whole codebase for my **self-balancing robot**. It has the code, circuit connections and setup information needed to make your own two-wheel self-balancing robot using an **ESP32, MPU9250, TB6612FNG motor driver and two BO motors**.

The main idea behind this project is to build a small two-wheel robot that can balance itself upright using an **MPU9250 IMU**. The MPU9250 measures the robot's tilt and angular velocity, while a **PID controller** continuously adjusts the motors to keep the robot balanced.

The ESP32 handles the sensor readings and PID calculations, while the TB6612FNG controls the two BO motors.

## Features

- Self-balancing two-wheel robot
- ESP32 as the main controller
- MPU9250 accelerometer and gyroscope
- PID-based balance control
- Complementary filter for tilt estimation
- Gyroscope calibration on startup
- Adjustable PID parameters
- Adjustable target balance angle
- Motor dead-zone compensation
- Left and right motor trim
- Automatic fall detection
- Automatic motor shutdown when the robot falls
- High-frequency PWM motor control
- TB6612FNG dual motor driver
- Battery powered
- Compact and easy-to-build design

## BOM – Bill of Materials

| Component | Quantity | Price (USD) | Purpose | Purchase Link |
|-----------|:--------:|------------:|---------|:-------------:|
| ESP32 Development Board | 1 | $4.18 | Main controller and PID processing | [Buy Here](https://robocraze.com/products/esp32-development-board-with-cp2102-wifi-bluetooth-dual-core-30-pin) |
| MPU9250 Module | 1 | $3.87 | Accelerometer and gyroscope for tilt sensing | [Buy Here](https://robocraze.com/products/mpu-9250-module) |
| 200 RPM BO Motor | 2 | $0.63 each | Left and right drive motors | [Buy Here](https://robocraze.com/products/200-rpm-single-shaft-bo-motor-straight) |
| BO Motor Wheels | 1 pack (4 pcs) | $0.94 | Robot wheels | [Buy Here](https://robocraze.com/products/bo-motor-wheels-4-pcs) |
| TB6612FNG Dual DC Motor Driver | 1 | $1.41 | Drives both BO motors | [Buy Here](https://robocraze.com/products/tb6612fng-dual-dc-motor-driver) |
| Battery | 2 | $2 | Power source | [Buy Here](https://robocraze.com/products/3-7v-2500mah-18650-li-ion-battery) |
| Battery Holder | 1 | $0.25 | Battery connection | [Buy Here](https://robocraze.com/products/18650-2-cell-holder) |
| Buck Convertor | 1 | $0.6 | To give stable 5V to ESP | [Buy Here](https://robocraze.com/products/lm2596-dc-dc-buck-module) |
| Jumper Wires | As required | — | Electrical connections | - |
| 3D Printed Parts | As required | — | Robot frame and structural parts | Self Printed |

## Circuit Diagram

I have added the complete circuit diagram below. You can use it as a reference while wiring your own self-balancing robot.
<img width="1662" height="1176" alt="image" src="https://github.com/user-attachments/assets/c5574bd1-f76d-46d0-b022-4110e69ebc32" />

I know it looks a mesh so here's a link to circuit you can check it out https://app.cirkitdesigner.com/project/33cf55be-d292-4ee9-8185-b1da4f74aae1

### Main Connections

**MPU9250 → ESP32**

| MPU9250 | ESP32 |
|---------|-------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

**TB6612FNG → ESP32**

| TB6612FNG | ESP32 |
|-----------|-------|
| AIN1 | GPIO 25 |
| AIN2 | GPIO 26 |
| PWMA | GPIO 27 |
| BIN1 | GPIO 32 |
| BIN2 | GPIO 33 |
| PWMB | GPIO 14 |
| STBY | GPIO 13 |
| VCC | 3.3V |
| GND | GND |

**Motors**

| TB6612FNG | Motor |
|-----------|-------|
| AO1 / AO2 | Left BO Motor |
| BO1 / BO2 | Right BO Motor |

The motor power supply should be connected to the **VM** input of the TB6612FNG according to the voltage requirements of your motors.

The ESP32 and MPU9250 should use a suitable regulated supply, and **all grounds must be connected together**.

## How to Build this...

**step 1:** First get all the components mentioned in the BOM. The main parts required are the ESP32, MPU9250, TB6612FNG, two BO motors, wheels and a suitable battery.

**step 2:** Connect the MPU9250 to the ESP32. Connect SDA to GPIO 21 and SCL to GPIO 22. Connect VCC to 3.3V and GND to GND.

**step 3:** Connect the TB6612FNG to the ESP32 according to the connection table above. Connect the two BO motors to the A and B motor outputs.

**step 4:** Connect the motor battery to the VM and GND terminals of the TB6612FNG. Make sure the battery voltage is suitable for the motors and motor driver.

**step 5:** Connect the ESP32 power supply separately through a suitable regulated supply if required. Do not connect an unregulated motor battery directly to a 3.3V ESP32 pin.

**step 6:** Upload the self-balancing code to the ESP32. In Arduino IDE select the appropriate ESP32 board for your hardware.

**step 7:** Once the ESP32 starts, keep the robot completely still and upright during startup. The code takes multiple readings from the MPU9250 gyroscope and calculates the gyro bias automatically.

**step 8:** After calibration, hold the robot and slowly tilt it forward and backward. Check the motor direction before allowing the robot to stand freely.

When the robot tilts forward:

**Forward tilt → Motors should drive forward → Robot returns upright**

When the robot tilts backward:

**Backward tilt → Motors should drive backward → Robot returns upright**

If the motor direction is reversed, invert the PID output in the code.

**step 9:** Place the robot on a flat surface and carefully allow it to balance. Start with conservative PID values and tune the controller gradually.

**step 10:** Finally, assemble everything inside the robot frame. The exact placement of the MPU9250 matters because its orientation determines how the robot measures its tilt.

## PID Tuning

The main PID parameters can be adjusted directly in the code:


float Kp = 28.0;
float Ki = 0.6;
float Kd = 1.2;

float targetAngle = 0.0;


The parameters control the balancing behavior:

| Parameter | Purpose |
|-----------|---------|
| `Kp` | Controls how strongly the robot reacts to tilt |
| `Ki` | Corrects long-term balance error |
| `Kd` | Controls damping and reduces oscillation |
| `targetAngle` | Sets the desired upright angle |

For tuning, start with:


Ki = 0;


Increase `Kp` until the robot starts reacting strongly to falling.

Then increase `Kd` until the robot becomes less oscillatory.

Finally, add a small amount of `Ki` if the robot consistently leans in one direction.

## Motor Calibration

The motors may not rotate at exactly the same speed even when they receive the same PWM value.

The code includes adjustable motor trim:


float leftTrim = 1.00;
float rightTrim = 1.00;


If the robot constantly moves toward one side while balancing, these values can be adjusted.

For example:


float leftTrim = 1.00;
float rightTrim = 0.96;


The exact values depend on the motors, gearbox, wheels and physical construction of the robot.

## Motor Dead Zone

BO motors usually do not start moving at very low PWM values.

The code therefore includes:


int motorDeadZone = 55;


If the motors don't respond to small corrections, increase this value.

If the robot makes unnecessarily large movements around the balance point, reduce it.

## Fall Detection

The robot automatically stops the motors when the measured angle exceeds the configured safety limit:


float fallAngle = 35.0;


This prevents the motors from continuously running when the robot has already fallen over.

After the robot is placed upright again, the controller can resume balancing.

## Code

The main code uses:

- `Wire.h` for I²C communication with the MPU9250
- MPU9250 accelerometer for tilt estimation
- MPU9250 gyroscope for angular velocity
- Complementary filter for stable angle estimation
- PID control for balancing
- ESP32 PWM for motor control
- TB6612FNG for driving the two BO motors

The main parameters can be adjusted directly in the code:


float Kp = 28.0;
float Ki = 0.6;
float Kd = 1.2;

float targetAngle = 0.0;

float complementaryAlpha = 0.98;

int motorDeadZone = 55;

float leftTrim = 1.00;
float rightTrim = 1.00;


## Known Issues

1. The robot requires accurate PID tuning for stable balancing.
2. The motor direction depends on the physical wiring of the BO motors.
3. The MPU9250 needs to remain still during startup gyro calibration.
4. Different MPU9250 modules can have slightly different gyro offsets.
5. Motor speed can vary between individual BO motors.
6. The physical position and orientation of the MPU9250 affects the balance angle.
7. Battery voltage affects motor response and balancing performance.
8. Wheel size and robot center of gravity significantly affect stability.
9. Very low PWM values may not be enough to start the BO motors.
10. A poorly centered or mechanically flexible frame can make balancing significantly harder.
11. The robot can become unstable if the battery voltage drops too low.
12. The PID parameters may need to be changed when the robot's weight, wheels or motor gearing are changed.

## CAD Models

<img width="3024" height="1964" alt="image" src="https://github.com/user-attachments/assets/ac1df77d-87ad-4e0b-b315-a1c4ca1dff9a" />
<img width="3024" height="1964" alt="image" src="https://github.com/user-attachments/assets/cc7d00a6-cc0a-438c-b90e-db2d0fbf254b" />
<img width="3024" height="1964" alt="image" src="https://github.com/user-attachments/assets/248175d8-4802-4ffe-8a9f-d9e4189811c3" />



## Working Principle

The MPU9250 continuously measures the robot's orientation.

The accelerometer provides information about the direction of gravity, while the gyroscope measures how quickly the robot is rotating.

These measurements are combined using a **complementary filter** to estimate the robot's current tilt angle.

The PID controller then calculates how much the motors need to move:


MPU9250
   ↓
Accelerometer + Gyroscope
   ↓
Complementary Filter
   ↓
Tilt Angle
   ↓
PID Controller
   ↓
Motor Output
   ↓
TB6612FNG
   ↓
BO Motors
   ↓
Robot moves back toward upright position


The controller continuously repeats this process at a high frequency, allowing the robot to make rapid corrections and remain balanced.

## Final Result

The whole idea is pretty simple but the control system makes it interesting — instead of manually controlling the robot, the robot continuously measures its own tilt and moves the wheels automatically to keep itself upright.
full video---- https://drive.google.com/file/d/15Oyu7o3CPmcscNef3DqKq84Rv_wBlsnM/view?usp=sharing
It is a compact introduction to **IMU sensing, sensor fusion, PID control, motor control and embedded robotics**, all combined into one project.
working video: https://youtube.com/shorts/BoeQEbhFIXY

mine bot------
<img width="542" height="608" alt="image" src="https://github.com/user-attachments/assets/4bcdfd31-084c-463d-818d-e27793bdb4cc" />



