#ifndef SOUND_LOUDNESS_H
#define SOUND_LOUDNESS_H

#define ARM_MATH_CM4
#include "arm_math.h"

// calculate the loudness of audio signal in array in_buffer which is of length n
float32_t calculate_sound_loudness(float32_t * in_buffer, uint32_t n);

#endif
