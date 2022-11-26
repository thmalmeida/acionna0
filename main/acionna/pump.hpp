#ifndef PUMP_HPP
#define PUMP_HPP

#include <adc.hpp>
#include <gpio.hpp>
#include <pinout.hpp>

#include "helper.hpp"
#include "hardware_defs.h"

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

enum class states_motor {
	off_idle = 0,
	on_nominal_k1,
	on_nominal_k2,
	on_nominal_delta,
	on_speeding_up,
	off_k3_short_circuit,
	off_thermal_activated,
	undefined
};

enum class start_types {
	direct_k1 = 1,		// Partida direta com um contator;
	direct_k2,			// ativação do contator k2;
	direct_k3,			// ativação do contator k3;
	to_delta,			// Partida direta com dois contatores;
	to_y,				// deslocamento para configuração Y;
	y_delta_req			//  requisição de Partida estrela/triangulo (three phase)
};
enum class states_stop {
	/*
	 * Reasons to halt the motor.
	 * 0x01 - command line request
	 * 0x02 - thermal relay occurs;
	 * 0x03 - high pressure
	 * 0x04 - low level
	 * 0x05 - low pressure for long time (broken pipe?)
	 * 0x06 - time out
	 * 0x07 - red time
	 * */
	command_line_user = 0,
	timeout,
	pressure_high,
	thermal_relay,
	contactor_not_on,
	level_low,
	pressure_low_long_time,
	red_time,
	system_lock
};

class Pump {
public:
	// Check flags
	states_flag flag_check_output_pin_only = states_flag::disable;
	states_flag flag_check_Rth = states_flag::disable;
	states_flag flag_check_timer = states_flag::enable;
	states_flag flag_check_wait_power_on = states_flag::disable;
	states_flag flag_check_k3_off = states_flag::disable;

	// Motor times
	unsigned int time_wait_power_on = 0;			// 
	unsigned int time_wait_power_on_config = 600;	// [s]
	unsigned int time_to_shutdown = 0;				// time variable to shutdown the motor [s].
	unsigned int time_to_shutdown_config = 30*60;	// [s]
	unsigned int time_switch_k_change = 400;		// [ms] delay on transition of k3 off to k2 on
	unsigned int time_delta_to_y_switch_ = 0;		// speeding up time from delta to Y start [s]
	unsigned int time_delta_to_y_switch_config = 5;	// speeding up time from delta to Y start [s]

	// unsigned int waitPowerOn_min_standBy = 0;	// when enabled (high) avoid load turn on;
	// unsigned int waitPowerOn_min = 0;
	// unsigned int waitPowerOn_sec = 0;

