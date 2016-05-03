/******************************************************************************
Тестовая программа реализации работы LCD 2.2 дюйма драйвер ILI9341
******************************************************************************/

//#include "stm32l1xx.h"
#include "stm32f4xx.h"

//#include "stm32l1xx_conf.h" // подключить все хидеры (ADC, GPIO, TIM и т.д.)
#include "stm32f4xx_conf.h"

//stuff for the screen
#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"
#include "defines.h"
#include "tm_stm32f4_ili9341.h"
#include "tm_stm32f4_fonts.h"

#include <stdio.h>
#include <stdlib.h>

#include "mic/microphone_functions.h"
#include "mic/sound_loudness.h"
#define AUDIO_BUF_SIZE 2048
float audio_buffer[AUDIO_BUF_SIZE];

int main(void)
{
	//Initialize system
	SystemInit();
	//Init mic
	microphone_init();
	microphone_start();
        /*/Initialize ILI9341
	TM_ILI9341_Init();
	//Rotate LCD for 90 degrees
	TM_ILI9341_Rotate(TM_ILI9341_Orientation_Landscape_2);
	//FIll lcd with color
	TM_ILI9341_DrawLine(10, 120, 310, 120, 0x0005);
	
	//Put string with black foreground color and blue background with 11x18px font
	TM_ILI9341_Puts(65, 130, "MOSQUITO", &TM_Font_11x18, ILI9341_COLOR_BLACK, ILI9341_COLOR_BLUE2);
	//Put string with black foreground color and blue background with 11x18px font
	TM_ILI9341_Puts(60, 150, "FINDER", &TM_Font_11x18, ILI9341_COLOR_BLACK, ILI9341_COLOR_BLUE2);
	//Put string with black foreground color and red background with 11x18px font
	TM_ILI9341_Puts(245, 225, "majerle.eu", &TM_Font_7x10, ILI9341_COLOR_BLACK, ILI9341_COLOR_ORANGE);
*/
	uint32_t audio_buffer_index = 0; // index variable
	float sound_loudness;
	int mod = 0;
	while (1)
	{
		// read one array of samples from the microphone
	uint32_t n_samples; // number of samples read in one batch from the microphone
	uint16_t *audio_samples;
	while (1)
	{
		audio_samples = microphone_get_data_if_ready(&n_samples); // try to read an array of samples from the microphone
		if (audio_samples != 0) break;       // if array of samples was read
	}
	
	// copy array of samples from microphone into our array audio_buffer
	for(int i = 0; i < n_samples; i++){
			audio_buffer[audio_buffer_index ++] = (int16_t) audio_samples[i];
		  if (audio_buffer_index >= AUDIO_BUF_SIZE) break;
	}
	
	// calculate sound loudness 
	if (audio_buffer_index >= AUDIO_BUF_SIZE)
	{
                 
		 sound_loudness = calculate_sound_loudness(audio_buffer, AUDIO_BUF_SIZE);
      
			uint32_t duty;
			if(sound_loudness < 500)
				duty = 5;
			else if(sound_loudness > 30000)
				duty = 90;
			else
				duty = 5 + 85 * (sound_loudness - 500) / 25000;

                        if(!((mod++)%10))
			  printf("Loudness:%d\n",duty);
			  //printf("%s", "ad\n");
			audio_buffer_index  = 0;
		}
	}
}


