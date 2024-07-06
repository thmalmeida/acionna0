#ifndef SYS_TIME_HPP__
#define SYS_TIME_HPP__

// System includes
// #include <cstdint>

// Specific for uC (ESP32)
#include "esp_timer.h"		// uC timer

/* System time is the internal microcontroller timer hardware.
 * It suppose to bring unix time from some hardware source like:
 *	- peripheral RTC;
 *	- peripheral timer counter;
*/

enum class sys_time_src {
	nd = 0,				// not defined or disable
	rtc,				// from rtc
	high_resolution		// high resolution timer
};

// using time_s = std::uint32_t;
using time_s = uint32_t;

class SYS_Time {
public:
	SYS_Time(void) {}
	// SYS_Time(sys_time_src ts);
	// ~SYS_Time(void);

	// basic functions
	// void init(void) noexcept;

	// get the uptime in seconds
	uint32_t uptime_sec(void) {
		return  static_cast<uint32_t>(esp_timer_get_time() / 1000000);
	}

	int64_t uptime_ms(void) {
		return  esp_timer_get_time()/1000;
	}

	int64_t uptime_us(void) {
		return  esp_timer_get_time();
	}


	// // Set unix time to internal hardware timer
	// void unix_time(time_s ut) {
	// 		uptime_sec_ = static_cast<value_time>(esp_timer_get_time() / 1000000);
	// }
	
// private:
// 	sys_time_src timer_src_;

};

#endif /* SYS_TIME_HPP__ */
