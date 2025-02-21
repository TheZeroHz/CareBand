#include "QMI8658.h"
#include <MadgwickAHRS.h>
#include "TFT_eSPI.h"
#include "quaternion.h"

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite sprite = TFT_eSprite(&tft);
Madgwick filter;
unsigned long microsPerReading, microsPrevious;
float accelScale, gyroScale;

const int graphWidth = 180;
const int graphHeight = 80;
const int graphX = 30;
const int graphY1 = 20;
const int graphY2 = 120;
const int maxDataPoints = graphWidth;

float accl[3], vel[3], quat[4], gyros[3];
unsigned int tim_count = 0;

Quaternion cubeRotation;
Quaternion calibrationOffset;
bool isCalibrated = false;

struct Point3D {
    float x, y, z;
};

struct Point2D {
    int x, y;
};

const int cubeSize = 30;
const int centerX = 120;
const int centerY = 120;
const uint16_t faceColors[6] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA};

void calibrateIMU() {
    isCalibrated = true;
    calibrationOffset = Quaternion::fromEuler(
        -filter.getRoll() * DEG_TO_RAD,
        -filter.getPitch() * DEG_TO_RAD,
        -filter.getYaw() * DEG_TO_RAD);
}

void draw3DObject(const Quaternion &rotation) {
    Point3D cube[8] = {
        {-cubeSize, -cubeSize, -cubeSize}, {cubeSize, -cubeSize, -cubeSize},
        {cubeSize, cubeSize, -cubeSize}, {-cubeSize, cubeSize, -cubeSize},
        {-cubeSize, -cubeSize, cubeSize}, {cubeSize, -cubeSize, cubeSize},
        {cubeSize, cubeSize, cubeSize}, {-cubeSize, cubeSize, cubeSize}
    };

    Point2D projected[8];

    for (int i = 0; i < 8; i++) {
        rotation.rotateVector(cube[i].x, cube[i].y, cube[i].z);
        float scale = 100.0 / (100.0 + cube[i].z);
        projected[i].x = centerX + (int)(cube[i].x * scale);
        projected[i].y = centerY + (int)(cube[i].y * scale);
    }

    int faces[6][4] = {
        {0, 1, 2, 3}, {4, 5, 6, 7},
        {0, 1, 5, 4}, {2, 3, 7, 6},
        {0, 3, 7, 4}, {1, 2, 6, 5}
    };

    sprite.fillScreen(TFT_BLACK);

    for (int i = 0; i < 6; i++) {
        sprite.fillTriangle(
            projected[faces[i][0]].x, projected[faces[i][0]].y,
            projected[faces[i][1]].x, projected[faces[i][1]].y,
            projected[faces[i][2]].x, projected[faces[i][2]].y, faceColors[i]);

        sprite.fillTriangle(
            projected[faces[i][0]].x, projected[faces[i][0]].y,
            projected[faces[i][2]].x, projected[faces[i][2]].y,
            projected[faces[i][3]].x, projected[faces[i][3]].y, faceColors[i]);
    }

    int edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };

    for (int i = 0; i < 12; i++) {
        sprite.drawLine(projected[edges[i][0]].x, projected[edges[i][0]].y,
                        projected[edges[i][1]].x, projected[edges[i][1]].y, TFT_WHITE);
    }

    sprite.pushSprite(0, 0);
}

void setup() {
    Serial.begin(115200);
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    sprite.createSprite(240, 240);

    if (DEV_Module_Init() != 0) Serial.println("GPIO Init Fail!");
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
    filter.begin(65.5);
    microsPerReading = 1000000 / 25;
    microsPrevious = micros();
    calibrateIMU();
}

void loop() {
    float ax, ay, az;
    float gx, gy, gz;

    QMI8658_read_xyz(accl, gyros, &tim_count);
    ax = accl[0]; ay = accl[1]; az = accl[2];
    gx = gyros[0]; gy = gyros[1]; gz = gyros[2];

    filter.updateIMU(gx, gy, gz, ax, ay, az);
    Quaternion newRotation = Quaternion::fromEuler(
        filter.getRoll() * DEG_TO_RAD,
        filter.getPitch() * DEG_TO_RAD,
        filter.getYaw() * DEG_TO_RAD);
    
    if (isCalibrated) {
        newRotation = calibrationOffset * newRotation;
    }
    
    cubeRotation = newRotation;
    draw3DObject(cubeRotation);
}
