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
#define valve01_nominal_pressure	57	// 20231123 //62 20220809
#define valve02_nominal_pressure	59	// 20231123
#define valve03_nominal_pressure	61	// 20231123
#define valve04_nominal_pressure	62
#define valve05_nominal_pressure	62
#define valve06_nominal_pressure	66
#define valve07_nominal_pressure	62
#define valve08_nominal_pressure	63
#define valve09_nominal_pressure	63
#define valve10_nominal_pressure	63	// 20231213
#define valve11_nominal_pressure	63	// 20231123

// area in m2 of each sector
#define valve01_area				7476
#define valve02_area				6468
#define valve03_area				6468
#define valve04_area				4326
#define valve05_area				4326
#define valve06_area				3620
#define valve07_area				4326
#define valve08_area				4326
#define valve09_area				4326
#define valve10_area				4326
#define valve11_area				4326

class Valves {
public:

	states_valves state_valves = states_valves::system_off;
	// unsigned int time_total_cfg = 0;					// sum of programmed valves time [s];	
	unsigned int time_valve_remain = 0;					// valve time elapsed before turn off [s];
	unsigned int time_valve_change = 0;					// between changes to verify if pressure has been pressure recovered [s];
	static const int number_valves = 12;
	states_flag flag_inverted_sequence = states_flag::disable;

	// uint16_t* stream_array;

	Valves(I2C_Master* i2c, uint32_t* epoch_time, int* pressure) : load_{i2c}, epoch_time_{epoch_time}, pressure_mca_{pressure} {
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

		// set time to each valve
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

		// set nominal pressure for each valve
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

		// fill struct valve area
		set_valve_area(1, valve01_area);
		set_valve_area(2, valve02_area);
		set_valve_area(3, valve03_area);
		set_valve_area(4, valve04_area);
		set_valve_area(5, valve05_area);
		set_valve_area(6, valve06_area);
		set_valve_area(7, valve07_area);
		set_valve_area(8, valve08_area);
		set_valve_area(9, valve09_area);
		set_valve_area(10, valve10_area);
		set_valve_area(11, valve11_area);

		set_valve_rain_mm_all(5.0);

		// calculate the flow rate [m3/h] for each valve
		calc_valve_flow_all();

		// For debug purpose only
		for(int i=1; i<number_valves; i++) {
			printf("valve[%d] area:%d, flow:%.1f\n", i, get_valve_area(i), valve_[i].flow);
		}		

		remove(12);
		// set_valve_time(12, 0);
	}

