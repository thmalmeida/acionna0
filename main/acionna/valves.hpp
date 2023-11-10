#ifndef _VALVES_HPP__
#define _VALVES_HPP__

#include <adc.hpp>
#include <gpio.hpp>
#include <pinout.hpp>

#include "esp_log.h"
#include "helper.hpp"

#include "pcy8575.hpp"

static const char *TAG_VALVES = "VALVES";

// Pressures of each sector in [m.c.a.]
#define valve01_nominal_pressure 57	//20230305	//62 20220809
#define valve02_nominal_pressure 59
#define valve03_nominal_pressure 58
#define valve04_nominal_pressure 62
#define valve05_nominal_pressure 62
#define valve06_nominal_pressure 66
#define valve07_nominal_pressure 62
#define valve08_nominal_pressure 62
#define valve09_nominal_pressure 62
#define valve10_nominal_pressure 62
#define valve11_nominal_pressure 62	// 20230325

class Valves {
public:

	states_valves state_valves = states_valves::system_off;
	// unsigned int time_total_cfg = 0;					// sum of programmed valves time [s];	
	unsigned int time_valve_remain = 0;					// valve time elapsed before turn off [s];
	unsigned int time_valve_change = 0;					// between changes to verify if pressure has been pressure recovered [s];
	static const int number_valves = 12;
	states_flag flag_inverted_sequence = states_flag::disable;

	// uint16_t* stream_array;

	Valves(I2C_Master* i2c, uint32_t* epoch_time) : load_{i2c}, epoch_time_{epoch_time} {
		init_valve_parameters();
	}
	// Valves() : ac_load_{{VALVE_01},{VALVE_02},{VALVE_03},{VALVE_04},{VALVE_05},{VALVE_06},{VALVE_07},{VALVE_08},{VALVE_09},{VALVE_10},{VALVE_11}} {
	// 	ESP_LOGI(TAG_VALVES, "The VALVES was initialized HERE!!!!");

	// 	// pins directions for drive switches;
	// 	for(std::size_t i = 0; i < ac_load_count_; i++)
	// 	{
	// 		ac_load_[i].mode(GPIO_MODE_INPUT_OUTPUT);
	// 		ac_load_[i].write(0);
	// 	}
	// 	init_valve_parameters();
	// }
	void init_valve_parameters(void) {
		set_valve_pressure(1, valve01_nominal_pressure);
		set_valve_pressure(2, valve02_nominal_pressure);
		set_valve_pressure(3, valve03_nominal_pressure);
		set_valve_pressure(4, valve04_nominal_pressure);
		set_valve_pressure(5, valve05_nominal_pressure);
		set_valve_pressure(6, valve06_nominal_pressure);
		set_valve_pressure(7, valve07_nominal_pressure);
		set_valve_pressure(8, valve08_nominal_pressure);
		set_valve_pressure(9, valve09_nominal_pressure);
		set_valve_pressure(10, valve10_nominal_pressure);
		set_valve_pressure(11, valve11_nominal_pressure);

		set_valve_time(1, 50);
		set_valve_time(2, 45);
		set_valve_time(3, 45);
		set_valve_time(4, 30);
		set_valve_time(5, 30);
		set_valve_time(6, 28);
		set_valve_time(7, 30);
		set_valve_time(8, 30);
		set_valve_time(9, 30);
		set_valve_time(10, 30);
		set_valve_time(11, 30);
		remove(12);
		// set_valve_time(12, 0);
	}

