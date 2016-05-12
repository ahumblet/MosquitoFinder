#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "defines.h"
#include "tm_stm32f4_gpio.h"
#include "tm_stm32f4_spi.h"
#include <stdio.h>
#include <stdlib.h>


#define FFT

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

#ifdef SLAVE
#define TM_SPI3_MASTERSLAVE SPI_Mode_Slave
#define DATAVAL 3
#endif

#ifdef MASTER
#define DATAVAL 5
#endif



int main(void)
{
  //Initialize system
  SystemInit();
	

#ifdef SLAVE
  TM_SPI_Init(SPI3, TM_SPI_PinsPack_1);
  TM_GPIO_Init(GPIOB, GPIO_PIN_9, TM_GPIO_Mode_IN, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_Medium);
#endif

#ifdef MASTER
  TM_SPI_Init(SPI3, TM_SPI_PinsPack_1);
  TM_GPIO_Init(GPIOB, GPIO_PIN_9, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_NOPULL, TM_GPIO_Speed_Medium);
  TM_GPIO_SetPinValue(GPIOB, GPIO_PIN_9, 1);
#endif

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


while (1) {

#ifdef SLAVE
  if(!TM_GPIO_GetInputPinValue(GPIOB, GPIO_PIN_9)){
    uint8_t data = TM_SPI_Send(SPI3, DATAVAL);
    printf("%d\n",data);
  }

#endif


#ifdef MASTER
  TM_GPIO_SetPinValue(GPIOB, GPIO_PIN_9, 0);
  uint8_t data = TM_SPI_Send(SPI3, DATAVAL);
  printf("%d\n",data);
  TM_GPIO_SetPinValue(GPIOB, GPIO_PIN_9, 1);
#endif



 }
}


