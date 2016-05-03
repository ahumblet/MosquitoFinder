#include "sound_loudness.h"
#include "microphone.h"

float32_t calculate_sound_loudness(float32_t * in_buffer, uint32_t n)
{
	float32_t sound_loudness = 0;
  arm_rms_f32(in_buffer,n,&sound_loudness); 
	return sound_loudness;
}
