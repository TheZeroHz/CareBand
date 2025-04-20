#include "FastIMU.h"
#include <Wire.h>
#include <TFT_eSPI.h>
#include "Madgwick.h"
#include <QMC5883LCompass.h>

#define I2C0_SDA 6
#define I2C0_SCL 7
#define I2C1_SDA 17  // Your custom SDA pin for second I2C
#define I2C1_SCL 16   // Your custom SCL pin for second I2C
#define I2C0_FREQ 400000  // 400 kHz fast mode
#define I2C1_FREQ 400000  // 400 kHz fast mode
#define IMU_ADDRESS 0x6B
#define MAG_ADDRESS 0x0D
#define PERFORM_CALIBRATION

QMC5883LCompass compass;
QMI8658 IMU;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
calData calib = { 0 };
AccelData accelData;
GyroData gyroData;
MagData magData;
Madgwick filter;
void setup() {
  Wire1.begin(I2C1_SDA, I2C1_SCL, I2C1_FREQ);
  Wire.begin(I2C0_SDA, I2C0_SCL, I2C0_FREQ);
  Serial.begin(115200);
  compass.init();
  tft.init();
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  spr.createSprite(240, 240);
  int err = IMU.init(calib, IMU_ADDRESS);
  if (err != 0) {
    tft.drawString("IMU Init Error", 120, 120, 2);
    while (true);
  }

#ifdef PERFORM_CALIBRATION
  tft.drawString("Calibrating Gyro+Acc...", 120, 120, 2);
  IMU.calibrateAccelGyro(&calib);
  tft.drawString("Calibrating Mag...", 120, 120, 2);
  compass.calibrate();
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Calibration Done!", 120, 120, 2);
  delay(2000);
  IMU.init(calib, IMU_ADDRESS);
  filter.begin(0.2f);
#endif
}

void loop() {
IMU.update();
compass.read();
IMU.getAccel(&accelData);
IMU.getGyro(&gyroData);
float mx=compass.getX(),my=compass.getY(),mz=compass.getZ();
filter.update(gyroData.gyroX, gyroData.gyroY, gyroData.gyroZ, accelData.accelX, accelData.accelY, accelData.accelZ,mx,my,mz);
    // --- Add this section for serial output ---
  // Create JSON string with necessary data
  String jsonStr = "{";
  jsonStr += "\"qw\":" + String(filter.getQuatW()) + ",";
  jsonStr += "\"qx\":" + String(filter.getQuatX()) + ",";
  jsonStr += "\"qy\":" + String(filter.getQuatY()) + ",";
  jsonStr += "\"qz\":" + String(filter.getQuatZ()) + ",";
  jsonStr += "\"ax\":" + String(accelData.accelX) + ",";
  jsonStr += "\"ay\":" + String(accelData.accelY) + ",";
  jsonStr += "\"az\":" + String(accelData.accelZ);
  jsonStr += "}";
  // Send JSON data over serial
  Serial.println(jsonStr);
}
