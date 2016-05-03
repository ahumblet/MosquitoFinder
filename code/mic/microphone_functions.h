#ifndef MICROPHONE_FUNCTIONS_H
#define MICROPHONE_FUNCTIONS_H

#include <stdint.h>

// initialize the microphone
void microphone_init();

// start the microphone
void microphone_start();

// is data ready -- returns pointer to uint16_t array if data ready or 0 otherwise
// if data is ready, also puts length of returned array into n 
// for example, n might be 16 (16 samples from the microphone)
uint16_t *microphone_get_data_if_ready(uint32_t *n);


#endif