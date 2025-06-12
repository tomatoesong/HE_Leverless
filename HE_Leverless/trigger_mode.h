#ifndef TRIGGER_MODE_H_
#define TRIGGER_MODE_H_

enum Mode {
  RAPID_FIRE_MODE,
  CONTINOUS_MODE,
  REGULAR
};

extern uint16_t* rT_Triggers;
extern uint16_t* rT_ResetDistances;
extern uint16_t* lastPos;
extern uint8_t* buttonState;

void printRT_Triggers();
void printRT_ResetDistances();
void fillArrayWith(uint16_t* arr, uint16_t value, int size);

#endif  /* TRIGGER_MODE_H_ */