	// log system
	static const int nLog_ = 7;						// history log size
	states_stop stops_history[nLog_];
	// states_stop state_stop_reason = states_stop::command_line;
	// uint8_t hourLog_ON[nLog], minuteLog_ON[nLog];
	// uint8_t hourLog_OFF[nLog], minuteLog_OFF[nLog];
	// uint8_t dayLog_ON[nLog], monthLog_ON[nLog];
	// uint8_t dayLog_OFF[nLog], monthLog_OFF[nLog];

//	Pump() : drive_k1_{AC_LOAD1}, drive_k2_{AC_LOAD2}, drive_k3_{AC_LOAD3}{}
	Pump() : ac_load_{{AC_LOAD1},{AC_LOAD2},{AC_LOAD3}},
				gpio_generic_{{GPIO_GENERIC1},{GPIO_GENERIC2},{GPIO_GENERIC3},{GPIO_GENERIC4}}
	{
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
	void update()
	{
		update_state_();
		update_time_();

		check_Rth_();

		check_start_req_();
	}
	states_switch state_k1() const // noexcept // const -> não modifica nada na classe
	{										// noexcept para dizer que não gera exceção
		return state_k1_;
	}
	states_switch state_k2() const // noexcept // const -> não modifica nada na classe
	{										// noexcept para dizer que não gera exceção
		return state_k2_;
	}
	states_switch state_k3() const // noexcept // const -> não modifica nada na classe
	{										// noexcept para dizer que não gera exceção
		return  state_k3_;
	}
	states_switch state_k1_pin() const
	{
		return state_k1_pin_;	
	}
	states_switch state_k2_pin() const
	{
		return state_k2_pin_;	
	}
	states_switch state_k3_pin() const
	{
		return state_k3_pin_;
	}
	states_switch state_Rth() const // noexcept // const -> não modifica nada na classe
	{										// noexcept para dizer que não gera exceção
		return state_Rth_;
	}
	states_motor state() const
	{
		return state_motor_;
	}
	int start(start_types _start_type)
	{
		switch (_start_type)
		{
			case start_types::direct_k1: {
				drive_k_(1, 1);

				// check K change with time
				int i = 0;
				while(state_k1() != states_switch::on)
				{
					i++;
					delay_us(1);
					update_switches_();

					if(i == time_switch_k_change*1000)
					{
						ESP_LOGI(TAG_PUMP, "error on k1 change");
						stop(states_stop::contactor_not_on);
						return 1;
					}
				}
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
					delay_us(1);
					update_switches_();

					if(i == time_switch_k_change*1000)
					{
						ESP_LOGI(TAG_PUMP, "error on k2 change");
						stop(states_stop::contactor_not_on);
						return 1;
					}
				}
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

					if(i == time_switch_k_change*1000)
					{
						ESP_LOGI(TAG_PUMP, "error on k3 change");
						stop(states_stop::contactor_not_on);
						return 1;
					}
				}
				ESP_LOGI(TAG_PUMP, "k3 on");
				break;
			}
			case start_types::to_delta: {
				// just verify if k3 is on to avoid short circuit with k2
				if((state_k3() == states_switch::on) || (state_k3_pin() == states_switch::on))
				{
					ESP_LOGI(TAG_PUMP, "from Y to delta. K3: OFF");						
					drive_k_(3, 0);
					
					int i = 0;
					while((state_k3() == states_switch::on) || (state_k3_pin() == states_switch::on))
					{
						i++;
						delay_us(1);
						update_switches_();
						if(i == time_switch_k_change*1000)
						{
							ESP_LOGI(TAG_PUMP, "maybe k3 is lock");
							drive_k_(1,0);
							drive_k_(2,0);
							drive_k_(3,0);
							ESP_LOGI(TAG_PUMP, "i: %d", i);
							return 1;
						}
					}
					ESP_LOGI(TAG_PUMP, "i: %d", i);
					_delay_ms(time_switch_k_change);
				}

				if((state_k3() == states_switch::off) && (state_k3_pin() == states_switch::off)) {
					ESP_LOGI(TAG_PUMP, "K1 and K2: ON");
					drive_k_(2,1);
					if((state_k1() == states_switch::off) && (state_k1_pin() == states_switch::off)) {
						ESP_LOGI(TAG_PUMP, "from zero to delta start");
						drive_k_(1, 1);
					}
					else {
						ESP_LOGI(TAG_PUMP, "from K1 on to delta start");
					}
				} else {
					ESP_LOGI(TAG_PUMP, "from Y to delta fail on K3 change");
					return 1;
				}

				// if((state_k3() == states_switch::off) && (state_k3_pin() == states_switch::off)) {
				// 	drive_k_(2, 1);
				// 	update_state_();
				// 	if((state_k1() == states_switch::off) && (state_k1_pin() == states_switch::off))
				// 	{
				// 		drive_k_(1, 1);
				// 		ESP_LOGI(TAG_PUMP, "from zero to delta start. K1 and K2: ON");
				// 	}
				// 	else
				// 		ESP_LOGI(TAG_PUMP, "from delta????");
				// } else {
				// 	drive_k_(3,0);
				// 	ESP_LOGI(TAG_PUMP, "from Y to delta start. K3: OFF");
				// 	delay_ms(time_switch_k_change);
				// 	update_state_();
				// 	// state_k3() substitute for
				// 	if((state_k3() == states_switch::off) && (state_k3_pin() == states_switch::off)) {
				// 		ESP_LOGI(TAG_PUMP, "K2: ON");
				// 		drive_k_(2,1);
				// 		drive_k_(1,1);
				// 	} else {
				// 		drive_k_(1,0);
				// 		drive_k_(2,0);
				// 		drive_k_(3,0);
				// 		ESP_LOGI(TAG_PUMP, "from Y to delta start fail on K3 change");
				// 		return 1;
				// 	}						
				// }
				break;
			}
			case start_types::to_y: {
				// just verify if k2 is on to avoid short circuit with k3
				if((state_k2() == states_switch::on) || (state_k2_pin() == states_switch::on))
				{
					ESP_LOGI(TAG_PUMP, "from delta to Y. K2: OFF");						
					drive_k_(2, 0);
					int i = 0;

					while((state_k2() == states_switch::on) || (state_k2_pin() == states_switch::on))
					{
						i++;
						delay_us(1);
						update_switches_();
						if(i == time_switch_k_change*1000)
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
					_delay_ms(time_switch_k_change);
				}

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
				break;
			}
			default:
				break;
		}

		time_on_ = 0;
		time_to_shutdown = time_to_shutdown_config;

		return 0;	// ok!
	}
	void stop(states_stop _reason)
	{
		ESP_LOGI(TAG_PUMP, "stop motor called with reason: %u", static_cast<uint8_t>(_reason));
		drive_k_(1, 0);
		drive_k_(2, 0);
		drive_k_(3, 0);

		// state_stop_reason = _reason;
		time_wait_power_on = time_wait_power_on_config;
		flag_check_wait_power_on = states_flag::enable;
		flag_start_y_delta_ = states_flag::disable;
		time_off_ = 0;

		// shift data to right at the end;
		for(int i=(nLog_-1);i>0;i--)
		{
			// minuteLog_OFF[i] = minuteLog_OFF[i-1];
			// hourLog_OFF[i] = hourLog_OFF[i-1];
			// dayLog_OFF[i] = dayLog_OFF[i-1];
			// monthLog_OFF[i] = monthLog_OFF[i-1];

			stops_history[i] = stops_history[i-1];
		}
		stops_history[0] = _reason;

		// reasonV[1] = reasonV[0];
		// reasonV[0] = reason;

		// hourLog_OFF[0] = tm.Hour;
		// minuteLog_OFF[0] = tm.Minute;

		// dayLog_OFF[0] = tm.Day;
		// monthLog_OFF[0] = tm.Month;

		// timeOff_min = 0;
		// timeOff_sec = 0;

		// flag_waitPowerOn = 1;
		// waitPowerOn_min = waitPowerOn_min_standBy;




		// time_to_shutdown = time_to_shutdown_config;
		//_delay_ms(750);
//		check_pump_state();
		// store turn off reason in some variable?
	}
	uint32_t time_on()
	{
		return time_on_;
	}
	uint32_t time_off()
	{
		return time_off_;
	}

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
	unsigned int time_on_ = 0;
	unsigned int time_off_ = 0;
	
