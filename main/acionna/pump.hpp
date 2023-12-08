#ifndef _PUMP_HPP__
#define _PUMP_HPP__

#include <adc.hpp>
#include <gpio.hpp>
#include <pinout.hpp>

#include "esp_log.h"

#include "helper.hpp"
#include "delay.hpp"

//GPIO_Basic ac_load_[]={
//		GPIO_Basic{AC_LOAD1},
//		GPIO_Basic{AC_LOAD2},
//		GPIO_Basic{AC_LOAD3}};
//const std::size_t ac_load_count_ = sizeof(ac_load_) / sizeof(ac_load_[0]);
//GPIO_Basic gpio_generic_[]{
//		GPIO_Basic{GPIO_GENERIC1},
//		GPIO_Basic{GPIO_GENERIC2},
//		GPIO_Basic{GPIO_GENERIC3},
//		GPIO_Basic{GPIO_GENERIC4}
//};
//const std::size_t gpio_generic_count_ = sizeof(gpio_generic_) / sizeof(gpio_generic_[0]);
static const char *TAG_PUMP = "PUMP";

class Pump {
public:
	// Check flags
	states_flag flag_check_output_pin_only = states_flag::disable;
	// states_flag flag_check_Rth = states_flag::disable;
	states_flag flag_check_timer = states_flag::enable;
	states_flag flag_check_wait_power_on = states_flag::disable;
	states_flag flag_check_k3_off = states_flag::disable;

	// Motor times
	unsigned int time_wait_power_on = 0;			// 
	unsigned int time_wait_power_on_config = 600;	// [s]
	unsigned int time_to_shutdown = 0;				// time variable to shutdown the motor [s].
	unsigned int time_to_shutdown_config = 30*60;	// [s]
	unsigned int time_switch_k_change = 700;		// [ms] delay on transition of k3 off to k2 on
	unsigned int time_delta_to_y_switch_ = 0;		// speeding up time from delta to Y start [s]
	unsigned int time_delta_to_y_switch_config = 7;	// speeding up time from delta to Y start [s]

	// Technical parameters for debug purpose
	int time_k1_on_ = 0;							// time take to k1 switch from off to on
	int time_k2_on_ = 0;
	int time_k3_on_ = 0;
	int time_k1_off_ = 0;							// time take to k1 switch from on to off
	int time_k2_off_ = 0;
	int time_k3_off_ = 0;


//	Pump() : drive_k1_{AC_LOAD1}, drive_k2_{AC_LOAD2}, drive_k3_{AC_LOAD3}{}
	Pump(uint32_t* epoch_time) :
								ac_load_{{AC_LOAD1},{AC_LOAD2},{AC_LOAD3}},
								gpio_generic_{{GPIO_GENERIC1},{GPIO_GENERIC2},{GPIO_GENERIC3},{GPIO_GENERIC4}},
								epoch_time_{epoch_time} {
		// pins directions for drive switches;
		for(std::size_t i = 0; i < ac_load_count_; i++)
		{
			ac_load_[i].mode(GPIO_MODE_INPUT_OUTPUT);
			ac_load_[i].write(0);
		}
		// pins directions for thermal relay and switches
		for(std::size_t i = 0; i < gpio_generic_count_; i++)
		{
			gpio_generic_[i].pull(GPIO_PULLUP_ONLY);
			gpio_generic_[i].mode(GPIO_MODE_INPUT);
			// gpio_generic_[i].enable_interrupt(GPIO_INTR_ANYEDGE);
			// gpio_generic_[i].register_interrupt(gpio_input_interrput, nullptr);
		}

		ESP_LOGI(TAG_PUMP, "PUMP class initialize!");
	}

