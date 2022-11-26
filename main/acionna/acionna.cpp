#include "acionna.hpp"

static const char *TAG_ACIONNA = "ACIONNA0";

static I2C_Master i2c(I2C_NUM_0, I2C_SCL, I2C_SDA, I2C_FAST_SPEED_HZ);
static DS3231 rtc{&i2c};
static Agro::RTC_Time device_clock;
static DateTime dt;

int timeout_sensors;
int timeout_sensors_cfg = 600;

uint32_t ACIONNA::get_uptime()
{
	return uptime_;
}
void ACIONNA::init()
{
	ESP_LOGI(TAG_ACIONNA, "initialize");

	//	int count0 = 0;

	i2c.init();

	dt.setDate(2022, 8, 3);
	dt.setTime(0, 0, 0, ND);
	device_clock.set_time(dt.getUnixTime());
	time_day_sec_ = dt.getHour()*3600 + dt.getMinute()*60 + dt.getSecond();

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
void ACIONNA::operation_motorPeriodDecision()
{
}
void ACIONNA::operation_system_off()
{
	if((pump1_.state() == states_motor::on_nominal_k1) || (pump1_.state() == states_motor::on_nominal_delta) || (pump1_.state() == states_motor::on_speeding_up))
	{
		pump1_.stop(states_stop::system_lock);
	}
}
void ACIONNA::operation_valve_control() {

}
void ACIONNA::operation_pump_control() {
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
void ACIONNA::operation_pump_valves_irrigation() {

}
void ACIONNA::update_RTC() {
	dt.setUnixTime(device_clock.get_time());
}
void ACIONNA::update_objects() {
	pump1_.update();
	pipe1_.update();

	#ifdef CONFIG_WELL_SUPPORT
	well1_.update();
	#endif

	#ifdef CONFIG_VALVES_SUPPORT
	valves1_.update();
	#endif
}
void ACIONNA::update_sensors()
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
void ACIONNA::update_stored_data() {
}
void ACIONNA::update_uptime()
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
void ACIONNA::update_all() {
	update_uptime();
	update_RTC();
	update_objects();
	update_stored_data();

	// update_sensors();		// test sensors
}
std::string ACIONNA::handle_message(uint8_t* command_str)
{
	/*
	$0X;				Verificar detalhes - Detalhes simples (tempo).
		$00;			- Detalhes simples (tempo).
		$00:[0|1];		- Json send back 1, stop 0;
		$01;			- Verifica histórico de quando ligou e desligou;
		$02;			- Mostra tempo que falta para ligar;
			$02:w;		- Zera o tempo;
			$02:w:30;	- Ajusta novo tempo para 30 min;
			$02:s:090;	- Tempo máximo ligado para 90 min. Para não utilizar, colocar zero;
			$02:f:045;	- Força o contador de tempo ligado para 45 min;
		$03;			- Verifica detalhes do motor, pressão e sensor termico;
			$03:s:72;	- Set max pressure ref for pipe [m.c.a.];
			$03:s;		- show max pressure configured;
			$03:v:32;	- Set pressure for valve load turn on and fill reservoir;
			$03:p:150;	- Set sensor max pressure ref to change the scale [psi];
			$03:b:85;	- Set to 85% the pressure min bellow the current pressure to avoid pipe broken;
			$03:m:3;	- Set 3 seconds while K1 and K3 are ON into delta tri start;
			$03:m;		- Just show the speeding up time;
			$03:t:400;	- Set 400 milliseconds to wait K3 go off before start K2;
			$03:t;		- Just show the time switch;
		$04;			- Verifica detalhes do nível de água no po�o e referência 10 bits;
			$04:0;		- Interrompe o envio continuo das vari�veis de press�o e n�vel;
			$04:1;		- Envia continuamente valores de press�o e n�vel;
			$04:s:0900;	- Adiciona nova referência para os sensores de nível. Valor de 0 a 1023;
		$05;			- Mostra os hor�rios que liga no modo $62;
		$06;			- Tempo ligado e tempo desligado;
		$07:x;			- ADC reference change.
			$07:0;		- AREF
			$07:1;		- AVCC with external cap at AREF pin
			$07:2;		- Internal 1.1 Voltage reference.

	$1:h:HHMMSS;		- Ajustes do calendário;
		$1:h:HHMMSS;	- Ajusta o horário do sistema;
		$1:h:123040;	- E.g. ajusta a hora para 12:30:40
		$1:d:DDMMAAAA;	- Ajusta a data do sistema no formato dia/mês/ano(4 d�gitos);
		$1:d:04091986;	- E.g. Altera a data para 04/09/1986;
		$1:c;			- Shows the LSI current prescaler value;
		$1:c:40123;		- Set new prescaler value;

	$2:DevName;			- Change bluetooth name;
		$2:Vassalo;		- Altera o nome do bluetooth para "Vassalo";

	$4:x:				- Is this applied fo stm32f10xxx series only;
		$4:r:07;		- Read address 0x07 * 2 of currently page;
		$4:w:07:03;		- Write variable 3 on address 7 of currently page;
		$4:f:64:03;		- fill page 64 with 3 value;
		$4:e:64;		- erase page 64;

	$3X;				- Acionamento do motor;
		$30;			- desliga todos contatores;
		$31;			- ligar contator K1;
		$32;			- ligar contator K2;
		$33;			- ligar contator K3;
		$34;			- direto para partida delta;
		$35;			- direto para partida Y;
		$36;			- liga motor com partida Y delta;


	$5:n:X; ou $5:hX:HHMM;
		$5;				- mostra os horários que irá ligar;
		$5:0|1;			- desabilita|habilita auto turn (habilita time match flag);
		$5:m:[1|3]		- tipo de partida automática: 1- k1; 3- y-Delta;
		$5:n;			- mostra a quantidade de vezes que irá ligar;
		$5:n:9;			- Configura para acionar 9 vezes. Necessário configurar 9 horários;
		$5:n:1;			- Configura o sistema para acionar uma única vez;
		$5:h1:2130;		- Configura o primeiro horário como 21:30 horas;
		$5:h8:0437;		- Configura o oitavo horário como 04:37 horas;

	$6X;				- Modos de funcionamento;
		$60; 			- Sistema Desligado não permite ligar;
		$61;			- Sistema ocioso com permissão de acionamento manual (nunca ligará sozinho);
		$62;			- Liga de modo automático nos determinados horários estipulados;
		$63;			- Função para válvula do reservatório;
		$64;			- Função para motobomba do reservatório;

	$7					- Funções que habilitam ou desabilitam checks
		$7:__:			- Habilita/desabilita verificação de:
		$7:rt:1|0;			- relé térmico;
		$7:rt;
		$7:kt:1|0;			- todos contatores
		$7:kt;
		$7:k1:1|0;			- contator K1 (not implemented);
		$7:k1;
		$7:k2:1|0;			- contator K2 (not implemented);
		$7:k2;
		$7:k3:1|0;			- contator K3 (not implemented);
		$7:k3;
		$7:ph:1|0;			- desligamento por alta pressão;
		$7:ph;
		$7:pl:0-9;			- desligamento por pressão baixa em min caso seja diferente de 0;
		$7:pl;
		$7:pv:1|0;			- desligamento por pressão alta por válvula;
		$7:pv;

	$8					- Funções de programação da irrigação;
		$81;			- start valves sequence;
		$80;			- stop valves sequence;
		$8:[0|1];		- 0 sentido direto; 1 - sentido inverso na troca dos setores;
		$8:01;			- mostra condições de configuração da válvula 01;
		$8:01:[0|1];	- desaciona|aciona válvula 01;
		$8:01:i;		- insere setor na programação;
		$8:01:r;		- remove setor da programação;
		$8:01:t:120;	- configura o tempo de irrigação [min];
		$8:01:p:68;		- configura pressão nominal do setor [m.c.a.];
		$8:01:t;		- mostra o tempo de irrigação do setor;
		$8:01:p;		- mostra pressão nominal do setor

	$9X;				- System administration
		$90;			- Wifi RSSI
		$91;			- WiFi Scan
		$92;			- show firwmare version;
		$95;			- do upgrade;
		$96;			- show ota info;
		$97;			- Show RAM usage;
		$98;			- Show reset reason;
		$99;			- Soft reset system;
	*/
	// Tx - Transmitter - If exists new package for decode and execute, do this.
			// if(flag_enable_decode_ == states_flag::enable)
			// {
			// 	flag_enable_decode_ = states_flag::disable;
			// 	ESP_LOGI(TAG_ACIONNA, "entrou! enableDecode=:%d", flag_enable_decode_);

	// std::stringstream ss_buffer;
	int opcode = -1;
	int statusCommand = -1;
	char _aux[3], _aux2[5];
	char buffer[100] = "not handled\n";
	// char aux2[5];

	// Getting the opcode (operation code)
	_aux[0] = '0';
	_aux[1] = command_str[1];		// '0' in uint8_t is 48. ASCII
	_aux[2] = '\0';
	opcode = atoi(_aux);
	ESP_LOGI(TAG_ACIONNA, "handleMessage(), opcode: %d!\n", opcode);

	switch (opcode)
	{
		case 0: { // $0X; Check status
			_aux[0] = '0';
			_aux[1] = command_str[2];		// '0' in uint8_t is 48. ASCII
			_aux[2] = '\0';
			statusCommand = atoi(_aux);
			ESP_LOGI(TAG_ACIONNA, "opcode: %d, handleMessage(), statusCommand: %d!", opcode, statusCommand);

			switch (statusCommand)
			{
				case 0:	{ // $00; Check basic parameters{
					if(command_str[3]==';')
					{
						// int a = dt.getHour();
						// sprintf(buffer, "%d", dt.getHour());
						sprintf(buffer, "%.2d:%.2d:%.2d %.2d/%.2d/%.4d up:%.2d:%.2d:%.2d d:%d, s:%d m:%d\n tday:%d", dt.getHour(),
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
					} else if((command_str[3] == ':') && (command_str[5] == ';'))
					{
						_aux[0] = '0';
						_aux[1] = command_str[4];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						int statusCommand2 = atoi(_aux);

						if(statusCommand2)
							signal_json_data_back = 1;
						else
							signal_json_data_back = 0;
					}
					break;
				}
				case 1: {
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
					
					break;
				}
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
					else if((command_str[3] == ':') && (command_str[4] == 's') && (command_str[5] == ':') && (command_str[8] == ';'))
					{
						_aux[0] = command_str[6];
						_aux[1] = command_str[7];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						pipe1_.pressure_max = atoi(_aux);

						sprintf(buffer, "set press max: %d\n", pipe1_.pressure_max); 
					} // 	$03:s:72;	- Set max pressure ref for pipe [m.c.a.];
					else if((command_str[3] == ':') && (command_str[4] == 's') && (command_str[5] == ';')) {
						sprintf(buffer, "press max: %d\n", pipe1_.pressure_max); 
					}
					else if((command_str[3] == ':') && (command_str[4] == 'm') && (command_str[5] == ':') && (command_str[7] == ';'))
					{
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
					else if((command_str[3] == ':') && (command_str[4] == 't') && (command_str[5] == ':') && (command_str[9] == ';'))
					{
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
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == ':') && (command_str[9] == ';'))
					{
						_aux2[0] = '0';
						_aux2[1] = command_str[6];
						_aux2[2] = command_str[7];		// '0' in uint8_t is 48. ASCII
						_aux2[3] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';

						pipe1_.pressure_max = atoi(_aux2);

						sprintf(buffer, "set press max ref: %d\n", pipe1_.pressure_max);
					} // 	$03:p:100;	- Set 100 psi the max pressure of sensor;
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == ';'))
					{
						sprintf(buffer, "sens press max ref: %d\n", pipe1_.pressure_max);
					} // 	$03:p:100;	- Set 100 psi the max pressure of sensor;
					break;


		// 	$03:v:32;	- Set pressure for valve load turn on and fill reservoir;
		// 	$03:p:150;	- Set sensor max pressure ref to change the scale [psi];
		// 	$03:b:85;	- Set to 85% the pressure min bellow the current pressure to avoid pipe broken;
		// 	
				}
				case 8: { // $08;
					if(command_str[3]==';')
					{
						signal_request_sensors = 1;
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
		case 1: { // $1:h:063015; set time clock to 06:30:15
			if((command_str[2] == ':') && (command_str[3] == 'h') && (command_str[4] == ':') && (command_str[11] == ';'))
			{
				// uint8_t hour, min, sec;
				_aux[0] = command_str[5];
				_aux[1] = command_str[6];
				_aux[2] = '\0';
				dt.setHour(atoi(_aux), ND);

				_aux[0] = command_str[7];
				_aux[1] = command_str[8];
				_aux[2] = '\0';
				dt.setMinute(static_cast<uint8_t>(atoi(_aux)));

				_aux[0] = command_str[9];
				_aux[1] = command_str[10];
				_aux[2] = '\0';
				dt.setSecond(static_cast<uint8_t>(atoi(_aux)));

				// dt.setTime(0, 0, 0, ND);
				device_clock.set_time(dt.getUnixTime());
				time_day_sec_ = dt.getHour()*3600 + dt.getMinute()*60 + dt.getSecond();

				sprintf(buffer, "Time: %.2d:%.2d:%.2d\n", dt.getHour(), dt.getMinute(), dt.getSecond());
			} else if ((command_str[2] == ':') && (command_str[3] == 'd') && (command_str[4] == ':') && (command_str[13] == ';')) {
								// uint8_t hour, min, sec;
				_aux[0] = command_str[5];
				_aux[1] = command_str[6];
				_aux[2] = '\0';
				dt.setDay(atoi(_aux));

				_aux[0] = command_str[7];
				_aux[1] = command_str[8];
				_aux[2] = '\0';
				dt.setMonth(static_cast<uint8_t>(atoi(_aux)));

				_aux2[0] = command_str[9];
				_aux2[1] = command_str[10];
				_aux2[2] = command_str[11];
				_aux2[3] = command_str[12];
				_aux2[4] = '\0';
				dt.setYear(static_cast<uint16_t>(atoi(_aux2)));

				// dt.setTime(0, 0, 0, ND);
				device_clock.set_time(dt.getUnixTime());

				sprintf(buffer, "Date:%.2d/%.2d/%.4d\n", dt.getDay(), dt.getMonth(), dt.getYear());
			} // $1:d:10082022;
			break;
		}
		case 2: {
			if((command_str[2] == '0') && (command_str[3] == ';'))
			{
				// signal.RSSI_show == states_flag::enable;
			}
			// show RSSI
			break;
		}
		case 3: { //
			_aux[0] = '0';
			_aux[1] = command_str[2];		// '0' in uint8_t is 48. ASCII
			_aux[2] = '\0';
			statusCommand = atoi(_aux);

			switch (statusCommand)
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
		case 5: { // $5:h1:0900;
			if(command_str[2] == ';') {	// $5;
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
			else if((command_str[2] == ':') && (command_str[4] == ';')) {
				_aux[0] = '0';
				_aux[1] = command_str[3];		// '0' in uint8_t is 48. ASCII
				_aux[2] = '\0';
				int status_set = atoi(_aux);

				if(status_set)
					flag_check_time_match_ = states_flag::enable;
				else
					flag_check_time_match_ = states_flag::disable;

				sprintf(buffer, "set auto tm: %d\n", (int)flag_check_time_match_);
			} // $5:[1|0];
			else if((command_str[2] == ':') && (command_str[3] == 'h') && (command_str[5] == ':') && (command_str[10] == ';')) {
				// $5:h1:2130; set time 1
				_aux[0] = '0';
				_aux[1] = command_str[4];		// '0' in uint8_t is 48. ASCII
				_aux[2] = '\0';
				int index = atoi(_aux)-1;

				_aux[0] = command_str[6];
				_aux[1] = command_str[7];		// '0' in uint8_t is 48. ASCII
				_aux[2] = '\0';
				uint32_t time_temp = (static_cast<uint32_t>(atoi(_aux)))*3600;

				_aux[0] = command_str[8];
				_aux[1] = command_str[9];		// '0' in uint8_t is 48. ASCII
				_aux[2] = '\0';
				time_temp += (static_cast<uint32_t>(atoi(_aux)))*60;

				time_match_list[index] = time_temp;

				sprintf(buffer, "set h%d %.2d:%.2d, t:%d\n", index+1, (int)timesec_to_hour(time_match_list[index]), (int)timesec_to_min(time_match_list[index]), time_match_list[index]);
			} // set $5:n:4; set n to turn 4 times;
			else if((command_str[2] == ':') && (command_str[3] == 'n') && (command_str[4] == ':') && (command_str[6] == ';')) {
				_aux[0] = '0';
				_aux[1] = command_str[5];		// '0' in uint8_t is 48. ASCII
				_aux[2] = '\0';
				int status_set = atoi(_aux);

				time_match_n = status_set;

				sprintf(buffer, "set n: %d\n", time_match_n);
			} // set $5:n;	show n number
			else if((command_str[2] == ':') && (command_str[3] == 'n') && (command_str[4] == ';')) {
				sprintf(buffer, "n: %d\n", time_match_n);
			} // $5:n;
			else if((command_str[2] == ':') && (command_str[3] == 'm') && (command_str[5] == ':') && (command_str[7] == ';')) {
				// $5:m1:1;
				_aux[0] = '0';
				_aux[1] = command_str[4];		// '0' in uint8_t is 48. ASCII
				_aux[2] = '\0';
				int index = atoi(_aux);

				_aux[0] = '0';
				_aux[1] = command_str[6];		// '0' in uint8_t is 48. ASCII
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
			} // $5:m[1-9]:[1-4]
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
			_aux[0] = '0';
			_aux[1] = command_str[6];		// '0' in uint8_t is 48. ASCII
			_aux[2] = '\0';
			int status_set = atoi(_aux);

			if (command_str[2] == ':')
			{
				if((command_str[3] == 'r') && (command_str[4] == 't'))
				{
					if((command_str[5] == ':') && (command_str[7] == ';'))
					{
						if(status_set)
							flag_check_thermal_relay_ = states_flag::enable;
						else
							flag_check_thermal_relay_ = states_flag::disable;

						sprintf(buffer, "set check Rth: %d\n", (int)flag_check_thermal_relay_);
					}
					else if(command_str[5] == ';')
					{
						sprintf(buffer, "check Rth: %d\n", (int)flag_check_thermal_relay_);
					}
				} // $7:rt:1|0;			- habilita/desabilita proteção com relé térmico;
				else if ((command_str[3] == 'k') && (command_str[4] == 't')) {
					if((command_str[5] == ':') && (command_str[7] == ';'))
					{
						if(status_set)
							pump1_.flag_check_output_pin_only = states_flag::enable;
						else
							pump1_.flag_check_output_pin_only = states_flag::disable;

						sprintf(buffer, "set check k1: %d\n", (int)pump1_.flag_check_output_pin_only);

					} else if (command_str[5] == ';')
					{
						sprintf(buffer, "check k: %d\n", (int)pump1_.flag_check_output_pin_only);
					}				
				} // $7:kt:1|0;
				else if ((command_str[3] == 'k') && (command_str[4] == '1'))
				{
					if((command_str[5] == ':') && (command_str[7] == ';'))
					{
						if(status_set)
							flag_check_k1_ = states_flag::enable;
						else
							flag_check_k1_ = states_flag::disable;

						sprintf(buffer, "set check k1: %d\n", (int)flag_check_k1_);
					}
					else if (command_str[5] == ';')
					{
						sprintf(buffer, "check k1: %d\n", (int)flag_check_k1_);
					}
				} // $7:k1:1|0;			- contator K1;
				else if((command_str[3] == 'k') && (command_str[4] == '2'))
				{
					if((command_str[5] == ':') && (command_str[7] == ';'))
					{
						if(status_set)
							flag_check_k2_ = states_flag::enable;
						else
							flag_check_k2_ = states_flag::disable;

						sprintf(buffer, "set check k2: %d\n", (int)flag_check_k2_);
					}
					else if (command_str[5] == ';')
					{
						sprintf(buffer, "check k2: %d\n", (int)flag_check_k2_);
					}
				}// $7:k2:1|0;			- contator K2;
				else if((command_str[3] == 'k') && (command_str[4] == '3'))
				{
					if((command_str[5] == ':') && (command_str[7] == ';'))
					{
						if(status_set)
							flag_check_k3_ = states_flag::enable;
						else
							flag_check_k3_ = states_flag::disable;

						sprintf(buffer, "set check k3: %d\n", (int)flag_check_k3_);
					}
					else if(command_str[5] == ';')
					{
						sprintf(buffer, "check k3: %d\n", (int)flag_check_k3_);
					}
				}// $7:k3:1|0;			- contator K3;
				else if((command_str[3] == 'p') && (command_str[4] == 'h'))
				{
					if((command_str[5] == ':') && (command_str[7] == ';'))
					{
						if(status_set)
							flag_check_pressure_high_ = states_flag::enable;
						else
							flag_check_pressure_high_ = states_flag::disable;

						sprintf(buffer, "set check press high: %d\n", (int)flag_check_pressure_high_);
					}
					else if(command_str[5] == ';')
					{
						sprintf(buffer, "check press high: %d\n", (int)flag_check_pressure_high_);
					}
				} // $7:ph:1|0;			- desligamento por alta pressão;
				else if ((command_str[3] == 'p') && (command_str[4] == 'l'))
				{
					if((command_str[5] == ':') && (command_str[7] == ';'))
					{
						if(status_set)
							flag_check_pressure_low_ = states_flag::enable;
						else
							flag_check_pressure_low_ = states_flag::disable;

						sprintf(buffer, "set check press low: %d\n", (int)flag_check_pressure_low_);
					}
					else if (command_str[5] == ';')
					{
						sprintf(buffer, "check press low: %d\n", (int)flag_check_pressure_low_);
					}
				} // $7:pl:0-9;			- desligamento por pressão baixa em min caso seja diferente de 0;
				else if ((command_str[3] == 'p') && (command_str[4] == 'v'))
				{
					if((command_str[5] == ':') && (command_str[7] == ';'))
					{
						if(status_set)
							flag_check_pressure_valve_ = states_flag::enable;
						else
							flag_check_pressure_valve_ = states_flag::disable;

						sprintf(buffer, "set check press valve: %d\n", (int)flag_check_pressure_valve_);
					}
					else if (command_str[5] == ';')
					{
						sprintf(buffer, "check press valve: %d\n", (int)flag_check_pressure_valve_);
					}
				} // $7:pv:1|0;			- desligamento por pressão alta por válvula;
				else
				{
					sprintf(buffer, "7: command not implemented\n");
				}
			}
			break;
		// $7:__:			- Habilita/desabilita verificação de
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
						} // $8:01:[0|1];		- desaciona|aciona válvula 01;
						else if((command_str[6] == 'i') && (command_str[7] == ';'))
						{
							valves1_.set_program_add(valve_id);
							sprintf(buffer, "added valve %d: %d\n", valve_id, (int)valves1_.get_program_status(valve_id));
						} // $8:01:i;		- insere setor na programação;
						else if((command_str[6] == 'r') && (command_str[7] == ';'))
						{
							valves1_.set_program_remove(valve_id);
							sprintf(buffer, "removed valve %d: %d\n", valve_id, (int)valves1_.get_program_status(valve_id));
						} // $8:01:r;		- remove setor da programação;
						else if ((command_str[6] == 't') && (command_str[7] == ';'))
						{
							int valve_time = valves1_.get_valve_time(valve_id);
							sprintf(buffer, "valve %d: %d min\n", valve_id, valve_time);
						} // $8:01:t;		- mostra o tempo de irrigação do setor;
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
						} // $8:01:t:120;	- configura o tempo de irrigação [min];
						else if((command_str[6] == 'p') && (command_str[7] == ';'))
						{
							sprintf(buffer, "get valve %d: %d m.c.a.\n", valve_id, valves1_.get_valve_pressure(valve_id));
						} // $8:01:p;		- mostra pressão nominal do setor
						else if((command_str[6] == 'p') && (command_str[7] == ':') && (command_str[10] == ';'))
						{
							_aux[0] = command_str[8];
							_aux[1] = command_str[9];
							_aux[2] = '\0';
							unsigned int valve_pressure = (unsigned int) atoi(_aux);
							valves1_.set_valve_pressure(valve_id, valve_pressure);
							sprintf(buffer, "set valve %d: %d m.c.a.\n", valve_id, valves1_.get_valve_pressure(valve_id));
						} // $8:01:p:60;	- configura pressão nominal do setor [m.c.a.];
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
			_aux[0] = '0';
			_aux[1] = command_str[2];		// '0' in uint8_t is 48. ASCII
			_aux[2] = '\0';
			statusCommand = atoi(_aux);
			ESP_LOGI(TAG_ACIONNA, "opcode: %d, handleMessage(), statusCommand: %d!", opcode, statusCommand);

			switch(statusCommand)
			{
				case 0: {
					if(command_str[3] == ';')
					{
						signal_wifi_info = 1;
						sprintf(buffer, "wifi info\n");
					} // $90;
					else if((command_str[3] == ':') && (command_str[4] == 's') && (command_str[5] == ':') && (command_str[7] == ';'))
					{
						_aux[0] = '0';
						_aux[1] = command_str[6];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						statusCommand = atoi(_aux);
						if(statusCommand)
						{
							signal_send_async = 1;
						}
						else
						{
							signal_send_async = 0;
						}
					}
					break;
				} // $90:s:1;
				case 1: {
					if(command_str[3] == ';')
					{
						signal_wifi_scan = 1;
						sprintf(buffer, "wifi scan\n");
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
				case 5: {
					signal_ota_update = 1;
					sprintf(buffer, "ota update\n");
					break;
					break;
				}
				case 6: {
					signal_ota_info = 1;
					sprintf(buffer, "ota info\n");
					break;
				}
				case 7: {
					signal_ram_usage = 1;
					sprintf(buffer, "ram usage\n");
					break;
				}
				case 8: {
					signal_reset_reason = 1;
					sprintf(buffer, "reset reason\n");
					break;
				}
				case 9: {
					signal_restart = 1;
					sprintf(buffer,"Restarting...\n");
					break;
				}
				default:
					sprintf(buffer,"opcode %d - sub not implemented\n", opcode);
					break;
			}
			break;
		}
		default:
			sprintf(buffer, "opcode not implemented\n");
			break;
	}
	ESP_LOGI(TAG_ACIONNA, "Buffer: %s", buffer);

	std::string str(buffer);
	return str;

// 
		// 								eeprom.write(eeprom.pageSet, eeprom.addr_standBy_min, waitPowerOn_min_standBy);
		// 								//eeprom_write_byte((uint8_t *)(addr_standBy_min), waitPowerOn_min_standBy);
		// //								Serial.print("powerOn min:");
		// //								Serial.println(powerOn_min_Standy);
		// 							}
		// 						}//$02:s:129;
		// 						else if(sInstr[2] == ':' && sInstr[3] == 's' && sInstr[4] == ':' && sInstr[8] == ';')
		// 						{
		// 							aux2[0] = '0';
		// 							aux2[1] = sInstr[5];
		// 							aux2[2] = sInstr[6];
		// 							aux2[3] = sInstr[7];
		// 							aux2[4] = '\0';
		// 							motorTimerE = (uint16_t) atoi(aux2);
		// 							eeprom.write(eeprom.pageSet, eeprom.addr_motorTimerE, motorTimerE);
		// //							uint8_t lbyteRef = 0, hbyteRef = 0;
		// //							lbyteRef = motorTimerE;
		// //							hbyteRef = (motorTimerE >> 8);
		// //							eeprom_write_byte((uint8_t *)(addr_motorTimerE+1), hbyteRef);
		// //							eeprom_write_byte((uint8_t *)(addr_motorTimerE), lbyteRef);
		// 						}
		// 						summary_Print(statusCommand);
		// 					}
		// 					break;
		// 					// ------------------------------
		// 					case 3:// $03:s:68;
		// 						if(sInstr[2]==':' && sInstr[3] == 's' && sInstr[4] == ':' && sInstr[7] == ';')
		// 						{
		// 							aux[0] = sInstr[5];
		// 							aux[1] = sInstr[6];
		// 							aux[2] = '\0';
		// 							PRessureRef = (uint8_t) atoi(aux);
		// 							eeprom.write(eeprom.pageSet, eeprom.addr_PRessureRef, PRessureRef);
		// 							//eeprom_write_byte((uint8_t *)(addr_PRessureRef), PRessureRef);
		// 							sprintf(Serial.buffer,"PRessRef: %d", PRessureRef);
		// 							Serial.println(Serial.buffer);
		// 						}
		// 						else if(sInstr[2]==':' && sInstr[3] == 'v' && sInstr[4] == ':' && sInstr[7] == ';')
		// 						{
		// 							aux[0] = sInstr[5];
		// 							aux[1] = sInstr[6];
		// 							aux[2] = '\0';
		// 							PRessureRef_Valve = (uint8_t) atoi(aux);
		// 							eeprom.write(eeprom.pageSet, eeprom.addr_PRessureRef_Valve, PRessureRef_Valve);
		// 							//eeprom_write_byte((uint8_t *)(addr_PRessureRef_Valve), PRessureRef_Valve);
		// 							sprintf(Serial.buffer,"PRessRef_Valve: %d", PRessureRef_Valve);
		// 							Serial.println(Serial.buffer);
		// 						}
		// 						else if(sInstr[2]==':' && sInstr[3] == 'p' && sInstr[4] == ':' && sInstr[8] == ';')
		// 						{// $03:p:150;
		// 							aux2[0] = '0';
		// 							aux2[1] = sInstr[5];
		// 							aux2[2] = sInstr[6];
		// 							aux2[3] = sInstr[7];
		// 							aux2[4] = '\0';
		// 							PRessureMax_Sensor = (uint8_t) atoi(aux2);
		// 							eeprom.write(eeprom.pageSet, eeprom.addr_PRessureMax_Sensor, PRessureMax_Sensor);
		// //							eeprom_write_byte((uint8_t *)(addr_PRessureMax_Sensor), PRessureMax_Sensor);
		// 						}
		// 						else if(sInstr[2]==':' && sInstr[3] == 'b' && sInstr[4] == ':' && sInstr[7] == ';')
		// 						{
		// 							aux[0] = sInstr[5];
		// 							aux[1] = sInstr[6];
		// 							aux[2] = '\0';
		// 							PRessurePer = (uint8_t) atoi(aux);
		// 							eeprom.write(eeprom.pageSet, eeprom.addr_PRessurePer, PRessurePer);
		// 							//eeprom_write_byte((uint8_t *)(addr_PREssurePer), PRessurePer);
		// 						}
		// 						else
		// 							summary_Print(statusCommand);
		// 						break;
		// 					// ------------------------------
		// 					case 4: // $04:1;
		// 						if(sInstr[2]==':' && sInstr[4]==';')
		// 						{
		// 							aux[0] = '0';
		// 							aux[1] = sInstr[3];
		// 							aux[2] = '\0';
		// 							flag_debug = (uint8_t) atoi(aux);
		// 						}//$04:s:0900;
		// 						else if(sInstr[2]==':' && sInstr[3]=='s' && sInstr[4]==':' && sInstr[9]==';')
		// 						{
		// 							aux2[0] = sInstr[5];
		// 							aux2[1] = sInstr[6];
		// 							aux2[2] = sInstr[7];
		// 							aux2[3] = sInstr[8];
		// 							aux2[4] = '\0';

		// 							levelRef_10bit = (uint16_t) atoi(aux2);

		// //							uint8_t lbyteRef = 0, hbyteRef = 0;
		// //							lbyteRef = levelRef_10bit;
		// //							hbyteRef = (levelRef_10bit >> 8);

		// 							eeprom.write(eeprom.pageSet, eeprom.addr_LevelRef, levelRef_10bit);

		// 							//eeprom_write_byte((uint8_t *)(addr_LevelRef+1), hbyteRef);
		// 							//eeprom_write_byte((uint8_t *)(addr_LevelRef), lbyteRef);
		// 						}
		// 						summary_Print(statusCommand);
		// 						sprintf(Serial.buffer,"Ref: %d", levelRef_10bit);
		// 						Serial.println(Serial.buffer);
		// 						break;
		// 					// ------------------------------
		// 					case 7:
		// 						if(sInstr[2]==':' && sInstr[4]==';')
		// 						{
		// 							aux[0] = '0';
		// 							aux[1] = sInstr[3];
		// 							aux[2] = '\0';
		// 							uint8_t adcCommand = (uint8_t) atoi(aux);

		// 							switch (adcCommand)
		// 							{
		// //								case 0:
		// //									ADMUX &=  ~(1<<REFS1);		// AREF, Internal Vref turned off
		// //									ADMUX &=  ~(1<<REFS0);
		// //									Serial.println("AREF");
		// //									break;
		// //
		// //								case 1:
		// //									ADMUX &=  ~(1<<REFS1);		// AVCC with external capacitor at AREF pin
		// //									ADMUX |=   (1<<REFS0);
		// //									Serial.println("AVCC");
		// //									break;
		// //
		// //								case 2:
		// //									ADMUX |=   (1<<REFS1);		// Internal 1.1V Voltage Reference with external capacitor at AREF pin
		// //									ADMUX |=   (1<<REFS0);
		// //									Serial.println("1.1V");
		// //									break;
		// 							}
		// 						}
		// 						break;
		// 					// ------------------------------
		// 					case 9:
		// 						Serial.println("Rebooting...");
		// 						NVIC_SystemReset();
		// 						//wdt_enable(WDTO_15MS);
		// //						flag_reset = 1;
		// 						break;

		// 					default:
		// 						summary_Print(statusCommand);
		// 						break;
		// 				}
		// 			}
		// 			break;
		// // -----------------------------------------------------------------
		// 			case 1: 	// Setup calendar
		// 			{
		// 				// Set-up clock -> $1:h:HHMMSS;
		// 				if(sInstr[1]==':' && sInstr[2]=='h' && sInstr[3]==':' && sInstr[10]==';')
		// 				{
		// 					aux[0] = sInstr[4];
		// 					aux[1] = sInstr[5];
		// 					aux[2] = '\0';
		// 					tm.Hour = (uint8_t) atoi(aux);

		// 					aux[0] = sInstr[6];
		// 					aux[1] = sInstr[7];
		// 					aux[2] = '\0';
		// 					tm.Minute = (uint8_t) atoi(aux);

		// 					aux[0] = sInstr[8];
		// 					aux[1] = sInstr[9];
		// 					aux[2] = '\0';
		// 					tm.Second = (uint8_t) atoi(aux);

		// //					RTC.write(tm);
		// 					rtc.write(tm);
		// 					summary_Print(0);
		// 				}
		// 				// 	Set-up date -> $1:d:DDMMAAAA;
		// 				else if(sInstr[1]==':' && sInstr[2]=='d' && sInstr[3]==':' && sInstr[12]==';')
		// 				{
		// 					// Getting the parameters
		// 					aux[0] = sInstr[4];
		// 					aux[1] = sInstr[5];
		// 					aux[2] = '\0';
		// 					tm.Day = (uint8_t) atoi(aux);

		// 					aux[0] = sInstr[6];
		// 					aux[1] = sInstr[7];
		// 					aux[2] = '\0';
		// 					tm.Month = (uint8_t) atoi(aux);

		// 					char aux2[5];
		// 					aux2[0] = sInstr[8];
		// 					aux2[1] = sInstr[9];
		// 					aux2[2] = sInstr[10];
		// 					aux2[3] = sInstr[11];
		// 					aux2[4] = '\0';
		// 					tm.Year = (uint8_t) (atoi(aux2)-1970);

		// //					RTC.write(tm);
		// 					rtc.write(tm);

		// 					summary_Print(0);

		// 				}
		// 				// Set-up date -> $1:d:DDMMAAAA;
		// 				else if(sInstr[1]==':' && sInstr[2]=='c' && sInstr[3]==':' && sInstr[9]==';')
		// 				{	// $1:c:40123;
		// 					char aux3[6];
		// 					aux3[0] = sInstr[4];
		// 					aux3[1] = sInstr[5];
		// 					aux3[2] = sInstr[6];
		// 					aux3[3] = sInstr[7];
		// 					aux3[4] = sInstr[8];
		// 					aux3[5] = '\0';
		// 					rtc.rtc_PRL = (uint32_t) atoi(aux3);

		// 					rtc.setRTC_DIV(rtc.rtc_PRL);
		// 					sprintf(Serial.buffer,"PRLw: %ld", rtc.rtc_PRL);
		// 					Serial.println(Serial.buffer);

		// 					eeprom.write(eeprom.pageSet, eeprom.addr_rtc_PRL, (uint16_t) rtc.rtc_PRL);

		// 					sprintf(Serial.buffer,"PRLr: %ld", rtc.getRTC_DIV());
		// 					Serial.println(Serial.buffer);
		// 				}
		// 				else if(sInstr[1]==':' && sInstr[2]=='c' && sInstr[3]==';')
		// 				{
		// 					sprintf(Serial.buffer,"PRLreg: %lu, PRLflash: %u", rtc.getRTC_DIV(), eeprom.read(eeprom.pageSet, eeprom.addr_rtc_PRL));
		// 					Serial.println(Serial.buffer);
		// 				}
		// 				// $1:s:S;
		// 				else if(sInstr[1]==':' && sInstr[2]=='s' && sInstr[3]==':' && sInstr[5]==';')
		// 				{
		// 					aux[0] = '0';
		// 					aux[1] = sInstr[4];
		// 					aux[2] = '\0';
		// 					rtc.rtc_clkSource = (uint8_t) atoi(aux);

		// 					eeprom.write(eeprom.pageSet, eeprom.addr_rtc_clkSource, rtc.rtc_clkSource);
		// 					rtc.bkpDomainReset();
		// 					rtc.begin_rtc(rtc.rtc_clkSource, rtc.rtc_PRL);

		// 					sprintf(Serial.buffer,"rtcSource: %u", rtc.rtc_clkSource);
		// 					Serial.println(Serial.buffer);
		// 				}
		// 				else if(sInstr[1]==':' && sInstr[2]=='s' && sInstr[3]==';')
		// 				{
		// 					eeprom.read(eeprom.pageSet, eeprom.addr_rtc_clkSource);

		// 					summary_Print(0);
		// 					sprintf(Serial.buffer,"rtcSource: %u", rtc.rtc_clkSource);
		// 					Serial.println(Serial.buffer);
		// 				}
		// 			}
		// 			break;
		// // -----------------------------------------------------------------
		// 			case 2:		// Setup Bluetooth device name
		// 			{
		// 				// Setup clock -> $2:BluetoothName;
		// 				char aux_str[sInstr_SIZE], aux_str2[sInstr_SIZE+7];
		// 				uint8_t k1=0;
		// 				if(sInstr[1]==':')
		// 				{
		// 					// 3 because 2 and : count 2 characters and one more for '\0'
		// 					while((k1<sInstr_SIZE-3) && sInstr[k1] != ';')
		// 					{
		// 						aux_str[k1] = sInstr[k1+2];
		// 						k1++;
		// //						Serial.println(k1);
		// 					}

		// 					aux_str[k1-2] = '\0';
		// 					Serial.println("Disconnect!");
		// //					wdt_reset();
		// //					_delay_ms(3000);
		// //					wdt_reset();
		// //					_delay_ms(3000);
		// //					wdt_reset();
		// //					_delay_ms(3000);
		// //					strcpy(aux_str2,"AT");
		// //					Serial.print(aux_str2);
		// 					//wdt_reset();
		// 					_delay_ms(3000);
		// 					strcpy(aux_str2,"AT+NAME");
		// 					strcat(aux_str2,aux_str);
		// 					Serial.print(aux_str2);
		// 					//wdt_reset();
		// 					_delay_ms(1000);
		// 				}
		// 			}
		// 			break;
		// // -----------------------------------------------------------------
		// 			case 3:		// Set motor ON/OFF
		// 			{
		// 				uint8_t motorCommand;
		// 				aux[0] = '0';
		// 				aux[1] = sInstr[1];
		// 				aux[2] = '\0';
		// 				motorCommand = (uint8_t) atoi(aux);

		// 				if (motorCommand && (!motorStatus))
		// 				{
		// 					motor_start();
		// 					Serial.println("value");
		// 				}
		// 				else
		// 				{
		// 					motor_stop(0x06);
		// 				}

		// 				summary_Print(3);
		// 			}
		// 			break;
		// // -----------------------------------------------------------------
		// 			case 4: 	// FLASH store test, //$4:r:23; and $4:w:23:03;
		// 			{
		// 				if(sInstr[1]==':' && sInstr[2]=='r' && sInstr[3]==':' && sInstr[6]==';')
		// 				{	// $4:r:05; read 05*2 address of page block
		// 					// Getting the parameters
		// 					aux[0] = sInstr[4];
		// 					aux[1] = sInstr[5];
		// 					aux[2] = '\0';
		// 					uint8_t addrt = (uint8_t) atoi(aux);
		// 					uint8_t var = eeprom.read(eeprom.pageSet, addrt);
		// 					sprintf(Serial.buffer,"EE read: %d ", var);
		// 					Serial.println(Serial.buffer);

		// 					sprintf(Serial.buffer,"Page %d ", eeprom.pageSet);
		// 					Serial.println(Serial.buffer);

		// 					sprintf(Serial.buffer,"Address %X ", (unsigned int) eeprom.pageAddress(eeprom.pageSet));
		// 					Serial.println(Serial.buffer);
		// 				}
		// 				if(sInstr[1]==':' && sInstr[2]=='w' && sInstr[3]==':' && sInstr[6]==':' && sInstr[9]==';')
		// 				{	// $4:w:03:07;
		// 					// Getting the parameters
		// 					aux[0] = sInstr[4];
		// 					aux[1] = sInstr[5];
		// 					aux[2] = '\0';
		// 					uint8_t addrt = (uint8_t) atoi(aux);

		// 					aux[0] = sInstr[7];
		// 					aux[1] = sInstr[8];
		// 					aux[2] = '\0';
		// 					uint8_t var = (uint8_t) atoi(aux);

		// 					eeprom.write(eeprom.pageSet, addrt, var);
		// 					sprintf(Serial.buffer,"EE write: %d ", var);
		// 					Serial.println(Serial.buffer);

		// 					var = eeprom.read(eeprom.pageSet, addrt);
		// 					sprintf(Serial.buffer,"EE read2: %d ", var);
		// 					Serial.println(Serial.buffer);

		// 					sprintf(Serial.buffer,"Page %d ", eeprom.pageSet);
		// 					Serial.println(Serial.buffer);

		// 					sprintf(Serial.buffer,"Address %X ", (unsigned int) eeprom.pageAddress(eeprom.pageSet));
		// 					Serial.println(Serial.buffer);
		// 				}
		// 				if(sInstr[1]==':' && sInstr[2]=='f' && sInstr[3]==':' && sInstr[6]==':' && sInstr[9]==';')
		// 				{	// $4:f:64:07;	fill page 64 with 07 value;
		// 					// Getting the parameters
		// 					aux[0] = sInstr[4];
		// 					aux[1] = sInstr[5];
		// 					aux[2] = '\0';
		// 					uint8_t page = (uint8_t) atoi(aux);

		// 					aux[0] = sInstr[7];
		// 					aux[1] = sInstr[8];
		// 					aux[2] = '\0';
		// 					uint8_t var = (uint8_t) atoi(aux);



		// 					eeprom.writePage(page, (var << 8 | var));
		// 					sprintf(Serial.buffer,"Filled page %d ", page);
		// 					Serial.println(Serial.buffer);

		// 					sprintf(Serial.buffer,"Address %X ", (unsigned int) eeprom.pageAddress(page));
		// 					Serial.println(Serial.buffer);
		// 				}
		// 				if(sInstr[1]==':' && sInstr[2]=='e' && sInstr[3]==':' && sInstr[6]==';')
		// 				{	// $4:e:64;	erase page 64
		// 					// Getting the parameters
		// 					aux[0] = sInstr[4];
		// 					aux[1] = sInstr[5];
		// 					aux[2] = '\0';
		// 					uint8_t page = (uint8_t) atoi(aux);

		// 					eeprom.erasePage(page);
		// 					sprintf(Serial.buffer,":Page %d erased!", page);
		// 					Serial.println(Serial.buffer);

		// 					sprintf(Serial.buffer,"Address %X ", (unsigned int) eeprom.pageAddress(page));
		// 					Serial.println(Serial.buffer);
		// 				}
		// 				break;
		// 			}
		// // -----------------------------------------------------------------
		// 			case 5: 	// Command is $5:h1:2130;
		// 			{
		// 				if(sInstr[1]==':' && sInstr[2]=='h' && sInstr[4]==':' && sInstr[9]==';')
		// 				{
		// 					aux[0] = '0';
		// 					aux[1] = sInstr[3];
		// 					aux[2] = '\0';
		// 					uint8_t indexV = (uint8_t) atoi(aux);

		// 					aux[0] = sInstr[5];
		// 					aux[1] = sInstr[6];
		// 					aux[2] = '\0';
		// 					HourOnTM[indexV-1] = (uint8_t) atoi(aux);
		// 					eeprom.write(eeprom.pageSet, eeprom.addr_HourOnTM+indexV-1, HourOnTM[indexV-1]);
		// 					//eeprom_write_byte(( uint8_t *)(addr_HourOnTM+indexV-1), HourOnTM[indexV-1]);

		// 					aux[0] = sInstr[7];
		// 					aux[1] = sInstr[8];
		// 					aux[2] = '\0';
		// 					MinOnTM[indexV-1] = (uint8_t) atoi(aux);
		// 					eeprom.write(eeprom.pageSet, eeprom.addr_MinOnTM+indexV-1, MinOnTM[indexV-1]);
		// //					eeprom_write_byte(( uint8_t *)(addr_MinOnTM+indexV-1), MinOnTM[indexV-1]);

		// 					summary_Print(5);
		// 				}
		// 				else if(sInstr[1]==':' && sInstr[2]=='n' && sInstr[3]==':' && sInstr[5]==';')
		// 				{
		// 					aux[0] = '0';
		// 					aux[1] = sInstr[4];
		// 					aux[2] = '\0';

		// 					nTM = (uint8_t) atoi(aux);
		// 					eeprom.write(eeprom.pageSet, eeprom.addr_nTM, nTM);
		// 					//eeprom_write_byte(( uint8_t *)(addr_nTM), nTM);

		// 					summary_Print(5);
		// 				}
		// 				else if(sInstr[1]==';')
		// 				{
		// 					summary_Print(5);
		// 				}
		// 			}
		// 			break;
		// // ----------------------------------------------------------------
		// 			case 6:		// Set working mode
		// 			{
		// 				aux[0] = '0';
		// 				aux[1] = sInstr[1];
		// 				aux[2] = '\0';
		// 				stateMode = (uint8_t) atoi(aux);
		// 				eeprom.write(eeprom.pageSet, eeprom.addr_stateMode, stateMode);
		// //				//eeprom_write_byte(( uint8_t *)(addr_stateMode), stateMode);

		// 				summary_Print(0);
		// 			}
		// 			break;
		// // -----------------------------------------------------------------
		// 			case 7:		// nRF24L01p test functions;
		// 			{
		// 				// $7:s:
		// 				uint8_t state =9;
		// 				if(sInstr[1]==':' && sInstr[2]=='s' && sInstr[3]==';')
		// 				{
		// 					radio.begin_nRF24L01p();
		// 				}
		// 				else if(sInstr[1]==':' && sInstr[2]=='g' && sInstr[3]==';')
		// 				{
		// 					state = radio.get_stateMode();
		// 				}
		// 				else if(sInstr[1]==':' && sInstr[2]=='t' && sInstr[3]==';')
		// 				{
		// 					state = radio.set_mode_tx(ENABLE);
		// 				}
		// 				else if(sInstr[1]==':' && sInstr[2]=='r' && sInstr[3]==';')
		// 				{
		// 					state = radio.set_mode_rx(ENABLE);
		// 				}
		// 				else if(sInstr[1]==':' && sInstr[2]=='b' && sInstr[3]==';')
		// 				{
		// 					state = radio.set_mode_standbyI();
		// 				}
		// 				else if(sInstr[1]==':' && sInstr[2]=='p' && sInstr[3]==';')
		// 				{
		// 					state = radio.set_mode_powerDown();
		// 				}

		// 				sprintf(Serial.buffer,"state: %u", state);
		// 				Serial.println(Serial.buffer);

		// //				radio.begin_nRF24L01p();
		// //				radio.set_250kbps();
		// //				if(sInstr[1]==':' && sInstr[2]=='h' && sInstr[4]==':' && sInstr[9]==';')
		// //				{
		// //					aux[0] = '0';
		// //					aux[1] = sInstr[3];
		// //					aux[2] = '\0';
		// //					uint8_t indexV = (uint8_t) atoi(aux);
		// //
		// //					aux[0] = sInstr[5];
		// //					aux[1] = sInstr[6];
		// //					aux[2] = '\0';
		// //					HourOnTM[indexV-1] = (uint8_t) atoi(aux);
		// //					eeprom.write(eeprom.pageSet, eeprom.addr_HourOnTM+indexV-1, HourOnTM[indexV-1]);
		// //					//eeprom_write_byte(( uint8_t *)(addr_HourOnTM+indexV-1), HourOnTM[indexV-1]);
		// //
		// //					aux[0] = sInstr[7];
		// //					aux[1] = sInstr[8];
		// //					aux[2] = '\0';
		// //					MinOnTM[indexV-1] = (uint8_t) atoi(aux);
		// //					eeprom.write(eeprom.pageSet, eeprom.addr_MinOnTM+indexV-1, MinOnTM[indexV-1]);
		// ////					eeprom_write_byte(( uint8_t *)(addr_MinOnTM+indexV-1), MinOnTM[indexV-1]);
		// //
		// //					summary_Print(5);
		// 			}
		// 			break;
		// // -----------------------------------------------------------------
		// 			default:
		// 				summary_Print(10);
		// 				break;
		// // -----------------------------------------------------------------
		// 		}
				// memset(sInstr_,0,sizeof(sInstr_));	// Clear all vector;
			// }
		// }
}
void ACIONNA::operation_mode() {
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
void ACIONNA::run() {
	// comm_Bluetooth();	// run into ws_setup.cpp

	// handle_message();	// handle message now is called when receive message into ws_setup.cpp

	update_all();		// update variables and rtc

	operation_mode();	// execution process
}
void ACIONNA::parser_1(uint8_t* payload_str, int payload_str_len, uint8_t *command_str, int& command_str_len)
{
	states_flag flag_instruction_write = states_flag::disable;

	int j = 0;												// aux counter;
	for(int i=0; i<payload_str_len; i++)
	{
		if (payload_str[i] == '$')								// found beginer of frame instruction
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

////void acn_motor_stop(uint8_t reason)
////{
////	int i;
////	for(i=(nLog-1);i>0;i--)
////	{
////		hourLog_OFF[i] = hourLog_OFF[i-1];
////		minuteLog_OFF[i] = minuteLog_OFF[i-1];
////
////		dayLog_OFF[i] = dayLog_OFF[i-1];
////		monthLog_OFF[i] = monthLog_OFF[i-1];
////	}
////
////	reasonV[1] = reasonV[0];
////	reasonV[0] = reason;
////
////	hourLog_OFF[0] = tm.Hour;
////	minuteLog_OFF[0] = tm.Minute;
////
////	dayLog_OFF[0] = tm.Day;
////	monthLog_OFF[0] = tm.Month;
////
////	timeOff_min = 0;
////	timeOff_sec = 0;
////
////	flag_waitPowerOn = 1;
////	waitPowerOn_min = waitPowerOn_min_standBy;
////
////	driveMotor_OFF();
////	drive_led_off();
//////	_delay_ms(1);
////
////	motorStatus = readPin_k1();
//////	sprintf(Serial.buffer,"offR:%d", reason);
//////	Serial.println(Serial.buffer);
////}
//void acn_motor_start()
//{
////	if(!flag_waitPowerOn && !k2_status)
////	{
////		int i;
////		for(i=(nLog-1);i>0;i--)
////		{
////			hourLog_ON[i] = hourLog_ON[i-1];
////			minuteLog_ON[i] = minuteLog_ON[i-1];
////
////			dayLog_ON[i] = dayLog_ON[i-1];
////			monthLog_ON[i] = monthLog_ON[i-1];
////		}
////
////		hourLog_ON[0] = tm.Hour;
////		minuteLog_ON[0] = tm.Minute;
////
////		dayLog_ON[0] = tm.Day;
////		monthLog_ON[0] = tm.Month;
////
////		timeOn_min = 0;
////		timeOn_sec = 0;
////
////		PRessHold = PRess;
////
////		driveMotor_ON(startTypeK);
////		motorStatus = readPin_k1();
////		drive_led_on();
////	}
//}
////void acn_driveMotor_ON(uint8_t startType)
////{
////	switch (startType)
////	{
////		case 1:	// Partida direta: monofásico
////			k1_on();
////			break;
////
////		case 2: // Partida direta: trifásico
////			k1_on();
////			k2_on();
////			break;
////
////		case 3:	// Partida estrela/triangulo
////			k1_on();
////			k3_on();
//////			_delay_ms(2000);
////			//wdt_reset();
////			_delay_ms(((double) 100.0*motorTimerStart1));
////			//wdt_reset();
////
////			k3_off();
//////			_delay_ms(500);
////			uint32_t countK = 0;
////			while(read_k3())
////			{
////				countK++;
////				if(countK>=250000)
////				{
////					k1_off();
////					k2_off();
////					k3_off();
////					return;
////				}
////			}
//////			Serial.println(count);
////			_delay_ms(motorTimerStart2);
////			k2_on();
////			break;
////	}
////}
////void acn_get_levelSensors()
////{
////	// Select ADC0 - LL sensor
////	levelSensorLL_d = adc_readChannel(pin_analog_LL);
////
////	if(levelSensorLL_d < levelRef_10bit)
////		levelSensorLL = 1;
////	else
////		levelSensorLL = 0;
////
////
////	// Select ADC1 - ML sensor
////	levelSensorML_d = adc_readChannel(pin_analog_ML);
////
////	if(levelSensorML_d < levelRef_10bit)
////		levelSensorML = 1;
////	else
////		levelSensorML = 0;
////
////
////	// Select ADC2 - HL sensor
////	levelSensorHL_d = adc_readChannel(pin_analog_HL);
////
////	if(levelSensorHL_d < levelRef_10bit)
////		levelSensorHL = 1;
////	else
////		levelSensorHL = 0;
////}
////double acn_get_Pressure()
////{
////	/*
////	Sensor details
////
////    Thread size : G 1/4" (BSP)
////    Sensor material:  Carbon steel alloy
////    Working voltage: 5 VDC
////    Output voltage: 0.5 to 4.5 VDC
////    Working Current: <= 10 mA
////    Working pressure range: 0 to  1.2 MPa
////    Maxi pressure: 2.4 MPa
////    Working temperature range: 0 to 100 graus C
////    Accuracy: +- 1.0%
////    Response time: <= 2.0 ms
////    Package include: 1 pc pressure sensor
////    Wires : Red---Power (+5V)  Black---Power (0V) - blue ---Pulse singal output
////
////
////    4.5 V___	   922___	1.2 MPa___	 12 Bar___	 120 m.c.a.___
////	  	  |				|			|			|				|
////	 	  |				|			|			|				|
////	 	  |				|			|			|				|
////	  out_|			Pd__|		  __|			|			Pa__|
////	 	  |				|			|			|				|
////	 	  |				|			|			|				|
////	 	  |				|			|			|				|
////		 _|_		   _|_		   _|_		   _|_			   _|_
////	0.5 V			103			0 MPa		0 Bar		0 m.c.a.
////
////	(out-0.5)/(4.5-0.5) = 1024
////
////	(out-0.0)/(5-0) = (x-0)/(1024-0)
////
////	(Pd - 103)/(922-103) = (Pa - 0)/(120 - 0)
////	Pa = 120.0*Pd/(1024.0);
////
////	(xs - 0) = temp - (0)
////	(255 - 0)  +50 - (0)
////
////	Direct Conversion
////	xs = 255*(temp+0)/51
////	tempNow_XS = (uint8_t) 255.0*(tempNow+0.0)/51.0;
////
////	Inverse Conversion
////	temp = (TempMax*xs/255) - TempMin
////	tempNow = (uint8_t) ((sTempMax*tempNow_XS)/255.0 - sTempMin);
////    */
////
////	const double Kpsi = 0.7030768118;
//////	const double PRessMax = 68.9475729;	// Sensor max pressure [m.c.a.] with 100 psi;
//////	const double PRessMax = 103.4212;	// Sensor max pressure [m.c.a.] with 150 psi;
//////	const double PRessMax = 120.658253;	// Sensor max pressure [m.c.a.] with 174.045 psi;
////
////	double PRessMax = Kpsi*((double) PRessureMax_Sensor);
////
////	int Pd = adc_readChannel(pin_analog_PRess);
////
//////	Pd = adc_readChannel(0);
//////	int  PRess1 = (int) ((PRessMax)*(Pd-102.4)/(921.6-102.4));
//////	sprintf(Serial.buffer, "Value: %d, Press: %d\n",Pd, PRess1);
//////	Serial.println(Serial.buffer);
////
////	return (PRessMax)*(Pd-102.4)/(921.6-102.4);
//////	(Pd - 103)/(922-103) = (Pa - 0)/(120 - 0);
////}
////void acn_process_valveControl()
////{
////	if(flag_timeMatch && (PRess >= PRessureRef_Valve))
////	{
////		flag_timeMatch &= 0;
////
////		if(!motorStatus)
////		{
////			motor_start();
////		}
////	}
////}
////void acn_process_waterPumpControl()
////{
////	/*
////	 * 0x01 - pressure
////	 * 0x02 - level
////	 * 0x03 - thermal
////	 * 0x04 - time out
////	 * 0x05 - red time
////	 * 0x06 - command line
////	 * 0x07 - broke pressure
////	 * */
////
//////	if(!levelSensorLL)
//////	{
//////		if(motorStatus)
//////		{
//////			motor_stop(0x02);
//////		}
//////	}
//////
//////	if(levelSensorHL && (stateMode == 4) && levelSensorLL)
//////	{
//////		if(!motorStatus)
//////		{
//////			motor_start();
//////		}
//////	}
////
////	if(flag_timeMatch && (stateMode != 0) && (stateMode != 4))
////	{
////		flag_timeMatch = 0;
////
////		if(!motorStatus)
////		{
////			motor_start();
////		}
////	}
////
//////	if(PRessureRef)					// Has a valid number mean this function is activated
//////	{
//////		if(PRess >= PRessureRef)
//////		{
//////			if(motorStatus)
//////			{
//////				motor_stop(0x01);
//////			}
//////		}
//////	}
//////
//////	if(stateMode == 5)
//////	{
//////		if(motorStatus && flag_PressureUnstable)
//////		{
//////			flag_PressureUnstable = 0;
//////			motor_stop(0x07);
//////		}
//////	}
////
////	if(flag_Th)
////	{
////		flag_Th = 0;
////
////		motor_stop(0x03);
////		stateMode = 0;
////	}
////}
////void acn_process_motorPeriodDecision()
////{
////	switch (periodo)
////	{
////	case redTime:
////		if(motorStatus)
////		{
////			motor_stop(0x05);
////		}
////		break;
////
////	case greenTime:
////		process_waterPumpControl();
////		break;
////	}
////}
////void acn_process_Mode()
////{
////	switch (stateMode)
////	{
////		case 0:	// System Down!
////			break;
////
////		case 1:	// Night Working;
////			process_motorPeriodDecision();
////			break;
////
////		case 2:	// For irrigation mode. Start in a programmed time.
////			process_waterPumpControl();
////			break;
////
////		case 3:	// For reservoir only. Works in a inverted pressured! Caution!
////			process_valveControl();
////			break;
////
////		case 4:	// Is that for a water pump controlled by water sensors. Do not use programmed time.
////			process_waterPumpControl();
////			break;
////
////		case 5:	// For irrigation mode and instantly low pressure turn motor off.
////			process_waterPumpControl();
////			break;
////
////		default:
////			stateMode = 0;
////			Serial.println("Standby");
////			break;
////	}
////
////	// maximum time drive keeps turned ON
////	if(motorTimerE)
////	{
////		if(motorStatus)
////		{
////			if(timeOn_min >= motorTimerE)
////			{
////				motor_stop(0x04);
////			}
////		}
////	}
////}
////void acn_summary_Print(uint8_t opt)
////{
////	switch (opt)
////	{
////		case 0:
////			sprintf(Serial.buffer,"Time:%.2d:%.2d:%.2d %.2d/%.2d/%.4d",tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year+1970);
////			Serial.print(Serial.buffer);
////
//////			sprintf(Serial.buffer," UP:%.2d:%.2d:%.2d, d:%d m:%d", hour(), minute(), second(), day()-1, month()-1);
////			sprintf(Serial.buffer," UP:%.2d:%.2d:%.2d, d:%d m:%d", rtc.rtc0->tm_hour, rtc.rtc0->tm_min, rtc.rtc0->tm_sec, rtc.rtc0->tm_mday-1, rtc.rtc0->tm_mon);
////			Serial.println(Serial.buffer);
////
////			sprintf(Serial.buffer,"P:%d k1Pin:%d Rth:%d k1:%d k2:%d k3:%d",periodo, motorStatus, Rth_status, k1_status, k2_status, k3_status);
////			Serial.println(Serial.buffer);
////
////			switch (stateMode)
////			{
////				case 0:
////					strcpy(Serial.buffer," Modo:Desligado");
////					break;
////
////				case 1:
////					sprintf(Serial.buffer," Modo:Liga Noite");
////					break;
////
////				case 2:
////					if(nTM == 1)
////					{
////						sprintf(Serial.buffer," Irrig: Liga %dx as %2.d:%.2d", nTM, HourOnTM[0], MinOnTM[0]);
////					}
////					else
////					{
////						sprintf(Serial.buffer," Irrig: Liga %dx/dia",nTM);
////					}
////					break;
////
////				case 3:
////					if(nTM == 1)
////					{
////						sprintf(Serial.buffer," Valve: Liga %dx as %2.d:%.2d", nTM, HourOnTM[0], MinOnTM[0]);
////					}
////					else
////					{
////						sprintf(Serial.buffer," Valve: Liga %dx/dia",nTM);
////					}
////					break;
////
////				case 4:
////					sprintf(Serial.buffer," Modo: Auto HL");
////					break;
////
////				case 5:
////					if(nTM == 1)
////					{
////						sprintf(Serial.buffer," IrrigLow: Liga %dx as %2.d:%.2d", nTM, HourOnTM[0], MinOnTM[0]);
////					}
////					else
////					{
////						sprintf(Serial.buffer," IrrigLow: Liga %dx/dia",nTM);
////					}
////					break;
////					break;
////
////
////				default:
////					strcpy(Serial.buffer,"sMode Err");
////					break;
////			}
////			Serial.println(Serial.buffer);
////			break;
////
////		case 1:
////			int i;
////			if(motorStatus)
////			{
////				for(i=(nLog-1);i>=0;i--)
////				{
////					memset(Serial.buffer,0,sizeof(Serial.buffer));
////					sprintf(Serial.buffer,"OFF_%.2d: %.2d:%.2d, %.2d/%.2d ",(i+1),hourLog_OFF[i], minuteLog_OFF[i], dayLog_OFF[i], monthLog_OFF[i]);
////					Serial.println(Serial.buffer);
////					_delay_ms(20);
////
////					memset(Serial.buffer,0,sizeof(Serial.buffer));
////					sprintf(Serial.buffer,"ON__%.2d: %.2d:%.2d, %.2d/%.2d ",(i+1),hourLog_ON[i], minuteLog_ON[i], dayLog_ON[i], monthLog_ON[i]);
////					Serial.println(Serial.buffer);
////					_delay_ms(20);
////				}
////			}
////			else
////			{
////				for(i=(nLog-1);i>=0;i--) //	for(i=0;i<nLog;i++)
////				{
////					memset(Serial.buffer,0,sizeof(Serial.buffer));
////					sprintf(Serial.buffer,"ON__%.2d: %.2d:%.2d, %.2d/%.2d " ,(i+1),hourLog_ON[i], minuteLog_ON[i], dayLog_ON[i], monthLog_ON[i]);
////					Serial.println(Serial.buffer);
////					_delay_ms(20);
////
////					memset(Serial.buffer,0,sizeof(Serial.buffer));
////					sprintf(Serial.buffer,"OFF_%.2d: %.2d:%.2d, %.2d/%.2d ",(i+1),hourLog_OFF[i], minuteLog_OFF[i], dayLog_OFF[i], monthLog_OFF[i]);
////					Serial.println(Serial.buffer);
////					_delay_ms(20);
////				}
////			}
////			sprintf(Serial.buffer,"r0:%d, r1:%d ",reasonV[0], reasonV[1]);
////			Serial.println(Serial.buffer);
////			break;
////
////		case 2:
////			sprintf(Serial.buffer,"f:%d t1:%.2d:%.2d c%dmin t2:%.2d:%.2d s:%dmin ", flag_waitPowerOn, waitPowerOn_min, waitPowerOn_sec, waitPowerOn_min_standBy, timeOn_min, timeOn_sec, motorTimerE);
////			Serial.println(Serial.buffer);
////			break;
////
////		case 3:
////			sprintf(Serial.buffer,"Motor:%d Fth:%d Rth:%d Pr:%d ", motorStatus, flag_Th, read_Rth(), PRess);
////			Serial.print(Serial.buffer);
////			switch (stateMode)
////			{
////				case 3:
////					sprintf(Serial.buffer,"Pref:%d Ptec: %d ", PRessureRef_Valve, PRessureMax_Sensor);
////					Serial.println(Serial.buffer);
////					break;
////
////				case 5:
////					sprintf(Serial.buffer,"Pref:%d, Pper:%d ", PRessureRef, PRessurePer);
////					Serial.println(Serial.buffer);
////					break;
////
////				default:
////					sprintf(Serial.buffer,"Pref:%d, Ptec: %d, Pper:%d ", PRessureRef, PRessureMax_Sensor, PRessurePer);
////					Serial.println(Serial.buffer);
////					break;
////
////			}
////			break;
////
////		case 4:
////			sprintf(Serial.buffer,"LL:%d ML:%d HL:%d ",levelSensorLL, levelSensorML, levelSensorHL);
////			Serial.println(Serial.buffer);
////
////			sprintf(Serial.buffer,"LL:%d ML:%d HL:%d",levelSensorLL_d, levelSensorML_d, levelSensorHL_d);
////			Serial.println(Serial.buffer);
////
////			sprintf(Serial.buffer,"Rth:%d k1:%d k2:%d k3:%d", Rth_status, k1_status, k2_status, k3_status);
////			Serial.println(Serial.buffer);
////			break;
////
////		case 5:
////			for(i=0;i<nTM;i++)
////			{
////				sprintf(Serial.buffer,"h%d: %.2d:%.2d",i+1, HourOnTM[i], MinOnTM[i]);
////				Serial.println(Serial.buffer);
////			}
////			break;
////
////		case 6:
////			sprintf(Serial.buffer,"tON:%.2d:%.2d ",timeOn_min, timeOn_sec);
////			Serial.println(Serial.buffer);
////			sprintf(Serial.buffer,"tOFF:%.2d:%.2d",timeOff_min, timeOff_sec);
////			Serial.println(Serial.buffer);
////			break;
////
////		case 7:
////			sprintf(Serial.buffer,"P:%d Fth:%d Rth:%d Ftm:%d k1:%d k2:%d k3:%d", PRess, flag_Th, read_Rth(), flag_timeMatch, motorStatus, read_k2(), read_k3());
////			Serial.println(Serial.buffer);
////
////			sprintf(Serial.buffer,"LL:%d ML:%d HL:%d  ",levelSensorLL, levelSensorML, levelSensorHL);
////			Serial.println(Serial.buffer);
////
////			sprintf(Serial.buffer,"LL:%d ML:%d HL:%d  ",levelSensorLL_d, levelSensorML_d, levelSensorHL_d);
////			Serial.println(Serial.buffer);
////			break;
////
////		case 8:
////			sprintf(Serial.buffer,"%.2d:%.2d:%.2d %.2d/%.2d/%.4d",tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year+1970);
////			Serial.println(Serial.buffer);
//////			sprintf(Serial.buffer,"WDRF:%d BORF:%d EXTRF:%d PORF:%d", flag_WDRF, flag_BORF, flag_EXTRF, flag_PORF);
//////			Serial.println(Serial.buffer);
////			break;
////
////		case 9:
////			sprintf(Serial.buffer,"Err");
////			Serial.println(Serial.buffer);
////			break;
////
////		default:
////			sprintf(Serial.buffer,"not implemented");
////			Serial.println(Serial.buffer);
////			break;
////	}
////}
////void acn_update_inputs()
////{
////	motorStatus = readPin_k1();
////	k1_status = read_k1();
////	k2_status = read_k2();
////	k3_status = read_k3();
////	Rth_status = read_Rth();
////}
////void acn_update_RTC()
////{
////	rtc.getUptime();
////	rtc.getTime();
////	tm.Year 	= rtc.timeinfo->tm_year;
////	tm.Month 	= rtc.timeinfo->tm_mon+1;
////	tm.Day 		= rtc.timeinfo->tm_mday;
////	tm.Hour 	= rtc.timeinfo->tm_hour;
////	tm.Minute 	= rtc.timeinfo->tm_min;
////	tm.Second 	= rtc.timeinfo->tm_sec;
////}
////void acn_RTC_update()
////{
////	if(tm.Second == 59)
////	{
////		tm.Second = 0;
////		if(tm.Minute == 59)
////		{
////			tm.Minute = 0;
////			if(tm.Hour == 23)
////			{
////				tm.Hour = 0;
////				uint8_t month31, month30;
////				if((tm.Month == 1) || (tm.Month == 3) || (tm.Month == 5) || (tm.Month == 7) || (tm.Month == 8) || (tm.Month == 10) || (tm.Month == 12))
////				{
////					month31 = 1;
////				}
////				else
////				{
////					month30 = 1;
////				}
////
////				if((tm.Day == 30 && month30) || (tm.Day == 31 && month31))
////				{
////					tm.Day = 1;
////					if(tm.Month == 12)
////					{
////						tm.Month = 1;
////						tm.Year++;
////					}
////					else
////					{
////						tm.Month++;
////					}
////				}
////				else
////				{
////					tm.Day++;
////				}
////			}
////			else
////			{
////				tm.Hour++;
////			}
////		}
////		else
////		{
////			tm.Minute++;
////		}
////	}
////	else
////	{
////		tm.Second++;
////	}
////}
////void acn_refreshVariables()
////{
////	if (flag_1s)
////	{
////		flag_1s = 0;
//////		gateToggle(1);
////
////		update_RTC();
////		update_inputs();
//////		RTC.read(tm);
////
////		check_thermalSafe();	// thermal relay check;
////		check_period();			// Period verify;
////		check_timeMatch();		// time matches flag;
////		check_TimerVar();		// drive timers
////
//////		check_pressure();		// get and check pressure system;
////
//////		check_levelSensors();	// level sensors;
////
//////		check_pressureDown();
////
////		if(flag_debug)
////		{
////			summary_Print(7);
////		}
////
////		if(flag_debug_time)
////		{
////			summary_Print(8);
////		}
////
////	}
////}
////void acn_refreshStoredData()
////{
////	stateMode = eeprom.read(eeprom.pageSet, eeprom.addr_stateMode);	//eeprom_read_byte((uint8_t *)(addr_stateMode));
////
////	if(stateMode == 0xFF)
////	{
////		stateMode = 0;
////		eeprom.write(eeprom.pageSet, eeprom.addr_stateMode, stateMode);
////		eeprom.write(eeprom.pageSet, eeprom.addr_LevelRef, 1023);
////		eeprom.write(eeprom.pageSet, eeprom.addr_standBy_min, 0);
////		eeprom.write(eeprom.pageSet, eeprom.addr_PRessureRef, 60);
////		eeprom.write(eeprom.pageSet, eeprom.addr_PRessureRef_Valve, 40);
////		eeprom.write(eeprom.pageSet, eeprom.addr_PRessureMax_Sensor, 100);
////		eeprom.write(eeprom.pageSet, eeprom.addr_motorTimerE, 60);
////		eeprom.write(eeprom.pageSet, eeprom.addr_HourOnTM, 21);
////		eeprom.write(eeprom.pageSet, eeprom.addr_MinOnTM, 40);
////		eeprom.write(eeprom.pageSet, eeprom.addr_nTM, 1);
////		eeprom.write(eeprom.pageSet, eeprom.addr_standBy_min, 10);
////
////
////		eeprom.write(eeprom.pageSet, eeprom.addr_PRessurePer, 85);
////		eeprom.write(eeprom.pageSet, eeprom.addr_rtc_PRL, 40000);
////		eeprom.write(eeprom.pageSet, eeprom.addr_rtc_clkSource, 0);
////
////		Serial.println("default values");
////	}
////
////	rtc.rtc_PRL = eeprom.read(eeprom.pageSet, eeprom.addr_rtc_PRL);
////	rtc.rtc_clkSource = eeprom.read(eeprom.pageSet, eeprom.addr_rtc_clkSource);
////
////	waitPowerOn_min_standBy = eeprom.read(eeprom.pageSet, eeprom.addr_standBy_min); //eeprom_read_byte((uint8_t *)(addr_standBy_min));
////	waitPowerOn_min = waitPowerOn_min_standBy;		// reset timer
////
//////	uint8_t lbyte, hbyte;
//////	hbyte = eeprom_read_byte((uint8_t *)(addr_LevelRef+1));
//////	lbyte = eeprom_read_byte((uint8_t *)(addr_LevelRef));
//////	levelRef_10bit = ((hbyte << 8) | lbyte);
////	levelRef_10bit = eeprom.read(eeprom.pageSet, eeprom.addr_LevelRef);
////
//////	hbyte = eeprom_read_byte((uint8_t *)(addr_motorTimerE+1));
//////	lbyte = eeprom_read_byte((uint8_t *)(addr_motorTimerE));
//////	motorTimerE = ((hbyte << 8) | lbyte);
////	motorTimerE = eeprom.read(eeprom.pageSet, eeprom.addr_motorTimerE);
////
////	PRessureRef = eeprom.read(eeprom.pageSet, eeprom.addr_PRessureRef); //eeprom_read_byte((uint8_t *)(addr_PRessureRef));
////	PRessureRef_Valve = eeprom.read(eeprom.pageSet, eeprom.addr_PRessureRef_Valve); //eeprom_read_byte((uint8_t *)(addr_PRessureRef_Valve));
////	PRessurePer = eeprom.read(eeprom.pageSet, eeprom.addr_PRessurePer); //eeprom_read_byte((uint8_t *)(addr_PREssurePer));
////	PRessureMax_Sensor = eeprom.read(eeprom.pageSet, eeprom.addr_PRessureMax_Sensor); //eeprom_read_byte((uint8_t *)(addr_PRessureMax_Sensor));
////
////	nTM = eeprom.read(eeprom.pageSet, eeprom.addr_nTM); //eeprom_read_byte((uint8_t *)(addr_nTM));
////
////	int i;
////	for(i=0;i<9;i++)
////	{
////		HourOnTM[i] = eeprom.read(eeprom.pageSet, eeprom.addr_HourOnTM+i); //eeprom_read_byte((uint8_t *)(addr_HourOnTM+i));
////		MinOnTM[i] = eeprom.read(eeprom.pageSet, eeprom.addr_MinOnTM+i);//eeprom_read_byte((uint8_t *)(addr_MinOnTM+i));
////	}
////}
////void acn_handleMessage()
////{
/////*
////$0X;				Verificar detalhes - Detalhes simples (tempo).
////	$00;			- Detalhes simples (tempo).
////	$01;			- Verifica histórico de quando ligou e desligou;
////	$02;			- Mostra tempo que falta para ligar;
////		$02:c;		- Zera o tempo;
////		$02:c:30;	- Ajusta novo tempo para 30 min;
////		$02:s:090;	- Tempo máximo ligado para 90 min. Para não utilizar, colocar zero;
////	$03;			- Verifica detalhes do motor, pressão e sensor termico;
////		$03:s:72;	- Set pressure ref [m.c.a.];
////		$03:v:32;	- Set pressure for valve load turn on and fill reservoir;
////		$03:p:150;	- Set sensor max pressure ref to change the scale [psi];
////		$03:b:85;	- Set to 85% the pressure min bellow the current pressure to avoid pipe broken;
////		$03:m:3;	- Set 3 seconds while K1 and K3 are ON into delta tri start;
////		$03:t:500;	- Set 500 milliseconds to wait K3 go off before start K2;
////	$04;			- Verifica detalhes do nível de �gua no po�o e referência 10 bits;
////		$04:0;		- Interrompe o envio continuo das vari�veis de press�o e n�vel;
////		$04:1;		- Envia continuamente valores de press�o e n�vel;
////		$04:s:0900;	- Adiciona nova referência para os sensores de nível. Valor de 0 a 1023;
////	$05;			- Mostra os hor�rios que liga no modo $62;
////	$06;			- Tempo ligado e tempo desligado;
////	$07:x;			- ADC reference change.
////		$07:0;		- AREF
////		$07:1;		- AVCC with external cap at AREF pin
////		$07:2;		- Internal 1.1 Voltage reference.
////	$08;			- Motivo do reboot.
////	$09;			- Reinicia o sistema.
////
////$1:h:HHMMSS;		- Ajustes do calend�rio;
////	$1:h:HHMMSS;	- Ajusta o hor�rio do sistema;
////	$1:h:123040;	- E.g. ajusta a hora para 12:30:40
////	$1:d:DDMMAAAA;	- Ajusta a data do sistema no formato dia/m�s/ano(4 d�gitos);
////	$1:d:04091986;	- E.g. Altera a data para 04/09/1986;
////	$1:c;			- Shows the LSI current prescaler value;
////	$1:c:40123;		- Set new prescaler value;
////
////$2:DevName;			- Change bluetooth name;
////	$2:Vassalo;		- Altera o nome do bluetooth para "Vassalo";
////
////$4:x:				- Is this applied fo stm32f10xxx series only;
////	$4:r:07;		- Read address 0x07 * 2 of currently page;
////	$4:w:07:03;		- Write variable 3 on address 7 of currently page;
////	$4:f:64:03;		- fill page 64 with 3 value;
////	$4:e:64;		- erase page 64;
////
////$3X;				- Acionamento do motor;
////	$31;			- liga o motor;
////	$30;			- desliga o motor;
////
////$5:n:X; ou $5:hX:HHMM;
////	$5:n:9;			- Configura para acionar 9 vezes. Necess�rio configurar 9 hor�rios;
////	$5:n:9;			- Configura o sistema para acionar uma �nica vez �s 21:30 horas;
////	$5:h1:2130;		- Configura o primeiro hor�rio como 21:30 horas;
////	$5:h8:0437;		- Configura o oitavo hor�rio como 04:37 horas;
////
////$6X;				- Modos de funcionamento;
////	$60; 			- Sistema Desligado (nunca ligar�);
////	$61;			- Liga somente à noite. Sensor superior;
////	$62;			- Liga nos determinados hor�rios estipulados;
////	$63;			- Função para válvula do reservat�rio;
////	$64;			- Função para motobomba do reservat�rio;
////*/
////	// Tx - Transmitter
////	if(enableDecode)
////	{
////		enableDecode = 0;
////
//////		int i;
//////		for(i=0;i<rLength;i++)
//////		{
//////			Serial1.println(sInstr[i]);
//////		}
//////		for(i=0;i<rLength;i++)
//////		{
//////			Serial1.println(sInstr[i],HEX);
//////		}
////
////		// Getting the opcode
////		aux[0] = '0';
////		aux[1] = sInstr[0];
////		aux[2] = '\0';
////		opcode = (uint8_t) atoi(aux);
//////		Serial.println("Got!");
//////		uint8_t statusCommand = 0;
////
////		switch (opcode)
////		{
////// -----------------------------------------------------------------
////			case 0:		// Check status
////			{
//////				aux[0] = '0';
//////				aux[1] = sInstr[1];
//////				aux[2] = '\0';
//////				statusCommand = (uint8_t) atoi(aux);
//////
//////				switch (statusCommand)
//////				{
//////					// ----------
//////					// $02:c;  -> clear time counter;
//////					// $02:c:mm;  -> set time counter ref;
//////					case 0:	// $00:1; or $00;
//////					{
//////						if(sInstr[2]==':' && sInstr[4]==';')
//////						{
//////							aux[0] = '0';
//////							aux[1] = sInstr[3];
//////							aux[2] = '\0';
//////							flag_debug_time = (uint8_t) atoi(aux);
//////						}
//////						else
//////						{
//////							summary_Print(statusCommand);
//////						}
//////					}
//////					break;
//////
//////					case 2:
//////					{
//////						if(sInstr[2]==':' && sInstr[3]=='c')
//////						{
//////							if(sInstr[4] == ';')
//////							{
//////								flag_waitPowerOn = 0;
//////								waitPowerOn_min = 0;
//////								waitPowerOn_sec = 0;
//////							}
//////							else if(sInstr[4] ==':' && sInstr[7] == ';')
//////							{
//////								aux[0] = sInstr[5];
//////								aux[1] = sInstr[6];
//////								aux[2] = '\0';
//////								waitPowerOn_min_standBy = (uint8_t) atoi(aux);
//////
//////								eeprom.write(eeprom.pageSet, eeprom.addr_standBy_min, waitPowerOn_min_standBy);
//////								//eeprom_write_byte((uint8_t *)(addr_standBy_min), waitPowerOn_min_standBy);
//////
////////								Serial.print("powerOn min:");
////////								Serial.println(powerOn_min_Standy);
//////							}
//////						}//$02:s:129;
//////						else if(sInstr[2] == ':' && sInstr[3] == 's' && sInstr[4] == ':' && sInstr[8] == ';')
//////						{
//////							aux2[0] = '0';
//////							aux2[1] = sInstr[5];
//////							aux2[2] = sInstr[6];
//////							aux2[3] = sInstr[7];
//////							aux2[4] = '\0';
//////							motorTimerE = (uint16_t) atoi(aux2);
//////
//////							eeprom.write(eeprom.pageSet, eeprom.addr_motorTimerE, motorTimerE);
//////
////////							uint8_t lbyteRef = 0, hbyteRef = 0;
////////							lbyteRef = motorTimerE;
////////							hbyteRef = (motorTimerE >> 8);
//////
////////							eeprom_write_byte((uint8_t *)(addr_motorTimerE+1), hbyteRef);
////////							eeprom_write_byte((uint8_t *)(addr_motorTimerE), lbyteRef);
//////						}
//////						summary_Print(statusCommand);
//////					}
//////					break;
//////					// ------------------------------
//////					case 3:// $03:s:68;
//////						if(sInstr[2]==':' && sInstr[3] == 's' && sInstr[4] == ':' && sInstr[7] == ';')
//////						{
//////							aux[0] = sInstr[5];
//////							aux[1] = sInstr[6];
//////							aux[2] = '\0';
//////							PRessureRef = (uint8_t) atoi(aux);
//////							eeprom.write(eeprom.pageSet, eeprom.addr_PRessureRef, PRessureRef);
//////							//eeprom_write_byte((uint8_t *)(addr_PRessureRef), PRessureRef);
//////
//////							sprintf(Serial.buffer,"PRessRef: %d", PRessureRef);
//////							Serial.println(Serial.buffer);
//////						}
//////						else if(sInstr[2]==':' && sInstr[3] == 'v' && sInstr[4] == ':' && sInstr[7] == ';')
//////						{
//////							aux[0] = sInstr[5];
//////							aux[1] = sInstr[6];
//////							aux[2] = '\0';
//////							PRessureRef_Valve = (uint8_t) atoi(aux);
//////							eeprom.write(eeprom.pageSet, eeprom.addr_PRessureRef_Valve, PRessureRef_Valve);
//////							//eeprom_write_byte((uint8_t *)(addr_PRessureRef_Valve), PRessureRef_Valve);
//////
//////							sprintf(Serial.buffer,"PRessRef_Valve: %d", PRessureRef_Valve);
//////							Serial.println(Serial.buffer);
//////						}
//////						else if(sInstr[2]==':' && sInstr[3] == 'p' && sInstr[4] == ':' && sInstr[8] == ';')
//////						{// $03:p:150;
//////							aux2[0] = '0';
//////							aux2[1] = sInstr[5];
//////							aux2[2] = sInstr[6];
//////							aux2[3] = sInstr[7];
//////							aux2[4] = '\0';
//////							PRessureMax_Sensor = (uint8_t) atoi(aux2);
//////
//////							eeprom.write(eeprom.pageSet, eeprom.addr_PRessureMax_Sensor, PRessureMax_Sensor);
////////							eeprom_write_byte((uint8_t *)(addr_PRessureMax_Sensor), PRessureMax_Sensor);
//////						}
//////						else if(sInstr[2]==':' && sInstr[3] == 'b' && sInstr[4] == ':' && sInstr[7] == ';')
//////						{
//////							aux[0] = sInstr[5];
//////							aux[1] = sInstr[6];
//////							aux[2] = '\0';
//////							PRessurePer = (uint8_t) atoi(aux);
//////							eeprom.write(eeprom.pageSet, eeprom.addr_PRessurePer, PRessurePer);
//////							//eeprom_write_byte((uint8_t *)(addr_PREssurePer), PRessurePer);
//////						}
//////						else
//////							summary_Print(statusCommand);
//////						break;
//////					// ------------------------------
//////					case 4: // $04:1;
//////						if(sInstr[2]==':' && sInstr[4]==';')
//////						{
//////							aux[0] = '0';
//////							aux[1] = sInstr[3];
//////							aux[2] = '\0';
//////							flag_debug = (uint8_t) atoi(aux);
//////						}//$04:s:0900;
//////						else if(sInstr[2]==':' && sInstr[3]=='s' && sInstr[4]==':' && sInstr[9]==';')
//////						{
//////							aux2[0] = sInstr[5];
//////							aux2[1] = sInstr[6];
//////							aux2[2] = sInstr[7];
//////							aux2[3] = sInstr[8];
//////							aux2[4] = '\0';
//////
//////							levelRef_10bit = (uint16_t) atoi(aux2);
//////
////////							uint8_t lbyteRef = 0, hbyteRef = 0;
////////							lbyteRef = levelRef_10bit;
////////							hbyteRef = (levelRef_10bit >> 8);
//////
//////							eeprom.write(eeprom.pageSet, eeprom.addr_LevelRef, levelRef_10bit);
//////
//////							//eeprom_write_byte((uint8_t *)(addr_LevelRef+1), hbyteRef);
//////							//eeprom_write_byte((uint8_t *)(addr_LevelRef), lbyteRef);
//////						}
//////						summary_Print(statusCommand);
//////						sprintf(Serial.buffer,"Ref: %d", levelRef_10bit);
//////						Serial.println(Serial.buffer);
//////						break;
//////					// ------------------------------
//////					case 7:
//////						if(sInstr[2]==':' && sInstr[4]==';')
//////						{
//////							aux[0] = '0';
//////							aux[1] = sInstr[3];
//////							aux[2] = '\0';
//////							uint8_t adcCommand = (uint8_t) atoi(aux);
//////
//////							switch (adcCommand)
//////							{
////////								case 0:
////////									ADMUX &=  ~(1<<REFS1);		// AREF, Internal Vref turned off
////////									ADMUX &=  ~(1<<REFS0);
////////									Serial.println("AREF");
////////									break;
////////
////////								case 1:
////////									ADMUX &=  ~(1<<REFS1);		// AVCC with external capacitor at AREF pin
////////									ADMUX |=   (1<<REFS0);
////////									Serial.println("AVCC");
////////									break;
////////
////////								case 2:
////////									ADMUX |=   (1<<REFS1);		// Internal 1.1V Voltage Reference with external capacitor at AREF pin
////////									ADMUX |=   (1<<REFS0);
////////									Serial.println("1.1V");
////////									break;
//////							}
//////						}
//////						break;
//////					// ------------------------------
//////					case 9:
//////						Serial.println("Rebooting...");
//////						NVIC_SystemReset();
//////						//wdt_enable(WDTO_15MS);
////////						flag_reset = 1;
//////						break;
//////
//////					default:
//////						summary_Print(statusCommand);
//////						break;
//////				}
////			}
////			break;
////// -----------------------------------------------------------------
////			case 1: 	// Setup calendar
////			{
//////				// Set-up clock -> $1:h:HHMMSS;
//////				if(sInstr[1]==':' && sInstr[2]=='h' && sInstr[3]==':' && sInstr[10]==';')
//////				{
//////					aux[0] = sInstr[4];
//////					aux[1] = sInstr[5];
//////					aux[2] = '\0';
//////					tm.Hour = (uint8_t) atoi(aux);
//////
//////					aux[0] = sInstr[6];
//////					aux[1] = sInstr[7];
//////					aux[2] = '\0';
//////					tm.Minute = (uint8_t) atoi(aux);
//////
//////					aux[0] = sInstr[8];
//////					aux[1] = sInstr[9];
//////					aux[2] = '\0';
//////					tm.Second = (uint8_t) atoi(aux);
//////
////////					RTC.write(tm);
//////					rtc.write(tm);
//////					summary_Print(0);
//////				}
//////				// 	Set-up date -> $1:d:DDMMAAAA;
//////				else if(sInstr[1]==':' && sInstr[2]=='d' && sInstr[3]==':' && sInstr[12]==';')
//////				{
//////					// Getting the parameters
//////					aux[0] = sInstr[4];
//////					aux[1] = sInstr[5];
//////					aux[2] = '\0';
//////					tm.Day = (uint8_t) atoi(aux);
//////
//////					aux[0] = sInstr[6];
//////					aux[1] = sInstr[7];
//////					aux[2] = '\0';
//////					tm.Month = (uint8_t) atoi(aux);
//////
//////					char aux2[5];
//////					aux2[0] = sInstr[8];
//////					aux2[1] = sInstr[9];
//////					aux2[2] = sInstr[10];
//////					aux2[3] = sInstr[11];
//////					aux2[4] = '\0';
//////					tm.Year = (uint8_t) (atoi(aux2)-1970);
//////
////////					RTC.write(tm);
//////					rtc.write(tm);
//////
//////					summary_Print(0);
//////
//////				}
//////				// Set-up date -> $1:d:DDMMAAAA;
//////				else if(sInstr[1]==':' && sInstr[2]=='c' && sInstr[3]==':' && sInstr[9]==';')
//////				{	// $1:c:40123;
//////					char aux3[6];
//////					aux3[0] = sInstr[4];
//////					aux3[1] = sInstr[5];
//////					aux3[2] = sInstr[6];
//////					aux3[3] = sInstr[7];
//////					aux3[4] = sInstr[8];
//////					aux3[5] = '\0';
//////					rtc.rtc_PRL = (uint32_t) atoi(aux3);
//////
//////					rtc.setRTC_DIV(rtc.rtc_PRL);
//////					sprintf(Serial.buffer,"PRLw: %ld", rtc.rtc_PRL);
//////					Serial.println(Serial.buffer);
//////
//////					eeprom.write(eeprom.pageSet, eeprom.addr_rtc_PRL, (uint16_t) rtc.rtc_PRL);
//////
//////					sprintf(Serial.buffer,"PRLr: %ld", rtc.getRTC_DIV());
//////					Serial.println(Serial.buffer);
//////				}
//////				else if(sInstr[1]==':' && sInstr[2]=='c' && sInstr[3]==';')
//////				{
//////					sprintf(Serial.buffer,"PRLreg: %lu, PRLflash: %u", rtc.getRTC_DIV(), eeprom.read(eeprom.pageSet, eeprom.addr_rtc_PRL));
//////					Serial.println(Serial.buffer);
//////				}
//////				// $1:s:S;
//////				else if(sInstr[1]==':' && sInstr[2]=='s' && sInstr[3]==':' && sInstr[5]==';')
//////				{
//////					aux[0] = '0';
//////					aux[1] = sInstr[4];
//////					aux[2] = '\0';
//////					rtc.rtc_clkSource = (uint8_t) atoi(aux);
//////
//////					eeprom.write(eeprom.pageSet, eeprom.addr_rtc_clkSource, rtc.rtc_clkSource);
//////					rtc.bkpDomainReset();
//////					rtc.begin_rtc(rtc.rtc_clkSource, rtc.rtc_PRL);
//////
//////					sprintf(Serial.buffer,"rtcSource: %u", rtc.rtc_clkSource);
//////					Serial.println(Serial.buffer);
//////				}
//////				else if(sInstr[1]==':' && sInstr[2]=='s' && sInstr[3]==';')
//////				{
//////					eeprom.read(eeprom.pageSet, eeprom.addr_rtc_clkSource);
//////
//////					summary_Print(0);
//////					sprintf(Serial.buffer,"rtcSource: %u", rtc.rtc_clkSource);
//////					Serial.println(Serial.buffer);
//////				}
////			}
////			break;
////// -----------------------------------------------------------------
////			case 2:		// Setup Bluetooth device name
////			{
//////				// Setup clock -> $2:BluetoothName;
//////				char aux_str[sInstr_SIZE], aux_str2[sInstr_SIZE+7];
//////				uint8_t k1=0;
//////				if(sInstr[1]==':')
//////				{
//////					// 3 because 2 and : count 2 characters and one more for '\0'
//////					while((k1<sInstr_SIZE-3) && sInstr[k1] != ';')
//////					{
//////						aux_str[k1] = sInstr[k1+2];
//////						k1++;
////////						Serial.println(k1);
//////					}
//////
//////					aux_str[k1-2] = '\0';
//////					Serial.println("Disconnect!");
////////					wdt_reset();
////////					_delay_ms(3000);
////////					wdt_reset();
////////					_delay_ms(3000);
////////					wdt_reset();
////////					_delay_ms(3000);
////////					strcpy(aux_str2,"AT");
////////					Serial.print(aux_str2);
//////					//wdt_reset();
//////					_delay_ms(3000);
//////					strcpy(aux_str2,"AT+NAME");
//////					strcat(aux_str2,aux_str);
//////					Serial.print(aux_str2);
//////					//wdt_reset();
//////					_delay_ms(1000);
//////				}
////			}
////			break;
////// -----------------------------------------------------------------
////			case 3:		// Set motor ON/OFF
////			{
//////				uint8_t motorCommand;
//////				aux[0] = '0';
//////				aux[1] = sInstr[1];
//////				aux[2] = '\0';
//////				motorCommand = (uint8_t) atoi(aux);
//////
//////				if (motorCommand && (!motorStatus))
//////				{
//////					motor_start();
//////					Serial.println("value");
//////				}
//////				else
//////				{
//////					motor_stop(0x06);
//////				}
//////
//////				summary_Print(3);
////			}
////			break;
////// -----------------------------------------------------------------
////			case 4: 	// FLASH store test, //$4:r:23; and $4:w:23:03;
////			{
//////				if(sInstr[1]==':' && sInstr[2]=='r' && sInstr[3]==':' && sInstr[6]==';')
//////				{	// $4:r:05; read 05*2 address of page block
//////					// Getting the parameters
//////					aux[0] = sInstr[4];
//////					aux[1] = sInstr[5];
//////					aux[2] = '\0';
//////					uint8_t addrt = (uint8_t) atoi(aux);
//////					uint8_t var = eeprom.read(eeprom.pageSet, addrt);
//////					sprintf(Serial.buffer,"EE read: %d ", var);
//////					Serial.println(Serial.buffer);
//////
//////					sprintf(Serial.buffer,"Page %d ", eeprom.pageSet);
//////					Serial.println(Serial.buffer);
//////
//////					sprintf(Serial.buffer,"Address %X ", (unsigned int) eeprom.pageAddress(eeprom.pageSet));
//////					Serial.println(Serial.buffer);
//////				}
//////				if(sInstr[1]==':' && sInstr[2]=='w' && sInstr[3]==':' && sInstr[6]==':' && sInstr[9]==';')
//////				{	// $4:w:03:07;
//////					// Getting the parameters
//////					aux[0] = sInstr[4];
//////					aux[1] = sInstr[5];
//////					aux[2] = '\0';
//////					uint8_t addrt = (uint8_t) atoi(aux);
//////
//////					aux[0] = sInstr[7];
//////					aux[1] = sInstr[8];
//////					aux[2] = '\0';
//////					uint8_t var = (uint8_t) atoi(aux);
//////
//////					eeprom.write(eeprom.pageSet, addrt, var);
//////					sprintf(Serial.buffer,"EE write: %d ", var);
//////					Serial.println(Serial.buffer);
//////
//////					var = eeprom.read(eeprom.pageSet, addrt);
//////					sprintf(Serial.buffer,"EE read2: %d ", var);
//////					Serial.println(Serial.buffer);
//////
//////					sprintf(Serial.buffer,"Page %d ", eeprom.pageSet);
//////					Serial.println(Serial.buffer);
//////
//////					sprintf(Serial.buffer,"Address %X ", (unsigned int) eeprom.pageAddress(eeprom.pageSet));
//////					Serial.println(Serial.buffer);
//////				}
//////				if(sInstr[1]==':' && sInstr[2]=='f' && sInstr[3]==':' && sInstr[6]==':' && sInstr[9]==';')
//////				{	// $4:f:64:07;	fill page 64 with 07 value;
//////					// Getting the parameters
//////					aux[0] = sInstr[4];
//////					aux[1] = sInstr[5];
//////					aux[2] = '\0';
//////					uint8_t page = (uint8_t) atoi(aux);
//////
//////					aux[0] = sInstr[7];
//////					aux[1] = sInstr[8];
//////					aux[2] = '\0';
//////					uint8_t var = (uint8_t) atoi(aux);
//////
//////
//////
//////					eeprom.writePage(page, (var << 8 | var));
//////					sprintf(Serial.buffer,"Filled page %d ", page);
//////					Serial.println(Serial.buffer);
//////
//////					sprintf(Serial.buffer,"Address %X ", (unsigned int) eeprom.pageAddress(page));
//////					Serial.println(Serial.buffer);
//////				}
//////				if(sInstr[1]==':' && sInstr[2]=='e' && sInstr[3]==':' && sInstr[6]==';')
//////				{	// $4:e:64;	erase page 64
//////					// Getting the parameters
//////					aux[0] = sInstr[4];
//////					aux[1] = sInstr[5];
//////					aux[2] = '\0';
//////					uint8_t page = (uint8_t) atoi(aux);
//////
//////					eeprom.erasePage(page);
//////					sprintf(Serial.buffer,":Page %d erased!", page);
//////					Serial.println(Serial.buffer);
//////
//////					sprintf(Serial.buffer,"Address %X ", (unsigned int) eeprom.pageAddress(page));
//////					Serial.println(Serial.buffer);
//////				}
//////				break;
////			}
////			break;
////// -----------------------------------------------------------------
////			case 5: 	// Command is $5:h1:2130;
////			{
//////				if(sInstr[1]==':' && sInstr[2]=='h' && sInstr[4]==':' && sInstr[9]==';')
//////				{
//////					aux[0] = '0';
//////					aux[1] = sInstr[3];
//////					aux[2] = '\0';
//////					uint8_t indexV = (uint8_t) atoi(aux);
//////
//////					aux[0] = sInstr[5];
//////					aux[1] = sInstr[6];
//////					aux[2] = '\0';
//////					HourOnTM[indexV-1] = (uint8_t) atoi(aux);
//////					eeprom.write(eeprom.pageSet, eeprom.addr_HourOnTM+indexV-1, HourOnTM[indexV-1]);
//////					//eeprom_write_byte(( uint8_t *)(addr_HourOnTM+indexV-1), HourOnTM[indexV-1]);
//////
//////					aux[0] = sInstr[7];
//////					aux[1] = sInstr[8];
//////					aux[2] = '\0';
//////					MinOnTM[indexV-1] = (uint8_t) atoi(aux);
//////					eeprom.write(eeprom.pageSet, eeprom.addr_MinOnTM+indexV-1, MinOnTM[indexV-1]);
////////					eeprom_write_byte(( uint8_t *)(addr_MinOnTM+indexV-1), MinOnTM[indexV-1]);
//////
//////					summary_Print(5);
//////				}
//////				else if(sInstr[1]==':' && sInstr[2]=='n' && sInstr[3]==':' && sInstr[5]==';')
//////				{
//////					aux[0] = '0';
//////					aux[1] = sInstr[4];
//////					aux[2] = '\0';
//////
//////					nTM = (uint8_t) atoi(aux);
//////					eeprom.write(eeprom.pageSet, eeprom.addr_nTM, nTM);
//////					//eeprom_write_byte(( uint8_t *)(addr_nTM), nTM);
//////
//////					summary_Print(5);
//////				}
//////				else if(sInstr[1]==';')
//////				{
//////					summary_Print(5);
//////				}
////			}
////			break;
////// ----------------------------------------------------------------
////			case 6:		// Set working mode
////			{
//////				aux[0] = '0';
//////				aux[1] = sInstr[1];
//////				aux[2] = '\0';
//////				stateMode = (uint8_t) atoi(aux);
//////				eeprom.write(eeprom.pageSet, eeprom.addr_stateMode, stateMode);
////////				//eeprom_write_byte(( uint8_t *)(addr_stateMode), stateMode);
//////
//////				summary_Print(0);
////			}
////			break;
////// -----------------------------------------------------------------
////			case 7:		// nRF24L01p test functions;
////			{
//////				// $7:s:
//////				uint8_t state =9;
//////				if(sInstr[1]==':' && sInstr[2]=='s' && sInstr[3]==';')
//////				{
//////					radio.begin_nRF24L01p();
//////				}
//////				else if(sInstr[1]==':' && sInstr[2]=='g' && sInstr[3]==';')
//////				{
//////					state = radio.get_stateMode();
//////				}
//////				else if(sInstr[1]==':' && sInstr[2]=='t' && sInstr[3]==';')
//////				{
//////					state = radio.set_mode_tx(ENABLE);
//////				}
//////				else if(sInstr[1]==':' && sInstr[2]=='r' && sInstr[3]==';')
//////				{
//////					state = radio.set_mode_rx(ENABLE);
//////				}
//////				else if(sInstr[1]==':' && sInstr[2]=='b' && sInstr[3]==';')
//////				{
//////					state = radio.set_mode_standbyI();
//////				}
//////				else if(sInstr[1]==':' && sInstr[2]=='p' && sInstr[3]==';')
//////				{
//////					state = radio.set_mode_powerDown();
//////				}
//////
//////				sprintf(Serial.buffer,"state: %u", state);
//////				Serial.println(Serial.buffer);
////
//////				radio.begin_nRF24L01p();
//////				radio.set_250kbps();
//////				if(sInstr[1]==':' && sInstr[2]=='h' && sInstr[4]==':' && sInstr[9]==';')
//////				{
//////					aux[0] = '0';
//////					aux[1] = sInstr[3];
//////					aux[2] = '\0';
//////					uint8_t indexV = (uint8_t) atoi(aux);
//////
//////					aux[0] = sInstr[5];
//////					aux[1] = sInstr[6];
//////					aux[2] = '\0';
//////					HourOnTM[indexV-1] = (uint8_t) atoi(aux);
//////					eeprom.write(eeprom.pageSet, eeprom.addr_HourOnTM+indexV-1, HourOnTM[indexV-1]);
//////					//eeprom_write_byte(( uint8_t *)(addr_HourOnTM+indexV-1), HourOnTM[indexV-1]);
//////
//////					aux[0] = sInstr[7];
//////					aux[1] = sInstr[8];
//////					aux[2] = '\0';
//////					MinOnTM[indexV-1] = (uint8_t) atoi(aux);
//////					eeprom.write(eeprom.pageSet, eeprom.addr_MinOnTM+indexV-1, MinOnTM[indexV-1]);
////////					eeprom_write_byte(( uint8_t *)(addr_MinOnTM+indexV-1), MinOnTM[indexV-1]);
//////
//////					summary_Print(5);
////			}
////			break;
////// -----------------------------------------------------------------
////			default:
//////				summary_Print(10);
////				break;
////// -----------------------------------------------------------------
////		}
////		memset(sInstr,0,sizeof(sInstr));	// Clear all vector;
////	}
////}
////void acn_comm_Bluetooth()
////{
////	// Rx - Always listening
//////	uint8_t j2 =0;
////	while((Serial.available()>0))	// Reading from serial
////	{
////		inChar = Serial.read();
////
////		if(inChar=='$')
////		{
////			j2 = 0;
////			flag_frameStartBT = 1;
//////			Serial.println("Frame Start!");
////		}
////
////		if(flag_frameStartBT)
////			sInstrBluetooth[j2] = inChar;
////
//////		sprintf(Serial.buffer,"J= %d",j2);
//////		Serial.println(Serial.buffer);
////
////		j2++;
////
////		if(j2>=sizeof(sInstrBluetooth))
////		{
////			memset(sInstrBluetooth,0,sizeof(sInstrBluetooth));
////			j2=0;
//////			Serial.println("ZEROU! sIntr BLuetooth Buffer!");
////		}
////
////		if(inChar==';')
////		{
//////			Serial.println("Encontrou ; !");
////			if(flag_frameStartBT)
////			{
//////				Serial.println("Frame Stop!");
////				flag_frameStartBT = 0;
////				rLength = j2;
////				j2 = 0;
////				enableTranslate_Bluetooth = 1;
////			}
////		}
////	}
//////	flag_frameStart = 0;
////
////	if(enableTranslate_Bluetooth)
////	{
//////		Serial.println("enableTranslate_Bluetooth");
////		enableTranslate_Bluetooth = 0;
////
////		char *pi0, *pf0;
////		pi0 = strchr(sInstrBluetooth,'$');
////		pf0 = strchr(sInstrBluetooth,';');
////
////		if(pi0!=NULL)
////		{
////			uint8_t l0=0;
////			l0 = pf0 - pi0;
////
////			int i;
////			for(i=1;i<=l0;i++)
////			{
////				sInstr[i-1] = pi0[i];
//////				Serial.write(sInstr[i-1]);
////			}
////			memset(sInstrBluetooth,0,sizeof(sInstrBluetooth));
////	//		Serial.println(sInstr);
////
////			enableDecode = 1;
////		}
////		else
////		{
//////			Serial.println("Err");
//////			Serial.write(pi0[0]);
//////			Serial.write(pf0[0]);
////		}
////	}
////}
//void
//#endif /* ACIONNA_HPP_ */
//
//
//
