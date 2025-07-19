#include <sys/_intsup.h>
#include "esp32-hal.h"
#include "esp32-hal-rgb-led.h"
#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"
#include "calibration.h"
#include "usb_helper.h"


uint16_t* highestPoints = new uint16_t[pincount];
uint16_t* lowestPoints = new uint16_t[pincount];

void getHighestPoints() {
  rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);
  while (digitalRead(pbpins[1]) == !activeState) {
    //idle
  }
  rgbLedWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);
  for (uint8_t i = 0; i < pincount; i++) {
    uint32_t average = 0;
    uint8_t count = 0;
    for (uint8_t j = 0; j < 100; j++) {
      average += analogRead(pins[i]);
      count++;
    }
    average /= count;
    highestPoints[i] = average;
  }
  delay(500);
}

void getLowestPoints() {
  rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);

  for (uint8_t i = 0; i < pincount; i++) {
    uint32_t average = 0;
    uint8_t count = 0;
    rgbLedWrite(RGB_BUILTIN, RGB_BRIGHTNESS, 0, 0);
    while (digitalRead(pbpins[2]) == !activeState) {
      //idle
    }
    rgbLedWrite(RGB_BUILTIN, 0, RGB_BRIGHTNESS, 0);
    for (uint8_t j = 0; j < 100; j++) {
      average += analogRead(pins[i]);
      count++;
    }
    average /= count;
    lowestPoints[i] = average;
    delay(1000);
  }
  rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
  digitalWrite(RGB_BUILTIN, LOW);
}

void rgbOff() {
  rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
  digitalWrite(RGB_BUILTIN, LOW);
}

void printHigest() {
  Serial.print("Highest sensor values: ");
  for (uint8_t i = 0; i < pincount; i++) {
    Serial.printf("%d, ", highestPoints[i]);
  }
  Serial.println();
}

void printLowest() {
  Serial.print("Lowest sensor values: ");
  for (uint8_t i = 0; i < pincount; i++) {
    Serial.printf("%d, ", lowestPoints[i]);
  }
  Serial.println();
}

int range_Calc(uint8_t* pins) {
  int max = 0;
  int min = 4096;

  for (int i = 0; i < 1000; i++) {
    int readVal = analogRead(pins[0]);
    if (readVal > max) {
      max = readVal;
    }
    if (readVal < min) {
      min = readVal;
    }
  }
  return max - min;
}

bool save_calib_Data() {
  for (uint8_t i = 0; i < pincount; i++) {
    EEPROM.writeUShort(LP_BASE_ADDRESS + (2 * i), lowestPoints[i]);
    EEPROM.writeUShort(HP_BASE_ADDRESS + (2 * i), highestPoints[i]);
  }
  return EEPROM.commit();
}

void load_calib_data() {
  for (uint8_t i = 0; i < pincount; i++) {
    lowestPoints[i] = EEPROM.readUShort(LP_BASE_ADDRESS + (2 * i));
    highestPoints[i] = EEPROM.readUShort(HP_BASE_ADDRESS + (2 * i));
  }
}