	// This functions should be called every 1 second interval
	void update() {
		if(state_valves == states_valves::automatic_switch) {
			if(!time_valve_remain) {					// next() function find new programmed valve_current_ and it's elapsed time (time_valve_ramain).
				set_valve_state(valve_current_, 0);		// Turn the current valve sector to find another programmed;
				make_log_update();						// refresh the elapsed time valve on log vector
				if(time_valve_elapsed_) {
						time_valve_elapsed_ = 0;		// Clear valve elapsed time [s].
				}
				// algorithm to find next programmed valve sector. 
				// This function turn on the next programmed valve found and refresh the time_valve_elapsed.
				if(next()) {							// Return 0 if none is found meaning finish working cycle.
					make_log();							// Found programmed valve sector. Make some log.
				} else {
					stop();								// Couldn't find new programmed valve sector or achieve end cycle. Stop valve switch process.
				}
			} else
				time_valve_remain--;

			time_valve_elapsed_++;
			time_system_on_++;
		}
	}
	void make_log(void) {
		// move all elements to next slot
		for(int i=(log_n-1); i>0; i--) {
			log_valves[i].valve_id = log_valves[i-1].valve_id;
			log_valves[i].started_time = log_valves[i-1].started_time;
	
			log_valves[i].elapsed_time = log_valves[i-1].elapsed_time;
		}

		// fill new element on the beginner of structure array.
		log_valves[0].valve_id = valve_current_;
		log_valves[0].started_time = *epoch_time_;

		log_valves[0].elapsed_time = 0;
	}
	void make_log_update(void) {
		log_valves[0].elapsed_time = time_valve_elapsed_;
	}
	// void make_log(void) {
	// 	log_valves[valve_seq].elapsed_time = valve_seq_elapsed_time;
	// }
	void set_valve_time(int valve_id, unsigned int value) {
		valve_[valve_id-1].time_elapsed_cfg = value*60.0;
	}
	unsigned int get_valve_time(int valve_id) {
		return valve_[valve_id-1].time_elapsed_cfg/60.0;
	}
	void set_valve_pressure(int valve_id, unsigned int value) {
		valve_[valve_id-1].pressure = value;
	}
	int get_valve_pressure(int valve_id) {
		return valve_[valve_id-1].pressure;
	}
	unsigned int get_total_time_programmed() {
		unsigned int _total_time = 0;

		for(int i=0; i<number_valves; i++)
		{
			if(valve_[i].programmed == states_flag::enable)
			{
				_total_time += valve_[i].time_elapsed_cfg;
			}
		}

		return _total_time;
	}
	void set_valve_state(int valve_id, int _valve_state) {
		if(valve_id && (valve_id <= number_valves))	{
			load_.write(valve_id, _valve_state);
		}
		else
			ESP_LOGI(TAG_VALVES, "id error");
	}
	states_switch get_valve_state(int valve_id) {
		// if(ac_load_[valve_id-1].read())
		if(load_.read(valve_id))
			return states_switch::on;
		else
			return states_switch::off;		
	}
	void start(void) {
		load_.put(0x0000);								// reset all valves;
		valve_seq = 0;									// reset valve sequence number;
		time_valve_remain = 0;							// reset current valve time;
		time_valve_elapsed_ = 0;						// reset time elapsed during on state;
		time_system_on_ = 0;							// reset working time cycle;
		state_valves = states_valves::automatic_switch;	// put automatic valve switch state on;
		flag_valve_found_ = states_flag::disable;		// disable found new programmed valve sector;

		if(flag_inverted_sequence == states_flag::enable)
			valve_current_ = number_valves+1;			// if inverted sequence, start from last valve sector;
		else
			valve_current_ = 0;							// if not, start from the first programmed valve sector.
	}
	void stop(void) {
		// for(int n=1; n<number_valves+1; n++)
		// {
		// 	set_valve_state(n, 0);
		// }
		load_.put(0x0000);		// reset all valves;	
		// because sometimes the motor turn off and do not refresh time due the state_valves = states_valves::system_off;
		if(time_valve_elapsed_) {
			make_log_update();
			time_valve_elapsed_ = 0;
		}
		valve_current_ = 0;
		state_valves = states_valves::system_off;		
	}
	unsigned int next(void) {
		states_flag flag_valve_found_ = states_flag::disable;
		valve_last_close_ = valve_current_;						// store the last working valve;

		if(flag_inverted_sequence == states_flag::enable) {
			do {
				valve_current_--;
				if(valve_current_)
				{
					if(get_valve_programmed(valve_current_) == states_flag::enable)
					{
						flag_valve_found_ = states_flag::enable;
						time_valve_remain = get_valve_time(valve_current_)*60.0;
						set_valve_state(valve_current_, 1);
						// set_valve_state(valve_current_+1, 0);
					}
					// else
					// 	valve_current_--;
				}
				else
				{
					flag_valve_found_ = states_flag::enable;
					state_valves = states_valves::system_off;
				}
			} while(flag_valve_found_ == states_flag::disable);
		}
		else {
			do {
				valve_current_++;
				if(valve_current_ < number_valves + 1)
				{
					if(get_valve_programmed(valve_current_) == states_flag::enable)
					{
						flag_valve_found_ = states_flag::enable;
						time_valve_remain = get_valve_time(valve_current_)*60.0;
						set_valve_state(valve_current_, 1);
						// set_valve_state(valve_current_-1, 0);
					}
					// else
						// valve_current_++;
				}
				else
				{
					flag_valve_found_ = states_flag::enable;
					state_valves = states_valves::system_off;
					valve_current_ = 0;
					return 0;
				}

			} while(flag_valve_found_ == states_flag::disable);
		}

		set_valve_state(valve_current_, 0);

		return valve_current_;
	}
	void next_forced() {
		time_valve_remain = 0;
	}
	void remove(unsigned int _valve_num) {
		valve_[_valve_num-1].programmed = states_flag::disable;
	}
	void add(unsigned int _valve_num) {
		valve_[_valve_num-1].programmed = states_flag::enable;
	}
	states_flag get_valve_programmed(unsigned _valve_num) {
		return valve_[_valve_num-1].programmed;
	}
	unsigned int get_time_on() {
		return time_system_on_;
	}
	unsigned int valves_test_routine() {
		/*
		turn all drives off;

		wait a moment;

		turn the first driver ON;
		wait a momemnt;
		read adc
		calculate
		
		*/
		return 0;
	}
	uint8_t valve_current(void) {
		return valve_current_;
	}

