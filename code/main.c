#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"

#include <stdio.h>
#include <stdlib.h>

//for display
#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"
#include "defines.h"
#include "tm_stm32f4_ili9341.h"
#include "tm_stm32f4_fonts.h"

//mic input
#include "stm32f4_discovery.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

//serial
//#include "tm_stm32f4_gpio.h"

//buffer for DMA
#define NUM_MICS 4
volatile uint16_t ADCBuffer[NUM_MICS];

//Buffers of samples, one for each mic
#define BUFFERSIZE 16
int bufferPos = 0;
uint16_t mic0Buffer[BUFFERSIZE];
uint16_t mic1Buffer[BUFFERSIZE];
uint16_t mic2Buffer[BUFFERSIZE];
uint16_t mic3Buffer[BUFFERSIZE];
uint32_t ready = 0;

//amplitude of all 4 mics
int amplitudes[NUM_MICS];
int loudestIndex = 0;

#define SERIAL
#define FFT

#ifdef SERIAL
#include "serial_port_usb/serial_port_usb.h"
void read_line_from_serial_usb(char *s)
{
  while (1)
  {
    uint8_t c;
    uint8_t read_byte = read_serial_usb_byte(&c); // read one character
    if (read_byte == 1) // if a character was read
    {
      write_serial_usb_bytes(&c,1); // echo the character back to the USB serial port
      *s = c; s++;
      if ( (c == '\n') || (c == '\r') ) break; // line feed or carriage return ASCII codes
    }
  }
}

// wait for a character from the serial port
void wait_for_byte_from_serial_usb()
{
  while(1)
  {
    uint8_t c;
    uint8_t read_byte = read_serial_usb_byte(&c); // read one character
    if (read_byte == 1) // if a character was read
      return;
  }
}
#endif

static void init_systick();
static void delay_ms(uint32_t n);
static volatile uint32_t msTicks; // counts 1 ms timeTicks

// SysTick Handler (Interrupt Service Routine for the System Tick interrupt)
void SysTick_Handler(void)
{
  msTicks++;
}

// initialize the system tick
void init_systick(void)
{
	SystemCoreClockUpdate();                      /* Get Core Clock Frequency   */
  if (SysTick_Config(SystemCoreClock / 1000)) { /* SysTick 1 msec interrupts  */
    while (1);                                  /* Capture error              */
  }
}

// pause for a specified number (n) of milliseconds
void delay_ms(uint32_t n)
{
  uint32_t msTicks2 = msTicks + n;
  while(msTicks < msTicks2) ;
}





#ifdef FFT
#include "math.h"
#define ARM_MATH_CM4
#include "arm_math.h"
#define TEST_LENGTH_SAMPLES 2048
float32_t testInput_f32_10khz[TEST_LENGTH_SAMPLES]; 
static float32_t testOutput[TEST_LENGTH_SAMPLES/2]; 
uint32_t fftSize = 1024; 
uint32_t ifftFlag = 0; 
uint32_t doBitReverse = 1;
uint32_t refIndex = 213, testIndex = 0;

#endif