	// public member functions
	void update(void) {
		update_state_();
		update_time_();

		// check_Rth_();

		check_start_req_();
	}
	states_switch state_k1(void) const { // noexcept // const -> não modifica nada na classe - noexcept para dizer que não gera exceção
		return state_k1_;
	}
	states_switch state_k2(void) const { // noexcept // const -> não modifica nada na classe - noexcept para dizer que não gera exceção
		return state_k2_;
	}
	states_switch state_k3(void) const { // noexcept // const -> não modifica nada na classe - noexcept para dizer que não gera exceção
		return  state_k3_;
	}
	states_switch state_k1_pin(void) const {
		return state_k1_pin_;	
	}
	states_switch state_k2_pin(void) const {
		return state_k2_pin_;	
	}
	states_switch state_k3_pin(void) const {
		return state_k3_pin_;
	}
	states_switch state_Rth(void) const { // noexcept // const -> não modifica nada na classe - noexcept para dizer que não gera exceção
		return state_Rth_;
	}
	states_motor state(void) const {
		return state_motor_;
	}
	int start(start_types _mode) {

		switch (_mode) {
			case start_types::direct_k1: {
				drive_k_(1, 1);

				// check K change with time
				int i = 0;
				while(state_k1() != states_switch::on)
				{
					i++;
					delay_us(1);
					update_switches_();

					if(i > time_switch_k_change*1000) {
						ESP_LOGI(TAG_PUMP, "error on k1 change");
						stop(stop_types::contactor_not_on);
						time_k1_on_ = i;
						return 1;
					}
				}
				time_k1_on_ = i;
				delay_us(10000);								// delay for debounce purpose
				ESP_LOGI(TAG_PUMP, "k1 on");
				break;
			}
			case start_types::direct_k2: {
				drive_k_(2, 1);

				// check K change with time
				int i = 0;
				while(state_k2() != states_switch::on)
				{
					i++;
					// delay_us(1);
					delay_us(1);
					update_switches_();

					if(i > time_switch_k_change*1000)
					{
						ESP_LOGI(TAG_PUMP, "error on k2 change");
						stop(stop_types::contactor_not_on);
						return 1;
					}
				}
				time_k2_on_ = i;
				delay_us(10000);								// delay for debounce purpose
				ESP_LOGI(TAG_PUMP, "k2 on");
				break;
			}
			case start_types::direct_k3: {
				drive_k_(3, 1);

				// check K change with time
				int i = 0;
				while(state_k3() != states_switch::on)
				{
					i++;
					delay_us(1);
					update_switches_();

					if(i > time_switch_k_change*1000)
					{
						ESP_LOGI(TAG_PUMP, "error on k3 change");
						stop(stop_types::contactor_not_on);
						return 1;
					}
				}
				time_k3_on_ = i;
				delay_us(10000);								// delay for debounce purpose
				ESP_LOGI(TAG_PUMP, "k3 on");
				break;
			}
			case start_types::to_delta: {
				// K3 algorithm to make sure that is of before turn K2 on to avoid short circuit
				if((state_k3() == states_switch::on) || (state_k3_pin() == states_switch::on)) {
					ESP_LOGI(TAG_PUMP, "from Y to delta. K3: OFF");						
					drive_k_(3, 0);									// turn k3 off
					
					int i = 0;
					while((state_k3() == states_switch::on) || (state_k3_pin() == states_switch::on)) {
						i++;										// loop wait count
						delay_us(1);								// wait 1 millisecond
						update_switches_();							// update states
						if(i > time_switch_k_change*1000)			// number of loops is the time delay that k3 takes to switch off
						{
							ESP_LOGI(TAG_PUMP, "maybe k3 is lock");
							drive_k_(1, 0);
							drive_k_(2, 0);
							drive_k_(3, 0);
							ESP_LOGI(TAG_PUMP, "i: %d", i);
							return 1;
						}
					}
					ESP_LOGI(TAG_PUMP, "i: %d", i);
					delay_ms(time_switch_k_change);					// wait more time to keep system free of k3 and k2 short circuit
				}
				// K2 algorithm to turn on checking k3 state;
				if((state_k3() == states_switch::off) && (state_k3_pin() == states_switch::off)) {
					ESP_LOGI(TAG_PUMP, "K1 and K2: ON");
					drive_k_(2, 1);
					if((state_k1() == states_switch::off) && (state_k1_pin() == states_switch::off)) {
						ESP_LOGI(TAG_PUMP, "from zero to delta start");
						drive_k_(1, 1);
					}
					else {
						ESP_LOGI(TAG_PUMP, "from K1 on to delta start");
					}
					delay_us(10000);								// delay for debounce purpose
				} else {
					ESP_LOGI(TAG_PUMP, "from Y to delta fail on K3 change");
					return 1;
				}

				if(flag_start_y_delta_ == states_flag::enable) {
					return 0;	// ok!
				}

				break;
			}
			case start_types::to_y: {
				// just verify if k2 is on to avoid short circuit with k3
				if((state_k2() == states_switch::on) || (state_k2_pin() == states_switch::on)) {
					ESP_LOGI(TAG_PUMP, "from delta to Y. K2: OFF");						
					drive_k_(2, 0);
					
					int i = 0;
					while((state_k2() == states_switch::on) || (state_k2_pin() == states_switch::on)) {
						i++;
						delay_us(1);
						update_switches_();
						if(i > time_switch_k_change*1000)
						{
							ESP_LOGI(TAG_PUMP, "maybe k2 is lock");
							drive_k_(1,0);
							drive_k_(2,0);
							drive_k_(3,0);
							ESP_LOGI(TAG_PUMP, "i: %d", i);
							return 1;
						}
					}
					ESP_LOGI(TAG_PUMP, "i: %d", i);
					delay_ms(time_switch_k_change);
				}

				// if k2 is off, start k3 and than k1.
				if((state_k2() == states_switch::off) && (state_k2_pin() == states_switch::off)) {
					ESP_LOGI(TAG_PUMP, "K1 and K3: ON");
					drive_k_(3,1);
					if((state_k1() == states_switch::off) && (state_k1_pin() == states_switch::off)) {
						ESP_LOGI(TAG_PUMP, "from zero to Y start");
						drive_k_(1, 1);
					}
					else {
						ESP_LOGI(TAG_PUMP, "from K1 on to Y start");
					}
					delay_us(10000);								// delay for debounce purpose
				} else {
					ESP_LOGI(TAG_PUMP, "from delta to Y fail on K2 change");
					return 1;
				}

				// 1 - wait for some seconds to speeding rotor up
 				// must follow this sequence to avoid short circuit (between k2 and k3) and system's shutdown.
 				//	2a - turn k3 off;
 				//  2b - verify if k3 switched off;
				//  2c - if k3 is properly off then, switch k2 on;

				if(flag_start_y_delta_ == states_flag::enable) {
					start_y_delta_state_ = start_types::to_delta;
					time_delta_to_y_switch_ = time_delta_to_y_switch_config;
					ESP_LOGI(TAG_PUMP, "to delta now");
				}
				break;
			}
			case start_types::y_delta_req: {
				flag_start_y_delta_ = states_flag::enable;
				start_y_delta_state_ = start_types::to_y;
				return 0;	// ok!
				break;
			}
			default:
				break;
		}

		update_state_();

		time_on_ = 0;									// clear timer on to start count
		time_to_shutdown = time_to_shutdown_config;		// set timer
		make_log(_mode, *epoch_time_);					// and make some log

		return 0;	// ok!
	}
	void stop(stop_types _reason) {
		// Make some log
		// if((pump1_.state() == states_motor::on_nominal_k1) || (pump1_.state() == states_motor::on_nominal_k2) || (pump1_.state() == states_motor::on_nominal_delta) || (pump1_.state() == states_motor::on_speeding_up)) {
			// make_log(stop_types::command_line_user, pump1_.time_on());
		make_log(_reason, time_on());
		// }

		// ESP_LOGI(TAG_PUMP, "stop motor called with _reason: %u", static_cast<uint8_t>(_reason));

		// prepare for debug purposes
		int k = 0, k1 = 0, k2 = 0, k3 = 0;
		int k1_en = 0, k2_en = 0, k3_en = 0;
		if(state_k1() == states_switch::on) {
			k1_en = 1;
		}
		if(state_k2() == states_switch::on) {
			k2_en = 1;
		}
		if(state_k3() == states_switch::on) {
			k3_en = 1;
		}

		// Turn off all switches;
		drive_k_(1, 0);
		drive_k_(2, 0);
		drive_k_(3, 0);

		while((state_k1() == states_switch::on) || (state_k2() == states_switch::on) || (state_k3() == states_switch::on)) {
			if(state_k1() != states_switch::off) {
				k1++;
			}

			if(state_k2() != states_switch::off) {
				k2++;
			}

			if(state_k3() != states_switch::off) {
				k3++;
			}

			update_switches_();
			delay_us(1);
			k++;

			if(k > time_switch_k_change*1000) {
				break;
			}
		}

		// refresh times release to switch off
		if(k1_en) {
			time_k1_off_ = k1;
		}
		if(k2_en) {
			time_k2_off_ = k2;
		}
		if(k3_en) {
			time_k3_off_ = k3;
		}

		// reset some variables and timers
		// state_stop_reason = _reason;
		time_wait_power_on = time_wait_power_on_config;
		flag_check_wait_power_on = states_flag::enable;
		flag_start_y_delta_ = states_flag::disable;
		time_off_ = 0;
		time_last_stopped_ = *epoch_time_;
	}
	uint32_t time_on(void) {
		return time_on_;
	}
	uint32_t time_off(void) {
		return time_off_;
	}
	uint32_t time_last_stopped(void) {
		return time_last_stopped_;
	}

