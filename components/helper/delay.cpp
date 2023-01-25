#include "delay.hpp"

void delay_ms(uint32_t milliseconds)
{
	sys_delay_ms(milliseconds);
}
void delay_us(uint32_t microseconds)
{
	ets_delay_us(microseconds);
}
