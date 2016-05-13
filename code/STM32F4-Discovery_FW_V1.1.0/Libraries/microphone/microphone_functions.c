#include "microphone_functions.h"
#include "microphone.h"

// buffers for microphone data collection and for processing audio data
uint16_t * mic_out_buf = 0;
static uint16_t * process_buf = 0;
static uint8_t Switch = 1; // variable to indicate which of the two buffers is being used
// two buffers for microphone data collection (switching between the two buffers)
// one buffer is used to collect data while the other is being processed
static uint16_t audiobuf1[MIC_OUT_BUF_SIZE];
static uint16_t audiobuf2[MIC_OUT_BUF_SIZE];

extern uint32_t Audio_Data_Status;


// initialize the microphone
void microphone_init()
{
	mp45dt02_init();
}

// start the microphone
void microphone_start()
{
  // Set the initial microphone buffer
	mic_out_buf = audiobuf1;
	mp45dt02_start();
}

// is data ready -- returns pointer to uint16_t array if data ready or 0 otherwise
// if data is ready, also puts length of returned array into n 
uint16_t *microphone_get_data_if_ready(uint32_t *n)
{
	if (!Audio_Data_Status) return 0;
	if (Switch)
	{
		mic_out_buf = audiobuf2;
		process_buf = audiobuf1;
		Switch = 0;
	}
	else{
		mic_out_buf = audiobuf1;
		process_buf = audiobuf2;
		Switch = 1;
	}
	*n = MIC_OUT_BUF_SIZE;
	Audio_Data_Status = 0; // reset Audio_Data_Status
	return process_buf;
}
