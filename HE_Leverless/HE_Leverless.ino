#include "config.h"
#include "usb_helper.h"
#include "calibration.h"
#include "trigger_mode.h"

bool calibrationMode = false;
Mode mode = RAPID_FIRE_MODE;

void setup() {
  Serial.begin(115200);
  rgbOff();
  // Set up pin as input
  for (uint8_t i = 0; i < pbcount; i++) {
    pinMode(pbpins[i], activeState ? INPUT_PULLDOWN : INPUT_PULLUP);
  }
  if (digitalRead(pbpins[0]) == activeState) {
    calibrationMode = true;
  }
  //  EEPROM Setup begin
  EEPROM.begin(ADC_BYTE_SIZE * (pincount * 4 + 10));  // triggerDown and triggerUp plus 10 extra
  load_calib_data();
  //  USB Setup begin
  init_hid();

  fillArrayWith(rT_Triggers, DEFAULT_TRIGGER, pincount);  //REMOVE THIS LATER!!!
  fillArrayWith(rT_ResetDistances, DEFAULT_RESET, pincount);  //REMOVE THIS LATER!!!
  fillArrayWith(lastPos, 1900, pincount); //REMOVE THIS LATER!!!
}

void loop() {
  if (!calibrationMode) {
    switch (mode) {
      case RAPID_FIRE_MODE:
        {
          //  State Machine Method
          for (uint8_t i = 0; i < pincount; i++) {
            uint16_t nowPos = analogRead(pins[i]);
            if (nowPos < (highestPoints[i] - DEADZONE)) {
              int keyDelta = nowPos - lastPos[i];
              switch (buttonState[i]) {
                case 0:  //Pressing State
                  if ((-keyDelta) > rT_Triggers[i]) {
                    //Key Press
                    keys |= 1 << i;
                    buttonState[i] = 1;
                  }
                  if (keyDelta > 25) {
                    lastPos[i] = nowPos;
                  }
                  // Serial.printf("State: %d, nowPos: %d, lastPos: %d, delta: %d\n", buttonState[0], nowPos, lastPos[0], keyDelta);
                  break;
                case 1:  //Releaseing State
                  if (keyDelta > rT_ResetDistances[i]) {
                    //Key Release
                    keys &= ~(1 << i);
                    buttonState[i] = 0;
                  }
                  if (keyDelta < -25) {
                    lastPos[i] = nowPos;
                  }
                  // Serial.printf("State: %d, nowPos: %d, lastPos: %d, delta: %d\n", buttonState[0], nowPos, lastPos[0], keyDelta);
                  break;
              }
            } else {
              keys &= ~(1 << i);
            }
          }
        }
        break;
      case CONTINOUS_MODE:
        break;
      default:
        for (uint8_t i = 0; i < pincount; i++) {
          keys = (keys & ~(1 << i)) | (analogRead(pins[i]) < rT_Triggers[i]) << i;
        }
        break;
    }
    // printKeys();
    process_hid();
    // Serial.println(EEPROM.readULong(120));
  } else {
    delay(500);
    Serial.println("Calibrating....");
    getHighestPoints();
    getLowestPoints();
    save_calib_Data();
    while (1) {
      printHigest();
      printLowest();
      delay(1000);
    }
  }
}
