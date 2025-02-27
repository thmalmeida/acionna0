#ifndef ACIONNA_HPP__
#define ACIONNA_HPP__

// esp32 specifics
#include "esp_chip_info.h"
#include "esp_flash.h"

// peripherals
#include "adc_driver.hpp"
#include "i2c_driver.hpp"
#include "timer_driver.hpp"

// math signal processing
#include "dsp.hpp"

// Time clock
#include "clockin.hpp"
#include "date_time.hpp"
#include "time_operations.hpp"

// Sync time over ntp
#include "ntp.hpp"

// Sensors and modules over I2C
// #include "ds3231.hpp"
// #include "ds1307.hpp"
// #include "aht10.hpp"
#include "ahtx0.hpp"
// #include "bmp180.hpp"
#include "bmp280.hpp"

// system 
#include "json/ArduinoJson-v6.19.4.h"
#include "helper.hpp"
#include "convert_char_to_hex.h"

// connectivity
#include "wifi_setup.hpp"
#include "https_ota.hpp"
#include "native_ota.hpp"
#ifdef CONFIG_BT_ENABLE
#include "bt_setup.hpp"
#endif

// Acionna parts
#include "pump.hpp"
#include "pipepvc.hpp"
#include "valves.hpp"
#ifdef CONFIG_WELL_SUPPORT
#include "well.hpp"
#endif

volatile extern uint8_t flag_1sec;
volatile extern uint8_t flag_100ms;

class Acionna {
public:
	// states_period state_period = states_period::redTime;
	states_mode state_mode = states_mode::system_ready;

	// group of variables to setup turn on starts
	// uint8_t time_match_n = 1;										// turn times range 1-9
	// uint32_t time_match[9] = {3600, 0, 0, 0, 0, 0, 0, 0, 0};			// time clock list [s] should convert to hours and minutes
	// uint32_t time_to_shutdown[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};		// time value to shutdown list [s]
	// start_types auto_start_mode[9]; 									// auto turn start type select

	// For make log history operation - Can be ported to inside the pump class?
	// static const int log_n_ = 9;										// history log size
	// uint32_t time_started_lasts[log_n_] = {0};
	// start_types start_mode_lasts[log_n_];
	
	// uint32_t time_elapsed_on_lasts[log_n_] = {0};
	// stop_types stops_lasts[log_n_];

	Acionna(ADC_Driver *adc, I2C_Driver *i2c);	// : pipe1_(&adc, 4), pipe2_(&adc, 7) {

	// Initialize - should run once;
	void init(void);

	// run every second;
	// void operation_pump_control(void);
	void operation_pump_start_match(void);
	void operation_pump_start_match_optimized(void);
	void operation_pump_stop_check(void);
	void operation_pump_lock_down(void);
	void operation_pump_optimized(void);
	void operation_pump_valves(void);
	

	uint32_t get_uptime();

	void update_clock();
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

	// DS3231 rtc{&i2c};
	// RTC_Time device_clock_;
	// Date_Time dt_;
	// RTC_Time rtc0_;

	Clockin dt_;

	Pipepvc pipe1_;
	Pipepvc pipe2_;
	Pump pump1_;
	Valves valves1_;

	AHTX0 s0_;
	BMP280 s1_;

	// SENSORS_CTRL sc0_;

	// I2C_Driver *i2c_;
	// ADC_Driver *adc_;