void configureADC() {
	
	//RCC
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	//GPIO
	TM_GPIO_Init(MIC_PORT, MIC1_PIN, TM_GPIO_Mode_AN, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_Medium);
	TM_GPIO_Init(MIC_PORT, MIC2_PIN, TM_GPIO_Mode_AN, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_Medium);
	TM_GPIO_Init(MIC_PORT, MIC3_PIN, TM_GPIO_Mode_AN, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_Medium);
	TM_GPIO_Init(MIC_PORT, MIC4_PIN, TM_GPIO_Mode_AN, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_Medium);
	
	//TIM
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 1999;
	TIM_TimeBaseStructure.TIM_Prescaler = 4000;
	//	TIM_TimeBaseStructure.TIM_Prescaler = 17999;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_SelectOutputTrigger(TIM2,TIM_TRGOSource_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
 
	//NVIC
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_Init(&NVIC_InitStructure);
	
	//DMA
	DMA_InitTypeDef DMA_InitStructure;
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_Channel = DMA_Channel_0; /* See Tab 20 */
	DMA_InitStructure.DMA_BufferSize = 4;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory; /* direction */
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; /* no FIFO */
	DMA_InitStructure.DMA_FIFOThreshold = 0;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; /* circular buffer */
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; /* high priority */
	/* config of memory */
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADCBuffer; /* target addr */
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; /* 16 bit */
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	//AH, i changed these to stream4 bc of conflict with TM
	DMA_Init(DMA2_Stream4, &DMA_InitStructure); /* See Table 20 for mapping */
	DMA_Cmd(DMA2_Stream4, ENABLE);
	
	/* reset configuration if needed, could be used for previous init */
	ADC_Cmd(ADC1, DISABLE);
	ADC_DeInit();
	
	//ADC
	ADC_InitTypeDef ADC_InitStructure;
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 4; /* 5 channels in total */
	ADC_Init(ADC1, &ADC_InitStructure);
	
	//ADC Common
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_CommonStructInit(&ADC_CommonInitStructure);
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	/* Enable Vref & Temperature channel */
	ADC_TempSensorVrefintCmd(ENABLE);
 
	/* Configure channels */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_112Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_112Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_112Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_112Cycles);
 
	/* Enable DMA request after last transfer (Single-ADC mode) */
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
 
	ADC_DMACmd(ADC1, ENABLE);
 
	ADC_Cmd(ADC1, ENABLE);
	
	//start Tim (used to be in main)
	//TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler() {
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	//printf("IRQ handler\n");
	mic0Buffer[bufferPos] = ADCBuffer[0];
	mic1Buffer[bufferPos] = ADCBuffer[1];
	mic2Buffer[bufferPos] = ADCBuffer[2];
	mic3Buffer[bufferPos] = ADCBuffer[3];
	bufferPos = (bufferPos + 1) % BUFFERSIZE;
	if(bufferPos == 0)
	  ready = 1;
}

void fillAmplitudeArray() {
	amplitudes[0] = getAmplitude(mic0Buffer);
	amplitudes[1] = getAmplitude(mic1Buffer);
	amplitudes[2] = getAmplitude(mic2Buffer);
	amplitudes[3] = getAmplitude(mic3Buffer);
	
	int maxAmpIndex = 0;
	int maxAmp = amplitudes[0];
	printf("amplitudes: ");
	for (int i = 0; i < NUM_MICS; i++) {
		printf("%d, ", amplitudes[i]);
		if (amplitudes[i] > maxAmp) {
			maxAmpIndex = i;
			maxAmp = amplitudes[i];
		}
	}
	loudestIndex = maxAmpIndex;
	printf("LOUDEST is mic %d\n", maxAmpIndex);
}

int getAmplitude(uint16_t *buffer) {
	int max = 0;
	int min = 4095;
	for (int i = 0; i < BUFFERSIZE; i++) {
		int val = buffer[i];
		if (val < min) min = val;
		if (val > max) max = val;
	}
	return max - min;
}

void displayLoudest(int loudestIndex) {
	char text[20];
	sprintf(&text, "mosquito near mic %d", loudestIndex);
	TM_ILI9341_Puts(30, 180, text, &TM_Font_11x18, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE);
}

void displayAmplitudes() {
	int colWidth = ILI9341_HEIGHT / NUM_MICS;
	int x = 0;
	int y = 240;
	for (int i = 0; i < NUM_MICS; i++) {
		int amp = amplitudes[i];
		int topY = 240 - ((amp/(float)4095) * 240);
		TM_ILI9341_DrawFilledRectangle(x, y, x+colWidth, topY, ILI9341_COLOR_RED);
		TM_ILI9341_DrawFilledRectangle(x, topY, x+colWidth, 0, ILI9341_COLOR_WHITE);
		x += colWidth;
	}
}

