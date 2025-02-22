#include "FastIMU.h"
#include <Wire.h>
#include <TFT_eSPI.h>
#include "Madgwick.h"


#define DEV_SDA_PIN     (6)
#define DEV_SCL_PIN     (7)
#define IMU_ADDRESS     0x6B
#define PERFORM_CALIBRATION 

QMI8658 IMU;
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
calData calib = { 0 };
AccelData accelData;
GyroData gyroData;
MagData magData;
Madgwick filter;
void setup() {
  Wire.setPins(DEV_SDA_PIN, DEV_SCL_PIN);
  Wire.begin();

  Wire.setClock(400000);
  Serial.begin(115200);

  tft.init();
  tft.setRotation(0);
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
  tft.drawString("Calibrating IMU...", 120, 120, 2);
  delay(5000);
  IMU.calibrateAccelGyro(&calib);
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Calibration Done!", 120, 120, 2);
  delay(2000);
  IMU.init(calib, IMU_ADDRESS);
  filter.begin(0.2f);
#endif
}

void loop() {
  IMU.update();
  IMU.getAccel(&accelData);
  IMU.getGyro(&gyroData);
  filter.updateIMU(gyroData.gyroX, gyroData.gyroY, gyroData.gyroZ, accelData.accelX, accelData.accelY, accelData.accelZ);
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_GREEN);
  spr.setTextDatum(MC_DATUM);

  spr.drawString("Accel (X, Y, Z):", 120, 90);
  spr.drawString(String(accelData.accelX) + ", " + String(accelData.accelY) + ", " + String(accelData.accelZ), 120, 110);
spr.setTextColor(TFT_BLUE);
  spr.drawString("Gyro (X, Y, Z):", 120, 130);
  spr.drawString(String(gyroData.gyroX) + ", " + String(gyroData.gyroY) + ", " + String(gyroData.gyroZ), 120, 150);

  if (IMU.hasMagnetometer()) {
    IMU.getMag(&magData);
    spr.drawString("Mag (X, Y, Z):", 120, 170);
    spr.drawString(String(magData.magX) + ", " + String(magData.magY) + ", " + String(magData.magZ), 120, 190);
  }

  if (IMU.hasTemperature()) {
    spr.setTextColor(TFT_RED);
    spr.drawString("Temp:", 120, 210);
    spr.drawString(String(IMU.getTemp()) + " C", 120, 230);
  }
    spr.setTextColor(TFT_WHITE);
  spr.drawString("(QW,QX,QY,QZ)", 100, 40);
  spr.drawString("[ "+String(filter.getQuatW())+", "+String(filter.getQuatX())+", "+String(filter.getQuatY())+", "+String(filter.getQuatZ())+" ]", 100, 60);
  spr.pushSprite(0, 0);
  delay(50);
}

// This setup should fit neatly on your round 240x240 screen! Let me know if you want adjustments. 🚀
