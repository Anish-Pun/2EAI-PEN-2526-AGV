#ifndef SPEED_H
#define SPEED_H

#include "cyhal.h"

extern volatile uint32_t Count_EncoderRight;
extern volatile uint32_t Count_EncoderLeft;

void speed_init(void);
float get_speed_right_kmh(void);
float get_speed_left_kmh(void);
void reset_encoders(void);

#define COUNTS_PER_METER 235.0f

#endif
