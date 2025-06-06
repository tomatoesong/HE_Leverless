#include "Config.h"
#include <Adafruit_TinyUSB.h>

int aCal;
int amin;
int amax;

// Single Report (no ID) descriptor
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD()
};

Adafruit_USBD_HID usb_hid(NULL, 0, HID_ITF_PROTOCOL_NONE, 1, false);

// Array of pins and its keycode.
uint8_t pins[] = { 17, 18, 38, 39 };
// number of pins
uint8_t pincount = sizeof(pins) / sizeof(pins[0]);
// For keycode definition check out https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h
uint8_t hidcode[] = { HID_KEY_0, HID_KEY_1, HID_KEY_2, HID_KEY_3 };
bool activeState = false;

void setup() {
  Serial.begin(115200);
  // aCal = analogRead(HE_PINS[0]);
  // rangCalc(amax, amin);

  if (!TinyUSBDevice.isInitialized()) {
    TinyUSBDevice.begin(0);
  }
  // Setup HID
  usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.setStringDescriptor("HE_Leverless");

  usb_hid.begin();

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  // Set up pin as input
  for (uint8_t i = 0; i < pincount; i++) {
    pinMode(pins[i], activeState ? INPUT_PULLDOWN : INPUT_PULLUP);
  }
}

void loop() {
  // String output;

  // int hall1 = analogRead(1);
  // Serial.println(hall1);
  // if(!digitalRead(17)){
  //   Serial.println(17);
  // }
  // if(!digitalRead(18)){
  //   Serial.println(18);
  // }
  // if(!digitalRead(38)){
  //   Serial.println(38);
  // }
  // if(!digitalRead(39)){
  //   Serial.println(39);
  // }
  // if(!digitalRead(40)){
  //   Serial.println(40);
  // }
  // if(!digitalRead(41)){
  //   Serial.println(41);
  // }
  // if(!digitalRead(42)){
  //   Serial.println(42);
  // }

  // int a1 = analogRead(HE_PINS[0]);
  // Serial.printf("Max: %d, Min: %d, Range: %d, %d\n", amax, amin, amax - amin, a1);


  // int af1 = map(a1, 980, aCal + 10, 0, 300);
  // // Serial.printf(a1 + "sdfsdfsd, ");
  // Serial.printf("%d, %d, %d\n", aCal, a1, af1);

  // for(uint8_t i = 0; i < 13; i++){
  //   output += analogRead(HE_PINS[i]);
  //   output += ", ";
  // }
  // Serial.println(output);
  // interfaceSend(HE_PINS);
  // delay(500);

    #ifdef TINYUSB_NEED_POLLING_TASK
  // Manual call tud_task since it isn't called by Core's background
  TinyUSBDevice.task();
  #endif

  // not enumerated()/mounted() yet: nothing to do
  if (!TinyUSBDevice.mounted()) {
    return;
  }

  // poll gpio once each 2 ms
  static uint32_t ms = 0;
  if (millis() - ms > 2) {
    ms = millis();
    process_hid();
  }
}

void rangCalc(int& max, int& min) {
  max = 0;
  min = 4096;

  for (int i = 0; i < 1000; i++) {
    int readVal = analogRead(HE_PINS[0]);
    if (readVal > max) {
      max = readVal;
    }
    if (readVal < min) {
      min = readVal;
    }
  }
}

void process_hid() {
  // used to avoid send multiple consecutive zero report for keyboard
  static bool keyPressedPreviously = false;

  uint8_t count = 0;
  uint8_t keycode[6] = {0};

  // scan normal key and send report
  for (uint8_t i = 0; i < pincount; i++) {
    if (activeState == digitalRead(pins[i])) {
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
