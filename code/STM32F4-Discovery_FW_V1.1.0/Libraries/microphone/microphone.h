#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"
#include "pdm_filter.h"


//sampling frequency
#define FS 16000

//PDM decimation factor
#define DECIMATION 64

//clock for mic
#define I2S_CLK ((FS*DECIMATION)/(16*2))

//uint16_t array length for microphone input buffer
#define MIC_IN_BUF_SIZE 64

//uint16_t array length for PDM filter output buffer
#define MIC_OUT_BUF_SIZE 16


void mp45dt02_init(void);

void mp45dt02_start(void);

void SPI2_IRQHandler(void);
#endif
