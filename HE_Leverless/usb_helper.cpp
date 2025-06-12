#include <algorithm>
#include <stdint.h>
#include "usb_helper.h"
#include <Adafruit_TinyUSB.h>


// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};

Adafruit_USBD_HID usb_hid(NULL, 0, HID_ITF_PROTOCOL_KEYBOARD, 1, false);
// Array of pins and its keycode.
uint8_t pins[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
uint8_t pbpins[] = { 38, 18, 17, 42, 41, 40 };
// For keycode definition check out https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h
uint8_t hidcode[] = { HID_KEY_0, HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5, HID_KEY_6, HID_KEY_7, HID_KEY_8, HID_KEY_9, HID_KEY_A, HID_KEY_B, HID_KEY_C };

// number of pins
uint8_t pincount = sizeof(pins) / sizeof(pins[0]);
// number of push buttons
uint8_t pbcount = sizeof(pbpins) / sizeof(pbpins[0]);

bool activeState = false;

uint16_t* triggerDown = new uint16_t[pincount];
uint16_t* triggerUp = new uint16_t[pincount];

unsigned long us = 0;
unsigned long maxTimeTaken = 0;
uint16_t counter = 0;

uint16_t keys = 0;

void init_hid() {
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }
  // Setup HID
  // usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
  // usb_hid.setPollInterval(1);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  TinyUSBDevice.setProductDescriptor("HE_Leverless");
  // usb_hid.setStringDescriptor("HE_Leverless");

  usb_hid.begin();

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }
}

void process_hid() {
  // used to avoid send multiple consecutive zero report for keyboard
  static bool keyPressedPreviously = false;

  uint8_t count = 0;
  uint8_t keycode[6] = { 0 };

  // scan normal key and send report
  for (uint8_t i = 0; i < pincount; i++) {
    if (keys & (1 << i)) {
      // if pin is active (low), add its hid code to key report
      keycode[count++] = hidcode[i];

      // 6 is max keycode per report
      if (count == 6) break;
    }
  }

  if (TinyUSBDevice.suspended() && count) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    TinyUSBDevice.remoteWakeup();
  }

  // skip if hid is not ready e.g still transferring previous report
  if (!usb_hid.ready()) return;

  if (count) {
    // Send report if there is key pressed
    uint8_t const report_id = 0;
    uint8_t const modifier = 0;

    keyPressedPreviously = true;
    usb_hid.keyboardReport(report_id, modifier, keycode);
  } else {
    // Send All-zero report to indicate there is no keys pressed
    // Most of the time, it is, though we don't need to send zero report
    // every loop(), only a key is pressed in previous loop()
    if (keyPressedPreviously) {
      keyPressedPreviously = false;
      usb_hid.keyboardRelease(0);
    }
  }
}

void timerStart() {
  us = micros();
}

unsigned long timerStop() {
  unsigned long timetaken = micros() - us;
  if (counter > 99) {
    if (timetaken > maxTimeTaken) {
      maxTimeTaken = timetaken;
    }
    Serial.printf("timeTaken: %d, MaxTimeTaken: %d\n", timetaken, maxTimeTaken);
  } else {
    counter++;
  }
  return maxTimeTaken;
}

void printKeys() {
  Serial.println(keys);
}