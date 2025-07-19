#include <stdint.h>
#include "trigger_mode.h"
#include <Arduino.h>
#include <EEPROM.h>
#include "usb_helper.h"
#include "config.h"


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

void load_lastPos() {
  for (uint8_t i = 0; i < pincount; i++) {
    lastPos[i] = EEPROM.readUShort(HP_BASE_ADDRESS + (2 * i));
  }
}

bool save_RT_Data() {
  for (uint8_t i = 0; i < pincount; i++) {
    EEPROM.writeUShort(RT_TRIGGER_ADDRESS + (ADC_BYTE_SIZE * i), rT_Triggers[i]);
    EEPROM.writeUShort(RT_RESET_ADDRESS + (ADC_BYTE_SIZE * i), rT_ResetDistances[i]);
  }
  return EEPROM.commit();
}

void load_RT_data() {
  for (uint8_t i = 0; i < pincount; i++) {
    rT_Triggers[i] = EEPROM.readUShort(RT_TRIGGER_ADDRESS + (ADC_BYTE_SIZE * i));
    rT_ResetDistances[i] = EEPROM.readUShort(RT_RESET_ADDRESS + (ADC_BYTE_SIZE * i));
  }
}