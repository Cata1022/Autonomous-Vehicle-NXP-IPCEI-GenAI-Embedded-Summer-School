#ifndef ML_INFERENCE_H
#define ML_INFERENCE_H

#include <Arduino.h>

extern uint8_t* ml_processed_roi;
extern char debug_ml_prediction[128];

void startMLTask();

#endif // ML_INFERENCE_H
