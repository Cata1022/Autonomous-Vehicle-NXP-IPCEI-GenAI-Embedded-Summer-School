#ifndef VISION_PROCESSING_H
#define VISION_PROCESSING_H

#include <Arduino.h>

extern uint8_t* full_frame_copy;
extern volatile int debug_centroid_x;
extern volatile int debug_intersection_pixels;
extern bool frame_ready;

void startVisionTask();

#endif // VISION_PROCESSING_H