	void update_switches_()
	{
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
	void update_state_()
	{
		update_switches_();

		if((state_Rth_ == states_switch::on) && (flag_check_Rth == states_flag::enable))
		{
			ESP_LOGI(TAG_PUMP, "pump: off_thermal_activated");
			state_motor_ = states_motor::off_thermal_activated;
		}
		else
		{
			if ((state_k1_ == states_switch::off) && (state_k2_ == states_switch::off) && (state_k3_ == states_switch::off))
			{
				// turn pin down if contactor is not on when pin is high
				if((state_k1_pin() == states_switch::on) || (state_k2_pin() == states_switch::on))
				{
					stop(states_stop::contactor_not_on);
				}

				if(state_motor_ != states_motor::off_idle)
				{
					state_motor_ = states_motor::off_idle;
					ESP_LOGI(TAG_PUMP, "pump: off_idle");
				}
			}
			else if((state_k1_ == states_switch::on) && (state_k2_ == states_switch::off) && (state_k3_ == states_switch::on))
			{	
				state_motor_ = states_motor::on_speeding_up;
				ESP_LOGI(TAG_PUMP, "pump: on_speeding_up");
			}
			else if((state_k1_ == states_switch::on) && (state_k2_ == states_switch::off) && (state_k3_ == states_switch::off))
			{
				ESP_LOGI(TAG_PUMP, "pump: on_nominal_k1");
				state_motor_ = states_motor::on_nominal_k1;
			}
			else if((state_k1_ == states_switch::off) && (state_k2_ == states_switch::on) && (state_k3_ == states_switch::off))
			{
				ESP_LOGI(TAG_PUMP, "pump: on_nominal_k2");
				state_motor_ = states_motor::on_nominal_k2;
			}
			else if((state_k1_ == states_switch::on) && (state_k2_ == states_switch::on) &&	(state_k3_ == states_switch::off))
			{
				ESP_LOGI(TAG_PUMP, "pump: on_nominal_delta");
				state_motor_ = states_motor::on_nominal_delta;
			}
			else if((state_k1_ == states_switch::off) && ((state_k2_ == states_switch::off) || (state_k3_ == states_switch::on)))
			{
				ESP_LOGI(TAG_PUMP, "pump: off_k3_short_circuit");
				state_motor_ = states_motor::off_k3_short_circuit;
			}
			else
				state_motor_ = states_motor::undefined;
		}
	}
	void update_time_()
	{
		if((state_motor_ == states_motor::on_nominal_k1) || (state_motor_ == states_motor::on_nominal_k2) || (state_motor_ == states_motor::on_nominal_delta) || (state_motor_ == states_motor::on_speeding_up))
		{
			time_on_++;

			if(flag_check_timer == states_flag::enable)
			{
				if(!time_to_shutdown)
				{
					stop(states_stop::timeout);		// stop motor by timeout;
					ESP_LOGI(TAG_PUMP, "timeout off");
				}
				else
				{
					time_to_shutdown--;
				}
			}
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
	void check_Rth_()
	{
		if(flag_check_Rth == states_flag::enable)
		{
			if(state_Rth() == states_switch::on)
			{
				if((state_motor_ != states_motor::off_thermal_activated))
				{
					stop(states_stop::thermal_relay);
				}
			}
		}
	}
	void check_timer_()
	{

	}
	void check_start_req_()
	{
		if(flag_start_y_delta_ == states_flag::enable)
		{
			if(start_y_delta_state_ == start_types::to_y) {
				ESP_LOGI(TAG_PUMP, "start1 type: %d", static_cast<int>(start_y_delta_state_));
				if(start(start_y_delta_state_))
					ESP_LOGI(TAG_PUMP, "start request %d fail", static_cast<int>(start_y_delta_state_));
			}

			if(!time_delta_to_y_switch_) {
				flag_start_y_delta_ = states_flag::disable;
				ESP_LOGI(TAG_PUMP, "start2 type: %d", static_cast<int>(start_y_delta_state_));
				if(start(start_y_delta_state_))
					ESP_LOGI(TAG_PUMP, "start request %d fail", static_cast<int>(start_y_delta_state_));
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
	states_switch read_Rth_()
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

#endif /* PUMP_H__ */


	// states_switch read_k2_()
	// {
	// 	if(gpio_generic_[1].read())
	// 		return states_switch::on;
	// 	else
	// 		return states_switch::off;
	// }
	// states_switch read_k3_()
	// {
	// 	if(gpio_generic_[2].read())
	// 		return states_switch::on;
	// 	else
	// 		return states_switch::off;
	// }