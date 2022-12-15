#ifndef ACIONNA_HPP__
#define ACIONNA_HPP__

#include <iostream>
#include <sstream>
#include <string>

#include <i2c_master.h>
#include "ds3231.hpp"
#include "rtc_time.hpp"

#ifdef CONFIG_BT_ENABLE
#include "bt_setup.hpp"
#endif
#include "wifi_setup.hpp"
#include "https_ota.hpp"
#include "native_ota.hpp"

#include "json/ArduinoJson-v6.19.4.h"

#include "pump.hpp"
#include "pipepvc.hpp"
#ifdef CONFIG_WELL_SUPPORT
#include "well.hpp"
#endif
#ifdef CONFIG_VALVES_SUPPORT
#include "valves.hpp"
#endif

#include "helper.hpp"

class Acionna {
public:
	// states_period state_period = states_period::redTime;
	states_mode state_mode = states_mode::system_idle;

	// group of variables to setup turn on starts
	uint8_t time_match_n = 1;											// turn times range 1-9
	uint32_t time_match_list[9] = {3600, 0, 0, 0, 0, 0, 0, 0, 0};	// time clock list [s]
	uint32_t time_to_shutdown[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};			// timer list to shutdown [s]
	start_types auto_start_mode[9]; 									// auto turn start type select

	// uint8_t signal_restart = 0;
	// uint8_t signal_wifi_info = 0;
	// uint8_t signal_wifi_scan = 0;
	// uint8_t signal_send_async = 0;
	// uint8_t signal_request_sensors = 0;
	// uint8_t signal_ram_usage = 0;
	// uint8_t signal_reset_reason = 0;

	// uint8_t signal_json_data_server = 0;
	// uint8_t signal_ota_update = 0;
	// uint8_t signal_ota_info = 0;

	Acionna() {
		init();
	}

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

	std::string msg_back_str_;

	// OS system;
	void run(void);

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
	states_flag flag_json_data_back = states_flag::disable;			// Continuously send data back. ws server mode.
	
	// Flags for communication purpose
	states_flag ws_server_ans_flag_ = states_flag::disable;
	states_flag bt_ans_flag_ = states_flag::disable;
	states_flag ws_client_ans_flag_ = states_flag::disable;
	
	uint8_t command_str_[20];
	// uint8_t command_str_len_ = 0;

	// Handle message process flags
	// states_flag flag_enable_decode_ = states_flag::disable;

	uint32_t uptime_ = 0;											// uptime in seconds
	uint32_t time_day_sec_ = 0;

	// communication member functions
	void msg_fetch_(void);
	void msg_exec_(void);
	void msg_back_(void);
	void msg_json_back_(void);

	void parser_(uint8_t* payload_str, int payload_str_len, uint8_t* command_str, int& command_str_len);

	void sys_fw_change_boot_(void);
	void sys_fw_info_app_(char* buffer_str);
	void sys_fw_info_partitions_(char* buffer_str);
	void sys_fw_info_sha256_(char* buffer_str);
	void sys_fw_mark_valid_(void);
	void sys_fw_mark_invalid_(void);
	void sys_fw_update_(void);
	void sys_fw_update_ans_async_(void);

	void sys_wifi_info_(char* buffer_str);
	void sys_wifi_scan_(char* buffer_str);
	void sys_ram_free_(char* buffer_str);
	void sys_reset_reason_(char* buffer_str);
	void sys_restart_(void);
	void sensor_dht(void);
};
#endif