	states_flag flag_check_k1_ = states_flag::disable;					// check hardware input pin of k1
	states_flag flag_check_k2_ = states_flag::disable;					// check hardware input pin of k2
	states_flag flag_check_k3_ = states_flag::disable;					// check hardware input pin of k3
	states_flag flag_check_high_level_ = states_flag::disable;
	states_flag flag_check_low_level_ = states_flag::disable;
	states_flag flag_check_period_ = states_flag::disable;
	states_flag flag_check_pressure_low_ = states_flag::enable;			// global flag for low pressure detection
	states_flag flag_check_pressure_low_k1_ = states_flag::disable;
	states_flag flag_check_pressure_low_k2_ = states_flag::enable;
	states_flag flag_check_pressure_low_delta_ = states_flag::enable;
	states_flag flag_check_pressure_high_ = states_flag::enable;		// enable by default to make system more safe;
	states_flag flag_check_pressure_valve_ = states_flag::disable;
	states_flag flag_check_time_match_ = states_flag::disable;
	states_flag flag_check_thermal_relay_ = states_flag::disable;
	states_flag flag_check_timer_ = states_flag::enable;
	states_flag flag_check_valves_ = states_flag::disable;				// continuously check valves through PCY8575 module
	states_flag flag_check_valves_time_match_ = states_flag::disable;
	states_flag flag_check_time_match_optimized_ = states_flag::enable;

	states_flag flag_time_match_optimized_ = states_flag::disable;		// time match flag for optimized logic. Must initialized disabled.
	states_flag flag_start_request_ = states_flag::disable;				// flag request to start motor;
	states_flag flag_time_match_ = states_flag::disable;				// flag when turn on time occurs;
	states_flag flag_json_data_back_ = states_flag::disable;			// Continuously send data back. ws server mode.

	// Flags for communication purpose
	states_flag ws_server_ans_flag_ = states_flag::disable;
	states_flag bt_ans_flag_ = states_flag::disable;
	states_flag ws_client_ans_flag_ = states_flag::disable;

	// static ADC_Driver adc0(adc_mode::noption);

	// Sensor time refresh
	int timeout_sensors_ = 0;
	int timeout_sensors_cfg_ = 10;
	
	uint8_t command_str_[20];											// command buffer array
	// uint8_t command_str_len_ = 0;

	// time match struct array
	uint8_t time_match_n = 1;											// turn times range 1 to 9
	struct {
		uint32_t time_match;											// time clock list [s] should convert to hours and minutes
		uint32_t time_to_shutdown;										// time value to shutdown list [s]
		start_types auto_start_mode; 									// auto turn start type select
	}time_match_list[9]={};

	uint32_t uptime_ = 0;												// uptime in seconds
	uint32_t time_day_sec_ = 0;											// time of day in seconds
	uint32_t epoch_time_ = 0;											// epoch system time
	int pressure_ = 0;													// pressure variable from pipe to valve through pointer

	// Optimized mode - basic variables
	struct {
		uint32_t time_match_start = 21*60*60+31*60;						// first start time epoch [s]
		uint32_t time_match_next = 0;									// next time programmed epoch [s];
		// uint32_t time_stop = 0;											// time stopped epoch [s]
		uint32_t time_delay = 5*60;										// delay time after low pressure dectect before turn on again;
		uint32_t time_red = 6*3600;										// day time to stop system;
		// states_flag started = states_flag::disable;
		states_flag flag_time_next_config = states_flag::disable;		// will enable when motor start into optimized cycle to enable next time setup when it turn off

		states_flag flag_status_running = states_flag::disable;			// Working status
		
		// indexes to fsm working
		int cycle_i = 0;												// counter for number o cycles for each event
		int event_i = 0;												// the current event (type of start mode)
		int event_n = 1;												// n of events enable

		struct {
			start_types start_mode = start_types::direct_k2;			// type of start motor
			uint32_t time_to_shutdown = 0;								// time to stay on
			int cycles_n = 1;											// number of cycles of this specific event
		}event[9];														// event structure vector;
	
	}optimized;															// optimized structure

	// communication member functions
	void msg_fetch_(void);
	void msg_exec_(void);
	void msg_back_(void);
	void msg_json_back_(void);

	void parser_(uint8_t* payload_str, int payload_str_len, uint8_t* command_str, int& command_str_len);

	// Peripherals
	void peripheral_i2c_sensors_list(char* buffer_str);

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
	void sys_ticks_per_us(char *buffer_str);

	void ntp_show(char *buffer_str);
};
#endif