#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "defines.h"

#define FFT

#ifdef FFT
#include "math.h"
#define PI 3.14
#define DATA_LEN 1024
#endif

#ifdef SLAVE
#define TM_SPI3_MASTERSLAVE SPI_Mode_Slave
#define DATAVAL 3
#endif

#ifdef MASTER
#define DATAVAL 5
#endif

#include "tm_stm32f4_gpio.h"
#include "tm_stm32f4_spi.h"
#include <stdio.h>
#include <stdlib.h>

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
  float data[DATA_LEN];
  for(int i = 0; i < DATA_LEN; i++) {
    data[i] = sin(2.0*PI*(float)i/(float)DATA_LEN);
  }

  
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


