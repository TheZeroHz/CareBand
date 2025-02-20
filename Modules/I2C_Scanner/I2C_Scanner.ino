#include <Wire.h>
#include <TFT_eSPI.h>  // Include the TFT_eSPI library

TFT_eSPI tft = TFT_eSPI();  // Create an instance of the TFT_eSPI class

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);

  // Initialize the TFT screen
  tft.init();
  tft.setRotation(0); // Set the rotation if needed
  tft.fillScreen(TFT_BLACK); // Clear the screen with black color
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set text color to white with black background
  tft.setTextSize(2); // Set text size

  // Display a title on the TFT screen
  tft.setCursor(50, 60);
  tft.println("I2C Scanner");
  tft.println("Scanning...");

  // Initialize I2C communication
  Wire.begin(6,7);

  // Start scanning for I2C devices
  scanI2CDevices();
}

void loop() {
  // Do nothing in the loop
}

void scanI2CDevices() {
  byte error, address;
  int nDevices = 0;
  int yPos = 60; // Y position for displaying device addresses

  // Scan all I2C addresses from 1 to 127
  for (address = 1; address < 127; address++) {
    // Begin transmission to the current address
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    // Check if a device responded at the current address
    if (error == 0) {
      // Device found
      Serial.print("I2C found at 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      // Display the found device address on the TFT screen
      tft.setCursor(0, yPos);
      tft.print("I2C found at 0x");
      if (address < 16)
        tft.print("0");
      tft.println(address, HEX);

      yPos += 20; // Move to the next line
      nDevices++;
    } else if (error == 4) {
      // Error occurred
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }

  // Display the result of the scan
  if (nDevices == 0) {
    Serial.println("No I2C found\n");
    tft.setCursor(0, yPos);
    tft.println("No I2C found");
  } else {
    Serial.println("done\n");
  }
}
