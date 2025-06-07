#include <Adafruit_TinyUSB.h>

// Array of pins and its keycode.
const uint8_t pins[] = {1,2,3,4,5,6,7,8,9,10,11,12,13};
// number of pins
uint8_t pincount = sizeof(pins) / sizeof(pins[0]);
// For keycode definition check out https://github.com/hathach/tinyusb/blob/master/src/class/hid/hid.h
uint8_t hidcode[] = { HID_KEY_0, HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5, HID_KEY_6, HID_KEY_7, HID_KEY_8, HID_KEY_9, HID_KEY_A, HID_KEY_B, HID_KEY_C };
bool activeState = false;