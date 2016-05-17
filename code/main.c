#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "defines.h"
#include "tm_stm32f4_gpio.h"
#include "tm_stm32f4_spi.h"
#include <stdio.h>
#include <stdlib.h>


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

#endif



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

#ifdef SERIAL
init_systick();
init_serial_port_usb();
char s[128];
delay_ms(3000);
int n_input = 0;

#endif


while (1) {

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


