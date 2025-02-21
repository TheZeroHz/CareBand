#include "QMI8658.h"
#include "TFT_eSPI.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);

const int graphWidth = 220;
const int graphHeight = 80;
const int graphX = 10;
const int graphY1 = 20;
const int graphY2 = 120;
const int maxDataPoints = graphWidth;

float accl[3], gyros[3];
unsigned int tim_count = 0;

float acclDataX[maxDataPoints] = {0};
float acclDataY[maxDataPoints] = {0};
float acclDataZ[maxDataPoints] = {0};
float gyroDataX[maxDataPoints] = {0};
float gyroDataY[maxDataPoints] = {0};
float gyroDataZ[maxDataPoints] = {0};

void setup() {
    Serial.begin(115200);
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    sprite.createSprite(240, 240);

    if (DEV_Module_Init() != 0)
        Serial.println("GPIO Init Fail!");
    if (QMI8658_init() == '0') {
        Serial.println("Failed to initialize QMI8658!");
        sprite.fillScreen(TFT_BLACK);
        sprite.setCursor(50, 60);
        sprite.setTextColor(TFT_WHITE, TFT_BLACK);
        sprite.println("Sensor Error!");
        sprite.pushSprite(0, 0);
        while (1);
    }
    sprite.fillScreen(TFT_BLACK);
    sprite.setCursor(50, 60);
    sprite.setTextColor(TFT_WHITE, TFT_BLACK);
    sprite.println("Sensor Ready!");
    sprite.pushSprite(0, 0);
    delay(2000);
}

void loop() {
    QMI8658_read_xyz(accl, gyros, &tim_count);

    shiftAndStore(acclDataX, accl[0]);
    shiftAndStore(acclDataY, accl[1]);
    shiftAndStore(acclDataZ, accl[2]);
    shiftAndStore(gyroDataX, gyros[0]);
    shiftAndStore(gyroDataY, gyros[1]);
    shiftAndStore(gyroDataZ, gyros[2]);

    sprite.fillScreen(TFT_BLACK);
    drawGraph(graphX, graphY1, acclDataX, acclDataY, acclDataZ, "Acc");
    drawGraph(graphX, graphY2, gyroDataX, gyroDataY, gyroDataZ, "Gyro");
    sprite.pushSprite(0, 0);
}

void shiftAndStore(float *dataArray, float newValue) {
    for (int i = 0; i < maxDataPoints - 1; i++) {
        dataArray[i] = dataArray[i + 1];
    }
    dataArray[maxDataPoints - 1] = newValue;
}

void drawGraph(int x, int y, float *dataX, float *dataY, float *dataZ, const char *label) {
    sprite.setTextColor(TFT_WHITE);
    sprite.setCursor(x, y - 10);
    sprite.print(label);
    
    int midY = y + graphHeight / 2;
    sprite.drawRect(x, y, graphWidth, graphHeight, TFT_WHITE);
    
    for (int i = 1; i < maxDataPoints; i++) {
        sprite.drawLine(x + i - 1, midY - mapValue(dataX[i - 1]), x + i, midY - mapValue(dataX[i]), TFT_RED);
        sprite.drawLine(x + i - 1, midY - mapValue(dataY[i - 1]), x + i, midY - mapValue(dataY[i]), TFT_GREEN);
        sprite.drawLine(x + i - 1, midY - mapValue(dataZ[i - 1]), x + i, midY - mapValue(dataZ[i]), TFT_BLUE);
    }
}

int mapValue(float value) {
    return (int)(value * 0.04); // Scaling for visualization
}
