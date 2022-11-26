#ifndef ACIONNA_HPP__
#define ACIONNA_HPP__

#include <iostream>
#include <sstream>
#include <string>

#include "../../build/config/sdkconfig.h"

#include "esp_timer.h"
#include "esp_eeprom.hpp"

#include "pump.hpp"
#include "pipepvc.hpp"

#include <i2c_master.h>
#include "ds3231.hpp"
#include "rtc_time.hpp"
#include "helper.hpp"

#ifdef CONFIG_WELL_SUPPORT
#include "well.hpp"
#endif

#ifdef CONFIG_VALVES_SUPPORT
#include "valves.hpp"
#endif

enum class states_period {
	greenTime,
	redTime
};
enum class states_mode {
	system_off = 0,					// never turn and don't let any load turn on;
	system_idle,					// idle means that never turn any load automaticaly. But can work manual with all updates and checks
	water_pump_control_night,
	irrigation_pump_valves,
	valve_control
};

class ACIONNA {
public:
	// states_period state_period = states_period::redTime;
	states_mode state_mode = states_mode::system_idle;

	// group of variables to setup turn on starts
	uint8_t time_match_n = 1;											// turn times range 1-9
	uint32_t time_match_list[9] = {3600, 0, 0, 0, 0, 0, 0, 0, 0};	// time clock list [s]
	uint32_t time_to_shutdown[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};			// timer list to shutdown [s]
	start_types auto_start_mode[9]; 									// auto turn start type select

	uint8_t signal_restart = 0;
	uint8_t signal_wifi_info = 0;
	uint8_t signal_wifi_scan = 0;
	uint8_t signal_send_async = 0;
	uint8_t signal_request_sensors = 0;
	uint8_t signal_ram_usage = 0;
	uint8_t signal_reset_reason = 0;
	uint8_t signal_json_data_back = 0;
	uint8_t signal_json_data_server = 0;
	uint8_t signal_ota_update = 0;
	uint8_t signal_ota_info = 0;

	// Initialize - should run once;
	void init();

	// run every second;
	void operation_motorPeriodDecision();
	void operation_system_off();
	void operation_valve_control();
	void operation_pump_control();
	void operation_pump_valves_irrigation();

	uint32_t get_uptime();

	void update_RTC();
	void update_objects();
	void update_stored_data();
	void update_uptime();
	void update_all();
	void update_sensors();
	std::string handle_message(uint8_t* command_str);
	void operation_mode();

	// OS system;
	void run();

	// communication member functions
	void parser_1(uint8_t* payload_str, int payload_str_len, uint8_t* command_str, int& command_str_len);

	Pipepvc pipe1_;
	Pump pump1_;
	// pipepvc pipe2_;

private:
	
	#ifdef CONFIG_WELL_SUPPORT
	well well1_;
	#endif
	#ifdef CONFIG_VALVES_SUPPORT
	valves valves1_;
	#endif

	states_flag flag_check_k1_ = states_flag::disable;
	states_flag flag_check_k2_ = states_flag::disable;
	states_flag flag_check_k3_ = states_flag::disable;
	states_flag flag_check_high_level_ = states_flag::disable;
	states_flag flag_check_low_level_ = states_flag::disable;
	states_flag flag_check_period_ = states_flag::disable;
	states_flag flag_check_pressure_low_ = states_flag::disable;
	states_flag flag_check_pressure_high_ = states_flag::disable;
	states_flag flag_check_pressure_valve_ = states_flag::disable;
	states_flag flag_check_thermal_relay_ = states_flag::disable;
	states_flag flag_start_request_ = states_flag::disable;			// flag request to start motor;
	states_flag flag_check_time_match_ = states_flag::disable;
	states_flag flag_time_match_ = states_flag::disable;			// flag when turn on time occurs;
	
	// Handle message process flags
	// states_flag flag_enable_decode_ = states_flag::disable;

	uint32_t uptime_ = 0;											// uptime in seconds
	uint32_t time_day_sec_ = 0;
};
#endif
