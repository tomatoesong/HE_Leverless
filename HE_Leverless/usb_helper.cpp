#include "Arduino.h"
#include <algorithm>
#include <stdint.h>
#include "usb_helper.h"
#include <Adafruit_TinyUSB.h>

#define LEFT_KEY (1 << 0)
#define DOWN_KEY (1 << 1)
#define RIGHT_KEY (1 << 2)
#define UP_KEY (1 << 3)

#define DPAD_MASK 0b1111
#define BUTTONS_MASK 0b1111111110000

// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};

uint8_t const desc_hid_gp_report[] = {
  TUD_HID_REPORT_DESC_GAMEPAD()
};

// Array of pins and its keycode.
uint8_t pins[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
uint8_t gp_d_pins[] = { 1, 2, 3, 4 };
uint8_t gp_buttons_pins[] = { 5, 6, 7, 8, 9, 10, 11, 12, 13 };
uint8_t pbpins[] = { 38, 18, 17, 42, 41, 40, 39 };
// For keycode definition check out https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h
uint8_t hidcode[] = { HID_KEY_A, HID_KEY_S, HID_KEY_D, HID_KEY_W, HID_KEY_4, HID_KEY_5, HID_KEY_6, HID_KEY_7, HID_KEY_8, HID_KEY_9, HID_KEY_A, HID_KEY_B, HID_KEY_C };


// number of pins
uint8_t pincount = sizeof(pins) / sizeof(pins[0]);
uint8_t pbcount = sizeof(pbpins) / sizeof(pbpins[0]);
uint8_t dpad_pincount = sizeof(gp_d_pins) / sizeof(gp_d_pins[0]);
uint8_t gp_button_pincount = sizeof(gp_buttons_pins) / sizeof(gp_buttons_pins[0]);

bool activeState = false;

uint16_t* triggerDown = new uint16_t[pincount];
uint16_t* triggerUp = new uint16_t[pincount];

unsigned long us = 0;
unsigned long maxTimeTaken = 0;
uint16_t counter = 0;

uint16_t keys = 0;

Adafruit_USBD_HID usb_hid(NULL, 0, HID_ITF_PROTOCOL_NONE, 1, false);
Controller_Mode controller_mode = GAMEPAD;

hid_gamepad_report_t gp;

void init_hid() {
  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }
  // Setup HID
  // usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
  // usb_hid.setPollInterval(1);
  switch (controller_mode) {
    case GAMEPAD:
      usb_hid.setReportDescriptor(desc_hid_gp_report, sizeof(desc_hid_gp_report));
      break;
    case KEYBOARD:
      usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
      break;
  }

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

void gamepad_process_hid() {
  if (!TinyUSBDevice.mounted()) {
    return;
  }

  if (!usb_hid.ready()) return;
  uint8_t dpad = 0;
  uint16_t buttons = 0;
  uint8_t menu = 0;

  dpad = keys & DPAD_MASK;
  buttons |= (keys & BUTTONS_MASK) >> 4;

  if (digitalRead(pbpins[pbcount - 1])) {  //  Check for tournament mode switch
    for (uint8_t i = 0; i < pbcount - 1; i++) {
      if (activeState == digitalRead(pbpins[i])) {
        menu |= 1 << i;
      }
    }
  }

  //  SOCD CLEANING start
  if ((dpad & UP_KEY) && (dpad & DOWN_KEY)) {
    // No UP or DOWN
    dpad &= ~(UP_KEY | DOWN_KEY);
  }
  if ((dpad & LEFT_KEY) && (dpad & RIGHT_KEY)) {
    // NO LEFT or RIGHT
    dpad &= ~(LEFT_KEY | RIGHT_KEY);
  }
  //  SOCD CLEANING end

  //  DPAD READ start
  if ((dpad & UP_KEY) && (dpad & RIGHT_KEY)) {
    gp.hat = GAMEPAD_HAT_UP_RIGHT;
  } else if ((dpad & UP_KEY) && (dpad & LEFT_KEY)) {
    gp.hat = GAMEPAD_HAT_UP_LEFT;
  } else if ((dpad & DOWN_KEY) && (dpad & RIGHT_KEY)) {
    gp.hat = GAMEPAD_HAT_DOWN_RIGHT;
  } else if ((dpad & DOWN_KEY) && (dpad & LEFT_KEY)) {
    gp.hat = GAMEPAD_HAT_DOWN_LEFT;
  } else if (dpad & UP_KEY) {
    gp.hat = GAMEPAD_HAT_UP;
  } else if (dpad & RIGHT_KEY) {
    gp.hat = GAMEPAD_HAT_RIGHT;
  } else if (dpad & DOWN_KEY) {
    gp.hat = GAMEPAD_HAT_DOWN;
  } else if (dpad & LEFT_KEY) {
    gp.hat = GAMEPAD_HAT_LEFT;
  } else {
    gp.hat = GAMEPAD_HAT_CENTERED;
  }
  //  DPAD READ end

  //  BUTTON READ start
  gp.buttons = buttons;
  //menu
  gp.buttons |= (menu << gp_button_pincount);
  //  BUTTON READ end

  usb_hid.sendReport(0, &gp, sizeof(gp));
}

void keyboard_process_hid() {
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