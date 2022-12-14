#include "acionna.hpp"

static const char *TAG_ACIONNA = "Acionna0";

static I2C_Master i2c(I2C_NUM_0, I2C_SCL, I2C_SDA, I2C_FAST_SPEED_HZ, 0);
static DS3231 rtc{&i2c};
static Agro::RTC_Time device_clock;
static DateTime dt;

int timeout_sensors;
int timeout_sensors_cfg = 600;

// GPIO_Basic led0(LED_0);
static pwm_ledc led_wifi_indicator(2, 1, 0, 1);


void Acionna::init() {
	ESP_LOGI(TAG_ACIONNA, "initialization");

	//
	// i2c.init();

	// Clock time init
	dt.setDate(2022, 12, 20);
	dt.setTime(0, 0, 0, ND);
	device_clock.set_time(dt.getUnixTime());
	time_day_sec_ = dt.getHour()*3600 + dt.getMinute()*60 + dt.getSecond();

	// Network connection init
	#if CONFIG_DEVICE_CLOCK_DS3231_SUPPORT
		device_clock.init(rtc);
	#endif /* CONFIG_DEVICE_CLOCK_DS3231_SUPPORT */

	// Sensors init
	// temp_sensor.begin();
	// gpio_iomux_out(GPIO_NUM_13, 2, false);
	// dht0.read2();
	// std::uint8_t temp_sensor_count = temp_sensor.getDeviceCount();

	// ESP_LOGI(TAG_ACIONNA, "Temp sensors count: %u", temp_sensor_count);


//		I2C_Master i2c(I2C_NUM_0, I2C_SCL, I2C_SDA, I2C_FAST_SPEED_HZ);
//
//		#if CONFIG_DEVICE_CLOCK_DS3231_SUPPORT
//		#include "agro/types.hpp"
//		#include "ds3231.hpp"
//
//		DS3231 rtc{&i2c};
//		extern Agro::RTC_Time device_clock;

//		#endif /* CONFIG_DEVICE_CLOCK_DS3231_SUPPORT */

	// restore parameters;
	// update clock;
}
uint32_t Acionna::get_uptime() {
	return uptime_;
}
void Acionna::operation_motorPeriodDecision() {
}
void Acionna::operation_system_off() {
	if((pump1_.state() == states_motor::on_nominal_k1) || (pump1_.state() == states_motor::on_nominal_delta) || (pump1_.state() == states_motor::on_speeding_up))
	{
		pump1_.stop(states_stop::system_lock);
	}
}
void Acionna::operation_valve_control() {

}
void Acionna::operation_pump_control() {
	/*
	* Motor stop conditions
	*
	* Reasons to halt the motor.
	* 0x01 - command line request
	* 0x02 - thermal relay occurs;
	* 0x03 - high pressure
	* 0x04 - low level
	* 0x05 - low pressure for long time (broken pipe?)
	* 0x06 - time out
	* 0x07 - red time
	* */

	// time matches occurred into check_time_match(), start motor
	if(flag_check_time_match_ == states_flag::enable)
	{
		int index = 0;
		for(int i=0; i<time_match_n; i++)
		{
			if(time_match_list[i] == time_day_sec_)
			{
				flag_time_match_ = states_flag::enable;
				index = i;
			}
			// ESP_LOGI(TAG_ACIONNA, "TM FOR CHECK! tml:%d tds:%d", time_match_list[i], time_day_sec_);
		}

		if(flag_time_match_ == states_flag::enable)
		{
			flag_time_match_ = states_flag::disable;

			// ESP_LOGI(TAG_ACIONNA, "FLAG TIME MATCH!");

			if(pump1_.state() == states_motor::off_idle)
			{
				pump1_.start(auto_start_mode[index]);	// motor_start();
				if(time_to_shutdown[index])
				{
					pump1_.time_to_shutdown = time_to_shutdown[index];
				}
			}
		}
	}

	// check high pressure
	if(flag_check_pressure_high_ == states_flag::enable)
	{
		if(pipe1_.pressure_mca() > pipe1_.pressure_max)
			if((pump1_.state() == states_motor::on_nominal_k1) || (pump1_.state() == states_motor::on_nominal_delta) || (pump1_.state() == states_motor::on_speeding_up))
				pump1_.stop(states_stop::pressure_high);
	}

	// check low pressure
	if(flag_check_pressure_low_ == states_flag::enable)
	{
		/*
		* needs better implementation. Check after some time since motor starts;
		*/
//			if(pipe1_.pressure_mca < pipe1_.pressure_min)
//			{
//
//			}
		// Irrigation valves

		// Well water pump fill reservoir
		// if(pipe1_.broke_pipe_detect())
		// {
		// 	if((pump1_.state() == states_motor::on_nominal_k1) || (pupm1_.state() == states_motor::on_nominal_delta))
		// 	{

		// 	}
		// }
		// if(k2 only)

	}

	// check thermal relay
	if(flag_check_thermal_relay_ == states_flag::enable)
	{
		if(pump1_.state_Rth() == states_switch::on)
			if(pump1_.state() != states_motor::off_thermal_activated)
				pump1_.stop(states_stop::thermal_relay);
	}

	// check low level
	#ifdef CONFIG_WELL_SUPPORT
	if(flag_check_low_level_ == states_flag::enable)
	{
		if(well1_.state_L1() == states_level::low)
			if((pump1_.state() == states_motor::on_nominal_k1) || (pump1_.state() == states_motor::on_nominal_delta) || (pump1_.state() == states_motor::on_speeding_up))
				pump1_.stop(states_stop::level_low);;
	}
	#endif
}
void Acionna::operation_pump_valves_irrigation() {

}
void Acionna::update_RTC() {
	dt.setUnixTime(device_clock.get_time());
}
void Acionna::update_objects() {
	pump1_.update();
	pipe1_.update();

	// #ifdef CONFIG_WELL_SUPPORT
	// well1_.update();
	// #endif

	// #ifdef CONFIG_VALVES_SUPPORT
	valves1_.update();
	// #endif
}
void Acionna::update_sensors()
{
	if(!timeout_sensors)
	{
		timeout_sensors = timeout_sensors_cfg;

		// signal_request_sensors = 1;

		// temp_sensor.requestTemperatures();
		// if(dht0.read2())
		// {
		// 	ESP_LOGI(TAG_ACIONNA, "Time: %.2d:%.2d:%.2d, Tout:%.2f, Tin:%.1f, Humidity: %.1f%%", dt.getHour(), dt.getMinute(), dt.getSecond(), temp_sensor.getTempCByIndex(0), (float)dht0.getTempCelsius(0)*0.1, (float)dht0.getHumidity(0)*0.1);
		// }
		// else
		// {
		// 	// for(int i=0; i<40; i++)
		// 	// {
		// 	// 	ESP_LOGI(TAG_DHT, "Cicle[%d] low: %d high: %d\n", i+1, dht0.cycles[i].expectLow, dht0.cycles[i].expectHigh);
		// 	// }
		// 	ESP_LOGI(TAG_DHT, "error reading");
		// }
		// vTaskDelay(10000 / portTICK_PERIOD_MS);
	}
	else
		timeout_sensors--;
}
void Acionna::update_stored_data() {
}
void Acionna::update_uptime()
{
	uptime_++;
	time_day_sec_++;

	// compare time_day_sec with whole day seconds 24*60*60 = 86400
	if(time_day_sec_ == 86400)
	{
		// time_day_sec_ ^= time_day_sec_;
		time_day_sec_ = 0;
	}
	// or
	// uptime = esp_timer_get_time();

	// ESP_LOGI(TAG_ACIONNA, "uptime: %d", device_clock.internal_time());
	// ESP_LOGI(TAG_ACIONNA, "uptime: %ld", static_cast<long int>((esp_timer_get_time() / 1000000)));
}
void Acionna::update_all() {
	update_uptime();
	update_RTC();
	update_objects();
	update_stored_data();

	// update_sensors();		// test sensors
}
std::string Acionna::handle_message(uint8_t* command_str) {
	/*
	$0X;				Verificar detalhes - Detalhes simples (tempo).
		$00;			- Detalhes simples (tempo).
		$00:[0|1];		- Json send back 1, stop 0;
		$01;			- Verifica hist??rico de quando ligou e desligou;
		$02;			- Mostra tempo que falta para ligar;
			$02:w;		- Zera o tempo;
			$02:w:30;	- Ajusta novo tempo para 30 min;
			$02:s:090;	- Tempo m??ximo ligado para 90 min. Para n??o utilizar, colocar zero;
			$02:f:045;	- For??a o contador de tempo ligado para 45 min;
		$03;			- Verifica detalhes do motor, press??o e sensor termico;
			$03:s:72;	- Set max pressure ref for pipe [m.c.a.];
			$03:s;		- show max pressure configured;
			$03:v:32;	- Set pressure for valve load turn on and fill reservoir;
			$03:p:150;	- Set sensor max pressure ref to change the scale [psi];
			$03:b:85;	- Set to 85% the pressure min bellow the current pressure to avoid pipe broken;
			$03:m:3;	- Set 3 seconds while K1 and K3 are ON into delta tri start;
			$03:m;		- Just show the speeding up time;
			$03:t:400;	- Set 400 milliseconds to wait K3 go off before start K2;
			$03:t;		- Just show the time switch;
		$04;			- Verifica detalhes do n??vel de ??gua no po??o e refer??ncia 10 bits;
			$04:0;		- Interrompe o envio continuo das vari??veis de press??o e n??vel;
			$04:1;		- Envia continuamente valores de press??o e n??vel;
			$04:s:0900;	- Adiciona nova refer??ncia para os sensores de n??vel. Valor de 0 a 1023;
		$05;			- Mostra os hor??rios que liga no modo $62;
		$06;			- Tempo ligado e tempo desligado;
		$07:x;			- ADC reference change.
			$07:0;		- AREF
			$07:1;		- AVCC with external cap at AREF pin
			$07:2;		- Internal 1.1 Voltage reference.

	$10:h:HHMMSS;		- Ajustes do calend??rio;
		$10:h:HHMMSS;	- Ajusta o hor??rio do sistema;
		$10:h:123040;	- E.g. ajusta a hora para 12:30:40
		$10:d:DDMMAAAA;	- Ajusta a data do sistema no formato dia/m??s/ano(4 d??gitos);
		$10:d:04091986;	- E.g. Altera a data para 04/09/1986;
		$10:c;			- Shows the LSI current prescaler value;
		$10:c:40123;		- Set new prescaler value;

	$2X;

		$20:DevName;			- Change bluetooth name;
		$20:n:
		$21:i:30;		- lo;		- Altera o nome do bluetooth para "Vassalo";
		$21:d:099		- pwm: change duty cicle [%];
		$21:f:0001		- pwm: change frequency [Hz];

	$3X;				- Acionamento do motor;
		$30;			- desliga todos contatores;
		$31;			- ligar contator K1;
		$32;			- ligar contator K2;
		$33;			- ligar contator K3;
		$34;			- direto para partida delta;
		$35;			- direto para partida Y;
		$36;			- liga motor com partida Y delta;
	
	(not implemented)
	$4:x:				- Is this applied fo stm32f10xxx series only;
		$4:r:07;		- Read address 0x07 * 2 of currently page;
		$4:w:07:03;		- Write variable 3 on address 7 of currently page;
		$4:f:64:03;		- fill page 64 with 3 value;
		$4:e:64;		- erase page 64;

	$50:n:X; ou $50:hX:HHMM;
		$50;				- mostra os hor??rios que ir?? ligar;
		$50:0|1;			- desabilita|habilita auto turn (habilita time match flag);
		$50:m:[1|3]		- tipo de partida autom??tica: 1- k1; 3- y-Delta;
		$50:n;			- mostra a quantidade de vezes que ir?? ligar;
		$50:n:9;			- Configura para acionar 9 vezes. Necess??rio configurar 9 hor??rios;
		$50:n:1;			- Configura o sistema para acionar uma ??nica vez;
		$50:h1:2130;		- Configura o primeiro hor??rio como 21:30 horas;
		$50:h8:0437;		- Configura o oitavo hor??rio como 04:37 horas;

	$6X;				- Modos de funcionamento;
		$60; 			- Sistema Desligado n??o permite ligar;
		$61;			- Sistema ocioso com permiss??o de acionamento manual (nunca ligar?? sozinho);
		$62;			- Liga de modo autom??tico nos determinados hor??rios estipulados;
		$63;			- Fun????o para v??lvula do reservat??rio;
		$64;			- Fun????o para motobomba do reservat??rio;

	$7X					- Fun????es que habilitam ou desabilitam verifica????es de:
		$70:rt;
		$70:rt:[0|1];			- rel?? t??rmico;
		$70:kt;
		$70:kt:[0|1];			- todos contatores
		$70:k1;
		$70:k1:[0|1];			- contator K1 (not implemented);
		$70:k2;
		$70:k2:[0|1];			- contator K2 (not implemented);
		$70:k3;
		$70:k3:[0|1];			- contator K3 (not implemented);
		$70:ph;
		$70:ph:1|0;			- desligamento por alta press??o;
		$70:pl;
		$70:pl:0-9;			- desligamento por press??o baixa em min caso seja diferente de 0;
		$70:pv;
		$70:pv:1|0;			- desligamento por press??o alta por v??lvula;

	$8					- Fun????es de programa????o da irriga????o;
		$81;			- start valves sequence;
		$80;			- stop valves sequence;
		$8:[0|1];		- 0 sentido direto; 1 - sentido inverso na troca dos setores;
		$8:01;			- mostra condi????es de configura????o da v??lvula 01;
		$8:01:[0|1];	- desaciona|aciona v??lvula 01;
		$8:01:i;		- insere setor na programa????o;
		$8:01:r;		- remove setor da programa????o;
		$8:01:t:120;	- configura o tempo de irriga????o [min];
		$8:01:p:68;		- configura press??o nominal do setor [m.c.a.];
		$8:01:t;		- mostra o tempo de irriga????o do setor;
		$8:01:p;		- mostra press??o nominal do setor

	$9X;				- System administration
		$90:[0-9];		- WiFi
			$90:0;		- WiFi AP info;
			$90:1;		- WiFi Scan;
			$90:2;		- Show mac address;
		$92;			- show firwmare version;
		$95:[0-9];		- firmware ota;
			$95:0;		- show ota partitions info;
			$95:1;		- show ota app info
			$95:2;		- print sha256;
			$94:3:		- mark invalid;
			$95:4;		- mark valid;
			$95:8:[0-1]	- change boot partition;
			$95:9;		- Start firmware update;
		$97;			- Show RAM usage;
		$98;			- Show reset reason;
		$99;			- Soft reset system;
*/
	int opcode0 = -1;
	int opcode1 = -1;
	int opcode_sub0 = -1;
	int statusCommand = -1;
	char _aux[3], _aux2[5];
	char buffer[400] = "not handled\n";

	// Getting the opcode0 (operation code)
	_aux[0] = '0';
	_aux[1] = command_str[1];		// '0' in uint8_t is 48. ASCII
	_aux[2] = '\0';
	opcode0 = atoi(_aux);

	// Getting the opcode1 (operation code)
	_aux[0] = '0';
	_aux[1] = command_str[2];		// '0' in uint8_t is 48. ASCII
	_aux[2] = '\0';
	opcode1 = atoi(_aux);

	switch (opcode0)
	{
		case 0: { // $0X; Check status
			switch (opcode1) 
			{
				case 0:	{ // $00; Check basic parameters{
					if(command_str[3]==';') {
						// int a = dt.getHour();
						// sprintf(buffer, "%d", dt.getHour());
						sprintf(buffer, "%.2d:%.2d:%.2d %.2d/%.2d/%.4d up:%.2d:%.2d:%.2d d:%d, s:%d m:%d tday:%d\n", dt.getHour(),
																						dt.getMinute(),
																						dt.getSecond(),
																						dt.getDay(),
																						dt.getMonth(),
																						dt.getYear(),
																						timesec_to_hour(get_uptime()),
																						timesec_to_min(get_uptime()),
																						timesec_to_sec(get_uptime()),
																						timesec_to_day(get_uptime()),
																						static_cast<int>(state_mode),
																						static_cast<int>(pump1_.state()),
																						time_day_sec_
																						);
					} else if((command_str[3] == ':') && (command_str[5] == ';')) {
						_aux[0] = '0';
						_aux[1] = command_str[4];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						opcode_sub0 = atoi(_aux);

						if(opcode_sub0 == 1) {
							flag_json_data_back = states_flag::enable;
						}
						else if(!opcode_sub0) {
							flag_json_data_back = states_flag::disable;
						}
					} // $00:[0|1]; 
					break;
				}
				// case 1: {
					// if(sInstr[2]==':' && sInstr[3]=='c')
					// {
					// 	if(sInstr[4] == ';')
					// 	{
					// 		flag_waitPowerOn = 0;
					// 		waitPowerOn_min = 0;
					// 		waitPowerOn_sec = 0;
					// 	}
					// 	else if(sInstr[4] ==':' && sInstr[7] == ';')
					// 	{
					// 		aux[0] = sInstr[5];
					// 		aux[1] = sInstr[6];
					// 		aux[2] = '\0';
					// 		waitPowerOn_min_standBy = (uint8_t) atoi(aux);
					// 	}
					// }
					// 
					// break;
				// }
				case 2: { // $02:c; time motor 
					if(command_str[3] == ';') {
						sprintf(buffer, "on:%.2d:%.2d:%.2d off:%.2d:%.2d:%.2d, t2sd:%.2d:%.2d:%.2d t2sd_cnf:%.2d:%.2d:%.2d won:%.2d:%.2d won_cfg:%.2d:%.2d\n",
																				timesec_to_hour(pump1_.time_on()),
																				timesec_to_min(pump1_.time_on()),
																				timesec_to_sec(pump1_.time_on()),
																				timesec_to_hour(pump1_.time_off()),
																				timesec_to_min(pump1_.time_off()),
																				timesec_to_sec(pump1_.time_off()),
																				timesec_to_hour(pump1_.time_to_shutdown),
																				timesec_to_min(pump1_.time_to_shutdown),
																				timesec_to_sec(pump1_.time_to_shutdown),
																				timesec_to_hour(pump1_.time_to_shutdown_config),
																				timesec_to_min(pump1_.time_to_shutdown_config),
																				timesec_to_sec(pump1_.time_to_shutdown_config),
																				timesec_to_min(pump1_.time_wait_power_on),
																				timesec_to_sec(pump1_.time_wait_power_on),																				
																				timesec_to_min(pump1_.time_wait_power_on_config),
																				timesec_to_sec(pump1_.time_wait_power_on_config)
																			);
					} 
					else if((command_str[3] == ':') && (command_str[4] == 'w') && (command_str[5] == ';')) {
						pump1_.time_wait_power_on = 0;
						sprintf(buffer, "clear time_wait_power_on: %d\n", pump1_.time_wait_power_on);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'w') && (command_str[5] == ':') && (command_str[9] == ';')) {
						_aux2[0] = '0';
						_aux2[1] = command_str[6];
						_aux2[2] = command_str[7];
						_aux2[3] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';
						pump1_.time_wait_power_on_config = atoi(_aux2)*60;
						sprintf(buffer, "set time_wait_power_on_config: %d min\n", pump1_.time_wait_power_on_config/60);
					} // $02:w:100;
					else if((command_str[3] == ':') & (command_str[4] == 's') & (command_str[5] == ':') && (command_str[9] == ';')) {
						_aux2[0] = '0';
						_aux2[1] = command_str[6];
						_aux2[2] = command_str[7];
						_aux2[3] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';
						pump1_.time_to_shutdown_config = atoi(_aux2)*60;
						sprintf(buffer, "set time_to_shutdown_config: %d min\n", pump1_.time_to_shutdown_config/60);
					} // $02:s:100;
					else if((command_str[3] == ':') && (command_str[4] == 'f') && (command_str[5] == ':') && (command_str[9] == ';')) {
						_aux2[0] = '0';
						_aux2[1] = command_str[6];
						_aux2[2] = command_str[7];
						_aux2[3] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';
						pump1_.time_to_shutdown = atoi(_aux2)*60;
						sprintf(buffer, "force time_to_shutdown: %d min\n", pump1_.time_to_shutdown/60);
					} // $02:f:010;
					break;
				}
				case 3: { // $03:_:__;
					if(command_str[3]==';') { // $03;
						sprintf(buffer, "m:%d k1:%d%d k2:%d%d k3:%d%d Rth:%d P:%d r:%d%d%d\n",
																							static_cast<int>(pump1_.state()),
																							static_cast<int>(pump1_.state_k1_pin()),
																							static_cast<int>(pump1_.state_k1()),
																							static_cast<int>(pump1_.state_k2_pin()),
																							static_cast<int>(pump1_.state_k2()),
																							static_cast<int>(pump1_.state_k3_pin()),
																							static_cast<int>(pump1_.state_k3()),
																							static_cast<int>(pump1_.state_Rth()),
																							pipe1_.pressure_mca(),
																							static_cast<int>(pump1_.stops_history[0]),
																							static_cast<int>(pump1_.stops_history[1]),
																							static_cast<int>(pump1_.stops_history[2])
																							);
					} // $03;
					else if((command_str[3] == ':') && (command_str[4] == 's') && (command_str[5] == ':') && (command_str[8] == ';')) {
						_aux[0] = command_str[6];
						_aux[1] = command_str[7];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						pipe1_.pressure_max = atoi(_aux);

						sprintf(buffer, "set press max: %d\n", pipe1_.pressure_max); 
					} // 	$03:s:72;	- Set max pressure ref for pipe [m.c.a.];
					else if((command_str[3] == ':') && (command_str[4] == 's') && (command_str[5] == ';')) {
						sprintf(buffer, "press max: %d\n", pipe1_.pressure_max); 
					}
					else if((command_str[3] == ':') && (command_str[4] == 'm') && (command_str[5] == ':') && (command_str[7] == ';')) {
						_aux[0] = '0';
						_aux[1] = command_str[6];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';

						pump1_.time_delta_to_y_switch_config = atoi(_aux);

						// pump1_.time_k3_on_speeding_up = atoi(_aux);
						sprintf(buffer, "set delta to y: %d s\n", pump1_.time_delta_to_y_switch_config);
					} // $03:m:3;	- Set 3 seconds while K1 and K3 are ON into delta tri start;
					else if((command_str[3] == ':') && (command_str[4] == 'm') && (command_str[5] == ';')) {
						sprintf(buffer, "time delta to y: %d s\n", pump1_.time_delta_to_y_switch_config);
					}
					else if((command_str[3] == ':') && (command_str[4] == 't') && (command_str[5] == ':') && (command_str[9] == ';')) {
						_aux2[0] = '0';
						_aux2[1] = command_str[6];
						_aux2[2] = command_str[7];		// '0' in uint8_t is 48. ASCII
						_aux2[3] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';

						pump1_.time_switch_k_change = atoi(_aux2);

						sprintf(buffer, "set time_switch: %d\n", pump1_.time_switch_k_change);
					} // 	$03:t:900;	- Set 500 milliseconds to wait K3 go off before start K2;
					else if((command_str[3] == ':') && (command_str[4] == 't') && (command_str[5] == ';')) {
						sprintf(buffer, "time_switch: %d\n", pump1_.time_switch_k_change);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == ':') && (command_str[9] == ';')) {
						_aux2[0] = '0';
						_aux2[1] = command_str[6];
						_aux2[2] = command_str[7];		// '0' in uint8_t is 48. ASCII
						_aux2[3] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';

						pipe1_.pressure_max = atoi(_aux2);

						sprintf(buffer, "set press max ref: %d\n", pipe1_.pressure_max);
					} // 	$03:p:100;	- Set 100 psi the max pressure of sensor;
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == ';')) {
						sprintf(buffer, "sens press max ref: %d\n", pipe1_.pressure_max);
					} // 	$03:p:100;	- Set 100 psi the max pressure of sensor;
					break;
		// 	$03:v:32;	- Set pressure for valve load turn on and fill reservoir;
		// 	$03:p:150;	- Set sensor max pressure ref to change the scale [psi];
		// 	$03:b:85;	- Set to 85% the pressure min bellow the current pressure to avoid pipe broken;
				}
				case 8: { // $08;
					if(command_str[3]==';')
					{
						sprintf(buffer, "sensors request. Not implemented\n");
						// signal_request_sensors = 1;
						// temp_sensor.requestTemperatures();
						// if(dht0.read2())
						// 	sprintf(buffer, "Time: %.2d:%.2d:%.2d, Tout:%.2f, Tin:%.1f, Humidity: %.1f%%\n", dt.getHour(), dt.getMinute(), dt.getSecond(), temp_sensor.getTempCByIndex(0), (float)dht0.getTempCelsius(0)*0.1, (float)dht0.getHumidity(0)*0.1);
						// else
						// 	sprintf(buffer, "Time: %.2d:%.2d:%.2d, Tout:%.2f, Tin:%.1f, Humidity: %.1f%% ER\n", dt.getHour(), dt.getMinute(), dt.getSecond(), temp_sensor.getTempCByIndex(0), (float)dht0.getTempCelsius(0)*0.1, (float)dht0.getHumidity(0)*0.1);
					}
					break;
				}
				default:
					sprintf(buffer, "command not implemented\n");
					break;
			}
			break;
		}
		case 1: {
			if((command_str[3] == ':') && (command_str[4] == 'h') && (command_str[5] == ':') && (command_str[12] == ';')) {
			// $10:h:063015; set time clock to 06:30:15
				_aux[0] = command_str[6];
				_aux[1] = command_str[7];
				_aux[2] = '\0';
				dt.setHour(atoi(_aux), ND);

				_aux[0] = command_str[8];
				_aux[1] = command_str[9];
				_aux[2] = '\0';
				dt.setMinute(static_cast<uint8_t>(atoi(_aux)));

				_aux[0] = command_str[10];
				_aux[1] = command_str[11];
				_aux[2] = '\0';
				dt.setSecond(static_cast<uint8_t>(atoi(_aux)));

				// dt.setTime(0, 0, 0, ND);
				device_clock.set_time(dt.getUnixTime());
				time_day_sec_ = dt.getHour()*3600 + dt.getMinute()*60 + dt.getSecond();

				sprintf(buffer, "Time: %.2d:%.2d:%.2d\n", dt.getHour(), dt.getMinute(), dt.getSecond());
			} else if ((command_str[3] == ':') && (command_str[4] == 'd') && (command_str[5] == ':') && (command_str[14] == ';')) {
			// $10:d:13122022;
				_aux[0] = command_str[6];
				_aux[1] = command_str[7];
				_aux[2] = '\0';
				dt.setDay(atoi(_aux));

				_aux[0] = command_str[8];
				_aux[1] = command_str[9];
				_aux[2] = '\0';
				dt.setMonth(static_cast<uint8_t>(atoi(_aux)));

				_aux2[0] = command_str[10];
				_aux2[1] = command_str[11];
				_aux2[2] = command_str[12];
				_aux2[3] = command_str[13];
				_aux2[4] = '\0';
				dt.setYear(static_cast<uint16_t>(atoi(_aux2)));

				// dt.setTime(0, 0, 0, ND);
				device_clock.set_time(dt.getUnixTime());

				sprintf(buffer, "Date:%.2d/%.2d/%.4d\n", dt.getDay(), dt.getMonth(), dt.getYear());
			} // $10:d:10082022;
			break;
		}
		case 2: { // $2x:
			switch (opcode1)
			{
				case 1: {
					if((command_str[3] == ':') && (command_str[4] == 'f') && (command_str[5] == ':') && (command_str[10] == ';')) {
					// $21:f:0060;
						_aux2[0] = command_str[6];
						_aux2[1] = command_str[7];
						_aux2[2] = command_str[8];
						_aux2[3] = command_str[9];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';
						uint32_t pwm_led_frequency = (uint32_t) atoi(_aux2);
						led_wifi_indicator.pwm_ledc_set_frequency(pwm_led_frequency);

						sprintf(buffer, "pwm freq: %d", pwm_led_frequency);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'd') && (command_str[5] == ':') && (command_str[9] == ';')) {
					// $21:d:098; - set led duty cycle to 98 %;
						_aux2[0] = '0';
						_aux2[1] = command_str[6];
						_aux2[2] = command_str[7];
						_aux2[3] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';
						uint32_t pwm_led_duty = (uint32_t) atoi(_aux2);
						led_wifi_indicator.pwm_ledc_set_duty(pwm_led_duty);

						sprintf(buffer, "pwm duty: %d", pwm_led_duty);
					}
					break;
				}
				default:
				break;
			}
			break;
		}
		case 3: { //
			switch (opcode1)
			{
				case 0: {
					if(command_str[3]==';')
					{
						// if(pump1_.state == states_motor::)
						pump1_.stop(states_stop::command_line_user);
						sprintf(buffer, "Motor: %d\n", static_cast<uint8_t>(pump1_.state()));
					}
					break;
				}
				case 1: {
					if(command_str[3]==';')
					{
						pump1_.start(start_types::direct_k1);
						sprintf(buffer, "Motor: %d\n", static_cast<uint8_t>(pump1_.state()));
					}
					break;
				}
				case 2: {
					if(command_str[3]==';')
					{
						pump1_.start(start_types::direct_k2);
						sprintf(buffer, "Motor: pump1_ start request");
						// sprintf(buffer, "k1: %d, k2: %d, k3: %d\n", static_cast<int>(pump1_.state_k1()), static_cast<int>(pump1_.state_k2()), static_cast<int>(pump1_.state_k3()));
					}
					break;
				}
				case 3: {
					if(command_str[3]==';')
					{
						pump1_.start(start_types::direct_k3);
						sprintf(buffer, "Motor: %d\n", static_cast<uint8_t>(pump1_.state()));
					}
					break;
				}
				case 4: {
					if(command_str[3]==';')
					{
						pump1_.start(start_types::to_delta);
						sprintf(buffer, "Motor: %d\n", static_cast<uint8_t>(pump1_.state()));
					}
					break;
				}
				case 5: {
					if(command_str[3]==';')
					{
						pump1_.start(start_types::to_y);
						sprintf(buffer, "Motor: %d\n", static_cast<uint8_t>(pump1_.state()));
					}
					break;
				}
				case 6: {
					if(command_str[3]==';')
					{
						pump1_.start(start_types::y_delta_req);
						sprintf(buffer, "Motor: %d\n", static_cast<uint8_t>(pump1_.state()));
					}
					break;
				}

				default:
				break;
			}
			break;
		}
		case 5: { // $50:h1:0900;
			switch(opcode1) {
				case 0: {
					if(command_str[3] == ';') {	// $50;
						memset(buffer, 0, sizeof(buffer));
						sprintf(buffer,"tm flag:%d, n:%d, ", (int)flag_check_time_match_, time_match_n);
						char buffer_temp[30];
						for(int i=0; i<time_match_n; i++)
						{
							sprintf(buffer_temp, "h%d:%.2d:%.2d m:%d ", i+1, (int)timesec_to_hour(time_match_list[i]), (int)timesec_to_min(time_match_list[i]), (int)auto_start_mode[i]);
							strcat(buffer, buffer_temp);
						}
						strcat(buffer, "\n");
					}
					else if((command_str[3] == ':') && (command_str[5] == ';')) {
						_aux[0] = '0';
						_aux[1] = command_str[4];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						int opcode_sub0 = atoi(_aux);

						if(opcode_sub0)
							flag_check_time_match_ = states_flag::enable;
						else
							flag_check_time_match_ = states_flag::disable;

						sprintf(buffer, "set auto tm: %d\n", (int)flag_check_time_match_);
					} // $50:[1|0];
					else if((command_str[3] == ':') && (command_str[4] == 'h') && (command_str[6] == ':') && (command_str[11] == ';')) {
						// $50:h1:2130; set time 1
						_aux[0] = '0';
						_aux[1] = command_str[5];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						int index = atoi(_aux)-1;

						_aux[0] = command_str[7];
						_aux[1] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						uint32_t time_temp = (static_cast<uint32_t>(atoi(_aux)))*3600;

						_aux[0] = command_str[9];
						_aux[1] = command_str[10];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						time_temp += (static_cast<uint32_t>(atoi(_aux)))*60;

						time_match_list[index] = time_temp;

						sprintf(buffer, "set h%d %.2d:%.2d, t:%d\n", index+1, (int)timesec_to_hour(time_match_list[index]), (int)timesec_to_min(time_match_list[index]), time_match_list[index]);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'n') && (command_str[5] == ':') && (command_str[7] == ';')) {
						// set $50:n:4; set n to turn 4 times;
						_aux[0] = '0';
						_aux[1] = command_str[6];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						time_match_n = static_cast<uint8_t>(atoi(_aux));

						sprintf(buffer, "set n: %d\n", time_match_n);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'n') && (command_str[5] == ';')) {
						// $50:n; show n number
						sprintf(buffer, "n: %d\n", time_match_n);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'm') && (command_str[6] == ':') && (command_str[8] == ';')) {
						// $50:m1:1; or // $50:m[1-9]:[1-4]
						_aux[0] = '0';
						_aux[1] = command_str[5];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						int index = atoi(_aux);

						_aux[0] = '0';
						_aux[1] = command_str[7];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						int status_set = atoi(_aux);

						if(status_set)
						{
							if(!index)	// fill all elements with same value
							{
								for(int i=0; i<9; i++)
								{
									auto_start_mode[i] = (start_types)status_set;
								}
								sprintf(buffer, "set all elements\n");
							}
							else		// fill one element only
							{
								auto_start_mode[index-1] = (start_types)status_set;
								sprintf(buffer, "set auto start m: %d\n", (int)auto_start_mode[index-1]);
							}
						}
					}
					break;
				}
			}
			break;
		}
		case 6: {
			_aux[0] = '0';
			_aux[1] = command_str[2];		// '0' in uint8_t is 48. ASCII
			_aux[2] = '\0';
			statusCommand = atoi(_aux);

			switch (statusCommand)
			{
				case 0:
				{
					state_mode = states_mode::system_off;
					sprintf(buffer,"state mode: system_off, s:%d\n", static_cast<int>(state_mode));
					break;
				}

				case 1:
				{
					state_mode = states_mode::system_idle;
					sprintf(buffer,"state mode: system_idle, s:%d\n", static_cast<int>(state_mode));
					break;
				}

				default:
				break;
			}
			// sprintf(buffer, "Acionna State2: %d\n", static_cast<int>(state_mode));
			ESP_LOGI(TAG_ACIONNA,"Acionna State2: %d\n", static_cast<int>(state_mode));
			break;
		}
		case 7: {
			switch(opcode1) {
				case 0: {
					_aux[0] = '0';
					_aux[1] = command_str[7];		// '0' in uint8_t is 48. ASCII
					_aux[2] = '\0';
					opcode_sub0 = atoi(_aux);

					if((command_str[3] == ':') && (command_str[4] == 'r') && (command_str[5] == 't') && (command_str[6] == ';')) {
					// $70:rt;
						sprintf(buffer, "check Rth: %d\n", (int)flag_check_thermal_relay_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'r') && (command_str[5] == 't') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:rt:[0|1]; disable|enable check thermal relay
						if(opcode_sub0)
							flag_check_thermal_relay_ = states_flag::enable;
						else
							flag_check_thermal_relay_ = states_flag::disable;

						sprintf(buffer, "set check Rth: %d\n", (int)flag_check_thermal_relay_);
					}
					else if ((command_str[3] == ':') && (command_str[4] == 'k') && (command_str[5] == 't') && (command_str[6] == ';')) {
					// $70:kt;
								sprintf(buffer, "check k: %d\n", (int)pump1_.flag_check_output_pin_only);
					}
					else if ((command_str[3] == ':') && (command_str[4] == 'k') && (command_str[5] == 't') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:kt:[0|1];
						if(opcode_sub0)
							pump1_.flag_check_output_pin_only = states_flag::enable;
						else
							pump1_.flag_check_output_pin_only = states_flag::disable;
						
						sprintf(buffer, "set check k1: %d\n", (int)pump1_.flag_check_output_pin_only);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'k') && (command_str[5] == '1') && (command_str[6] == ';')) {
					// $70:k1;
						sprintf(buffer, "check k1: %d\n", (int)flag_check_k1_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'k') && (command_str[5] == '1') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:k1:[0|1];
						if(opcode_sub0)
							flag_check_k1_ = states_flag::enable;
						else
							flag_check_k1_ = states_flag::disable;

						sprintf(buffer, "set check k1: %d\n", (int)flag_check_k1_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'k') && (command_str[5] == '2') && (command_str[6] == ';')) {
					// $70:k2;
						sprintf(buffer, "check k2: %d\n", (int)flag_check_k2_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'k') && (command_str[5] == '2') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:k2:[0|1];
						if(opcode_sub0)
							flag_check_k2_ = states_flag::enable;
						else
							flag_check_k2_ = states_flag::disable;

						sprintf(buffer, "set check k2: %d\n", (int)flag_check_k2_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'k') && (command_str[5] == '3') && (command_str[6] == ';')) {
					// $70:k3;
						sprintf(buffer, "check k3: %d\n", (int)flag_check_k3_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'k') && (command_str[5] == '3') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:k3:[0|1];
						if(opcode_sub0)
							flag_check_k3_ = states_flag::enable;
						else
							flag_check_k3_ = states_flag::disable;
					
						sprintf(buffer, "set check k3: %d\n", (int)flag_check_k3_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == 'h') && (command_str[6] == ';')) {
					// $70:ph;
						sprintf(buffer, "check press high: %d\n", (int)flag_check_pressure_high_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == 'h') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:ph:[0|1]; desligamento por alta press??o;
						if(opcode_sub0)
							flag_check_pressure_high_ = states_flag::enable;
						else
							flag_check_pressure_high_ = states_flag::disable;

						sprintf(buffer, "set check press high: %d\n", (int)flag_check_pressure_high_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == 'l') && (command_str[6] == ';')) {
					// $70:pl;
						sprintf(buffer, "check press low: %d\n", (int)flag_check_pressure_low_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == 'l') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:pl:[0-9];			- desligamento por press??o baixa em min caso seja diferente de 0;
						if(opcode_sub0)
							flag_check_pressure_low_ = states_flag::enable;
						else
							flag_check_pressure_low_ = states_flag::disable;

						sprintf(buffer, "set check press low: %d\n", (int)flag_check_pressure_low_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == 'v') && (command_str[6] == ';')) {
					// $70:pv;
						sprintf(buffer, "check press valve: %d\n", (int)flag_check_pressure_valve_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == 'v') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:pv:[0|1];			- desligamento por press??o alta por v??lvula;
						if(opcode_sub0)
							flag_check_pressure_valve_ = states_flag::enable;
						else
							flag_check_pressure_valve_ = states_flag::disable;

						sprintf(buffer, "set check press valve: %d\n", (int)flag_check_pressure_valve_);
					}
					break;
				}
				default: {
					sprintf(buffer, "7: command not implemented\n");
					break;
				}
			}
			break;
		}
		case 8: {
			if (command_str[2] == ';') {
				sprintf(buffer, "sV:%d d_inv:%d tsOn:%.2d:%.2d:%.2d Tcfg:%d, v:%d tvOn:%.2d:%.2d:%.2d Pcfg:%d tv_cfg:%d, \n",
																									(int)valves1_.state_valves,
																									(int)valves1_.flag_inverted_sequence,
																									timesec_to_hour(valves1_.get_time_on()),
																									timesec_to_min(valves1_.get_time_on()),
																									timesec_to_sec(valves1_.get_time_on()),
																									valves1_.get_total_time_programmed(),
																									valves1_.valve_current,
																									timesec_to_hour(valves1_.time_valve_remain),
																									timesec_to_min(valves1_.time_valve_remain),
																									timesec_to_sec(valves1_.time_valve_remain),
																									valves1_.get_valve_pressure(valves1_.valve_current),
																									valves1_.get_valve_time(valves1_.valve_current)																									
																									);
			} // $8;
			else if (command_str[3] == ';')
			{
				if(command_str[2] == '1') {
					// start sequence;
					valves1_.start();
					sprintf(buffer, "Started valves");
				} // $81;
				else if(command_str[2] == '0') {
					valves1_.stop();
					sprintf(buffer, "Stoped valves");
				} // $80;
				else if(command_str[2] == 'd') {
					valves1_.flag_inverted_sequence = states_flag::disable;
					sprintf(buffer, "sentido %d\n", (int)valves1_.flag_inverted_sequence);
				}
				else if(command_str[2] == 'i') {
					valves1_.flag_inverted_sequence = states_flag::enable;
					sprintf(buffer, "sentido %d\n", (int)valves1_.flag_inverted_sequence);
				}
			} // $8[0|1];
			else if(command_str[2] == ':') // $8:...
			{
				_aux[0] = command_str[3];
				_aux[1] = command_str[4];		// '0' in uint8_t is 48. ASCII
				_aux[2] = '\0';
				int valve_id = atoi(_aux);

				if (valve_id < 12)
				{
					if(command_str[5] == ';') // $8:01;
					{
						sprintf(buffer, "valve[%d]:%d, prog[%d], time: %d, pn: %d m.c.a.\n", valve_id, (int)valves1_.get_valve_state(valve_id), (int)valves1_.get_program_status(valve_id), valves1_.get_valve_time(valve_id), valves1_.get_valve_pressure(valve_id));
					} // $8:01;
					else if(command_str[5] == ':') // $8:01:...
					{
						if(((command_str[6] == '0') || (command_str[6] == '1')) && (command_str[7] == ';'))
						{
							_aux[0] = '0';
							_aux[1] = command_str[6];		// '0' in uint8_t is 48. ASCII
							_aux[2] = '\0';
							int valve_state = atoi(_aux);
				
							valves1_.set_valve_state(valve_id, valve_state);
							sprintf(buffer, "set valve[%d]:%d", valve_id, (int)valves1_.get_program_status(valve_id));
						} // $8:01:[0|1];		- desaciona|aciona v??lvula 01;
						else if((command_str[6] == 'i') && (command_str[7] == ';'))
						{
							valves1_.set_program_add(valve_id);
							sprintf(buffer, "added valve %d: %d\n", valve_id, (int)valves1_.get_program_status(valve_id));
						} // $8:01:i;		- insere setor na programa????o;
						else if((command_str[6] == 'r') && (command_str[7] == ';'))
						{
							valves1_.set_program_remove(valve_id);
							sprintf(buffer, "removed valve %d: %d\n", valve_id, (int)valves1_.get_program_status(valve_id));
						} // $8:01:r;		- remove setor da programa????o;
						else if ((command_str[6] == 't') && (command_str[7] == ';'))
						{
							int valve_time = valves1_.get_valve_time(valve_id);
							sprintf(buffer, "valve %d: %d min\n", valve_id, valve_time);
						} // $8:01:t;		- mostra o tempo de irriga????o do setor;
						else if ((command_str[6] == 't') && (command_str[7] == ':') && (command_str[11] == ';'))
						{
							_aux2[0] = '0';
							_aux2[1] = command_str[8];
							_aux2[2] = command_str[9];
							_aux2[3] = command_str[10];		// '0' in uint8_t is 48. ASCII
							_aux2[4] = '\0';
							uint8_t valve_time = (uint8_t) atoi(_aux2);
							valves1_.set_valve_time(valve_id, valve_time);
							sprintf(buffer, "valve %d: %d min\n", valve_id, valves1_.get_valve_time(valve_id));
						} // $8:01:t:120;	- configura o tempo de irriga????o [min];
						else if((command_str[6] == 'p') && (command_str[7] == ';'))
						{
							sprintf(buffer, "get valve %d: %d m.c.a.\n", valve_id, valves1_.get_valve_pressure(valve_id));
						} // $8:01:p;		- mostra press??o nominal do setor
						else if((command_str[6] == 'p') && (command_str[7] == ':') && (command_str[10] == ';'))
						{
							_aux[0] = command_str[8];
							_aux[1] = command_str[9];
							_aux[2] = '\0';
							unsigned int valve_pressure = (unsigned int) atoi(_aux);
							valves1_.set_valve_pressure(valve_id, valve_pressure);
							sprintf(buffer, "set valve %d: %d m.c.a.\n", valve_id, valves1_.get_valve_pressure(valve_id));
						} // $8:01:p:60;	- configura press??o nominal do setor [m.c.a.];
					}
				}
				else
				{
					sprintf(buffer, "8 valve not installed");
				}
			} // $8:
			break;
		}
		case 9: {
			switch(opcode1)
			{
				case 0: {
					if(command_str[3] == ';') {
						sys_wifi_info_(buffer);
					} // $90;

					break;
				} // $90:s:1;
				case 1: {
					if(command_str[3] == ';')
					{
						sys_wifi_scan_(buffer);
						// signal_wifi_scan = 1;
						// sprintf(buffer, "wifi scan\n");
					} // $91;
					break;
				}
				case 2: {
					char fw_version[33];
					memset(buffer, 0, sizeof(buffer));
					strcpy(fw_version, "fw: ");
					strcat(fw_version, __DATE__);
					strcat(fw_version, " ");
					strcat(fw_version, __TIME__);
					strcat(fw_version, "\n");
					strcpy(buffer, fw_version);
					break;
				}
				case 5: { //$95:[0-9];
					if(command_str[3] == ':' && command_str[5] == ';') {
						_aux[0] = '0';
						_aux[1] = command_str[4];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						int statusCommand_2 = atoi(_aux);

						switch (statusCommand_2) {
							case 0: {
								sys_fw_info_partitions_(buffer);
								break;
							}
							case 1: {
								sys_fw_info_app_(buffer);
								break;
							}
							case 2: {
								sys_fw_info_sha256_(buffer);
								break;
							}
							case 3: {
								sys_fw_mark_invalid_();
								sprintf(buffer, "Mark invalid\n");
								break;
							}
							case 4: {
								sys_fw_mark_valid_();
								sprintf(buffer, "Mark valid\n");
								break;
							}
							case 8: {
								sys_fw_change_boot_();
								sprintf(buffer, "change boot\n");
								break;
							}
							case 9: {
								sys_fw_update_();
								sprintf(buffer, "Firmware update request\n");
								break;
							}
							default:
							break;
						}
					}
					break;
				}
				case 7: {
					sys_ram_free_(buffer);
					break;
				}
				case 8: {
					sys_reset_reason_(buffer);
					break;
				}
				case 9: {
					sys_restart_();
					break;
				}
				default:
					sprintf(buffer,"opcode %d - sub not implemented\n", opcode0);
					break;
			}
			break;
		}
		default: {
			sprintf(buffer, "opcode0 not implemented\n");
			break;
		}
	}
	// ESP_LOGI(TAG_ACIONNA, "Buffer: %s", buffer);
	std::string str(buffer);
	return str;
}
void Acionna::operation_mode() {
	switch (state_mode)
	{
		case states_mode::system_off:
			operation_system_off();
			break;

		case states_mode::system_idle:
			operation_pump_control();
			break;

		case states_mode::water_pump_control_night:
			break;

		case states_mode::irrigation_pump_valves:
			operation_pump_valves_irrigation();
			break;

		case states_mode::valve_control:
			break;

		default:
			break;
	}
}
void Acionna::run(void) {
	msg_fetch_();		// fetch for a new command;

	msg_exec_();		// parse and execute the commmand;

	msg_back_();		// send answer back to origin;

	update_all();		// update variables and rtc

	operation_mode();	// execution process
}
void Acionna::msg_fetch_(void) {

	#ifdef CONFIG_BT_ENABLE
	if(bt_data_flag) {
		bt_data_flag = 0;
		bt_ans_flag_ = states_flag::enable;

		uint8_t* command_str = new uint8_t[20];
		int command_str_len = 0;
		parser_(bt_data, bt_data_len, command_str, command_str_len);
		// command_str_len_ = command_str_len;
		memcpy(command_str_, command_str, command_str_len);

		delete[] command_str;
	}
	#endif
	
	if(ws_server_data_flag) {
		ws_server_data_flag = 0;
		ws_server_ans_flag_ = states_flag::enable;

		uint8_t* command_str = new uint8_t[20];
		int command_str_len = 0;
		parser_(ws_server_data, ws_server_data_len, command_str, command_str_len);
		// command_str_len_ = command_str_len;
		memcpy(command_str_, command_str, command_str_len);

		delete[] command_str;
	}

	// if(ws_client_data_flag) {

	// }
}
void Acionna::msg_exec_(void) {

	#ifdef CONFIG_BT_ENABLE
	if((bt_ans_flag_ == states_flag::enable) || (ws_server_ans_flag_ == states_flag::enable) || (ws_client_ans_flag_ == states_flag::enable)) {
	#else
		if((ws_server_ans_flag_ == states_flag::enable) || (ws_client_ans_flag_ == states_flag::enable)) {
	#endif
		msg_back_str_ = handle_message(command_str_);
	}
}
void Acionna::msg_back_(void) {

	#ifdef CONFIG_BT_ENABLE
	if(bt_ans_flag_ == states_flag::enable)
	{
		bt_ans_flag_ = states_flag::disable;
		// Send bt msg back;
		ESP_LOGI(TAG_ACIONNA, "Sending msg back over BT\n");
		bt_send(msg_back_str_);
	}
	#endif

	// This is enable when receive some msg from websocket
	if(ws_server_ans_flag_ == states_flag::enable)
	{
		ws_server_ans_flag_ = states_flag::disable;
		// Send websocket msg back
		ws_server_send(msg_back_str_);
	}

	if(ws_client_ans_flag_ == states_flag::enable)
	{
		ws_client_ans_flag_ = states_flag::disable;

		// send ws client back
		ws_client_send(msg_back_str_);
	}

	sys_fw_update_ans_async_();

	msg_json_back_();
}
void Acionna::msg_json_back_(void) {
	if(ws_server_client_state == conn_states::connected) {
		if(flag_json_data_back == states_flag::enable) {

			char buffer_str[150];
			DynamicJsonDocument doc(1024);
			doc["id"] = IP_END;
			doc["p1"] = pipe1_.pressure_mca();
			doc["p2"] = pipe1_.pressure_mca();
			doc["ton"] = pump1_.time_on();
			doc["toff"] = pump1_.time_off();
			doc["k1"] = static_cast<int>(pump1_.state_k1());
			doc["k2"] = static_cast<int>(pump1_.state_k2());
			doc["k3"] = static_cast<int>(pump1_.state_k3());
			doc["rth"] = static_cast<int>(pump1_.state_Rth());

			serializeJson(doc, buffer_str);
			std::string str_json(buffer_str);
			ws_server_send(str_json);
		}
	}
	else {
		if(flag_json_data_back == states_flag::enable) {
			flag_json_data_back = states_flag::disable;
		}
	}
}
void Acionna::parser_(uint8_t* payload_str, int payload_str_len, uint8_t *command_str, int& command_str_len)
{
	states_flag flag_instruction_write = states_flag::disable;

	int j = 0;												// aux counter;
	for(int i=0; i<payload_str_len; i++)
	{
		if (payload_str[i] == '$')							// found beginer of frame instruction
		{
			j = 0;											// clear counter
			flag_instruction_write = states_flag::enable;	// enable write
		}

		if(flag_instruction_write == states_flag::enable)
		{
			command_str[j] = payload_str[i];
			j++;
			if(payload_str[i] == ';')
			{
				command_str_len = j;
				break;
			}
		}
	}
}
void Acionna::sys_fw_change_boot_(void) {
	ota_change_boot_partition();
}
void Acionna::sys_fw_info_app_(char* buffer_str) {
	memset(buffer_str, 0, sizeof(*buffer_str));
	char buffer_temp[70];
	ota_info();

	// App info
	sprintf(buffer_temp, "Ver:%s\n", OTA_update.running_app_info.version);
	strcat(buffer_str, buffer_temp);
	sprintf(buffer_temp, "Proj name:%s\n", OTA_update.running_app_info.project_name);
	strcat(buffer_str, buffer_temp);
	sprintf(buffer_temp, "Date:%s\n", OTA_update.running_app_info.date);
	strcat(buffer_str, buffer_temp);
	sprintf(buffer_temp, "Time:%s\n", OTA_update.running_app_info.time);
	strcat(buffer_str, buffer_temp);
	sprintf(buffer_temp, "Sec Ver:%d\n", OTA_update.running_app_info.secure_version);
	strcat(buffer_str, buffer_temp);

	// print info ota stuffs here!
	if(OTA_update.update_partition != NULL)
	{
		sprintf(buffer_temp, "Up part\n");
		strcat(buffer_str, buffer_temp);
		sprintf(buffer_temp, "Ver:%s\n", OTA_update.update_app_info.version);
		strcat(buffer_str, buffer_temp);
		sprintf(buffer_temp, "Proj. name:%s\n", OTA_update.update_app_info.project_name);
		strcat(buffer_str, buffer_temp);
		sprintf(buffer_temp, "Date:%s\n", OTA_update.update_app_info.date);
		strcat(buffer_str, buffer_temp);
		sprintf(buffer_temp, "Time:%s\n", OTA_update.update_app_info.time);
		strcat(buffer_str, buffer_temp);
		sprintf(buffer_temp, "Sec Ver: %d\n", OTA_update.update_app_info.secure_version);
		strcat(buffer_str, buffer_temp);
	}
}
void Acionna::sys_fw_info_partitions_(char* buffer_str) {
	memset(buffer_str, 0, sizeof(*buffer_str));
	char buffer_temp[70];
	ota_info();

	sprintf(buffer_str, "Bt-> L:%s, o:0x%08x, t:%d, s:%d\n",
												OTA_update.configured_partition->label,
												OTA_update.configured_partition->address,
												static_cast<int>(OTA_update.configured_partition->type),
												static_cast<int>(OTA_update.configured_partition->subtype));
	
	sprintf(buffer_temp, "Ru-> L:%s, o:0x%08x, t:%d, s:%d\n",
													OTA_update.running_partition->label,
													OTA_update.running_partition->address,
													static_cast<int>(OTA_update.running_partition->type),
													static_cast<int>(OTA_update.running_partition->subtype));
	strcat(buffer_str, buffer_temp);

	if(OTA_update.update_partition != NULL)
	{
		sprintf(buffer_temp, "Up-> L:%s, o:0x%02x, t:%d, s:%d\n",
														OTA_update.update_partition->label,
														OTA_update.update_partition->address,
														static_cast<int>(OTA_update.update_partition->type),
														static_cast<int>(OTA_update.update_partition->subtype));
		strcat(buffer_str, buffer_temp);
	}

	sprintf(buffer_temp, "Ru-> state: 0x%08x\n", static_cast<unsigned int>(OTA_update.running_state));
	strcat(buffer_str, buffer_temp);

	sprintf(buffer_temp, "Ru-> ver.:%s, date:%s\n", OTA_update.running_app_info.version, OTA_update.running_app_info.date);
	strcat(buffer_str, buffer_temp);

	sprintf(buffer_temp, "n:%d, size:%d\n", OTA_update.num_ota_partitions, sizeof(OTA_update));
	strcat(buffer_str, buffer_temp);
}
void Acionna::sys_fw_info_sha256_(char* buffer_str) {
	memset(buffer_str, 0, sizeof(*buffer_str));
	char buffer_temp[80];
	char image_hash_temp[HASH_LEN*2+1];
	ota_conv_img_sha256(OTA_update.running_app_info.app_elf_sha256, image_hash_temp);
	sprintf(buffer_temp, "a:%s\n", image_hash_temp);
	strcat(buffer_str, buffer_temp);

	ota_conv_img_sha256(OTA_update.running_partition_sha_256, image_hash_temp);
	sprintf(buffer_temp, "b:%s\n", image_hash_temp);
	strcat(buffer_str, buffer_temp);

	if(OTA_update.update_partition != NULL)
	{
		char image_hash_temp[HASH_LEN*2+1];
		ota_conv_img_sha256(OTA_update.update_app_info.app_elf_sha256, image_hash_temp);
		sprintf(buffer_temp, "SHA256:%s\n", image_hash_temp);
		strcat(buffer_str, buffer_temp);
	}
}
void Acionna::sys_fw_mark_valid_(void) {
	ota_mark_valid();
}
void Acionna::sys_fw_mark_invalid_(void) {
	ota_mark_invalid();
}
void Acionna::sys_fw_update_(void) {
	// httpd_server_stop();

	// advanced_ota_start();
	ota_start();
}
void Acionna::sys_fw_update_ans_async_(void)
{
	#ifdef CONFIG_BT_ENABLE
	if((ws_server_client_state == conn_states::connected) || (bt_state == conn_states::connected)) {
	#else
	if(ws_server_client_state == conn_states::connected) {
	#endif
		uint8_t flag_send = 0;
		char buffer_str[250] = {"OTA ans!"};

		if(OTA_update.state == OTA_process_states::updating) {
			flag_send = 1;
			sprintf(buffer_str, "%d %%\n", static_cast<int>(OTA_update.binary_file_length_write/(float)OTA_update.image_size*100.0));
		}
		else if(OTA_update.state == OTA_process_states::finish_update) {
			flag_send = 1;
			OTA_update.state = OTA_process_states::idle;
			char buffer_temp[50];
			// print info ota stuffs here!
			sprintf(buffer_str, "%d %%. write %d bytes. Finish update!\n", static_cast<int>(OTA_update.binary_file_length_write/(float)OTA_update.image_size*100.0), OTA_update.binary_file_length_write);
			sprintf(buffer_temp, "Version: %s\n", OTA_update.update_app_info.version);
			strcat(buffer_str, buffer_temp);
			sprintf(buffer_temp, "Proj. name: %s\n", OTA_update.update_app_info.project_name);
			strcat(buffer_str, buffer_temp);
			sprintf(buffer_temp, "Date: %s\n", OTA_update.update_app_info.date);
			strcat(buffer_str, buffer_temp);
			sprintf(buffer_temp, "Time: %s\n", OTA_update.update_app_info.time);
			strcat(buffer_str, buffer_temp);
			sprintf(buffer_temp, "Sec. Ver.: %d\n", OTA_update.update_app_info.secure_version);
			strcat(buffer_str, buffer_temp);

			// char image_hash_temp[32];
			// ota_conv_img_sha256(OTA_update.update_app_info.app_elf_sha256, image_hash_temp);
			// sprintf(buffer_temp, "Img. SHA256: %s\n", image_hash_temp);
			// strcat(buffer_str, buffer_temp);
		}

		if(flag_send) {
			std::string str(buffer_str);

			if(ws_server_client_state == conn_states::connected) {
				ws_server_send(str);
			} 
			#ifdef CONFIG_BT_ENABLE
			else if (bt_state == conn_states::connected) {
				bt_send(str);
			}
			#endif
				
		}
	}
}
void Acionna::sys_wifi_info_(char* buffer_str) {

	wifi_ap_record_t wifi_info;
	memset(buffer_str, 0, sizeof(*buffer_str));

	if (esp_wifi_sta_get_ap_info(&wifi_info)== ESP_OK)
	{
		char *str0 = (char*) wifi_info.ssid;
		sprintf(buffer_str, "SSID: %s, RSSI: %d\n", str0, static_cast<int>(wifi_info.rssi));
	}

	// httpd_ws_frame_t ws_pkt;
	// memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	// // ws_pkt.payload = reinterpret_cast<uint8_t*>(&buffer[0]);
	// ws_pkt.payload = (uint8_t*)buffer;
	// ws_pkt.len = strlen(buffer);
	// ws_pkt.type = HTTPD_WS_TYPE_TEXT;

	// if(httpd_ws_get_fd_info(sock0.hd, sock0.fd) == HTTPD_WS_CLIENT_WEBSOCKET)
	// {
	// 	httpd_ws_send_frame_async(sock0.hd, sock0.fd, &ws_pkt);
	// }
	// else
	// {
	// 	acionna0.signal_send_async = 0;
	// 	ESP_LOGI(TAG_WS, "SOCK0: connection closed");
	// }
}
void Acionna::sys_wifi_scan_(char* buffer_str) {

	uint16_t ap_count = 0;
	uint16_t number = DEFAULT_SCAN_LIST_SIZE;
	wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
	wifi_ap_record_t *ap_info_ptr = &ap_info[0];

	// char buffer_str[200];
	char buffer_temp[40];

	memset(ap_info, 0, sizeof(ap_info));
	memset(buffer_str, 0, sizeof(*buffer_str));
	memset(buffer_temp, 0, sizeof(buffer_temp));

	wifi_scan2(number, ap_info_ptr, ap_count);
	for(int i=0; (i<DEFAULT_SCAN_LIST_SIZE) && (i<ap_count); i++)
	{
		sprintf(buffer_temp, "Ch: %d, RSSI: %d, SSID: %s\n", ap_info[i].primary, ap_info[i].rssi, ap_info[i].ssid);
		strcat(buffer_str, buffer_temp);;
	}
	// std::string str(buffer_str);
	// ws_server_send(str);
}
void Acionna::sys_ram_free_(char* buffer_str) {

	memset(buffer_str, 0, sizeof(*buffer_str));
	sprintf(buffer_str, "RAM free:%d, min:%d\n", esp_get_free_internal_heap_size(), esp_get_minimum_free_heap_size());
}
void Acionna::sys_reset_reason_(char* buffer_str) {

	memset(buffer_str, 0, sizeof(*buffer_str));
	esp_reset_reason_t last_rst = esp_reset_reason();
	switch (last_rst)
	{
		case ESP_RST_UNKNOWN:	// 0
			sprintf(buffer_str, "%d: can not be determined\n", static_cast<int>(last_rst));
			break;

		case ESP_RST_POWERON:	// 1
			sprintf(buffer_str, "%d: power-on\n", static_cast<int>(last_rst));
			break;

		case ESP_RST_EXT:		// 2
			sprintf(buffer_str, "%d: ext pin\n", static_cast<int>(last_rst));
			break;

		case ESP_RST_SW:		// 3
			sprintf(buffer_str, "%d: ext pin\n", static_cast<int>(last_rst));
			break;

		case ESP_RST_PANIC:		// 4
			sprintf(buffer_str, "%d: panic\n", static_cast<int>(last_rst));
			break;

		case ESP_RST_INT_WDT:	// 5
			sprintf(buffer_str, "%d: int wdt\n", static_cast<int>(last_rst));
			break;

		case ESP_RST_TASK_WDT:	// 6
			sprintf(buffer_str, "%d: task wdt\n", static_cast<int>(last_rst));
			break;

		case ESP_RST_WDT:		// 7
			sprintf(buffer_str, "%d: other wdt\n", static_cast<int>(last_rst));
			break;

		case ESP_RST_DEEPSLEEP:	// 8
			sprintf(buffer_str, "%d: after exit deep sleep\n", static_cast<int>(last_rst));
			break;

		case ESP_RST_BROWNOUT:	// 9
			sprintf(buffer_str, "%d: brownout rst\n", static_cast<int>(last_rst));
			break;

		case ESP_RST_SDIO:	// 10
			sprintf(buffer_str, "%d: over SDIO\n", static_cast<int>(last_rst));
			break;

		default:
			sprintf(buffer_str, "reset not found\n");
			break;
	}
}
void Acionna::sys_restart_(void) {
	// Network stop
	wifi_sta_stop();

	// Restart system
	esp_restart();
}
void Acionna::sensor_dht(void) {


	// ESP_LOGI(TAG_SENSORS, "signal_request_sensors:%d", acionna0.signal_request_sensors);
	// acionna0.signal_request_sensors = 0;

	// char buffer[100] = "not changed!!\n";
	// _delay_ms(2000);
	// temp_sensor.requestTemperatures();

	// if(dht0.read2())
	// sprintf(buffer, "Tout:%.2f, Tin:%.1f, Humidity: %.1f%%\n", temp_sensor.getTempCByIndex(0), (float)dht0.getTempCelsius(0)*0.1, (float)dht0.getHumidity(0)*0.1);
	// else
	// sprintf(buffer, "Tout:%.2f, Tin:%.1f, Humidity: %.1f%% ER\n", temp_sensor.getTempCByIndex(0), (float)dht0.getTempCelsius(0)*0.1, (float)dht0.getHumidity(0)*0.1);
	// _delay_ms(1000);


	// if(dht0.read2())
	// {
	// ESP_LOGI(TAG_SENSORS, "Temp outside: %.2f, Temp inside: %.2f, Humidity: %.2f%%", temp_sensor.getTempCByIndex(0), (float)dht0.getTempCelsius(0)*0.1, (float)dht0.getHumidity(0)*0.1);
	// // count_down = 10*60;
	// }
	// else
	// {
	// ESP_LOGI(TAG_DHT, "error reading");
	// }
	// }

	// if(acionna0.signal_DS18B20)
	// {
	// if(temp_sensor_count)
	// {
	// temp_sensor.requestTemperatures();
	// temp_sensor.getTempCByIndex(0);
	// }
	// }

		// dht0.begin();
	// temp_sensor.begin();
	// temp_sensor_count = temp_sensor.getDeviceCount();
	// ESP_LOGI(TAG_SENSORS, "Temp sensors count: %u", temp_sensor_count)
}


////void acn_check_pressureUnstable()	// this starts to check quick variation of pressure from high to low after 2 minutes on
////{
////	if(stateMode == 5 && timeOn_min > 2)
////	{
////		if(motorStatus)
////		{
////			if(PRess > PRessHold)
////			{
////				PRessHold = PRess;
////			}
////			else if(PRess < ((PRessurePer/100.0)*PRessHold))
////			{
////				flag_PressureUnstable = 1;
////			}
////		}
////	}
////}
////void acn_check_pressureDown()	// this check if pressure is still low after 3 minutes on;
////{
//////	if(flag_conf_pressureUnstable && (timeOn_min > 3)
////	if(stateMode == 5 && (timeOn_min > 3))
////	{
////		if(motorStatus)
////		{
////			if(PRess < PRessureRef_Low)
////			{
////				// turn load off;
////			}
////		}
////	}
////}