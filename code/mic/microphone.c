#include "microphone.h"

static PDMFilter_InitStruct pdm_filter;
static uint16_t mic_in_buf[MIC_IN_BUF_SIZE];
extern uint16_t * mic_out_buf;
static uint32_t mic_buf_index;
volatile uint32_t Audio_Data_Status = 0;

// initialize
void mp45dt02_init(){
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
	pdm_filter.LP_HZ=8000;
	pdm_filter.HP_HZ=10;
	pdm_filter.Fs=FS;
	pdm_filter.Out_MicChannels=1;
	pdm_filter.In_MicChannels=1;
	PDM_Filter_Init(&pdm_filter);

	//MP45DT02 CLK-PB10
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef gpio;
	gpio.GPIO_Pin=GPIO_Pin_10;
	gpio.GPIO_Mode=GPIO_Mode_AF;
	gpio.GPIO_OType=GPIO_OType_PP;
	gpio.GPIO_PuPd=GPIO_PuPd_NOPULL;
	gpio.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &gpio);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_SPI2);

	//MP45DT02 DOUT-PC3
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	gpio.GPIO_Pin=GPIO_Pin_3;
	gpio.GPIO_Mode=GPIO_Mode_AF;
	gpio.GPIO_OType=GPIO_OType_PP;
	gpio.GPIO_PuPd=GPIO_PuPd_NOPULL;
	gpio.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &gpio);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_SPI2);

	NVIC_InitTypeDef nvic;
	nvic.NVIC_IRQChannel = SPI2_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 1;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	//I2S2 config
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_PLLI2SCmd(ENABLE);
	while(!RCC_GetFlagStatus(RCC_FLAG_PLLI2SRDY));
	I2S_InitTypeDef i2s;
	i2s.I2S_AudioFreq=I2S_CLK;
	i2s.I2S_Standard=I2S_Standard_LSB;
	i2s.I2S_DataFormat=I2S_DataFormat_16b;
	i2s.I2S_CPOL=I2S_CPOL_High;
	i2s.I2S_Mode=I2S_Mode_MasterRx;
	i2s.I2S_MCLKOutput=I2S_MCLKOutput_Disable;
	I2S_Init(SPI2, &i2s);
	I2S_Cmd(SPI2, ENABLE);
}

// start microphone
void mp45dt02_start(){
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
}

// interrupt service routine
void SPI2_IRQHandler(){	
	if(SPI_GetITStatus(SPI2, SPI_I2S_IT_RXNE)){
		uint16_t sample=SPI_I2S_ReceiveData(SPI2);
		mic_in_buf[mic_buf_index++]=HTONS(sample);
		if (Audio_Data_Status == 1) return;
		if(mic_buf_index == MIC_IN_BUF_SIZE){
			mic_buf_index=0;
			PDM_Filter_64_LSB((uint8_t *)mic_in_buf, mic_out_buf, 40, &pdm_filter);
			Audio_Data_Status = 1;
		}
	}
}
