/******************************************************************************
“ÂÒÚÓ‚‡ˇ ÔÓ„‡ÏÏ‡ Â‡ÎËÁ‡ˆËË ‡·ÓÚ˚ LCD 2.2 ‰˛ÈÏ‡ ‰‡È‚Â ILI9341
******************************************************************************/

//#include "stm32l1xx.h"
#include "stm32f4xx.h"

//#include "stm32l1xx_conf.h" // ÔÓ‰ÍÎ˛˜ËÚ¸ ‚ÒÂ ıË‰Â˚ (ADC, GPIO, TIM Ë Ú.‰.)
#include "stm32f4xx_conf.h"

//stuff for the screen
/*#include "stm32f4xx.h"
#include "stm32f4xx_spi.h"
#include "defines.h"
#include "tm_stm32f4_ili9341.h"
#include "tm_stm32f4_fonts.h"*/

#include <stdio.h>
#include <stdlib.h>

///adc testing
#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

#include "stm32f4_discovery.h"

#include "stm32f4xx_adc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

volatile uint16_t ADCBuffer[] = {0xAAAA, 0xAAAA};

void adc_configure(){
	/* Define ADC init structures */
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
 
	/* Enable timer (timer runs at 21 MHz)*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 1999;
	TIM_TimeBaseStructure.TIM_Prescaler = 17999;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	TIM_SelectOutputTrigger(TIM2,TIM_TRGOSource_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
 
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_Init(&NVIC_InitStructure);
 
	/* Enable clock on DMA1 & GPIOC */
	/* Enable DMA2, thats where ADC is hooked on -> see Tab 20 (RM00090) */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_DMA2, ENABLE);
 
	/* Initialise GPIOs C0 (ADC123_IN10), C1 (ADC123_IN11)*/
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
 
	/* Initialise DMA */
	DMA_StructInit(&DMA_InitStructure);
 
	/* config of DMAC */
	DMA_InitStructure.DMA_Channel = DMA_Channel_0; /* See Tab 20 */
	DMA_InitStructure.DMA_BufferSize = 2; /* 5 * memsize */
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
	DMA_Init(DMA2_Stream0, &DMA_InitStructure); /* See Table 20 for mapping */
	DMA_Cmd(DMA2_Stream0, ENABLE);
 
	/* IMPORTANT: populate default values before use */
	ADC_StructInit(&ADC_InitStructure);
	ADC_CommonStructInit(&ADC_CommonInitStructure);
 
	/* reset configuration if needed, could be used for previous init */
	ADC_Cmd(ADC1, DISABLE);
	ADC_DeInit();
 
	/* init ADC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
 
	/* init ADC */
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);
 
	/* ADC1 Init: this is mostly done with ADC1->CR */
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfConversion = 2; /* 5 channels in total */
	ADC_Init(ADC1, &ADC_InitStructure);
 
	/* Enable Vref & Temperature channel */
	ADC_TempSensorVrefintCmd(ENABLE);
 
	/* Configure channels */
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_480Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_480Cycles);
 
	/* Enable DMA request after last transfer (Single-ADC mode) */
	ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
 
	/* Enable ADC3 DMA */
	ADC_DMACmd(ADC1, ENABLE);
 
	/* Enable ADC1 **************************************************************/
	ADC_Cmd(ADC1, ENABLE);
}


int counter = 0;

void TIM2_IRQHandler() {
	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	printf("%d\t%d\n", ADCBuffer[0], ADCBuffer[1]);
//	printf("%d\t%d\t%d\t%d\t%d\n", ADCBuffer[0], ADCBuffer[1], ADCBuffer[2], ADCBuffer[3], ADCBuffer[4]);
}

void ADC_IRQHandler() {
	/* acknowledge interrupt */
	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);

	printf("%d\t%d\n", ADCBuffer[0], ADCBuffer[1]);
	
}


int main(void){
	adc_configure();//Start configuration
	
	/* in main */
	TIM_Cmd(TIM2, ENABLE);
	
	ADC_SoftwareStartConv(ADC1);//Start the conversion
	
	while(1){
	}
}