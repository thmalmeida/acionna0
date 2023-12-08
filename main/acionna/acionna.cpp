#include "acionna.hpp"

static const char *TAG_ACIONNA = "Acionna0";

// static I2C_Master i2c(I2C_NUM_0, I2C_SCL, I2C_SDA, I2C_NORMAL_SPEED_HZ, 0);
static I2C_Master i2c(I2C_NUM_1, I2C_SCL, I2C_SDA, I2C_NORMAL_SPEED_HZ, 1);
static DS3231 rtc{&i2c};
static Agro::RTC_Time device_clock;
static DateTime dt;

// static ADC_driver adc0(adc_mode::noption);
int timeout_sensors;
int timeout_sensors_cfg = 600;

volatile uint8_t flag_1sec = 0;
volatile uint8_t flag_100ms = 0;

// static pwm_ledc led_wifi_indicator(2, 1, 0, 1);

Acionna::Acionna(ADC_driver* adc) : pipe1_(adc, 4, 150), pipe2_(adc, 7, 220), pump1_{&epoch_time_}, valves1_{&i2c, &epoch_time_, &pressure_} {
// Acionna::Acionna(void) : valves1_{&i2c} {
	// ADC_driver adc0(adc_mode::oneshot);
	// ADC_driver adc0(adc_mode::oneshot);
	// pipe1_(&adc0, 4, 150);
	// pipe2_(&adc0, 7, 100);
	init();
}
uint32_t Acionna::get_uptime() {
	return uptime_;
}
std::string Acionna::handle_message(uint8_t* command_str) {
	/*
	$0X;						- Verificar detalhes - Detalhes simples (tempo).
		$00;					- Detalhes simples (tempo).
		$00:[0|1];				- Json send back 1, stop 0;
		$01;					- Verifica histórico de quando ligou e desligou;
		$02;					- Mostra tempo que falta para ligar;
			$02:w;				- Zera o tempo;
			$02:w:30;			- Ajusta novo tempo para 30 min;
			$02:s:090;			- Tempo máximo ligado para 90 min. Para não utilizar, colocar zero;
			$02:f:045;			- Força o contador de tempo ligado para 45 min;
		$03;					- Verifica detalhes do motor, pressão e sensor termico;
			$03:s[1|2]:72;		- Set max pressure ref for pipe [m.c.a.];
			$03:s[1|2];			- show max pressure configured;
			$03:p[1|2]:150;		- Set sensor max pressure ref to change the scale [psi];
			$03:p[1|2];			- Show sensor max pressure [psi];
			$03:v:32;			- Set pressure for valve load turn on and fill reservoir;
			$03:b2;				- Show pipepvc intake air variables
			$03:b:85;			- Set to 85% the pressure min bellow the current pressure to avoid pipe broken;
			$03:m:3;			- Set 3 seconds while K1 and K3 are ON into delta tri start;
			$03:m;				- Just show the speeding up time;
			$03:t:0400;			- Set 400 milliseconds to wait K3 go off before start K2;
			$03:t;				- Just show the time switch;
			$03:u;				- show k time logs;

		$04;					- Verifica detalhes do nível de água no poço e referência 10 bits;
			$04:0;				- Interrompe o envio continuo das variáveis de pressão e nível;
			$04:1;				- Envia continuamente valores de pressão e nível;
			$04:s:0900;			- Adiciona nova referência para os sensores de nível. Valor de 0 a 1023;
		$05;					- Mostra os horários que liga no modo $62;
		$06;					- Tempo ligado e tempo desligado;
		$07;					- Show low pressure algorithm variables
			$07:l1;				- show threshold low pressure to slope machine;
			$07:l1:20; 			- set threshold low pressure to slope machine;
			$07:l2;				- show threshold low pressure to slope machine;
			$07:l2:10; 			- set threshold low pressure to slope machine;
			$07:0;				- ADC reference change; AREF
			$07:1;				- AVCC with external cap at AREF pin
			$07:2;				- Internal 1.1 Voltage reference.

	$10:h:HHMMSS;				- Ajustes do calendário;
		$10:h:HHMMSS;			- Ajusta o horário do sistema;
		$10:h:123040;			- E.g. ajusta a hora para 12:30:40
		$10:d:DDMMAAAA;			- Ajusta a data do sistema no formato dia/mês/ano(4 dígitos);
		$10:d:04091986;			- E.g. Altera a data para 04/09/1986;
		$10:c;					- Shows the LSI current prescaler value;
		$10:c:40123;			- Set new prescaler value;

	$2X;						(not implemented)
		$20:DevName;			- Change bluetooth name;
		$20:n:
		$21:i:30;				- lo;		- Altera o nome do bluetooth para "Vassalo";
		$21:d:099;				- pwm: change duty cicle [%];
		$21:f:0001				- pwm: change frequency [Hz];

	$3X;						- Acionamento do motor;
		$30;					- desliga todos contatores;
		$31;					- ligar contator K1;
		$32;					- ligar contator K2;
		$33;					- ligar contator K3;
		$34;					- direto para partida delta;
		$35;					- direto para partida Y;
		$36;					- liga motor com partida Y delta;
	
	$4:x:						(not implemented) - Is this applied fo stm32f10xxx series only;
		$4:r:07;				- Read address 0x07 * 2 of currently page;
		$4:w:07:03;				- Write variable 3 on address 7 of currently page;
		$4:f:64:03;				- fill page 64 with 3 value;
		$4:e:64;				- erase page 64;

	$5X
		$50:n:X; 				- Time match cycles and array for auto start mode
			$50;				- show all timers to turn on and info;
			$50:s:[0|1];		- desabilita|habilita auto turn (habilita time match flag);
			$50:m:[1|3]			- tipo de partida automática: 1- k1; 3- y-Delta;
			$50:n;				- mostra a quantidade de vezes que irá ligar;
			$50:n:9;			- Configura para acionar 9 vezes. Necessário configurar 9 horários;
			$50:n:1;			- Configura o sistema para acionar uma única vez;
			$50:h1:2130;		- Configura o primeiro horário como 21:30 horas $50:hX:HHMM;
			$50:h8:0437;		- Configura o oitavo horário como 04:37 horas;
			$50:t1:120;			- configura o primeiro horário de partida com tempo de 120 min;

		$51;					- optimized mode;
			$51;				- show optimized configuration
			$51:s:[0|1];		- disable/enable process;
			$51:d;				- time delay before next start;
			$51:d:05;			- time delay in minutes between stop and the new start;
			$51:h;				- first time start show
			$51:h:2300;			- time to the first start cycle;
			$51:n:[1-9];		- set the number of events to run;
			$51:r;				- red time show
			$51:r:0600;			- red time beginner (time to stop all process);
			$51:e1:m3:2:120;	- Event [1] can turn motor into [m3] mode for [2] cycles on the maximum of 120 minuntes each one (not implemented yet);

	$6X;						- Modos de funcionamento;
		$60; 					- Sistema Desligado não permite ligar;
		$61;					- Sistema ocioso esperando um time match automático ou acionamento manual por linha de comando.
		$62;					- Modo irrigação com controle das válvulas em modo de acionamento automático.
		$63;					- Pumping all night long mode (optimized mode);

	$7X							- Funções que habilitam ou desabilitam verificações de:
		$70;					- not implemented
			$70:rt;
			$70:rt:[0|1];		- relé térmico;
			$70:kt;
			$70:kt:[0|1];		- todos contatores
			$70:k1;
			$70:k1:[0|1];		- contator K1 (not implemented);
			$70:k2;
			$70:k2:[0|1];		- contator K2 (not implemented);
			$70:k3;
			$70:k3:[0|1];		- contator K3 (not implemented);
			$70:l1:[0|1];		- low pressure detection for on_nominal_k1 state motor using pipe1.
			$70:l2:[0|1];		- low pressure detection for on_nominal_k2 state motor using pipe2.
			$70:l4:[0|1];		- low pressure detection for on_nominal_delta state motor using pipe1.
			$70:ph;
			$70:ph:1|0;			- desligamento por alta pressão;
			$70:pl;				- low pressure global flag detection show.
			$70:pl[0|1];		- Enable/disable low pressure global flag detection.
			$70:pl:0-9;			- desligamento por pressão baixa em min caso seja diferente de 0;
			$70:pv;
			$70:pv:1|0;			- desligamento por pressão alta por válvula;
			$70:vc;				- Show valve check status;
			$70:vc:[0|1];		- Enable disable valve check status;

	$8x							- Funções de programação da irrigação;
		$80;					- show info
			$80:s:[0|1];		- start/stop valves sequence;
			$80:d:[0|1];		- 0 sentido direto; 1 - sentido inverso na troca dos setores;
			$80:h;				- show valves history log. Time on, elapsed time, avg pressure;
			$80:n;				- next valve forcing time valve to zero;
			$80:v:01;			- mostra condições de configuração da válvula 01;
			$80:v:01:[0|1];		- desaciona|aciona válvula 01;
			$80:v:01:i;			- insere setor na programação;
			$80:v:01:r;			- remove setor da programação;
			$80:v:01:t:120;		- configura o tempo de irrigação [min];
			$80:v:01:t;			- mostra o tempo de irrigação do setor;
			$80:v:01:p:68;		- configura pressão nominal do setor [m.c.a.];
			$80:v:01:p;			- mostra pressão nominal do setor;
			$80:t;				- test all solenoid drivers;

		$84:04F3;				- PCY8575 put 16 bit hex value directly to PCY8575;
		$85;					- PCY8575 get output
		$86:x;
			$86:d;				- PCY8575 get adc stream data;
			$86:i;				- PCY8575 get Irms;
			$86:n:397;			- PCY8575 define the number of points to ADC array (default is 397);
			$86:n;				- PCY8575 ask the current n samples;
			$86:u;				- PCY8575 uptime;
		$87;					- PCY8575 temperature;
		$88;					- PCY8574 probe;
		$89;					- PCY8575 soft reset;

	$9X;						- System administration
		$90:[0-9];				- WiFi
			$90:0;				- WiFi AP info;
			$90:1;				- WiFi Scan;
			$90:2;				- Show mac address;
		$91;					- show firwmare version;
		$95:[0-9];				- OTA firmware update
			$95:0;				- show ota partitions info;
			$95:1;				- show ota app info
			$95:2;				- print sha256;
			$94:3:				- mark invalid;
			$95:4;				- mark valid;
			$95:8:[0-1]			- change boot partition;
			$95:9;				- Start firmware update;
		$96;
			$96:b;				- Show reset reason;
			$96:c;				- Show chip info;
			$96:m;				- Show RAM usage;
			$96:t;				- show ticks per us;
			$96:t:xxxx			- set ticks per clock;
		$99;					- Soft reset system;
*/
	int opcode0 = -1;
	int opcode1 = -1;
	int opcode_sub0 = -1;
	int statusCommand = -1;
	char _aux[3], _aux2[5];
	char buffer[2500] = "not handled\n";

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

	switch (opcode0) {
		case 0: { // $0X; Check status
			switch (opcode1) 
			{
				case 0:	{ // $00; Check basic parameters{
					if(command_str[3]==';') {
						// int a = dt.getHour();
						// sprintf(buffer, "%d", dt.getHour());
						sprintf(buffer, "%.2d:%.2d:%.2d %.2d/%.2d/%.4d up:%.2d:%.2d:%.2d d:%d, s:%d m:%d tday:%lu\n", dt.getHour(),
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
							flag_json_data_back_ = states_flag::enable;
						}
						else if(!opcode_sub0) {
							flag_json_data_back_ = states_flag::disable;
						}
					} // $00:[0|1]; 
					break;
				}
				case 1: { // $01
					if(command_str[3] == ';') {
						// $01; lasts time on values and reasons to shutdown
						memset(buffer, 0, sizeof(buffer));
						char buffer_temp[60];
						DateTime dt0;
						
						for(int i=0; i<9; i++)
						{
							dt0.setUnixTime(pump1_.log_motors[i].time_start);
							sprintf(buffer_temp, "s%d- %.2d/%.2d %.2d:%.2d m:%d t:%lu:%.2lu r:%u\n", i+1, dt0.getDay(),
																								dt0.getMonth(),
																								dt0.getHour(),
																								dt0.getMinute(),
																								static_cast<int>(pump1_.log_motors[i].start_mode),
																								pump1_.log_motors[i].time_elapsed_on/60,
																								timesec_to_sec(pump1_.log_motors[i].time_elapsed_on);
																								static_cast<int>(pump1_.log_motors[i].stop_reason));
							strcat(buffer, buffer_temp);
						}
						strcat(buffer, "\n");
					}
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
						sprintf(buffer, "m:%d k1:%d%d k2:%d%d k3:%d%d Rth:%d P1:%d P2:%d r:%d%d%d\n",
																							static_cast<int>(pump1_.state()),
																							static_cast<int>(pump1_.state_k1_pin()),
																							static_cast<int>(pump1_.state_k1()),
																							static_cast<int>(pump1_.state_k2_pin()),
																							static_cast<int>(pump1_.state_k2()),
																							static_cast<int>(pump1_.state_k3_pin()),
																							static_cast<int>(pump1_.state_k3()),
																							static_cast<int>(pump1_.state_Rth()),
																							pipe1_.pressure_mca(),
																							pipe2_.pressure_mca(),
																							static_cast<int>(pump1_.log_motors[0].stop_reason),
																							static_cast<int>(pump1_.log_motors[1].stop_reason),
																							static_cast<int>(pump1_.log_motors[2].stop_reason)
																							);
					} // $03;
					else if((command_str[3] == ':') && (command_str[4] == 's') && (command_str[5] == '1') && (command_str[6] == ':') && (command_str[9] == ';')) {
						_aux[0] = command_str[7];
						_aux[1] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						pipe1_.pressure_max = atoi(_aux);

						sprintf(buffer, "set press1 max: %d\n", pipe1_.pressure_max); 
					} // $03:s1:72;	- Set max pressure ref for pipe [m.c.a.];
					else if((command_str[3] == ':') && (command_str[4] == 's') && (command_str[5] == '2') && (command_str[6] == ':') && (command_str[9] == ';')) {
						_aux[0] = command_str[7];
						_aux[1] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						pipe2_.pressure_max = atoi(_aux);

						sprintf(buffer, "set press2 max: %d\n", pipe2_.pressure_max); 
					} // $03:s2:72;	- Set max pressure ref for pipe [m.c.a.];
					else if((command_str[3] == ':') && (command_str[4] == 's') && (command_str[5] == '1') && (command_str[6] == ';')) {
						sprintf(buffer, "max press1:%d\n", pipe1_.pressure_max); 
					} // $03:s1;
					else if((command_str[3] == ':') && (command_str[4] == 's') && (command_str[5] == '2') && (command_str[6] == ';')) {
						sprintf(buffer, "max press2:%d\n", pipe2_.pressure_max); 
					} // $03:s2;
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
					} // $03:m;
					else if((command_str[3] == ':') && (command_str[4] == 't') && (command_str[5] == ':') && (command_str[10] == ';')) {
						_aux2[0] = command_str[6];
						_aux2[1] = command_str[7];
						_aux2[2] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux2[3] = command_str[9];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';

						pump1_.time_switch_k_change = atoi(_aux2);

						sprintf(buffer, "set time_switch: %d\n", pump1_.time_switch_k_change);
					} // $03:t:0900;	- Set 900 milliseconds to wait K3 go off before start K2;
					else if((command_str[3] == ':') && (command_str[4] == 't') && (command_str[5] == ';')) {
						sprintf(buffer, "time_switch: %d\n", pump1_.time_switch_k_change);
					} // $03:t; - show time K3 wait to turn on;
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == '1') && (command_str[6] == ':') && (command_str[10] == ';')) {
						_aux2[0] = '0';
						_aux2[1] = command_str[7];
						_aux2[2] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux2[3] = command_str[9];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';

						pipe1_.sensor_pressure_ref = atoi(_aux2);

						sprintf(buffer, "set press1 max ref: %d\n", pipe1_.sensor_pressure_ref);
					} // 	$03:p1:100;	- Set 100 psi the max pressure of sensor;
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == '2') && (command_str[6] == ':') && (command_str[10] == ';')) {
						_aux2[0] = '0';
						_aux2[1] = command_str[7];
						_aux2[2] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux2[3] = command_str[9];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';

						pipe2_.sensor_pressure_ref = atoi(_aux2);

						sprintf(buffer, "set press2 max ref: %d\n", pipe2_.sensor_pressure_ref);
					} // 	$03:p2:100;	- Set 100 psi the max pressure of sensor;
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == '1') && (command_str[6] == ';')) {
						sprintf(buffer, "press1 max ref: %d\n", pipe1_.sensor_pressure_ref);
					} // 	$03:p1;	- Show max pressure of sensor;
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == '2') && (command_str[6] == ';')) {
						sprintf(buffer, "press1 max ref: %d\n", pipe2_.sensor_pressure_ref);
					} // 	$03:p1;	- Show max pressure of sensor;
					else if((command_str[3] == ':') && (command_str[4] == 'b') && (command_str[5] == '2') && (command_str[6] == ';')) {
						sprintf(buffer, "State:%d, count:%lu \n", pipe2_.air_intake_detect_state(), pipe2_.air_detect_count_increase);
					} //	$03:b; - show intake air variables;
					else if((command_str[3] == ':') && (command_str[4] == 'u') && (command_str[5] == ';')) {	
						sprintf(buffer, "k1_on:%d, k1_off:%d\nk2_on:%d, k2_off:%d\nk3_on:%d, k3_off:%d\n", pump1_.time_k1_on_, pump1_.time_k1_off_, pump1_.time_k2_on_, pump1_.time_k2_off_, pump1_.time_k3_on_, pump1_.time_k3_off_);
					} // $03:u; show k time switches;
					break;
				// 	$03:v:32;	- Set pressure for valve load turn on and fill reservoir;
				// 	$03:p:150;	- Set sensor max pressure ref to change the scale [psi];
				// 	$03:b:85;	- Set to 85% the pressure min bellow the current pressure to avoid pipe broken;
				}
				case 7: { // $07; show low press check variables
					if(command_str[3]==';') {
						sprintf(buffer, "pipe1: s:%d c_inc:%d t_inc:%d t_stable:%d\npipe2: s:%d c_inc:%d t_inc:%d t_stable:%d\n",
																					static_cast<int>(pipe1_.air_detect_state),
																					static_cast<int>(pipe1_.air_detect_count_increase),
																					static_cast<int>(pipe1_.air_detect_timer_increase),
																					static_cast<int>(pipe1_.air_detect_timer_stable),
																					static_cast<int>(pipe2_.air_detect_state),
																					static_cast<int>(pipe2_.air_detect_count_increase),
																					static_cast<int>(pipe2_.air_detect_timer_increase),
																					static_cast<int>(pipe2_.air_detect_timer_stable));
					} else if((command_str[3] == ':') && (command_str[4] == 'l') && (command_str[5] == '1') && (command_str[6] == ';')) {
					//	$07:l1; - show low pressure to slope machine;
						sprintf(buffer, "press1 low threshold: %d\n", pipe1_.air_detect_pressure_low_ref);
					} else if((command_str[3] == ':') && (command_str[4] == 'l') && (command_str[5] == '1') && (command_str[6] == ':') && (command_str[9] == ';')) {
					//	$07:l1:10; - set threshold low pressure to slope machine;
						_aux[0] = command_str[7];
						_aux[1] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';

						pipe1_.air_detect_pressure_low_ref = atoi(_aux);
						sprintf(buffer, "set press1 low threshold: %d\n", pipe1_.air_detect_pressure_low_ref);
					} else if((command_str[3] == ':') && (command_str[4] == 'l') && (command_str[5] == '2') && (command_str[6] == ';')) {
					//	$07:l2; - show low pressure to slope machine;
						sprintf(buffer, "press2 low threshold: %d\n", pipe2_.air_detect_pressure_low_ref);
					} else if((command_str[3] == ':') && (command_str[4] == 'l') && (command_str[5] == '2') && (command_str[6] == ':') && (command_str[9] == ';')) {
					//	$07:l2:10; - set threshold low pressure to slope machine;
						_aux[0] = command_str[7];
						_aux[1] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';

						pipe2_.air_detect_pressure_low_ref = atoi(_aux);
						sprintf(buffer, "set press2 low threshold: %d\n", pipe2_.air_detect_pressure_low_ref);
					}
					break;
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
				default:{
					sprintf(buffer, "command not implemented\n");
					break;
				}
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
					if((command_str[3] == ':') && (command_str[4] == 'd') && (command_str[5] == ':') && (command_str[9] == ';')) {
					// $21:d:098; - set led duty cycle to 98 %;
						_aux2[0] = '0';
						_aux2[1] = command_str[6];
						_aux2[2] = command_str[7];
						_aux2[3] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';
						uint32_t pwm_led_duty = (uint32_t) atoi(_aux2);
						led_wifi.set_duty(pwm_led_duty);

						sprintf(buffer, "pwm duty: %lu", pwm_led_duty);
					}					
					else if((command_str[3] == ':') && (command_str[4] == 'f') && (command_str[5] == ':') && (command_str[10] == ';')) {
					// $21:f:0060;
						_aux2[0] = command_str[6];
						_aux2[1] = command_str[7];
						_aux2[2] = command_str[8];
						_aux2[3] = command_str[9];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';
						uint32_t pwm_led_frequency = (uint32_t) atoi(_aux2);
						led_wifi.set_frequency(pwm_led_frequency);
						sprintf(buffer, "pwm freq: %lu", pwm_led_frequency);
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
					if(command_str[3]==';') {
						// $30; turn off by command line
						pump1_.stop(stop_types::command_line_user);
						sprintf(buffer, "Motor stop. s:%d\n", static_cast<uint8_t>(pump1_.state()));
					}
					break;
				}
				case 1: {
					if(command_str[3]==';')
					{
						pump1_.start(start_types::direct_k1);
						sprintf(buffer, "Motor start. k%d s:%d\n", opcode1, static_cast<uint8_t>(pump1_.state()));
					}
					break;
				}
				case 2: {
					if(command_str[3]==';')
					{
						pump1_.start(start_types::direct_k2);
						sprintf(buffer, "Motor start. k%d s:%d\n", opcode1, static_cast<uint8_t>(pump1_.state()));
					}
					break;
				}
				case 3: {
					if(command_str[3]==';')
					{
						pump1_.start(start_types::direct_k3);
						sprintf(buffer, "Motor start. k%d s:%d\n", opcode1, static_cast<uint8_t>(pump1_.state()));
					}
					break;
				}
				case 4: {
					if(command_str[3]==';')
					{
						pump1_.start(start_types::to_delta);
						sprintf(buffer, "Motor start Delta. s:%d\n", static_cast<uint8_t>(pump1_.state()));
					}
					break;
				}
				case 5: {
					if(command_str[3]==';')
					{
						pump1_.start(start_types::to_y);
						sprintf(buffer, "Motor start Y. s:%d\n", static_cast<uint8_t>(pump1_.state()));
					}
					break;
				}
				case 6: {
					if(command_str[3]==';')
					{
						pump1_.start(start_types::y_delta_req);
						sprintf(buffer, "Motor start Y/Delta. s:%d\n", static_cast<uint8_t>(pump1_.state()));
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
					if(command_str[3] == ';') {
						// $50; show all info and timers
						memset(buffer, 0, sizeof(buffer));
						sprintf(buffer,"auto- f:%d n:%d\n", (int)flag_check_time_match_, time_match_n);
						char buffer_temp[30];
						for(int i=0; i<time_match_n; i++)
						{
							sprintf(buffer_temp, "h%d:%.2d:%.2d m:%d t:%d\n", i+1, (int)timesec_to_hour(time_match_list[i].time_match), (int)timesec_to_min(time_match_list[i].time_match), (int)time_match_list[i].auto_start_mode, static_cast<int>(time_match_list[i].time_to_shutdown/60));
							strcat(buffer, buffer_temp);
						}
						// strcat(buffer, "\n");
					}
					else if((command_str[3] == ':') && (command_str[4] == 's') && (command_str[5] == ':') && (command_str[7] == ';')) {
						// $50:X;	set auto mode [1|0] ON/OFF;
						_aux[0] = '0';
						_aux[1] = command_str[6];		// '0' in uint8_t is 48. ASCII
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

						// get the hours, transform to seconds;
						_aux[0] = command_str[7];
						_aux[1] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						uint32_t time_temp = (static_cast<uint32_t>(atoi(_aux)))*3600;

						// get the minutes, transform to seconds and sum it;
						_aux[0] = command_str[9];
						_aux[1] = command_str[10];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						time_temp += (static_cast<uint32_t>(atoi(_aux)))*60;

						time_match_list[index].time_match = time_temp;

						sprintf(buffer, "set h%u %.2u:%.2u tm:%lu\n", index+1, timesec_to_hour(time_match_list[index].time_match), timesec_to_min(time_match_list[index].time_match), time_match_list[index].time_match);
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
									time_match_list[i].auto_start_mode = (start_types)status_set;
								}
								sprintf(buffer, "set all elements\n");
							}
							else		// fill one element only
							{
								time_match_list[index-1].auto_start_mode = (start_types)status_set;
								sprintf(buffer, "set auto start m: %d\n", static_cast<int>(time_match_list[index-1].auto_start_mode));
							}
						}
					}
					else if((command_str[3] == ':') && (command_str[4] == 't') && (command_str[6] == ':') && (command_str[10] == ';')) {
						// $50:t1:060;
						_aux[0] = '0';
						_aux[1] = command_str[5];
						_aux[2] = '\0';
						int index = atoi(_aux);
						
						_aux2[0] = '0';
						_aux2[1] = command_str[7];
						_aux2[2] = command_str[8];
						_aux2[3] = command_str[9];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';

						uint32_t time_temp = static_cast<uint32_t>(atoi(_aux2)*60.0);

						if(!index)	// fill all elements with same value
						{
							for(int i=0; i<9; i++)
							{
								time_match_list[i].time_to_shutdown = time_temp;
							}
							sprintf(buffer, "set all t:%d\n", static_cast<int>(time_temp));
						}
						else		// fill one element only
						{
							time_match_list[index-1].time_to_shutdown = time_temp;
							sprintf(buffer, "set t2sd[%d]: %d min\n", index, static_cast<int>(time_match_list[index-1].time_to_shutdown/60.0));
						}
					}
					break;
				}
				case 1: { // for optimized
					if(command_str[3] == ';') {
						// $51; show tm optimized setup

						memset(buffer, 0, sizeof(buffer));
						char buffer_temp[60];

						sprintf(buffer, "opt- f:%d n:%d h:%.2d:%.2d td:%d nf:%d ns:%.2d:%.2d r:%.2d:%.2d ev:%d cy:%d\n",
																		static_cast<int>(flag_check_time_match_optimized_),
																		optimized.event0_n_max,
																		timesec_to_hour(optimized.time_match_start),
																		timesec_to_min(optimized.time_match_start),
																		timesec_to_min(optimized.time_delay),
																		static_cast<int>(optimized.flag_time_next_config),
																		timesec_to_hour(optimized.time_match_next),
																		timesec_to_min(optimized.time_match_next),
																		timesec_to_hour(optimized.time_red),
																		timesec_to_min(optimized.time_red),
																		optimized.event0_i,
																		optimized.cycles_i);

						for(int i=0; i<optimized.event0_n_max; i++) {
							sprintf(buffer_temp, "%d- m:%d t:%lu c:%d\n", i+1,
																		static_cast<int>(optimized.event0[i].start_mode),
																		optimized.event0[i].time_to_shutdown/60,
																		optimized.event0[i].cycles_n_max);
							strcat(buffer, buffer_temp);
						}
						strcat(buffer, "\n");
					}
					else if((command_str[3] == ':') && (command_str[4] == 's') && (command_str[5] == ':') && (command_str[7] == ';')) {
						// $51:s:X;	set auto mode [1|0] ON/OFF;
						_aux[0] = '0';
						_aux[1] = command_str[6];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						int opcode_sub0 = atoi(_aux);

						if(opcode_sub0)
							flag_check_time_match_optimized_ = states_flag::enable;
						else
							flag_check_time_match_optimized_ = states_flag::disable;

						sprintf(buffer, "set optz tm: %d\n", static_cast<int>(flag_check_time_match_optimized_));
					}
					else if((command_str[3] == ':') && (command_str[4] == 'd') && (command_str[5] == ';')) {
					// $51:d;	- time delay before next start in minutes;
						sprintf(buffer, "HeloW! optz delay:%lu\n", optimized.time_delay/60);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'd') && (command_str[5] == ':') && (command_str[8] == ';')) {
					// $51:d:05;	- time delay in minutes;
						_aux[0] = command_str[6];
						_aux[1] = command_str[7];
						_aux[2] = '\0';
						optimized.time_delay = static_cast<uint32_t>(atoi(_aux))*60;

						sprintf(buffer, "set optz delay:%lu min\n", optimized.time_delay/60);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'n') && (command_str[5] == ':') && (command_str[7] == ';')) {
					// $51:n:N;
						_aux[0] = command_str[6];
						_aux[1] = command_str[7];
						_aux[2] = '\0';
						optimized.event0_n_max = atoi(_aux);

						sprintf(buffer, "set evt n:%d\n", optimized.event0_n_max);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'h') && (command_str[5] == ':') && (command_str[10] == ';')) {
						// $51:h:hhmm;
						// get the hours, transform to seconds;
						_aux[0] = command_str[6];
						_aux[1] = command_str[7];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						uint32_t _time_temp = (static_cast<uint32_t>(atoi(_aux)))*3600;

						// get the minutes, transform to seconds and sum it;
						_aux[0] = command_str[8];
						_aux[1] = command_str[9];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						_time_temp += (static_cast<uint32_t>(atoi(_aux)))*60;

						optimized.time_match_start = _time_temp;

						sprintf(buffer, "opt set h%.2u:%.2u tm:%lu\n", timesec_to_hour(optimized.time_match_start), timesec_to_min(optimized.time_match_start), optimized.time_match_start);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'r') && (command_str[5] == ';')) {
						// $51:r;
						sprintf(buffer, "opt red time %.2u:%.2u\n", timesec_to_hour(optimized.time_red), timesec_to_min(optimized.time_red));
					}
					else if((command_str[3] == ':') && (command_str[4] == 'r') && (command_str[5] == ':') && (command_str[10] == ';')) {
						// $51:r:HHMM- set red time;
						// get the hours, transform to seconds;
						_aux[0] = command_str[6];
						_aux[1] = command_str[7];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						uint32_t _time_temp = (static_cast<uint32_t>(atoi(_aux)))*3600;

						// get the minutes, transform to seconds and sum it;
						_aux[0] = command_str[8];
						_aux[1] = command_str[9];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						optimized.time_red = _time_temp + (static_cast<uint32_t>(atoi(_aux)))*60;

						sprintf(buffer, "optz set r:%.2u:%.2u ts:%lu\n", timesec_to_hour(optimized.time_red), timesec_to_min(optimized.time_red), optimized.time_red);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'e') && (command_str[6] == ':') && (command_str[7] == 'm')  && (command_str[9] == ':') && (command_str[11] == ':') && (command_str[15] == ';')) {
						// $51:e1:m3:2:120;

						// event number
						_aux[0] = '0';
						_aux[1] = command_str[5];
						_aux[2] = '\0';
						int index = static_cast<int>(atoi(_aux)-1);

						// motor start type (must convert to enum class)
						_aux[0] = '0';
						_aux[1] = command_str[8];
						_aux[2] = '\0';
						optimized.event0[index].start_mode = static_cast<start_types>(atoi(_aux));

						// number of cycles for this specific event
						_aux[0] = '0';
						_aux[1] = command_str[10];
						_aux[2] = '\0';
						optimized.event0[index].cycles_n_max = static_cast<int>(atoi(_aux));

						// maximum time it can be on after start
						_aux2[0] = '0';
						_aux2[1] = command_str[12];
						_aux2[2] = command_str[13];
						_aux2[3] = command_str[14];
						_aux2[4] = '\0';
						optimized.event0[index].time_to_shutdown = static_cast<uint32_t>(atoi(_aux2)*60);

						sprintf(buffer, "set e%d m%d c:%d t:%lu\n", index+1, static_cast<int>(optimized.event0[index].start_mode), optimized.event0[index].cycles_n_max, optimized.event0[index].time_to_shutdown/60);
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
				case 0: {
					state_mode = states_mode::system_off;
					sprintf(buffer,"state mode: system_off, s:%d\n", static_cast<int>(state_mode));
					break;
				}
				case 1:	{
					state_mode = states_mode::system_ready;
					sprintf(buffer,"state mode: system_ready, s:%d\n", static_cast<int>(state_mode));
					break;
				}
				case 2: {
					state_mode = states_mode::irrigation_pump_valves;
					sprintf(buffer,"state mode: auto irrigation s:%d\n", static_cast<int>(state_mode));
					break;
				}
				case 3: {
					state_mode = states_mode::water_pump_control_night;
					sprintf(buffer,"state mode: pumping all night s:%d\n", static_cast<int>(state_mode));
					break;
				}
				default:
					sprintf(buffer,"mode not implemented\n");
					break;
			}
			ESP_LOGI(TAG_ACIONNA,"Acionna State: %d", static_cast<int>(state_mode));
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
								sprintf(buffer, "don't check k: %d\n", (int)pump1_.flag_check_output_pin_only);
					}
					else if ((command_str[3] == ':') && (command_str[4] == 'k') && (command_str[5] == 't') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:kt:[0|1];
						if(opcode_sub0)
							pump1_.flag_check_output_pin_only = states_flag::enable;
						else
							pump1_.flag_check_output_pin_only = states_flag::disable;
						
						sprintf(buffer, "set don't check k:%d\n", (int)pump1_.flag_check_output_pin_only);
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
					else if((command_str[3] == ':') && (command_str[4] == 'l') && (command_str[5] == '1') && (command_str[6] == ';')) {
					// $70:l1;
						sprintf(buffer, "check low press k1: %d\n", static_cast<int>(flag_check_low_pressure_k1_));
					}
					else if((command_str[3] == ':') && (command_str[4] == 'l') && (command_str[5] == '1') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:l1:[0|1];
						if(opcode_sub0) {
							flag_check_low_pressure_k1_ = states_flag::enable;
							flag_check_low_pressure_delta_ = states_flag::disable;
						}
						else
							flag_check_low_pressure_k1_ = states_flag::disable;
					
						sprintf(buffer, "set check low press k1: %d\n", static_cast<int>(flag_check_low_pressure_k1_));
					}
					else if((command_str[3] == ':') && (command_str[4] == 'l') && (command_str[5] == '2') && (command_str[6] == ';')) {
					// $70:l2;
						sprintf(buffer, "check low press k2: %d\n", static_cast<int>(flag_check_low_pressure_k2_));
					}
					else if((command_str[3] == ':') && (command_str[4] == 'l') && (command_str[5] == '2') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:l2:[0|1];
						if(opcode_sub0)
							flag_check_low_pressure_k2_ = states_flag::enable;
						else
							flag_check_low_pressure_k2_ = states_flag::disable;
					
						sprintf(buffer, "set check low press k2: %d\n", static_cast<int>(flag_check_low_pressure_k2_));
					}
					else if((command_str[3] == ':') && (command_str[4] == 'l') && (command_str[5] == '4') && (command_str[6] == ';')) {
					// $70:l4;
						sprintf(buffer, "check low press delta: %d\n", static_cast<int>(flag_check_low_pressure_delta_));
					}
					else if((command_str[3] == ':') && (command_str[4] == 'l') && (command_str[5] == '4') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:l4:[0|1];
						if(opcode_sub0) {
							flag_check_low_pressure_delta_ = states_flag::enable;
							flag_check_low_pressure_k1_ = states_flag::disable;
						}
						else
							flag_check_low_pressure_delta_ = states_flag::disable;
					
						sprintf(buffer, "set check k1k2 %d\n", static_cast<int>(flag_check_low_pressure_delta_));
					}
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == 'h') && (command_str[6] == ';')) {
					// $70:ph;
						sprintf(buffer, "check press high: %d\n", (int)flag_check_pressure_high_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == 'h') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:ph:[0|1]; desligamento por alta pressão;
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
					// $70:pl:[0-9];			- desligamento por pressão baixa em min caso seja diferente de 0;
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
					// $70:pv:[0|1];			- desligamento por pressão alta por válvula;
						if(opcode_sub0)
							flag_check_pressure_valve_ = states_flag::enable;
						else
							flag_check_pressure_valve_ = states_flag::disable;

						sprintf(buffer, "set check press valve: %d\n", (int)flag_check_pressure_valve_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'v') && (command_str[5] == 'c') && (command_str[6] == ';')) {
					// $70:vc;			- show verificação constante das valvulas em pleno funcionamento
						sprintf(buffer, "check valves: %d\n", (int)flag_check_valves_);
					}
					else if((command_str[3] == ':') && (command_str[4] == 'v') && (command_str[5] == 'c') && (command_str[6] == ':') && (command_str[8] == ';')) {
					// $70:vc:[0|1];			- verificação constante das valvulas em pleno funcionamento
						if(opcode_sub0)
							flag_check_valves_ = states_flag::enable;
						else
							flag_check_valves_ = states_flag::disable;

						sprintf(buffer, "set check valves: %d\n", (int)flag_check_valves_);
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
			switch (opcode1) 
			{
				case 0: {
					if (command_str[3] == ';') {
						// $80;
						sprintf(buffer, "sV:%d d_inv:%d tsOn:%.2d:%.2d:%.2d Tcfg:%.2d:%.2d:%.2d, v:%.2d tvOn:%.2d:%.2d:%.2d Pcfg:%d tv_cfg:%d, \n",
																											(int)valves1_.state_valves,
																											(int)valves1_.flag_inverted_sequence,
																											timesec_to_hour(valves1_.get_time_on()),
																											timesec_to_min(valves1_.get_time_on()),
																											timesec_to_sec(valves1_.get_time_on()),
																											timesec_to_hour(valves1_.get_total_time_programmed()),
																											timesec_to_min(valves1_.get_total_time_programmed()),
																											timesec_to_sec(valves1_.get_total_time_programmed()),
																											valves1_.valve_current(),
																											timesec_to_hour(valves1_.time_valve_remain),
																											timesec_to_min(valves1_.time_valve_remain),
																											timesec_to_sec(valves1_.time_valve_remain),
																											valves1_.get_valve_pressure(valves1_.valve_current()),
																											valves1_.get_valve_time(valves1_.valve_current())																									
																											);
					}
					else if ((command_str[3] == ':') && (command_str[4] == 's') && (command_str[5] == ':') && (command_str[7] == ';')) {
					// $80:s:[0|1];
						_aux[0] = '0';
						_aux[1] = command_str[6];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						int status_code = atoi(_aux);

						if(status_code) {
							valves1_.start();
							sprintf(buffer, "Started valves");
						}
						else {
							valves1_.stop();
							sprintf(buffer, "Stoped valves");
						}
					} else if ((command_str[3] == ':') && (command_str[4] == 'd') && (command_str[5] == ':') && (command_str[7] == ';')) {
					// $80:d:[0|1];
						_aux[0] = '0';
						_aux[1] = command_str[6];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						int status_code = atoi(_aux);

						if(status_code) {
							valves1_.flag_inverted_sequence = states_flag::enable;
							sprintf(buffer, "sentido %d\n", (int)valves1_.flag_inverted_sequence);
						} else {
							valves1_.flag_inverted_sequence = states_flag::disable;
							sprintf(buffer, "sentido %d\n", (int)valves1_.flag_inverted_sequence);
						}
					} else if ((command_str[3] == ':') && (command_str[4] == 'h') && (command_str[5] == ';')) {
					// $80:h;	// show valves sequence history

						char buffer_temp[85] = {};
						sprintf(buffer, "Valves history: \n");
						DateTime dt0;

						for(auto i=0; i<valves1_.log_n; i++) {
							dt0.setUnixTime(valves1_.log_valves[i].started_time);
							sprintf(buffer_temp, "%d- v%d %.2d/%.2d %.2d:%.2d t:%d p:%d [%d:%d]\n", i,
																						valves1_.log_valves[i].valve_id,
																						dt0.getDay(),
																						dt0.getMonth(),
																						dt0.getHour(),
																						dt0.getMinute(),
																						static_cast<int>(valves1_.log_valves[i].elapsed_time/60.0),
																						valves1_.log_valves[i].pressure_avg,
																						valves1_.log_valves[i].pressure_min,
																						valves1_.log_valves[i].pressure_max);
							strcat(buffer, buffer_temp);
						}
						strcat(buffer, "\n");
					} else if ((command_str[3] == ':') && (command_str[4] == 'n') && (command_str[5] == ';')) {
					// $80:n;
						valves1_.next_forced();
						sprintf(buffer, "valves: next command\n");
					} else if ((command_str[3] == ':') && (command_str[4] == 'v') && (command_str[5] == ':')) {
					// $80:v:01
						_aux[0] = command_str[6];
						_aux[1] = command_str[7];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						int valve_id = atoi(_aux);

						if (valve_id <= valves1_.number_valves)
						{
							if(command_str[8] == ';') {
								// $80:v:01;
								if(!valve_id) {
									memset(buffer, 0, sizeof(buffer));
									char buffer_temp[30];
									for(int i=1; i<=valves1_.number_valves; i++) {
										sprintf(buffer_temp, "v[%02d]:%d pg:%d t:%d p:%d\n", i, (int)valves1_.get_valve_state(i), (int)valves1_.get_valve_programmed(i), valves1_.get_valve_time(i), valves1_.get_valve_pressure(i));
										strcat(buffer, buffer_temp);
									}
									strcat(buffer, "\n");
								}
								else {
									sprintf(buffer, "v[%02d]:%d pg:%d t:%d min p:%d m.c.a.\n", valve_id, (int)valves1_.get_valve_state(valve_id), (int)valves1_.get_valve_programmed(valve_id), valves1_.get_valve_time(valve_id), valves1_.get_valve_pressure(valve_id));
								}
							} else if((command_str[8] == ':') && (command_str[9] == '0') && (command_str[10] == ';')) {
								// $80:v:01:0; - set valve off
								if(valve_id) {
									valves1_.set_valve_state(valve_id, 0);
									sprintf(buffer, "set valve[%d]:%d", valve_id, (int)valves1_.get_valve_state(valve_id));
								} else {
									// for(int i=0; i<valves1_.number_valves; i++) {
									// 	valves1_.set_valve_state(i+1, 0);
									// }
									valves1_.module_put(0x0000);
									sprintf(buffer, "reset all valves");
								}
							} else if((command_str[8] == ':') && (command_str[9] == '1') && (command_str[10] == ';')) {
								// $80:v:01:1; - set valve on
								if(valve_id) {
									valves1_.set_valve_state(valve_id, 1);
									sprintf(buffer, "set valve[%d]:%d", valve_id, (int)valves1_.get_valve_state(valve_id));
								} else {
									// for(int i=0; i<valves1_.number_valves; i++) {
									// 	valves1_.set_valve_state(i+1, 1);
									// }
									valves1_.module_put(0x07FF);
									sprintf(buffer, "set all valves");
								}
							} else if((command_str[8] == ':') && (command_str[9] == 'i') && (command_str[10] == ';')) {
								// $80:v:01:i;	insere setor na programação;
								if(valve_id) {
									valves1_.add(valve_id);
									sprintf(buffer, "added valve %d: %d\n", valve_id, (int)valves1_.get_valve_programmed(valve_id));
								} else {
									for(int i=0; i<valves1_.number_valves; i++) {
										valves1_.add(i+1);
									}
									sprintf(buffer, "added all valves");
								}
							} else if((command_str[8] == ':') && (command_str[9] == 'r') && (command_str[10] == ';')) {
								// $80:v:01:r;	remove setor da programação;
								if(valve_id) {
									valves1_.remove(valve_id);
									sprintf(buffer, "removed valve %d: %d\n", valve_id, (int)valves1_.get_valve_programmed(valve_id));								
								} else {
									for(int i=0; i<valves1_.number_valves; i++) {
										valves1_.remove(i+1);
									}
									sprintf(buffer, "removed all valves");
								}
							} else if((command_str[8] == ':') && (command_str[9] == 't') && (command_str[10] == ';')) {
								int valve_time = valves1_.get_valve_time(valve_id);
								sprintf(buffer, "valve %d: %d min\n", valve_id, valve_time);
							} else if((command_str[8] == ':') && (command_str[9] == 't') && (command_str[10] == ':') && (command_str[14] == ';')) {
								// $80:v:01:t:120;	- configura o tempo de irrigação [min];
								_aux2[0] = '0';
								_aux2[1] = command_str[11];
								_aux2[2] = command_str[12];
								_aux2[3] = command_str[13];		// '0' in uint8_t is 48. ASCII
								_aux2[4] = '\0';
								uint8_t valve_time = (uint8_t) atoi(_aux2);

								if(valve_id) {
									valves1_.set_valve_time(valve_id, valve_time);
									sprintf(buffer, "valve %d: %d min\n", valve_id, valves1_.get_valve_time(valve_id));
								} else {
									for(int i=0; i<valves1_.number_valves; i++) {
										valves1_.set_valve_time(i+1, valve_time);
									}
									sprintf(buffer, "all times to %d min\n", valve_time);
								}
							} else if((command_str[8] == ':') && (command_str[9] == 'p') && (command_str[10] == ';')) {
								// $80:v:01:p;		- mostra pressão nominal do setor
								sprintf(buffer, "get valve %d: %d m.c.a.\n", valve_id, valves1_.get_valve_pressure(valve_id));
							} else if((command_str[8] == ':') && (command_str[9] == 'p') && (command_str[10] == ':') && (command_str[13] == ';')) {
								// $80:v:01:p:68; - configura pressão nominal do setor [m.c.a.];
								_aux[0] = command_str[11];
								_aux[1] = command_str[12];
								_aux[2] = '\0';
								unsigned int valve_pressure = (unsigned int) atoi(_aux);
								valves1_.set_valve_pressure(valve_id, valve_pressure);
								sprintf(buffer, "set valve %d: %d m.c.a.\n", valve_id, valves1_.get_valve_pressure(valve_id));
							}
						}
					} else {
						sprintf(buffer, "80 not handled\n");
					}
				break;
				}
				case 3: {
					// $83;	get valve irms
					sprintf(buffer, "PCY8575 irms: 0x%04x\n", valves1_.module_irms());
					break;
				}
				case 4: {
					if((command_str[3] == ':') && (command_str[8] == ';')) {
						// $84:04F3;	put directly to PCY8575
						_aux2[0] = command_str[4];
						_aux2[1] = command_str[5];
						_aux2[2] = command_str[6];
						_aux2[3] = command_str[7];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';
						uint16_t output_temp = static_cast<uint16_t>(hexstr_to_int(&_aux2[0], 4));
						ESP_LOGI(TAG_ACIONNA, "output_temp: 0x%04x", output_temp);
						valves1_.module_put(output_temp);
						sprintf(buffer, "PCY8575 put. verify get: 0x%04x\n", valves1_.module_get());
					}
					break;
				}
				case 5: {
					// $85;	get output value
					uint16_t output_temp = valves1_.module_get();
					sprintf(buffer, "PCY8575 output: 0x%04x\n", output_temp);
					break;
				}
				case 6: {
					// $86:x;
					if ((command_str[3] == ':') && (command_str[4] == 'd') && (command_str[5] == ';')) {
						// $86:d;   get adc array data raw;
						valves1_.module_i_data_transfer();
						int length = valves1_.module_i_n_points();
						memset(buffer, 0, sizeof(buffer));
						sprintf(buffer, "ADC array size %d: ", length);
						char buffer_temp[8];
						for(int i=0; i<length; i++) {
							sprintf(buffer_temp, "%u, ", valves1_.module_read_i_sample(i));
							strcat(buffer, buffer_temp);
						}
						strcat(buffer, "\n");
					} else if((command_str[3] == ':') && (command_str[4] == 'i') && (command_str[5] == ';')) {
						// $86:i;  get Irms solenoids calculated value;
						sprintf(buffer, "PCY8575 Irms %.3f A\n", static_cast<float>(valves1_.module_irms()/1000.0));
					} else if((command_str[3] == ':') && (command_str[4] == 'n') && (command_str[5] == ':') && (command_str[9] == ';')) {
						// $86:n:397;
						_aux2[0] = '0';
						_aux2[1] = command_str[6];
						_aux2[2] = command_str[7];
						_aux2[3] = command_str[8];		// '0' in uint8_t is 48. ASCII
						_aux2[4] = '\0';
						int _n_points = atoi(_aux2);
						valves1_.module_i_n_points(_n_points);

						sprintf(buffer, "PCY8575 n points to: %d\n", _n_points);

					} else if((command_str[3] == ':') && (command_str[4] == 'n') && (command_str[5] == ';')) {
						// $86:n;
						sprintf(buffer, "PCY8575 n points is: %d\n", valves1_.module_i_n_points());
					}
					 else if((command_str[3] == ':') && (command_str[4] == 'p') && (command_str[5] == ';')) {
						// $86:p;  process adc conversion and Irms calculation on pcy8575 module;
						valves1_.module_i_process();
						sprintf(buffer, "PCY8575 process");
					} else if((command_str[3] == ':') && (command_str[4] == 't') && (command_str[5] == ';')) {
						// $87;	get module temperature
						sprintf(buffer, "PCY8575 temp: 0x%04x\n", valves1_.module_temperature());
					} else if((command_str[3] == ':') && (command_str[4] == 'u') && (command_str[5] == ';')) {
						// $86:u; - get module uptime
						uint32_t uptime_temp = valves1_.module_uptime();
						sprintf(buffer, "PCY8575 uptime %02u:%02u:%02u d:%d\n", timesec_to_hour(uptime_temp), timesec_to_min(uptime_temp), timesec_to_sec(uptime_temp), timesec_to_day(uptime_temp));
					} else if((command_str[3] == ':') && (command_str[4] == 'v') && (command_str[5] == ';')) {
						sprintf(buffer, "PCY8575 data test\n");
						valves1_.module_data_test();
					}
					break;					
				}
				case 8: {
					// $88;	just probe
					sprintf(buffer, "PCY8575 probe: %s\n", valves1_.module_probe() ? "true" : "false");
					break;
				}
				case 9: {
					// $89; module soft reset
					valves1_.module_reset();
					sprintf(buffer, "reset valve mod ctrl\n");
					break;
				}
				default: {
					break;
				}
			}
			break;
		}
		case 9: {
			switch(opcode1)
			{
				case 0: {
					if((command_str[3] == ':') && (command_str[5] == ';')) {
						// $90:X;
						_aux[0] = '0';
						_aux[1] = command_str[4];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						int opcode_sub2 = atoi(_aux);
						
						switch (opcode_sub2) {
							case 0: {
								sys_wifi_info_(buffer);
								break;
							}
							case 1: {
								// $90:1; scan
								sys_wifi_scan_(buffer);
								break;
							}
							case 2: {
								// $90:1; scan
								sys_wifi_mac_(buffer);
								break;
							}
							default:
							break;
						}
					}
					// $90:2;
					break;
				}
				case 1: {
					if(command_str[3] == ';') {
						// $91;	- firmware version
						char fw_version[33];
						memset(buffer, 0, sizeof(buffer));
						strcpy(fw_version, "fw: ");
						strcat(fw_version, __DATE__);
						strcat(fw_version, " ");
						strcat(fw_version, __TIME__);
						strcat(fw_version, "\n");
						strcpy(buffer, fw_version);
					}
					break;
				}
				case 5: { //$95:[0-9];
					if(command_str[3] == ':' && command_str[5] == ';') {
						_aux[0] = '0';
						_aux[1] = command_str[4];		// '0' in uint8_t is 48. ASCII
						_aux[2] = '\0';
						int opcode_sub2 = atoi(_aux);

						switch (opcode_sub2) {
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
				case 6: {
					// $96;
					if((command_str[3] == ':') && (command_str[4] == 'b') && (command_str[5] == ';')) {
						// $96:b;
						sys_reset_reason_(buffer);
					} else if((command_str[3] == ':') && (command_str[4] == 'c') && (command_str[5] == ';')) {
						// $96:c;
						sys_chip_info(buffer);
					} else if((command_str[3] == ':') && (command_str[4] == 'm') && (command_str[5] == ';')) {
						// $96:m;
						sys_ram_free_(buffer);
					} else if((command_str[3] == ':') && (command_str[4] == 't') && (command_str[5] == ';')) {
						// $96:t;
						sys_ticks_per_us(buffer);
					}
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
void Acionna::init() {

	// Clock time init
	dt.setDate(2023, 07, 10);
	dt.setTime(0, 0, 0, ND);
	device_clock.set_time(dt.getUnixTime());
	time_day_sec_ = dt.getHour()*3600 + dt.getMinute()*60 + dt.getSecond();

	// wifi and ws server init - ws server will ask to init into connection_event_handler when got IP.
	#ifdef CONFIG_IP_END_FORCE
	wifi_ip_end = CONFIG_IP_END;
	wifi_sta_init(wifi_ip_end);
	#else

	// static ip select using the inner mac address.
	ip_get_mode = ip_get_types::static_ip;
	char mac_device[18] = {};						// must initialize with zeros because the use of strcat();

	char buffer_temp[5] = {};
	uint8_t wifi_mac_[6] = {};
	wifi_get_mac(&wifi_mac_[0]);

	// 3 mac address listed - 18 length + 1 '\0' = 19 
	char mac_table[4][18] = {
							{"84:cc:a8:69:f6:f0"},	// .31 - test device;
							{"84:cc:a8:69:97:7c"},	// .32 - poço cacimba;
							{"84:cc:a8:69:9c:4c"},	// .33 - irrigação.
							{"84:cc:a8:69:41:34"}};	// .34 - new one.

	// Converting uint8_t vector mac address to string
	for(int i=0; i<6; i++) {
		sprintf(buffer_temp, "%02x", wifi_mac_[i]);
		strcat(mac_device, buffer_temp);
		if(i < 5) {
			strcat(mac_device, ":");
		}
	}
	// Compare the mac and set ip address end byte;
	if(strcmp(mac_device, &mac_table[0][0]) == 0) {
		wifi_ip_end = 31;
	} else if(strcmp(mac_device, &mac_table[1][0]) == 0) {
		wifi_ip_end = 32;
	} else if(strcmp(mac_device, &mac_table[2][0]) == 0) {
		wifi_ip_end = 33;
	} else if(strcmp(mac_device, &mac_table[3][0]) == 0) {
		wifi_ip_end = 34;
	} else {
		wifi_ip_end = 30;
	}
	
	wifi_sta_init(wifi_ip_end);
	#endif

	// Bluetooth init
	#ifdef CONFIG_BT_ENABLE
	bt_init();
	#endif
	// Acionna init
	// ws client initialize
	// ws_client_start();


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
	ESP_LOGI(TAG_ACIONNA, "initialized!");
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
	
	// this variable is changed on ws_server_event_handler() event function into httpd_setup.cpp
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

	// if(ws_client_data_flag_) {

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

	// bluetooth
	#ifdef CONFIG_BT_ENABLE
	if(bt_ans_flag_ == states_flag::enable)
	{
		bt_ans_flag_ = states_flag::disable;
		// Send bt msg back;
		ESP_LOGI(TAG_ACIONNA, "Sending msg back over BT\n");
		bt_send(msg_back_str_);
	}
	#endif

	// websocket server
	if(ws_server_ans_flag_ == states_flag::enable)
	{
		ws_server_ans_flag_ = states_flag::disable;
		// Send websocket msg back
		ws_server_send(msg_back_str_);
	}

	// websocket client answer to server
	if(ws_client_ans_flag_ == states_flag::enable)
	{
		ws_client_ans_flag_ = states_flag::disable;

		// send ws client back
		ws_client_send(msg_back_str_);
	}
}
void Acionna::msg_json_back_(void) {
	if(ws_server_client_state == conn_states::connected) {
		if(flag_json_data_back_ == states_flag::enable) {

			char buffer_str[150];
			DynamicJsonDocument doc(1024);
			doc["id"] = wifi_ip_end;
			doc["p1"] = pipe1_.pressure_mca();
			doc["p2"] = pipe2_.pressure_mca();
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
		if(flag_json_data_back_ == states_flag::enable) {
			flag_json_data_back_ = states_flag::disable;
		}
	}
}
void Acionna::operation_mode(void) {

	// for all operation modes
	operation_pump_stop_check();

	switch (state_mode)
	{
		case states_mode::system_off:				// $60;
			operation_pump_lock_down();
			break;

		case states_mode::system_ready:				// $61;
			operation_pump_start_match();
			break;

		case states_mode::irrigation_pump_valves:	// $62;
			operation_pump_start_match();
			operation_pump_valves();
			break;

		case states_mode::water_pump_control_night:	// $63;
			operation_pump_start_match_optimized();
			operation_pump_optimized();
			break;

		default:
			operation_pump_lock_down();
			break;
	}
}
void Acionna::operation_pump_start_match(void) {

	// time matches occurred into check_time_match(), start motor
	if(flag_check_time_match_ == states_flag::enable) {

		int index_found = 0;
		// sweep vector programmed time and check each one with current time.
		for(int i=0; i<time_match_n; i++) {
			// compare the list time match with current time day second
			if(time_match_list[i].time_match == time_day_sec_)
			{
				flag_time_match_ = states_flag::enable;
				index_found = i;
				// break;
			}
			// ESP_LOGI(TAG_ACIONNA, "TM FOR CHECK! tml:%d tds:%d", time_match_list[i].time_match, time_day_sec_);
		}


		if(flag_time_match_ == states_flag::enable) {
			flag_time_match_ = states_flag::disable;

			// ESP_LOGI(TAG_ACIONNA, "FLAG TIME MATCH!");
			if(pump1_.state() == states_motor::off_idle) {
				// If time match occurs and motor state is idle, turn it on! And make some log;
				pump1_.start(time_match_list[index_found].auto_start_mode);
				// If exists time value registered, use it. Else, use default.
				if(time_match_list[index_found].time_to_shutdown) {
					pump1_.time_to_shutdown = time_match_list[index_found].time_to_shutdown;
				}
			}
		}
	}
}
void Acionna::operation_pump_start_match_optimized(void) {
	// global enable flag. If time match optimized enable, working on it for pump the water to reservoir
	if(flag_check_time_match_optimized_ == states_flag::enable) {

		// if day time match with start time else, and clear counter parameters or
		// else if day time match with next time, enable start motor flag only
		if(optimized.time_match_start == time_day_sec_) {
			flag_time_match_optimized_ = states_flag::enable;

			// Reset counter parameters for next event cycle
			optimized.event0_i = 0;
			optimized.cycles_i = -1;
			// ESP_LOGI(TAG_ACIONNA, "optimized: start time match");
		} else if(optimized.time_match_next == time_day_sec_) {
			flag_time_match_optimized_ = states_flag::enable;
			// ESP_LOGI(TAG_ACIONNA, "optimized: next time match");
		}

		// if found time match optimized (from start time or next)
		if(flag_time_match_optimized_ == states_flag::enable) {
			flag_time_match_optimized_ = states_flag::disable;

			// loop counter
			int count_loop = 0;
			
			// find one valid event programmed cycle
			do {
				// increment the cycle into the same event.
				optimized.cycles_i++;
				if(optimized.cycles_i < optimized.event0[optimized.event0_i].cycles_n_max) {
					// ESP_LOGI(TAG_ACIONNA, "optimized: IF event0_i:%d, cycles_i:%d", optimized.event0_i, optimized.cycles_i);
					// If motor state is idle, turn it on! (log will be make by pump class?);
					if(pump1_.state() == states_motor::off_idle) {
						pump1_.start(optimized.event0[optimized.event0_i].start_mode);
						// ESP_LOGI(TAG_ACIONNA, "optimized: start motor event:%d cycles_i:%d", optimized.event0_i, optimized.cycles_i);
					}
					// this is for algorithm to calculate the next turn on after turn off;
					optimized.flag_time_next_config = states_flag::enable;

					// if does exists programmed shutdown time, use it!
					if(optimized.event0[optimized.event0_i].time_to_shutdown) {
						pump1_.time_to_shutdown = optimized.event0[optimized.event0_i].time_to_shutdown;
					}
				} else {
					// go to next event
					optimized.event0_i++;
					optimized.cycles_i = -1;
					// ESP_LOGI(TAG_ACIONNA, "optimized: ELSE event0_i:%d, cycles_i:%d", optimized.event0_i, optimized.cycles_i);
				}

				// counter to avoid an infinity loop
				count_loop++;
				// ESP_LOGI(TAG_ACIONNA, "optimized: loop count_i++:%d", count_loop);
				if(count_loop > 4) {
					// ESP_LOGI(TAG_ACIONNA, "optimized: loop count max");
					break;
				}
			// if does not exists more events programmed, finish the process.	
			} while((optimized.event0_i < (optimized.event0_n_max)) && (optimized.flag_time_next_config == states_flag::disable));
		}

		// here, we suppose the pump is on after start by time match or time match next
		if(optimized.flag_time_next_config == states_flag::enable) {
			// if pump turn off, update those values;
			// ESP_LOGI(TAG_ACIONNA, "optimized: config next time match 0");
			if(pump1_.state() == states_motor::off_idle) {
				optimized.flag_time_next_config = states_flag::disable;
				// optimized.time_stop = time_day_sec_;
				optimized.time_match_next = time_day_sec_ + optimized.time_delay;
				// ESP_LOGI(TAG_ACIONNA, "optimized: config next time match t:%.2d:%.2d", timesec_to_hour(optimized.time_match_next), timesec_to_min(optimized.time_match_next));
			}
		}

		// turn system off when red time began. End of process.
		if(optimized.time_red == time_day_sec_) {
			if(pump1_.state() != states_motor::off_idle) {
				pump1_.stop(stop_types::red_time);
				// optimized.time_match_next = optimized.time_match_start;
				optimized.flag_time_next_config = states_flag::disable;
				// optimized.time_stop = time_day_sec_;
			} else {
				// these occurs when motor is idle on time delay period and next start time is programmed after time red.
				if(optimized.time_match_next >= optimized.time_red) {
					optimized.time_match_next = time_day_sec_ - 60;
				}
			}
			// ESP_LOGI(TAG_ACIONNA, "optimized: red time founded");
		}
	}
}
void Acionna::operation_pump_stop_check(void) {
	// check time to shutdown
	if(flag_check_timer_ == states_flag::enable) {
		if((pump1_.state() == states_motor::on_nominal_k1) || (pump1_.state() == states_motor::on_nominal_k2) || (pump1_.state() == states_motor::on_nominal_delta) || (pump1_.state() == states_motor::on_speeding_up)) {
			if(!pump1_.time_to_shutdown) {
				pump1_.stop(stop_types::timeout);		// stop motor by timeout;
			}
		}
	}

	// check high pressure (0x02)
	if(flag_check_pressure_high_ == states_flag::enable) {
		if(pipe1_.pressure_mca() > pipe1_.pressure_max) {
			if((pump1_.state() == states_motor::on_nominal_k1) || (pump1_.state() == states_motor::on_nominal_delta) || (pump1_.state() == states_motor::on_speeding_up)) {
				pump1_.stop(stop_types::pressure_high);
			}
		}
	}

	// check low pressure (0x03)
	if(flag_check_pressure_low_ == states_flag::enable) {
		if(flag_check_low_pressure_k1_ == states_flag::enable) {
			// To main waterpump with 3 stages;
			if(pipe1_.air_intake_detect(pump1_.state(), states_motor::on_nominal_k1, 60)) {
				pump1_.stop(stop_types::pressure_low);
			}
		}

		if(flag_check_low_pressure_delta_ == states_flag::enable) {
			// Trying for irrigation. It needs some tests.
			if(pipe1_.air_intake_detect(pump1_.state(), states_motor::on_nominal_delta, 60)) {
				pump1_.stop(stop_types::pressure_low);
			}
		}

		if(flag_check_low_pressure_k2_ == states_flag::enable) {
			// set pump state and expected pressure
			if(pipe2_.air_intake_detect(pump1_.state(), states_motor::on_nominal_k2, 60)) {
				pump1_.stop(stop_types::pressure_low);
			}
		}
	}

	// check thermal relay
	if(flag_check_thermal_relay_ == states_flag::enable) {
		if(pump1_.state_Rth() == states_switch::on) {
			if(pump1_.state() != states_motor::off_thermal_activated) {
				pump1_.stop(stop_types::thermal_relay);
			}
		}
	}

	// check low level
	#ifdef CONFIG_WELL_SUPPORT
	if(flag_check_low_level_ == states_flag::enable)
	{
		if(well1_.state_L1() == states_level::low)
			if((pump1_.state() == states_motor::on_nominal_k1) || (pump1_.state() == states_motor::on_nominal_delta) || (pump1_.state() == states_motor::on_speeding_up))
				pump1_.stop(stop_types::level_low);;
	}
	#endif
}
void Acionna::operation_pump_lock_down(void) {
	if((pump1_.state() == states_motor::on_nominal_k1) || (pump1_.state() == states_motor::on_nominal_delta) || (pump1_.state() == states_motor::on_speeding_up)) {
		pump1_.stop(stop_types::system_lock);
	}
}
void Acionna::operation_pump_optimized(void) {

}
void Acionna::operation_pump_valves(void) {
	// If PCY8575 module resets, all ports go down and valve stop. To prevent it, keep setting on the current valve every second.
	// P.S.: the module has implemented backup registers to keep output buffer even if PCY8575 reset occurs.
	/*
		A better function suppose to be implemented checking the current [mA] and valve current state asking PCY8575 module;
	*/
	if(flag_check_valves_ == states_flag::enable) {
		if((pump1_.state() == states_motor::on_nominal_delta) && (valves1_.state_valves == states_valves::automatic_switch)) {
			valves1_.set_valve_state(valves1_.valve_current(), 1);
		}
	}

	// If valves still not working but motor is turned on, start valves working cycle.
	if(valves1_.state_valves == states_valves::system_off) {
		if((pump1_.state() == states_motor::on_nominal_delta) || pump1_.state() == states_motor::on_speeding_up) {
			valves1_.start();
			pump1_.time_to_shutdown = valves1_.get_total_time_programmed()-2;	// to last valve keep on a little time to aliviate the pipe line pressure;
			pump1_.time_to_shutdown_config = valves1_.get_total_time_programmed();
		}
	}

	// If valves cycle are working but motor is turned off, stop valves working cycle.
	if(valves1_.state_valves == states_valves::automatic_switch) {
		if(pump1_.state() != states_motor::on_nominal_delta)
			if(pump1_.state() != states_motor::on_speeding_up)
				valves1_.stop();

		// This variable makes the interconnection between pipe pressure and valve class to make average sector pressure.
		pressure_ = pipe1_.pressure_mca();
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
void Acionna::run(void) {

	msg_fetch_();					// fetch for a new command;

	msg_exec_();					// parse and execute the commmand;

	msg_back_();					// send answer back to origin;
}
void Acionna::run_every_second(void) {

	sys_fw_update_ans_async_();		// return msg when firmware is in upgrade process;

	msg_json_back_();				// return json msg when request auto refresh mode;

	update_all();					// update variables and rtc in a 1 second period time;

	operation_mode();				// execution process
}
void Acionna::sys_chip_info(char* buffer_str) {
	/* Print chip information */

	memset(buffer_str, 0, sizeof(*buffer_str));
	
	uint32_t flash_size;
	esp_chip_info_t chip_info;
	
	if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
		flash_size = -1;
	}
	esp_chip_info(&chip_info);

	unsigned major_rev = chip_info.revision / 100;
	unsigned minor_rev = chip_info.revision % 100;

	sprintf(buffer_str, "Mod:%d cores:%d rev%d.%d flash:%lu MB type:%s WiFi:%s%s%s\n", static_cast<int>(chip_info.model),
														chip_info.cores,
														major_rev, minor_rev, // silicon revision
														flash_size/ (uint32_t)(1024*1024),
														(chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "emb" : "ext",
														(chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
														(chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
														(chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");
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
	sprintf(buffer_temp, "Sec Ver:%lu\n", OTA_update.running_app_info.secure_version);
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
		sprintf(buffer_temp, "Sec Ver: %lu\n", OTA_update.update_app_info.secure_version);
		strcat(buffer_str, buffer_temp);
	}
}
void Acionna::sys_fw_info_partitions_(char* buffer_str) {
	memset(buffer_str, 0, sizeof(*buffer_str));
	char buffer_temp[70];
	ota_info();

	sprintf(buffer_str, "Bt-> L:%s, o:0x%08lx, t:%d, s:%d\n",
												OTA_update.configured_partition->label,
												OTA_update.configured_partition->address,
												static_cast<int>(OTA_update.configured_partition->type),
												static_cast<int>(OTA_update.configured_partition->subtype));
	
	sprintf(buffer_temp, "Ru-> L:%s, o:0x%08lx, t:%d, s:%d\n",
													OTA_update.running_partition->label,
													OTA_update.running_partition->address,
													static_cast<int>(OTA_update.running_partition->type),
													static_cast<int>(OTA_update.running_partition->subtype));
	strcat(buffer_str, buffer_temp);

	if(OTA_update.update_partition != NULL)
	{
		sprintf(buffer_temp, "Up-> L:%s, o:0x%08lx, t:%d, s:%d\n",
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
			sprintf(buffer_str, "%d %%", static_cast<int>(OTA_update.binary_file_length_write/(float)OTA_update.image_size*100.0));
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
			sprintf(buffer_temp, "Sec. Ver.: %lu\n", OTA_update.update_app_info.secure_version);
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
void Acionna::sys_ram_free_(char* buffer_str) {

	memset(buffer_str, 0, sizeof(*buffer_str));
	sprintf(buffer_str, "RAM free:%lu, min:%lu\n", esp_get_free_internal_heap_size(), esp_get_minimum_free_heap_size());
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
void Acionna::sys_ticks_per_us(char *buffer_str) {
	memset(buffer_str, 0, sizeof(*buffer_str));
	sprintf(buffer_str, "num ticks:%lu\n", esp_rom_get_cpu_ticks_per_us());
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
	char buffer_temp[62];

	memset(ap_info, 0, sizeof(ap_info));
	memset(buffer_str, 0, sizeof(*buffer_str));
	memset(buffer_temp, 0, sizeof(buffer_temp));

	wifi_scan2(number, ap_info_ptr, ap_count);
	for(int i=0; (i<DEFAULT_SCAN_LIST_SIZE) && (i<ap_count); i++)
	{
		sprintf(buffer_temp, "Ch: %d, RSSI: %d, SSID: %s\n", ap_info[i].primary, ap_info[i].rssi, ap_info[i].ssid);
		strcat(buffer_str, buffer_temp);
	}
	// std::string str(buffer_str);
	// ws_server_send(str);
}
void Acionna::sys_wifi_mac_(char* buffer_str) {
	
	uint8_t wifi_mac[6];
	wifi_get_mac(&wifi_mac[0]);

	memset(buffer_str, 0, sizeof(*buffer_str));
	char buffer_temp[10];
	
	sprintf(buffer_str, "MAC");
	for(int i=0; i<6; i++) {
		sprintf(buffer_temp, ":%02x", wifi_mac[i]);
		strcat(buffer_str, buffer_temp);
	}
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
void Acionna::update_all() {

	update_uptime();
	update_RTC();
	update_objects();
	update_stored_data();

	// update_sensors();		// test sensors
}
void Acionna::update_RTC() {
	epoch_time_ = device_clock.get_time();
	dt.setUnixTime(epoch_time_);

	time_day_sec_++;
	// compare time_day_sec with whole day seconds 24*60*60 = 86400
	if(time_day_sec_ == 86400) {
		// time_day_sec_ ^= time_day_sec_;
		time_day_sec_ = 0;
	}
}
void Acionna::update_objects() {

	pump1_.update();

	pipe1_.update();

	pipe2_.update();

	// #ifdef CONFIG_WELL_SUPPORT
	// well1_.update();
	// #endif

	// #ifdef CONFIG_VALVES_SUPPORT
	valves1_.update();
	// #endif
}
void Acionna::update_sensors()
{
	// if(!timeout_sensors)
	// {
	// 	timeout_sensors = timeout_sensors_cfg;

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
	// }
	// else
	// 	timeout_sensors--;
}
void Acionna::update_stored_data() {
}
void Acionna::update_uptime()
{
	uptime_++;
	// ESP_LOGI(TAG_ACIONNA, "uptime: %lu, esp_uptime: %ld", uptime_, static_cast<long int>(esp_timer_get_time() / 1000000);
	// or
	// uptime = esp_timer_get_time();

	// ESP_LOGI(TAG_ACIONNA, "uptime: %d", device_clock.internal_time());
	// ESP_LOGI(TAG_ACIONNA, "uptime: %ld", static_cast<long int>((esp_timer_get_time() / 1000000)));
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