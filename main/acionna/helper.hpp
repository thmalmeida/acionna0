#ifndef HELPER_HPP__
#define HELPER_HPP__

#include <iostream>
#include "arch/sys_arch.h"
#include "hardware_defs.h"  // include for delay times
#include "ArduinoJson-v6.19.4.h"

typedef struct {
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} time_hour_format;

enum class states_flag {
	disable = 0,
	enable
};

enum class states_switch {
	off = 0,
	on
};

enum class state_conn {
    disconnected = 0,
    connected
};

uint16_t timesec_to_day(uint32_t seconds);
uint8_t timesec_to_hour(uint32_t seconds);
uint8_t timesec_to_min(uint32_t seconds);
uint8_t timesec_to_sec(uint32_t seconds);
uint32_t daytime_to_timesec(uint8_t hour, uint8_t min, uint8_t sec);
void _delay_ms(uint32_t milliseconds);
void _delay_us(uint32_t microseconds);
void json_test(void);

#endif