	// Functions directly to PCY8575
	int module_probe(void) {
		return load_.probe();
	}
	void module_reset(void) {
		load_.soft_reset();
	}
	void module_put(uint16_t word) {
		load_.put(word);
	}
	uint16_t module_get(void) {
		return load_.get();
	}
	uint16_t module_temperature(void) {
		return load_.temperature();
	}
	uint32_t module_uptime(void) {
		return load_.uptime();
	}
	void module_i_process(void) {
		load_.i_process(0x00);
	}
	uint16_t module_irms(void) {
		return load_.irms();
	}
	void module_i_data_transfer(void) {
		load_.i_data();
	}
	void module_i_n_points(int length) {
		load_.i_n_points(length);
	}
	int module_i_n_points(void) {
		
		return load_.i_n_points();
	}
	uint16_t module_read_i_sample(int i) {
		return load_.stream_array_raw[i];
	}
	void module_data_test(void) {
		load_.data_test();
	}

	static const int log_n = 12;

	struct {
		uint8_t valve_id;									// valve id
		uint32_t started_time;								// start time since epoch [s]
		uint16_t elapsed_time;								// total time it was on [s]
		int max_pressure;
		int min_pressure;
		int avg_pressure;
	}log_valves[log_n] = {};

	uint8_t valve_current_ = 0;								// current working valve;
	uint8_t valve_last_close_ = 0;							// last current valve to close after time_delay_close_
	uint8_t valve_seq = 0;									// valve sequence number during the cycle;
	uint32_t valve_seq_elapsed_time = 0;					// last elapsed time [s];

private:
	// GPIO_Basic ac_load_[11];
	pcy8575 load_;											// connection with i2c_to_gpio module;
	// const std::size_t ac_load_count_ = sizeof(ac_load_) / sizeof(ac_load_[0]);

	struct {
		int pressure = 0;									// pressão nominal daquele setor [m.c.a.];
		int pressure_avg = 0;									// average pressure while on state;
		// states_switch state = states_switch::off;		// estado da válvula;
		states_flag programmed = states_flag::enable;		// enable or disable to schedule list;
		unsigned int time_elapsed_cfg = 0;					// tempo que o setor ficará ligado [s];
		unsigned int time_on_last = 0;						// tempo ligado ou último tempo ligado [s];
	} valve_[number_valves];

	uint32_t time_system_on_ = 0;							// current time on [s];
	uint32_t time_valve_elapsed_ = 0;						// reset time elapsed during on state;
	uint32_t time_delay_close_ = 0;							// delay time to turn solenoide off after sector change on next() function;
	uint32_t *epoch_time_;									// epoch time linked with system;
	states_flag flag_valve_found_ = states_flag::disable;
	states_flag flag_valve_close_ = states_flag::disable;	// flag to close last current valve;


	//	GPIO_Basic drive_kn_[3]={GPIO_Basic{AC_LOAD1},GPIO_Basic{AC_LOAD2},GPIO_Basic{AC_LOAD3}};
};
#endif
