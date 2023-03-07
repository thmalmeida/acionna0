#ifndef VALVES_HPP
#define VALVES_HPP

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
#define valve05_nominal_pressure 64
#define valve06_nominal_pressure 68
#define valve07_nominal_pressure 64
#define valve08_nominal_pressure 64
#define valve09_nominal_pressure 64
#define valve10_nominal_pressure 65
#define valve11_nominal_pressure 65

class Valves {
public:

	states_valves state_valves = states_valves::system_off;
	unsigned int valve_current = 0;		// 
	unsigned int time_total_cfg = 0;	// sum of programmed valves time [s];	
	unsigned int time_valve_remain = 0;	// valve time elapsed before turn off [s];
	unsigned int time_valve_change = 0;	// between changes to verify if pressure has been pressure recovered [s];
	static const int number_valves = 12;
	states_flag flag_inverted_sequence = states_flag::disable;

	Valves(I2C_Master *i2c) : load_{i2c} {
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
	void update()
	{
		if(state_valves == states_valves::automatic_switch)
		{
			time_system_on_++;

			if(!time_valve_remain)
			{
				valve_current = next();
				if(!valve_current)
				{
					stop();
				}
			}
			else
				time_valve_remain--;
		}
		// to implement
	}
	void set_valve_time(int valve_number, unsigned int value)
	{
		valve_[valve_number-1].time_elapsed_cfg = value*60.0;
	}
	unsigned int get_valve_time(int valve_number)
	{
		return valve_[valve_number-1].time_elapsed_cfg/60.0;
	}
	void set_valve_pressure(int valve_number, unsigned int value)
	{
		valve_[valve_number-1].pressure = value;
	}
	int get_valve_pressure(int valve_number)
	{
		return valve_[valve_number-1].pressure;
	}
	unsigned int get_total_time_programmed()
	{
		unsigned int _total_time = 0;

		for(int i=0; i<number_valves; i++)
		{
			if(valve_[i].programmed == states_flag::enable)
			{
				_total_time += valve_[i].time_elapsed_cfg;
			}
		}

		return _total_time/60.0;
	}
	void set_valve_state(int _valve_id, int _valve_state) {
		if(_valve_id && (_valve_id <= number_valves))
		{
			// ac_load_[_valve_id-1].write(_valve_state);
			load_.write(_valve_id, _valve_state);
			// ESP_LOGI(TAG_VALVES, "valve[%d]: %d", _valve_id, static_cast<int>(get_valve_state(_valve_id)));
			// ESP_LOGI(TAG_VALVES, "valve[%d], set to %d", _valve_id, _valve_state);
		}
		else
			ESP_LOGI(TAG_VALVES, "valve[%d], NOT set: %d", _valve_id, _valve_state);
	}
	states_switch get_valve_state(int _valve_id) {
		// if(ac_load_[_valve_id-1].read())
		if(load_.read(_valve_id))
			return states_switch::on;
		else
			return states_switch::off;		
	}
	void start()
	{
		load_.put(0x0000);		// reset all valves;
		time_valve_remain = 0;	// reset current valve time;
		time_system_on_ = 0;	// reset valve time system;
		state_valves = states_valves::automatic_switch;
		flag_valve_found_ = states_flag::disable;

		if(flag_inverted_sequence == states_flag::enable)
			valve_current = number_valves+1;
		else
			valve_current = 0;
	}
	void stop()
	{
		load_.put(0x0000);		// reset all valves;
		// for(int n=1; n<number_valves+1; n++)
		// {
		// 	set_valve_state(n, 0);
		// }

		state_valves = states_valves::system_off;
		valve_current = 0;
	}
	void next_forced(void) {
		time_valve_remain = 0;
	}
	unsigned int next() {
		states_flag flag_valve_found_ = states_flag::disable;
		set_valve_state(valve_current, 0);

		if(flag_inverted_sequence == states_flag::enable)
		{
			do {
				valve_current--;
				if(valve_current)
				{
					if(get_valve_programmed(valve_current) == states_flag::enable)
					{
						flag_valve_found_ = states_flag::enable;
						time_valve_remain = get_valve_time(valve_current)*60.0;
						set_valve_state(valve_current, 1);
						// set_valve_state(valve_current+1, 0);
					}
					// else
					// 	valve_current--;
				}
				else
				{
					flag_valve_found_ = states_flag::enable;
					state_valves = states_valves::system_off;
				}
			} while(flag_valve_found_ == states_flag::disable);
		}
		else
		{
			do{
				valve_current++;
				if(valve_current < number_valves + 1)
				{
					if(get_valve_programmed(valve_current) == states_flag::enable)
					{
						flag_valve_found_ = states_flag::enable;
						time_valve_remain = get_valve_time(valve_current)*60.0;
						set_valve_state(valve_current, 1);
						// set_valve_state(valve_current-1, 0);
					}
					// else
						// valve_current++;
				}
				else
				{
					flag_valve_found_ = states_flag::enable;
					state_valves = states_valves::system_off;
					valve_current = 0;
					return 0;
				}

			} while(flag_valve_found_ == states_flag::disable);
		}

		return valve_current;
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
	uint16_t module_irms(void) {
		return load_.irms();
	}
	
	private:
		// GPIO_Basic ac_load_[11];
		pcy8575 load_;

		// const std::size_t ac_load_count_ = sizeof(ac_load_) / sizeof(ac_load_[0]);

		struct valves_sector{
			int pressure;									// pressão nominal daquele setor [m.c.a.];
			states_switch state = states_switch::off;		// estado da válvula;
			states_flag programmed = states_flag::enable;	// se entra para a jornada ou não. enable or disable;
			unsigned int time_elapsed_cfg;					// tempo que o setor ficará ligado [s];
			unsigned int time_on_last;						// tempo ligado ou último tempo ligado [s];
		} valve_[number_valves];

		unsigned int time_system_on_ = 0;					// current time on [s];
		states_flag flag_valve_found_ = states_flag::disable;


	//	GPIO_Basic drive_kn_[3]={GPIO_Basic{AC_LOAD1},GPIO_Basic{AC_LOAD2},GPIO_Basic{AC_LOAD3}};
};
#endif