	void make_log(start_types start_type, uint32_t time_now) {

		for(int i=(log_n-1); i>0; i--) {
			// Start - shift data to right at the end;
			log_motors[i].start_mode = log_motors[i-1].start_mode;
			log_motors[i].time_start = log_motors[i-1].time_start;
			
			// STOP - shift data to right at the end;
			log_motors[i].stop_reason = log_motors[i-1].stop_reason;
			log_motors[i].time_elapsed_on = log_motors[i-1].time_elapsed_on;
		}

		log_motors[0].start_mode = start_type;
		log_motors[0].time_start = time_now;

		log_motors[0].stop_reason = stop_types::other;
		log_motors[0].time_elapsed_on = 0;
	}
	void make_log(stop_types stop_type, uint32_t time_elapsed_on) {
		
		log_motors[0].stop_reason = stop_type;
		log_motors[0].time_elapsed_on = time_elapsed_on;
	}

	// log history of turn on/off mode and timers;
	static const int log_n = 10;					// history log size
	struct {
		start_types start_mode;						// motor start mode;
		uint32_t time_start;						// started time. Epoch format [s];
		uint32_t time_elapsed_on;					// time on [s];
		stop_types stop_reason;						// reason to stop;
	}log_motors[log_n]={};

private:

	// Output TRIACS hardware drive to contactors
	GPIO_Basic ac_load_[3];
	const std::size_t ac_load_count_ = sizeof(ac_load_) / sizeof(ac_load_[0]);
//	GPIO_Basic drive_kn_[3]={GPIO_Basic{AC_LOAD1},GPIO_Basic{AC_LOAD2},GPIO_Basic{AC_LOAD3}};

