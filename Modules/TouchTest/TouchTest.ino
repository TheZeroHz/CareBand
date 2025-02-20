#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include "CST816S.h"

// Define screen dimensions
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 240

// Define touch sensor pins
#define SDA_PIN 6
#define SCL_PIN 7
#define RST_PIN 13
#define IRQ_PIN 5

// Initialize display and touch sensor
TFT_eSPI tft = TFT_eSPI();
CST816S touch(SDA_PIN, SCL_PIN, RST_PIN, IRQ_PIN);

void setup() {
    Serial.begin(115200);
    Wire.begin(SDA_PIN, SCL_PIN);
    
    tft.begin();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(50, 100);
    tft.println("Touch the screen!");

    touch.begin();
}

void loop() {
    if (touch.available()) {
        int x = touch.data.x;
        int y = touch.data.y;

        // Draw touch point
        tft.fillCircle(x, y, 5, TFT_RED);
        Serial.printf("Touch detected at: X=%d, Y=%d\n", x, y);
    }
}
