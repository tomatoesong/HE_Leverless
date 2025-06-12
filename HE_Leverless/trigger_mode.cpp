#include <stdint.h>
#include "trigger_mode.h"
#include <Arduino.h>
#include "usb_helper.h"


uint16_t* rT_Triggers = new uint16_t[pincount]();
uint16_t* rT_ResetDistances = new uint16_t[pincount]();

uint16_t* lastPos = new uint16_t[pincount]();
uint8_t* buttonState = new uint8_t[pincount]();

void printRT_Triggers() {
  Serial.print("rT_Triggers: ");
  for (uint8_t i = 0; i < pincount; i++) {
    Serial.printf("%d, ", rT_Triggers[i]);
  }
  Serial.println();
}

void printRT_ResetDistances() {
  Serial.print("rT_Triggers: ");
  for (uint8_t i = 0; i < pincount; i++) {
    Serial.printf("%d, ", rT_ResetDistances[i]);
  }
  Serial.println();
}

void fillArrayWith(uint16_t* arr, uint16_t value, int size) {
  for (int i = 0; i < size; i++) {
    arr[i] = value;
  }
}