	// input of contators
	GPIO_Basic gpio_generic_[4];
	const std::size_t gpio_generic_count_ = sizeof(gpio_generic_) / sizeof(gpio_generic_[0]);

	// States of pins and devs stores into variables
	states_motor state_motor_ = states_motor::off_idle;
	states_switch state_k1_ = states_switch::off;		// kontactor state
	states_switch state_k2_ = states_switch::off;
	states_switch state_k3_ = states_switch::off;
	states_switch state_k1_pin_ = states_switch::off;	// pin output state
	states_switch state_k2_pin_ = states_switch::off;
	states_switch state_k3_pin_ = states_switch::off;
	states_switch state_k1_dev_ = states_switch::off;
	states_switch state_k2_dev_ = states_switch::off;
	states_switch state_k3_dev_ = states_switch::off;
	states_switch state_Rth_ = states_switch::off;		// thermal relay output



	// motor start variables used on y-delta start process
	start_types start_y_delta_state_ = start_types::y_delta_req;

	// help flag to start y-delta process (FSM) 
	states_flag flag_start_y_delta_ = states_flag::disable;

	// Motor times
	uint32_t time_on_ = 0;
	uint32_t time_off_ = 0;

	// System RTC
	uint32_t *epoch_time_;								// epoch time linked with system;
	uint32_t time_last_stopped_ = 0;					// time of last stopped
	
