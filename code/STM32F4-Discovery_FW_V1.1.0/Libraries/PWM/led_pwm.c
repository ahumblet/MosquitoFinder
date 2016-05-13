#include "led_pwm.h"

#define TIM4_ARR 0x4000082C
#define TIM4_PSC 0x40000828
#define TIM4_CCR1 0x40000834
#define TIM4_CCR2 0x40000838
#define TIM4_CCR3 0x4000083C
#define TIM4_CCR4 0x40000840


// set PWM frequency (not counter clock frequency!) -- in Hz
void setTIM4_PWMFrequency( uint32_t frequency )
{
	uint32_t base_clock = 84000000; // 84 MHz
	uint32_t div = base_clock/frequency; // total division required
	uint32_t prescaler1 = (int)(div/65535 + 1); // prescaler should be at least sufficient to get the required PWM frequency
  // largest value of ARR is 65535; hence, smallest value of prescaler1 is div/65535
	// note that integer division truncates; prescaler 1 will be at least 1
if (prescaler1 < 1) prescaler1 = 1;
if (prescaler1 > 65535) prescaler1 = 65535;
  * ( (volatile uint32_t *) TIM4_PSC) = (prescaler1 - 1); // actual division is by (prescaler value + 1)
  uint32_t scaled_clock = base_clock / prescaler1;
	uint32_t arr = scaled_clock/frequency;
if (arr < 1) arr = 1;
if (arr > 65535) arr = 65535;
	* ( (volatile uint32_t *) TIM4_ARR) = arr - 1;
}

// set duty cycle of channel (1,2,3,or 4) to duty_cycle specified as percentage
// channel 2 is connected to the LED
void setTIM4_PWMDuty( uint32_t channel , uint32_t duty_cycle )
{
	uint32_t arr = * ( (volatile uint32_t *) TIM4_ARR);
	uint32_t ccr = ((float)(arr+1) * (duty_cycle / 100.0));
if (ccr < 0) ccr = 0;
if (ccr > 65535) ccr = 65535;
	if (channel == 1) { * ( (volatile uint32_t *) TIM4_CCR1) = ccr; }
  if (channel == 2) { * ( (volatile uint32_t *) TIM4_CCR2) = ccr; }
  if (channel == 3) { * ( (volatile uint32_t *) TIM4_CCR3) = ccr; }
  if (channel == 4) { * ( (volatile uint32_t *) TIM4_CCR4) = ccr; }	
}