void displayWelcomeScreen() {
	//circles
	TM_ILI9341_DrawCircle(60, 60, 40, ILI9341_COLOR_GREEN);
	TM_ILI9341_DrawFilledCircle(60, 60, 35, ILI9341_COLOR_RED);
	
	//Draw blue rectangle
	TM_ILI9341_DrawRectangle(120, 20, 200, 100, ILI9341_COLOR_BLUE);
	//Draw black filled rectangle
	TM_ILI9341_DrawFilledRectangle(130, 30, 190, 90, ILI9341_COLOR_BLACK);
	
	//circles
	TM_ILI9341_DrawCircle(260, 60, 40, ILI9341_COLOR_GREEN);
	TM_ILI9341_DrawFilledCircle(260, 60, 35, ILI9341_COLOR_RED);
	
	//Draw line with custom color 0x0005
	TM_ILI9341_DrawLine(10, 120, 310, 120, 0x0005);
	
	//Put string with black foreground color and blue background with 11x18px font
	TM_ILI9341_Puts(20, 130, "MOSQUITO DETECTOR", &TM_Font_16x26, ILI9341_COLOR_BLACK, ILI9341_COLOR_WHITE);
	
	//Draw line with custom color 0x0005
	TM_ILI9341_DrawLine(10, 165, 310, 165, 0x0005);
	
	TM_ILI9341_Puts(30, 180, "Adrienne Humblet\nAlex Thomson", &TM_Font_11x18, ILI9341_COLOR_BLUE, ILI9341_COLOR_WHITE);
}

int main(void)
{
	//Initialize system
	SystemInit();
	init_systick();
	//init adc dma system for mics
	configureADC();
	//Initialize ILI9341
	//TM_ILI9341_Init();
	
	//Rotate LCD for 90 degrees
	//TM_ILI9341_Rotate(TM_ILI9341_Orientation_Landscape_1);

	//displayWelcomeScreen();
	//TM_ILI9341_DrawFilledRectangle(0, 0, ILI9341_HEIGHT, ILI9341_WIDTH, ILI9341_COLOR_WHITE);

#ifdef FFT
	for(int i = 0; i < TEST_LENGTH_SAMPLES/2; i++) {
	  testInput_f32_10khz[2*i] = sin(600*PI*(float)i/(float)TEST_LENGTH_SAMPLES);
	  testInput_f32_10khz[2*i+1] = 0;
	}

	arm_cfft_radix4_instance_f32 S; 
	float32_t maxValue; 
	arm_cfft_radix4_init_f32(&S, fftSize, ifftFlag, doBitReverse);
	arm_cfft_radix4_f32(&S, testInput_f32_10khz);
	arm_cmplx_mag_f32(testInput_f32_10khz, testOutput, fftSize);  
	arm_max_f32(testOutput, fftSize, &maxValue, &testIndex); 
	printf("The Max Value is: %f, at %d\n", maxValue, testIndex);
#endif

#ifdef SERIAL
	//init_systick();
	init_serial_port_usb();
	char s[128];
	delay_ms(3000);
	int n_input = 0;

#endif
#ifndef SERIAL
	TIM_Cmd(TIM2, ENABLE);
#endif

	while(1){

#ifndef SERIAL
	  if(ready){
	    TIM_Cmd(TIM2, DISABLE);
	    ready = 0;
	    
	    int lmax = 0;
	    for(int i = 0; i < BUFFERSIZE; i++) {
	      int val = mic3Buffer[i];
	      if(val > lmax) lmax = val;
	    }
	    printf("Max: %d\n", lmax);
	    delay_ms(3000);
	    printf("Enabled!\n");
	    TIM_Cmd(TIM2, ENABLE);
	  }
#endif

#ifdef SERIAL
	  //write_serial_usb_bytes("Hello\n", 6);

	  while (1)
	    {
	      uint8_t c;
	      uint8_t read_byte = read_serial_usb_byte(&c); // read one character
	      if (read_byte == 1) // if a character was read
		{
		  if(c == 'a'){
		    //float g = 515.123;
		    //write_serial_usb_bytes(&g,4); // 
		    int length = 2;
		    float32_t data = 16.16;
		    float32_t data2 = 17.17;
		    write_serial_usb_bytes(&length, 4);  
		    write_serial_usb_bytes(&data, 4);
		    write_serial_usb_bytes(&data2, 4);

		  }

		}
	      else break; // if no more characters at the current time
	    }
	  delay_ms(1000);
#endif


	}
	  /*	
	int prevLoudestIndex;
	while (1) {
		if (loudestIndex != prevLoudestIndex) {
			displayLoudest(loudestIndex);
			prevLoudestIndex = loudestIndex;
		}
		if (bufferPos == (BUFFERSIZE - 1)) {
			fillAmplitudeArray();
		}
	}
	*/

	}


