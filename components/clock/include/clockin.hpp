#ifndef CLOCKIN_HPP__
#define CLOCKIN_HPP__

// System includes
#include <cstdint>

// Specific for uC (ESP32)
// #include "nvs_flash.h"		// eeprom store
#include "esp_timer.h"		// uC timer

// Specific for RTC time external module
// #include "ds3231.hpp"
// #include "ds1307.hpp"

// date and time operations
#include "date_time.hpp"
#include "sys_time.hpp"
// #include "ntp.hpp"

using time_s = std::uint32_t;
// using value_time = std::uint32_t;
// using fuse_type = int;
// using fuse_type = int;

enum class source_time {
	none = 0,				// RTC disabled
	rtc_internal,			// internal uC RTC
	hr_timer,				// high resolution uC internal timer
	system_time,			// not defined yet
	ds3231,					// external RTC module
	ds1307					// external RTC module
};

class Clockin {
public:
		// Clock(void) : dt{new Date_Time()} {

		// }
		Clockin() {
			init();
		}

		// init the rtc timer
		void init(void) {

		}

		// Bring data
		void fetch(void) {
			unix_time_ = epoch_time_();
			dt_.unix_time(unix_time_);
		}

		// update to upstream data registers
		void update(void) {
			time_offset_ = dt_.unix_time() - time0_.uptime_sec();
		}

		// timer register hardware
		void select_source(source_time source) {
			/* source from internal
			*	- RTC
			*	- High Resolution Timer */			
		}
		uint32_t unix_time(void) {
			return unix_time_;
		}

		// Set functions
		void year(uint16_t yr) {
			dt_.year(yr);
		}
		void month(uint8_t mo) {
			dt_.month(mo);
		}
		void day(uint8_t da) {
			dt_.day(da);
		}
		void hour(uint8_t hou) {
			dt_.hour(hou);
		}
		void minute(uint8_t min) {
			dt_.minute(min);
		}
		void second(uint8_t sec) {
			dt_.second(sec);
		}
		void date(uint16_t yr, uint8_t mo , uint8_t da) {
			year(yr);
			month(mo);
			day(da);
		}
		void time(uint8_t hou, uint8_t min, uint8_t sec) {
			hour(hou);
			minute(min);
			second(sec);
		}
		void date_time(uint16_t yr, uint8_t mo, uint8_t da, uint8_t hou, uint8_t min, uint8_t sec) {
			date(yr, mo, da);
			time(hou, min, sec);
		}

		// Get functions
		uint16_t year(void) {
			return dt_.year();
		}
		uint8_t month(void) {
			return dt_.month();
		}
		uint8_t day(void) {
			return dt_.day();
		}

		uint8_t hour(void) {
			return dt_.hour();
		}
		uint8_t minute(void) {
			return dt_.minute();
		}
		uint8_t second(void) {
			return dt_.second();
		}




		// Time values get and set member functions
private:
	SYS_Time time0_;
	Date_Time dt_;

	uint32_t time_offset_;		// wake up time

	uint32_t unix_time_;		// suppose to be refreshed when on fetch()

	uint32_t epoch_time_(void) {
		return time0_.uptime_sec() + time_offset_;
	}
};

#endif /* CLOCK_HPP__ */



// 		void init(DS3231& i2c) noexcept;

// 		bool has_rtc() const noexcept;

// 		fuse_type fuse() const noexcept;
// 		void fuse(fuse_type) noexcept;

// 		value_time get_time() noexcept;
// 		value_time get_local_time() noexcept;

// 		value_time internal_time() const noexcept;
// 		void set_time(value_time time) noexcept;
// 	private:
// #if CONFIG_DEVICE_CLOCK_DS3231_SUPPORT
// 		DS3231*			rtc_ = nullptr;
// 		bool			rtc_present_ = false;
// #endif /* CONFIG_DEVICE_CLOCK_DS3231_SUPPORT */
// 		value_time		uptime_sec_ = 0;
// 		value_time		time_ = 0;
// 		fuse_type		fuse_;