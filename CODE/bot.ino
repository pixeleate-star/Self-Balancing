


#include <Wire.h>

#define MPU_ADDR 0x68

#define SDA_PIN 21
#define SCL_PIN 22

#define AIN1 25
#define AIN2 26
#define PWMA 27

#define BIN1 32
#define BIN2 33
#define PWMB 14

#define STBY 13

#define PWM_FREQ 20000
#define PWM_RES 8

#define PWM_A 0
#define PWM_B 1

float Kp = 28.0;
float Ki = 0.6;
float Kd = 1.2;

float targetAngle = 0.0;

float complementaryAlpha = 0.98;

float integralLimit = 80.0;

int motorDeadZone = 55;

float leftTrim = 1.00;
float rightTrim = 1.00;

int maxPWM = 255;

float fallAngle = 35.0;

float maxAcceleration = 100.0;

float angle = 0.0;
float gyroBias = 0.0;

float integral = 0.0;

float previousOutput = 0.0;

unsigned long lastMicros = 0;

const uint32_t CONTROL_PERIOD_US = 4000;

void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

void readMPU(
  int16_t &ax,
  int16_t &ay,
  int16_t &az,
  int16_t &gx,
  int16_t &gy,
  int16_t &gz
) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  Wire.requestFrom(MPU_ADDR, 14);

  ax = ((int16_t)Wire.read() << 8) | Wire.read();
  ay = ((int16_t)Wire.read() << 8) | Wire.read();
  az = ((int16_t)Wire.read() << 8) | Wire.read();

  Wire.read();
  Wire.read();

  gx = ((int16_t)Wire.read() << 8) | Wire.read();
  gy = ((int16_t)Wire.read() << 8) | Wire.read();
  gz = ((int16_t)Wire.read() << 8) | Wire.read();
}

void setMotorA(int speed) {
  speed = constrain(speed, -255, 255);

  if (speed > 0) {
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    ledcWrite(PWM_A, speed);
  }
  else if (speed < 0) {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, HIGH);
    ledcWrite(PWM_A, -speed);
  }
  else {
    digitalWrite(AIN1, LOW);
    digitalWrite(AIN2, LOW);
    ledcWrite(PWM_A, 0);
  }
}

void setMotorB(int speed) {
  speed = constrain(speed, -255, 255);

  if (speed > 0) {
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    ledcWrite(PWM_B, speed);
  }
  else if (speed < 0) {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, HIGH);
    ledcWrite(PWM_B, -speed);
  }
  else {
    digitalWrite(BIN1, LOW);
    digitalWrite(BIN2, LOW);
    ledcWrite(PWM_B, 0);
  }
}

int applyDeadZone(float value) {
  if (value == 0) {
    return 0;
  }

  float sign = value > 0 ? 1.0 : -1.0;

  float magnitude = abs(value);

  if (magnitude < 1.0) {
    return 0;
  }

  magnitude = motorDeadZone +
              (magnitude / maxPWM) *
              (maxPWM - motorDeadZone);

  magnitude = constrain(magnitude, motorDeadZone, maxPWM);

  return (int)(sign * magnitude);
}

void setMotors(float output) {
  output = constrain(output, -maxPWM, maxPWM);

  float left = output * leftTrim;
  float right = output * rightTrim;

  left = applyDeadZone(left);
  right = applyDeadZone(right);

  setMotorA(left);
  setMotorB(right);
}

void stopMotors() {
  setMotorA(0);
  setMotorB(0);
}

void calibrateGyro() {
  const int samples = 2500;

  long sum = 0;

  stopMotors();

  delay(500);

  for (int i = 0; i < samples; i++) {
    int16_t ax, ay, az, gx, gy, gz;

    readMPU(ax, ay, az, gx, gy, gz);

    sum += gy;

    delayMicroseconds(1000);
  }

  gyroBias = (float)sum / samples;
}

float getAccelAngle(int16_t ax, int16_t az) {
  return atan2((float)ax, (float)az) * 180.0 / PI;
}

void setupMPU() {
  writeRegister(0x6B, 0x00);

  delay(100);

  writeRegister(0x1A, 0x03);

  writeRegister(0x1B, 0x00);

  writeRegister(0x1C, 0x00);

  writeRegister(0x19, 0x04);
}

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(STBY, OUTPUT);

  digitalWrite(STBY, HIGH);

  ledcSetup(PWM_A, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_B, PWM_FREQ, PWM_RES);

  ledcAttachPin(PWMA, PWM_A);
  ledcAttachPin(PWMB, PWM_B);

  setupMPU();

  delay(500);

  calibrateGyro();

  int16_t ax, ay, az, gx, gy, gz;

  readMPU(ax, ay, az, gx, gy, gz);

  angle = getAccelAngle(ax, az);

  lastMicros = micros();

  stopMotors();

  delay(500);
}

void loop() {
  uint32_t now = micros();

  if ((uint32_t)(now - lastMicros) < CONTROL_PERIOD_US) {
    return;
  }

  float dt = (now - lastMicros) / 1000000.0;

  lastMicros = now;

  int16_t ax, ay, az, gx, gy, gz;

  readMPU(ax, ay, az, gx, gy, gz);

  float accelX = (float)ax / 16384.0;
  float accelY = (float)ay / 16384.0;
  float accelZ = (float)az / 16384.0;

  float accelerationMagnitude =
      sqrt(
        accelX * accelX +
        accelY * accelY +
        accelZ * accelZ
      );

  if (accelerationMagnitude > maxAcceleration / 9.81) {
    return;
  }

  float accelAngle = getAccelAngle(ax, az);

  float gyroRate = ((float)gy - gyroBias) / 131.0;

  angle =
      complementaryAlpha *
      (angle + gyroRate * dt)
      +
      (1.0 - complementaryAlpha) *
      accelAngle;

  if (abs(angle) > fallAngle) {
    stopMotors();

    integral = 0;
    previousOutput = 0;

    return;
  }

  float error = targetAngle - angle;

  integral += error * dt;

  integral = constrain(
      integral,
      -integralLimit,
      integralLimit
  );

  float P = Kp * error;

  float I = Ki * integral;

  float D = -Kd * gyroRate;

  float output = P + I + D;

  output = constrain(
      output,
      -maxPWM,
      maxPWM
  );

  float maxChange = 30.0;

  if (output > previousOutput + maxChange) {
    output = previousOutput + maxChange;
  }

  if (output < previousOutput - maxChange) {
    output = previousOutput - maxChange;
  }

  previousOutput = output;

  setMotors(output);

  static uint32_t lastPrint = 0;

  if (millis() - lastPrint > 100) {
    lastPrint = millis();

    Serial.print("Angle: ");
    Serial.print(angle, 2);

    Serial.print(" | Gyro: ");
    Serial.print(gyroRate, 2);

    Serial.print(" | P: ");
    Serial.print(P, 2);

    Serial.print(" | I: ");
    Serial.print(I, 2);

    Serial.print(" | D: ");
    Serial.print(D, 2);

    Serial.print(" | OUT: ");
    Serial.println(output, 2);
  }
}