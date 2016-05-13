#ifndef LED_PWM_H
#define LED_PWM_H

#include <stdint.h>

// set PWM frequency (not counter clock frequency!) -- in Hz
void setTIM4_PWMFrequency( uint32_t frequency );

// set duty cycle of channel (1,2,3,or 4) to duty_cycle specified as percentage
// channel 2 is connected to the LED
void setTIM4_PWMDuty( uint32_t channel , uint32_t duty_cycle );
	


#endif