	void update_switches_(void)
	{
		// we save all states to debug on $03; command.
		state_k1_pin_ = read_k_pin_(1);
		state_k2_pin_ = read_k_pin_(2);
		state_k3_pin_ = read_k_pin_(3);

		state_k1_dev_ = read_k_(1);
		state_k2_dev_ = read_k_(2);
		state_k3_dev_ = read_k_(3);

		state_Rth_ = read_Rth_();

		if(flag_check_output_pin_only == states_flag::enable) {
			state_k1_ = state_k1_pin_;
			state_k2_ = state_k2_pin_;
			state_k3_ = state_k3_pin_;
		}
		else {
			state_k1_ = state_k1_dev_;
			state_k2_ = state_k2_dev_;
			state_k3_ = state_k3_dev_;
		}
	}
	void update_state_(void) {
		update_switches_();

		if((state_Rth_ == states_switch::on) && (state_motor_ != states_motor::off_thermal_activated))
		{
			ESP_LOGI(TAG_PUMP, "off_thermal_activated");
			state_motor_ = states_motor::off_thermal_activated;
		}
		else
		{
			if ((state_k1_ == states_switch::off) && (state_k2_ == states_switch::off) && (state_k3_ == states_switch::off))
			{
				// turn pin down if contactor is not on when pin is high
				if((state_k1_pin() == states_switch::on) || (state_k2_pin() == states_switch::on))
				{
					stop(stop_types::contactor_not_on);
				}

				if(state_motor_ != states_motor::off_idle)
				{
					state_motor_ = states_motor::off_idle;
					ESP_LOGI(TAG_PUMP, "off_idle");
				}
			}
			else if((state_k1_ == states_switch::on) && (state_k2_ == states_switch::off) && (state_k3_ == states_switch::off))
			{
				ESP_LOGI(TAG_PUMP, "on_nominal_k1");
				state_motor_ = states_motor::on_nominal_k1;
			}
			else if((state_k1_ == states_switch::off) && (state_k2_ == states_switch::on) && (state_k3_ == states_switch::off))
			{
				ESP_LOGI(TAG_PUMP, "on_nominal_k2");
				state_motor_ = states_motor::on_nominal_k2;
			}
			else if((state_k1_ == states_switch::on) && (state_k2_ == states_switch::off) && (state_k3_ == states_switch::on))
			{	
				state_motor_ = states_motor::on_speeding_up;
				ESP_LOGI(TAG_PUMP, "on_speeding_up");
			}
			else if((state_k1_ == states_switch::on) && (state_k2_ == states_switch::on) &&	(state_k3_ == states_switch::off))
			{
				ESP_LOGI(TAG_PUMP, "on_nominal_delta");
				state_motor_ = states_motor::on_nominal_delta;
			}
			else if((state_k1_ == states_switch::off) && ((state_k2_ == states_switch::off) || (state_k3_ == states_switch::on)))
			{
				ESP_LOGI(TAG_PUMP, "off_k3_short_circuit");
				state_motor_ = states_motor::off_k3_short_circuit;
			}
			else
				state_motor_ = states_motor::undefined;
		}
	}
	void update_time_(void)
	{
		if((state_motor_ == states_motor::on_nominal_k1) || (state_motor_ == states_motor::on_nominal_k2) || (state_motor_ == states_motor::on_nominal_delta) || (state_motor_ == states_motor::on_speeding_up))
		{
			time_on_++;

			if(time_to_shutdown)
				time_to_shutdown--;
		}
		else {
			time_off_++;
		}

		if(flag_check_wait_power_on == states_flag::enable)
		{
			if(!time_wait_power_on)
			{
				flag_check_wait_power_on = states_flag::disable;
			}
			else
			{
				time_wait_power_on--;
			}
		}
	}
	void check_start_req_(void) {
		// aready passed over start() with delta_to_y_req to enable this following flag
		if(flag_start_y_delta_ == states_flag::enable) {
			if(start_y_delta_state_ == start_types::to_y) {
				ESP_LOGI(TAG_PUMP, "start1 type: %d", static_cast<int>(start_y_delta_state_));
				if(start(start_y_delta_state_))
					ESP_LOGI(TAG_PUMP, "start request %d fail", static_cast<int>(start_y_delta_state_));
			}

			if(!time_delta_to_y_switch_) {
				if(start(start_y_delta_state_)) {
					ESP_LOGI(TAG_PUMP, "start request %d fail", static_cast<int>(start_y_delta_state_));
				}
				flag_start_y_delta_ = states_flag::disable;
				ESP_LOGI(TAG_PUMP, "start2 type: %d", static_cast<int>(start_y_delta_state_));
			} else {
				time_delta_to_y_switch_--;
			}
			ESP_LOGI(TAG_PUMP,"time_delta_to_y_switch_: %d", time_delta_to_y_switch_);
		}
	}
	void drive_k_(int k_number, int level)
	{
		ac_load_[k_number-1].write(level);
	}
	void drive_led_(states_switch state)
	{

	}
	states_switch read_k_(int k_number)
	{
		if(!gpio_generic_[k_number-1].read())
			return states_switch::on;
		else
			return states_switch::off;
	}
	states_switch read_Rth_(void)
	{
		if(!gpio_generic_[3].read())
			return states_switch::on;
		else
			return states_switch::off;
	}
	states_switch read_k_pin_(int k_number)
	{
			if(ac_load_[k_number-1].read())
				return states_switch::on;
			else
				return states_switch::off;
	}
};

#endif /* PUMP_HPP__ */










	// void check_Rth_() {
		// if(flag_check_Rth == states_flag::enable)
		// {
		// 	if(state_Rth() == states_switch::on) {
		// 		if((state_motor_ != states_motor::off_thermal_activated)) {
		// 			stop(stop_types::thermal_relay);
		// 		}
		// 	}
		// }
	// }
	// void check_timer_()
	// {

	// }