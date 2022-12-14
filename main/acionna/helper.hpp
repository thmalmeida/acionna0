#ifndef HELPER_HPP__
#define HELPER_HPP__

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
	automatic_switch
};
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
enum class states_mode {
	system_off = 0,					// never turn and don't let any load turn on;
	system_idle,					// idle means that never turn any load automaticaly. But can work manual with all updates and checks
	water_pump_control_night,
	irrigation_pump_valves,
	valve_control
};
#endif