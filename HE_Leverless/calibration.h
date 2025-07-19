#ifndef CALIBRATION_H_
#define CALIBRATION_H_

#define RGB_BRIGHTNESS 10

extern uint16_t* highestPoints;
extern uint16_t* lowestPoints;

void getHighestPoints();
void getLowestPoints();
int range_Calc(uint8_t* pins);
void printHigest();
void printLowest();
void rgbOff();
bool save_calib_Data();
void load_calib_data();

#endif /* CALIBRATION_H_ */