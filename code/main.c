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
#include "mic/tm_stm32f4_delay.h"
#include "mic/tm_stm32f4_adc.h"
#include "mic/tm_stm32f4_disco.h"
//#include "mic/tm_stm32f4_sdram.h"
#include "mic/tm_stm32f4_dac_signal.h"
#include "mic/tm_stm32f4_fft.h"
#include <stdio.h>
#include <stdlib.h>

#include "arm_math.h"

#include "mic/microphone_functions.h"
#include "mic/sound_loudness.h"
#define AUDIO_BUF_SIZE 2048
float audio_buffer[AUDIO_BUF_SIZE];

#define SAMPLES                    (512)         /* 256 real party and 256 imaginary parts */
#define FFT_SIZE                (SAMPLES / 2) /* FFT size is always the same size as we have samples, so 256 in our case */
 
#define FFT_BAR_MAX_HEIGHT        120        
float32_t Input[SAMPLES];   /*!< Input buffer is always 2 * FFT_SIZE */
float32_t Output[FFT_SIZE]; /*!< Output buffer is always FFT_SIZE */
 
/* Draw bar for LCD */
/* Simple library to draw bars */
void DrawBar(uint16_t bottomX, uint16_t bottomY, uint16_t maxHeight, uint16_t maxValue, float32_t value, uint16_t foreground, uint16_t background) {
    uint16_t height;
    height = (uint16_t)((float32_t)value / (float32_t)maxValue * (float32_t)maxHeight);
    if (height == maxHeight) {
        TM_ILI9341_DrawLine(bottomX, bottomY, bottomX, bottomY - height, foreground);
    } else if (height < maxHeight) {
        TM_ILI9341_DrawLine(bottomX, bottomY, bottomX, bottomY - height, foreground);
        TM_ILI9341_DrawLine(bottomX, bottomY - height, bottomX, bottomY - maxHeight, background);
    }
}


int main(void)
{
	TM_FFT_F32_t FFT;    /*!< FFT structure */
    uint16_t i;
    uint32_t frequency = 10000;
    
    /* Initialize system */
    SystemInit();
    
    /* Delay init */
    TM_DELAY_Init();
    
    /* Initialize LED's on board */
    TM_DISCO_LedInit();
    
    /* Initialize LCD */
    TM_ILI9341_Init();
    TM_ILI9341_Rotate(TM_ILI9341_Orientation_Landscape_1);
    
    /* Initialize DAC2, PA5 for fake sinus, use TIM4 to generate DMA */
    TM_DAC_SIGNAL_Init(TM_DAC2, TIM4);
    
    /* Set sinus with 10kHz */
    TM_DAC_SIGNAL_SetSignal(TM_DAC2, TM_DAC_SIGNAL_Signal_Sinus, frequency);
 
    /* Initialize ADC, PA0 is used */
    TM_ADC_Init(ADC1, ADC_Channel_0);
    
    /* Print something on LCD */
    TM_ILI9341_Puts(10, 10, "Mosquitos", &TM_Font_11x18, ILI9341_COLOR_BLACK, ILI9341_COLOR_GREEN2);
    
    /* Init FFT, FFT_SIZE define is used for FFT_SIZE, samples count is FFT_SIZE * 2, don't use malloc for memory allocation */
    TM_FFT_Init_F32(&FFT, FFT_SIZE, 0);
    
    /* We didn't used malloc for allocation, so we have to set pointers ourself */
    /* Input buffer must be 2 * FFT_SIZE in length because of real and imaginary part */
    /* Output buffer must be FFT_SIZE in length */
    TM_FFT_SetBuffers_F32(&FFT, Input, Output);
    
    while (1) {
        /* This part should be done with DMA and timer for ADC treshold */
        /* Actually, best solution is double buffered DMA with timer for ADC treshold */
        /* But this is only for show principle on how FFT works */
        /* Fill buffer until function returns 1 = Buffer full and samples ready to be calculated */
        while (!TM_FFT_AddToBuffer(&FFT, (TM_ADC_Read(ADC1, ADC_Channel_0) - (float32_t)2048.0))) {
            /* Little delay, fake 45kHz sample rate for audio */
            Delay(21);
        }
            
        /* Do FFT on signal, values at each bin and calculate max value and index where max value happened */
        TM_FFT_Process_F32(&FFT);
 
        /* Display data on LCD */
        for (i = 0; i < TM_FFT_GetFFTSize(&FFT); i++) {
            /* Draw FFT results */
            DrawBar(30 + 2 * i,
                    220,
                    FFT_BAR_MAX_HEIGHT,
                    TM_FFT_GetMaxValue(&FFT),
                    TM_FFT_GetFromBuffer(&FFT, i),
                    0x1234,
                    0xFFFF
            );
        }
    }
}



















	//Init mic
	//microphone_init();
	//microphone_start();
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
	*//*
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
	  */



