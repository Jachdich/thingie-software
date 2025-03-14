#ifndef _HARDWARE_PWM_H
#define _HARDWARE_PWM_H
#include "gpio.h"
#define PWM_CHAN_A 1
void pwm_set_chan_level(int slice_num, int pwm_channel, int amt);
#endif