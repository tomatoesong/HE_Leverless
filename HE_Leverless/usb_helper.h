#ifndef USB_HELPER_H_
#define USB_HELPER_H_

#include <Arduino.h>

extern uint8_t pins[];
extern uint8_t pbpins[];
extern uint8_t hidcode[];

// number of pins
extern uint8_t pincount;
// number of push buttons
extern uint8_t pbcount;

extern bool activeState;

extern uint16_t* triggerDown;
extern uint16_t* triggerUp;

extern uint16_t keys;

void process_hid();
void init_hid();
void timerStart();
unsigned long timerStop();
void printKeys();

#endif /* USB_HELPER_H_ */