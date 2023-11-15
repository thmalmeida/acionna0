#ifndef _ACIONNA_HPP__
#define _ACIONNA_HPP__

#include <iostream>
#include <sstream>
#include <string>

#include <adc.hpp>

#include "esp_chip_info.h"
#include "esp_flash.h"

#include <i2c_master.hpp>
#include "ds3231.hpp"
#include "rtc_time.hpp"
#include "time_operations.hpp"

#ifdef CONFIG_BT_ENABLE
#include "bt_setup.hpp"
#endif
#include "wifi_setup.hpp"
#include "https_ota.hpp"
#include "native_ota.hpp"

#include "json/ArduinoJson-v6.19.4.h"

#ifdef CONFIG_WELL_SUPPORT
#include "well.hpp"
#endif

#include "pump.hpp"
#include "pipepvc.hpp"
#include "valves.hpp"

// Sensors
#include "bmp180.hpp"
#include "aht10.hpp"
#include "dsp.hpp"
#include "timer_driver.hpp"

#include "helper.hpp"
#include "convert_char_to_hex.h"

volatile extern uint8_t flag_1sec;
volatile extern uint8_t flag_100ms;

class Acionna {
public:
	// states_period state_period = states_period::redTime;
	states_mode state_mode = states_mode::system_ready;

	// group of variables to setup turn on starts
	// uint8_t time_match_n = 1;									// turn times range 1-9
	// uint32_t time_match[9] = {3600, 0, 0, 0, 0, 0, 0, 0, 0};	// time clock list [s] should convert to hours and minutes
	// uint32_t time_to_shutdown[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};	// time value to shutdown list [s]
	// start_types auto_start_mode[9]; 							// auto turn start type select

	// For make log history operation - Can be ported to inside the pump class?
	// static const int log_n_ = 9;									// history log size
	// uint32_t time_started_lasts[log_n_] = {0};
	// start_types start_mode_lasts[log_n_];
	
	// uint32_t time_elapsed_on_lasts[log_n_] = {0};
	// stop_types stops_lasts[log_n_];

	Acionna(ADC_driver* adc);	// : pipe1_(&adc, 4), pipe2_(&adc, 7) {

	// Initialize - should run once;
	void init(void);

	// run every second;
	void operation_system_off(void);
	void operation_pump_control(void);
	void operation_pump_valves_irrigation(void);
	void operation_pump_water_optimized(void);

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

	// void make_log(start_types start_type, uint32_t time_now);
	// void make_log(stop_types stop_type, uint32_t time_elapsed_on);

	// OS system;
	void run(void);
	void run_every_second(void);

private:
	
	#ifdef CONFIG_WELL_SUPPORT
	well well1_;
	#endif

	Pipepvc pipe1_;
	Pipepvc pipe2_;
	Pump pump1_;
	Valves valves1_;

	states_flag flag_check_k1_ = states_flag::disable;
	states_flag flag_check_k2_ = states_flag::disable;
	states_flag flag_check_k3_ = states_flag::disable;
	states_flag flag_check_high_level_ = states_flag::disable;
	states_flag flag_check_low_level_ = states_flag::disable;
	states_flag flag_check_period_ = states_flag::disable;
	states_flag flag_check_pressure_low_ = states_flag::disable;
	states_flag flag_check_pressure_high_ = states_flag::enable;		// enable by default to make system more safe;
	states_flag flag_check_pressure_valve_ = states_flag::disable;
	states_flag flag_check_thermal_relay_ = states_flag::disable;
	states_flag flag_start_request_ = states_flag::disable;				// flag request to start motor;
	states_flag flag_check_time_match_ = states_flag::disable;
	states_flag flag_time_match_ = states_flag::disable;				// flag when turn on time occurs;
	states_flag flag_check_timer_ = states_flag::enable;
	states_flag flag_json_data_back_ = states_flag::disable;			// Continuously send data back. ws server mode.
	states_flag flag_check_valves_ = states_flag::disable;				// continuously check valves through PCY8575 module
	states_flag flag_check_valves_time_match_ = states_flag::disable;
	states_flag flag_check_low_pressure_ = states_flag::enable;			// global flag
	states_flag flag_check_low_pressure_k1_ = states_flag::disable;
	states_flag flag_check_low_pressure_k2_ = states_flag::enable;
	states_flag flag_check_low_pressure_delta_ = states_flag::enable;
	states_flag flag_check_time_match_optimized_ = states_flag::enable;
	states_flag flag_time_match_optimized_ = states_flag::enable;
	// states_flag flag_enable_decode_ = states_flag::disable;

	// Flags for communication purpose
	states_flag ws_server_ans_flag_ = states_flag::disable;
	states_flag bt_ans_flag_ = states_flag::disable;
	states_flag ws_client_ans_flag_ = states_flag::disable;
	
	uint8_t command_str_[20];
	// uint8_t command_str_len_ = 0;

	// time match struct array
	uint8_t time_match_n = 1;									// turn times range 1-9
	struct {
		uint32_t time_match;									// time clock list [s] should convert to hours and minutes
		uint32_t time_to_shutdown;								// time value to shutdown list [s]
		start_types auto_start_mode; 							// auto turn start type select
	}time_match_list[9]={};

	uint32_t uptime_ = 0;										// uptime in seconds
	uint32_t time_day_sec_ = 0;									// time of day in seconds
	uint32_t epoch_time_ = 0;									// epoch system time
	int pressure_ = 0;											// pressure variable to bring from pipe to valve

	// Optimized mode
	struct {
		uint32_t time_match_start = 0;	// first start time epoch [s]
		uint32_t time_match_next = 0;	// next time programmed epoch [s];
		uint32_t time_stop;				// time stopped epoch [s]
		uint32_t time_delay = 5*60;		// delay time after low pressure dectect before turn on again;
		uint32_t time_red = 6*3600;		// day time to stop system;
		states_flag started = states_flag::disable;
		states_flag flag_time_next_config = states_flag::disable;	// will enable when motor start into optimized cycle to enable next time setup when it turn off
		start_types start_mode = start_types::direct_k2;
		uint32_t time_to_shutdown = 0;
	}optimized;


	// communication member functions
	void msg_fetch_(void);
	void msg_exec_(void);
	void msg_back_(void);
	void msg_json_back_(void);

	void parser_(uint8_t* payload_str, int payload_str_len, uint8_t* command_str, int& command_str_len);

	void sys_chip_info(char* buffer_str);

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
	void sys_wifi_mac_(char* buffer_str);
	void sys_ram_free_(char* buffer_str);
	void sys_reset_reason_(char* buffer_str);
	void sys_restart_(void);
	void sensor_dht(void);
};
#endif