#include <EEPROM.h>
#include "esp_adc/adc_oneshot.h"

unsigned long lastTime;
unsigned long totalTime;


uint16_t adcVal[101];

adc_oneshot_unit_handle_t adc1_handle;
adc_oneshot_unit_init_cfg_t init_config1 = {
    .unit_id = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
};
adc_oneshot_chan_cfg_t config = {
  .atten = ADC_ATTEN_DB_12,
  .bitwidth = ADC_BITWIDTH_DEFAULT,
};

void setup() {
  // Initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  // Optional for ESP32: Set the resolution to 9-12 bits (default is 12 bits)
  analogContinuousSetWidth(12);

  // Optional: Set different attenaution (default is ADC_11db)
  analogContinuousSetAtten(ADC_11db);
  lastTime = micros();
  while (!Serial) {
  }
  uint8_t* ad = 0x00;
  EEPROM.begin(3);
  Serial.printf("EEPROM 0x00 ~ 0x02: %d, %d, %d", EEPROM.read(0), EEPROM.read(1), EEPROM.read(2));
}

void loop() {
  // Check if conversion is done and try to read data
  // for (int i = 0; i < 13; i++) {
  //   if (i == 1) lastTime = micros();
  //   adcVal[i] = analogRead(i);
  // }
  lastTime = micros();
  adcVal[0] = analogRead(1);
  adcVal[1] = analogRead(2);
  adcVal[2] = analogRead(3);
  adcVal[3] = analogRead(4);
  adcVal[4] = analogRead(5);
  adcVal[5] = analogRead(6);
  adcVal[6] = analogRead(7);
  adcVal[7] = analogRead(8);
  adcVal[8] = analogRead(9);
  adcVal[9] = analogRead(10);
  adcVal[10] = analogRead(11);
  adcVal[11] = analogRead(12);
  adcVal[12] = analogRead(13);
  totalTime = micros() - lastTime;
  Serial.printf("  Total Time: %d \n adc[12]: %d", totalTime, adcVal[11]);
  delay(333);
}