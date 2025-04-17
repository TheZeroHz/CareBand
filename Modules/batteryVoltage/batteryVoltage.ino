#include <TFT_eSPI.h>
#include <SPI.h>
#include <Wire.h>
#include "esp32-hal-cpu.h"
#include "DEV_Config.h"
#define TOUCH_INT_PIN 5  // TP_INT pin from CST816S
#define screenW 240
#define screenH 240
const int centerX = screenW / 2;
const int centerY = screenH / 2;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

unsigned long lastTouchMillis = 0;
const unsigned long timeoutMillis = 5000;  // 10 seconds
const float conversion_factor = (3.3f / 4096) * 4.38;
void drawHourglass() {
  sprite.fillSprite(TFT_BLACK);
  for (int y = 0; y < centerY; y++) {
    float ratio = (float)y / centerY;
    int leftX = (int)(centerX * ratio);
    int rightX = (int)(screenW - centerX * ratio);
    sprite.drawFastHLine(leftX, y, rightX - leftX + 1, TFT_RED);
  }
  for (int y = centerY; y < screenH; y++) {
    float ratio = (float)(y - centerY) / (screenH - centerY);
    int leftX = (int)(centerX - centerX * ratio);
    int rightX = (int)(centerX + (screenW - centerX) * ratio);
    sprite.drawFastHLine(leftX, y, rightX - leftX + 1, TFT_RED);
  }
    sprite.setTextColor(TFT_GREEN, TFT_RED);
  sprite.setTextDatum(MC_DATUM);
  sprite.drawString(String(DEC_ADC_Read()*conversion_factor)+" V", centerX-60, centerY);
  sprite.pushSprite(0, 0);
}

bool isTouched() {
  return digitalRead(TOUCH_INT_PIN) == LOW;
}

void goToSleep() {
  sprite.fillSprite(TFT_BLACK);
  sprite.setTextColor(TFT_BLUE, TFT_BLACK);
  sprite.setTextDatum(MC_DATUM);
  sprite.drawString("Sleeping...", centerX, centerY);
  sprite.pushSprite(0, 0);
  delay(1000);

  esp_sleep_enable_ext0_wakeup((gpio_num_t)TOUCH_INT_PIN, 0); // Wake on LOW
  esp_deep_sleep_start();
}

void setup() {
  setCpuFrequencyMhz(240);
  Serial.print("CPU Frequency: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");
  Serial.begin(115200);
  pinMode(TOUCH_INT_PIN, INPUT_PULLUP);
  Wire.begin(6, 7);
  // TFT + Sprite init
  tft.init();
  tft.setRotation(1);
  sprite.setColorDepth(16);
  sprite.createSprite(screenW, screenH);
  sprite.setTextDatum(MC_DATUM);

  // Wake up reason
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("Woke from touch");
    sprite.fillSprite(TFT_BLACK);
    sprite.setTextColor(TFT_GREEN, TFT_BLACK);
    sprite.drawString("Woke up caused by touch!", centerX, centerY);
    sprite.pushSprite(0, 0);
    delay(2000);
  }


  drawHourglass();
  setCpuFrequencyMhz(10);
  Serial.print("CPU Frequency: ");
  Serial.print(getCpuFrequencyMhz());
  Serial.println(" MHz");
  lastTouchMillis = millis();  // Start inactivity timer
}

void loop() {
  if (isTouched()) {
    Serial.println("Touch detected!");
    lastTouchMillis = millis();  // Reset timer
    drawHourglass();             // Redraw something for feedback
    delay(200);
                  // Debounce / visual delay
  }

  // Check for timeout
  if (millis() - lastTouchMillis > timeoutMillis) {
    Serial.println("No touch for 10 seconds. Going to sleep.");
    goToSleep();
  }
}
