#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#define RGB_BRIGHTNESS 10

#define ADC_BYTE_SIZE 2
#define LP_BASE_ADDRESS 0
#define HP_BASE_ADDRESS LP_BASE_ADDRESS + pincount * 2

#include <EEPROM.h>

extern uint16_t* highestPoints;
extern uint16_t* lowestPoints;

void getHighestPoints();
void getLowestPoints();
int range_Calc(uint8_t* pins);
void printHigest();
void printLowest();
void rgbOff();
void save_calib_Data();
void load_calib_data();

#endif /* CALIBRATION_H_ */