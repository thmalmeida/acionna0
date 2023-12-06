#include "delay.hpp"

void delay_ms(uint32_t milliseconds)
{
	sys_delay_ms(milliseconds);
}
void delay_us(uint32_t microseconds)
{
	uint64_t microseconds_0 = esp_timer_get_time();

	if(microseconds) {
		// while (((uint64_t) esp_timer_get_time() - microseconds_0) <= number_of_microseconds) {
			while((static_cast<uint64_t>(esp_timer_get_time()) - microseconds_0) < static_cast<uint64_t>(microseconds)) {
			// Wait
		}
	}

	// 2 - old way using internal function (not recommended)
	// ets_delay_us(microseconds);
}
