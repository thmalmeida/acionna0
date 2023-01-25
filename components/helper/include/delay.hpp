#ifndef DELAY_HPP__
#define DELAY_HPP__

#include "arch/sys_arch.h"	        // include for delays
#include "esp32/rom/ets_sys.h"      // include for ets_delay_us()

void delay_ms(uint32_t milliseconds);
void delay_us(uint32_t microseconds);

#endif /* DELAY_HPP__ */
