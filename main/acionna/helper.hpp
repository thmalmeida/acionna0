#ifndef __HELPER_HPP__
#define __HELPER_HPP__

#include <stdio.h>
#include <stdint.h>
#include <math.h>

enum class states_flag {
	disable = 0,
	enable
};
enum class states_period {
	greenTime,
	redTime
};
enum class states_switch {
	off = 0,
	on
};
enum class states_valves {
	system_off = 0,
	automatic_switch,
	auto_test
};
enum class states_motor {
	off_idle = 0,
	on_nominal_k1,
	on_nominal_k2,
	on_speeding_up,
	on_nominal_delta,
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
enum class stop_types {
	/*
	 * Reasons to halt the motor.
	 * 0x00 - command line request
	 * 0x01 - time out
	 * 0x02 - high pressure
	 * 0x03 - low pressure(broken pipe?)
	 * 0x04 - thermal relay occurs;
	 * 0x05 - contactor not on
	 * 0x06 - low level
	 * 0x07 - red time
	 * 0x08 - system lock??
	 * */
	command_line_user = 0,
	timeout,
	pressure_high,
	pressure_low,
	thermal_relay,
	contactor_not_on,
	level_low,
	red_time,
	system_lock,
	other
};
enum class states_mode {
	system_off = 0,					// never turn and don't let any load turn on;
	system_ready,					// ready means that is waiting for time match or manual command line with all updates and checks;
	irrigation_pump_valves,
	pump_optimized_control
};

#endif