	// This functions should be called every 1 second interval
	void update() {
		if(state_valves == states_valves::automatic_switch) {

			if(!time_valve_remain) {				// next() function find new programmed valve_current_ and it's elapsed time (time_valve_ramain).
				set_valve_state(valve_current_, 0);	// Turn off the current valve sector to find another programmed;
				
				if(time_valve_elapsed_) {
						time_valve_elapsed_ = 0;	// Clear valve elapsed time [s].
				}

				// Algorithm to find next programmed valve sector:
				// 	This function turn on the next programmed valve found and refresh the time_valve_elapsed.
				if(next()) {						// Ruturn true if some next programmed sector was found. Else return 0 if none is found meaning finish working cycle.
					make_log();						// Found programmed valve sector. Make some log.
				} else {
					stop();							// Couldn't find new programmed valve sector or achieve end cycle. Stop valve switch process.
				}
			} else {
				time_valve_remain--;
			}

			make_log_update();						// refresh the elapsed time valve on log vector
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
			log_valves[i].pressure_avg = log_valves[i-1].pressure_avg;
			log_valves[i].pressure_min = log_valves[i-1].pressure_min;
			log_valves[i].pressure_min = log_valves[i-1].pressure_max;
		}

		// fill new element on the beginner of structure array.
		log_valves[0].valve_id = valve_current_;
		log_valves[0].started_time = *epoch_time_;

		log_valves[0].elapsed_time = 0;

		// maybe this starting values suppose to take after some seconds of valves on state.
		log_valves[0].pressure_avg = pressure_avg();
		log_valves[0].pressure_min = pressure_avg();
		log_valves[0].pressure_max = pressure_avg();
	}
	void make_log_update(void) {
		log_valves[0].elapsed_time = time_valve_elapsed_;

		log_valves[0].pressure_avg = pressure_avg();


		if(*pressure_mca_ < log_valves[0].pressure_min) {
			log_valves[0].pressure_min = *pressure_mca_;
		}

		if(*pressure_mca_ > log_valves[0].pressure_max) {
			log_valves[0].pressure_max = *pressure_mca_;
		}
	}
	// void make_log(void) {
	// 	log_valves[valve_seq].elapsed_time = valve_seq_elapsed_time;
	// }
	int pressure_avg(void) {
		// find current valve on;

		// 
		for(int i=(press_vec_n_-2); i>=0; i--) {
			press_vec_[i+1] = press_vec_[i];
		}

		press_vec_[0] = *pressure_mca_;

		int p_avg = 0;
		for(int i=0; i<press_vec_n_; i++) {
			p_avg += press_vec_[i];
		}

		return p_avg/press_vec_n_;
	}
	void set_valve_time(int valve_id, unsigned int value) {
		valve_[valve_id-1].time_elapsed_cfg = value*60.0;
	}
	uint32_t get_valve_time(int valve_id) {
		return valve_[valve_id-1].time_elapsed_cfg/60.0;
	}
	void set_valve_pressure(int valve_id, unsigned int value) {
		valve_[valve_id-1].pressure_exp = value;
	}
	int get_valve_pressure(int valve_id) {
		return valve_[valve_id-1].pressure_exp;
	}
	uint32_t get_total_time_programmed() {
		uint32_t _total_time = 0;

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

		// Set a kind of pointer to next() function find the next valve
		if(flag_inverted_sequence == states_flag::enable)
			valve_current_ = number_valves+1;			// if inverted sequence, start from last valve sector;
		else
			valve_current_ = 0;							// if not, start from the first programmed valve sector.

		// the update will call the next() function and find the next programmed valve.	
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
					}
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
					}
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

		// set_valve_state(valve_current_, 0);

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
	uint8_t valve_current(void) {
		return valve_current_;
	}


	/*
	* The following functions is used to find the flow rate
	* Math functions like pow are used.
	* This automate to find a specific time for each valve/sector given an amount of rain measured in mm
	*/
	// water volume by sector (math part)
	void set_valve_area(int valve_id, int area) {
		valve_[valve_id - 1].area = area;
	}
	int get_valve_area(int valve_id) {
		return valve_[valve_id - 1].area;
	}



	/*
	* @brief Flow rate calculate by pressure
	*
	* It calculates the flow rate by pressure of ME-32125 12,5 cv Schneider waterpump. 
	* The curve fits on a third degree polinomial function;
	*
	* @param p pressure on output pump [m.c.a.]
	* @return flow flow rate found [m3/h]
	*/
	float calc_flow_by_press(float p) {
		float a[6] = {-7.81404597668744e-07, 0.0001344964277056774, -0.007860563868574868, 0.1461761489432135, 1.196684722526538, 0.08659938993111427};

		int pdegree = 5;
		float flow = 0.0;

		for(int i=0; i<=pdegree; i++) {
			flow += a[i]*pow(p, pdegree-i);
		}

		return flow;
	}
	/* @brief Calculate the flow rate for each valve in [m3/h] the calculation is based on the sector area and the water pump flow with it's pressure
	*  @param valve_id valve id
	*/
	void calc_valve_flow(int valve_id) {
		// for(int i=0; i<number_valves; i++) {
			valve_[valve_id-1].flow = calc_flow_by_press(static_cast<float>(valve_[valve_id-1].pressure_exp));
		// }
	}
	/* @brief Automated function to calculate flow rate for all valves [m3/h]
	*/
	void calc_valve_flow_all(void) {
		for(int i=1; i<=number_valves; i++) {
			calc_valve_flow(i);
		}
	}
	/* @Desired rain in mm
	 *
	 * Rain in mm is a parameter to calculate the volume of water based on flow and rate
	 * 
	 * @param r_mm Rain in mm
	*/
	void set_valve_rain_mm(int valve_id, float r_mm) {
		valve_[valve_id - 1].rain_mm = r_mm;
	}
	void set_valve_rain_mm_all(float r_mm) {
		for(int i=1; i<=number_valves; i++) {
			set_valve_rain_mm(i, r_mm);
		}
	}
	float get_valve_rain_mm(int valve_id) {
		return valve_[valve_id-1].rain_mm;
	}
	/* @brief Find time to obtain a mm of rain
	*
	* 	Flow rate and mm volume per area determined by each valve sector.
	*
	* 	mm*A*10^-3 = V [m3]
	* 	Q*t = V
	* 	Q*t = mm*A*10^-3
	* 	Time in seconds
	* 	t = mm*A*10^-3/Q*3600 [s]
	* @param valve_id valve id number
	*/
	void calc_time_by_rain_mm(int valve_id) {
		int i = valve_id - 1;
		valve_[i].volume = valve_[i].rain_mm*valve_[i].area/1000.0;
		valve_[i].time_elapsed_cfg = valve_[i].volume/valve_[i].flow*3600.0;
		ESP_LOGI(TAG_VALVES, "volume:%.1f, rain_mm:%.1f, area:%d, flow:%.1f, time:%lu", valve_[i].volume, valve_[i].rain_mm, valve_[i].area, valve_[i].flow, valve_[i].time_elapsed_cfg);
	}
	void calc_time_by_rain_mm_all(void) {
		for(int i=1; i<=number_valves; i++)
			calc_time_by_rain_mm(i);
	}

	// Test routines
	unsigned int valves_test_routine() {
		
		// for(int i=0; i<number_valves; i++) {

		// }
		// turn all drives off (wait a moment)

		// turn the first driver ON (wait a momemnt)

		// read adc and calculate

		// store on a vector
		// valves.last_test_irms
		// valves[]

		return 0;
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

	static const int log_n = 15;

	struct {
		uint8_t valve_id;									// valve id
		uint32_t started_time;								// start time since epoch [s]
		uint16_t elapsed_time;								// total time it was on [s]
		int pressure_max;
		int pressure_min;
		int pressure_avg;									// average pressure while on state;
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
		// states_switch state = states_switch::off;		// estado da válvula;
		states_flag programmed = states_flag::enable;		// enable or disable to schedule list;
		uint32_t time_elapsed_cfg = 0;						// tempo que o setor ficará ligado [s];
		uint32_t time_on_last = 0;							// tempo ligado ou último tempo ligado [s];
		int last_test_irms = 0;								// last current found on test

		// physical parameters
		int pressure_exp = 0;								// pressão nominal esperada daquele setor [m.c.a.];
		int pressure_avg = 0;								// last average pressure [m.c.a.];
		int area = 0;										// the area that sector occupies [m2];
		float flow = 0.0;									// the flow rate by water pump with it's pressure [m3/h]
		float volume = 0.0;									// volume of rain_mm in [m3]
		float rain_mm = 0.0;								// how much mm of rain it's desired [mm];
	} valve_[number_valves];

	static const int press_vec_n_ = 10;
	int press_vec_[press_vec_n_] = {0};

	uint32_t time_system_on_ = 0;							// current time on [s];
	uint32_t time_valve_elapsed_ = 0;						// reset time elapsed during on state;
	uint32_t time_delay_close_ = 0;							// delay time to turn solenoide off after sector change on next() function;
	uint32_t *epoch_time_;									// epoch time linked with system;
	int *pressure_mca_;										// pipe pressure from pointer to valves class;
	states_flag flag_valve_found_ = states_flag::disable;
	states_flag flag_valve_close_ = states_flag::disable;	// flag to close last current valve;


	//	GPIO_Basic drive_kn_[3]={GPIO_Basic{AC_LOAD1},GPIO_Basic{AC_LOAD2},GPIO_Basic{AC_LOAD3}};
};
#endif
