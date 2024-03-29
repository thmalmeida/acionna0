//#include "motor_ac.hpp"
#include "pump.hpp"

//void pump::init()
//{
	// 3 TRIAC drive output setup
//	for(std::size_t i = 0; i < ac_load_count; i++)
//	{
//		ac_load[i].mode(GPIO_MODE_INPUT_OUTPUT);
//		ac_load[i].write(0);
//	}
//
//	// 4 digital general inputs. (e.g. k1 in, k2 in, k3 in and T1 in).
//	for(std::size_t i = 0; i < gpio_generic_count; i++)
//	{
//		gpio_generic[i].pull(GPIO_PULLUP_ONLY);
//		gpio_generic[i].mode(GPIO_MODE_INPUT);
////		gpio_generic[i].enable_interrupt(GPIO_INTR_ANYEDGE);
////		gpio_generic[i].register_interrupt(gpio_input_interuput, nullptr);
//	}

	// 2 pressure sensor ADC input
//	adc_press_init();

	// 2 current sensor ADC input

	// 1 voltage sensor ADC input

	// restore working variables from flash/eeprom memory

	// DS3231 RTC timer init

	// Temperature sensor DS18B20 init

	// Air pressure sensor init

	// Humidity sensor init

//	gateConfig(pin_out_led, 1);		// Configure pin led as output;
////	drive_led_on();
//	gateConfig(pin_out_k1, 1);
//	gateConfig(pin_out_k2, 1);
//	gateConfig(pin_out_k3, 1);
////
//	gateConfig(pin_in_Rth, 0);
//	gateConfig(pin_in_k1, 0);
//	gateConfig(pin_in_k2, 0);
//	gateConfig(pin_in_k3, 0);
//
//	gateConfig(pin_analog_PRess_Dig, 0);
//
//	adc_begin();
//	eeprom.begin_eeprom();
//	refreshStoredData();
//}
//	 3 TRIAC drive output setup
//	for(std::size_t i = 0; i < ac_load_count; i++)
//	ac_load[i].mode(GPIO_MODE_INPUT_OUTPUT);
/*



 * motor_ac.hpp
 *
 *  Created on: 08 de fev de 2022
 *      Author: titi
 */

//#ifndef MOTOR_AC_HPP_
//#define MOTOR_AC_HPP_
//
////GPIO_Basic gpio_sensor(DS18B20_DATA);
//
//GPIO_Basic led0(LED_0);
//
//GPIO_Basic ac_load[]={
//		GPIO_Basic{AC_LOAD1},
//		GPIO_Basic{AC_LOAD2},
//		GPIO_Basic{AC_LOAD3}
//};
//
//extern const std::size_t ac_load_count = sizeof(ac_load) / sizeof(ac_load[0]);
//
//GPIO_Basic water_level[]{
//		GPIO_Basic{WATER_LEVEL1},
//		GPIO_Basic{WATER_LEVEL2},
//		GPIO_Basic{WATER_LEVEL3},
//		GPIO_Basic{WATER_LEVEL4}
//};
//
//const std::size_t water_level_count = sizeof(water_level) / sizeof(water_level[0]);
//
//
//GPIO_Basic gpio_generic[]{
//		GPIO_Basic{GPIO_GENERIC1},
//		GPIO_Basic{GPIO_GENERIC2},
//		GPIO_Basic{GPIO_GENERIC3},
//		GPIO_Basic{GPIO_GENERIC4}
//};
//
//const std::size_t gpio_generic_count = sizeof(gpio_generic) / sizeof(gpio_generic[0]);
//
//ADC_Basic press1(PRESS_SENSOR1_ADC);
//
//void acn_init()
//{
//	// 3 TRIAC drive output setup
//	for(std::size_t i = 0; i < ac_load_count; i++)
//	{
//		ac_load[i].mode(GPIO_MODE_INPUT_OUTPUT);
//		ac_load[i].write(0);
//	}
//
//	// 4 water level sensor digital input setup
//	for(std::size_t i = 0; i < water_level_count; i++)
//	{
//		water_level[i].pull(GPIO_PULLUP_ONLY);
//		water_level[i].mode(GPIO_MODE_INPUT);
////		water_level[i].enable_interrupt(GPIO_INTR_ANYEDGE);
////		water_level[i].register_interrupt(gpio_input_interrput, nullptr);
//	}
//
//	// 4 digital general inputs. (e.g. k1 in, k2 in, k3 in and T1 in).
//	for(std::size_t i = 0; i < gpio_generic_count; i++)
//	{
//		gpio_generic[i].pull(GPIO_PULLUP_ONLY);
//		gpio_generic[i].mode(GPIO_MODE_INPUT);
////		gpio_generic[i].enable_interrupt(GPIO_INTR_ANYEDGE);
////		gpio_generic[i].register_interrupt(gpio_input_interuput, nullptr);
//	}
//
//	// 1 signal onboard led output setup
//	led0.mode(GPIO_MODE_OUTPUT);
//
//
//	// 2 pressure sensor ADC input
////	adc_press_init();
//
//	// 2 current sensor ADC input
//
//	// 1 voltage sensor ADC input
//
//	// restore working variables from flash/eeprom memory
//
//	// DS3231 RTC timer init
//
//	// Temperature sensor DS18B20 init
//
//	// Air pressure sensor init
//
//	// Humidity sensor init
//
//
//	//	while(1)
////	{
////		led0.toggle();
////        vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
////	}
////	gateConfig(pin_out_led, 1);		// Configure pin led as output;
//////	drive_led_on();
////	gateConfig(pin_out_k1, 1);
////	gateConfig(pin_out_k2, 1);
////	gateConfig(pin_out_k3, 1);
//////
////	gateConfig(pin_in_Rth, 0);
////	gateConfig(pin_in_k1, 0);
////	gateConfig(pin_in_k2, 0);
////	gateConfig(pin_in_k3, 0);
////
////	gateConfig(pin_analog_PRess_Dig, 0);
////
////	adc_begin();
////	eeprom.begin_eeprom();
////	refreshStoredData();
//}
//void acn_test_1()
//{
//	uint32_t a;
//	while (1)
//	{
////		adc_run_example();
//		led0.toggle();
//		a = press1.read();
//		printf("a: %d\n", a);
//		vTaskDelay(pdMS_TO_TICKS(1000));
//	}
//}
//void acn_check_levelSensors()
//{
//	get_levelSensors();
//}
//void acn_check_period()
//{
//	// Season time verify
//	if(((tm.Hour == HourOn) && (tm.Minute == MinOn)) || (tm.Hour > HourOn) || (tm.Hour < HourOff) || ((tm.Hour == HourOff) && (tm.Minute < MinOff)))
//	{
//		periodo = greenTime;
//
//		if(flag_01)
//		{
//			flag_01 = 0;
////			flag_timeMatch = 1;
//		}
//	}
//
//	if (((tm.Hour == HourOff) && (tm.Minute >= MinOff))	|| ((tm.Hour > HourOff) && (tm.Hour < HourOn))	|| ((tm.Hour == HourOn) && (tm.Minute < MinOn)))
//	{
//		periodo = redTime;
//
////		flag_timeMatch = 0;
//		flag_01 = 1;
//	}
//}
//void acn_check_pressure()
//{
//	PRess = get_Pressure();	// Get current pressure
//
////	switch (stateMode)
////	{
////		case 1:
////			break;
////
////		case 2:
////			break;
////
////		case 3:	// Valve mode case. Do not turn off.
////			break;
////
////		default:
////			break;
////	}
//}
//void acn_check_pressureUnstable()	// this starts to check quick variation of pressure from high to low after 2 minutes on
//{
//	if(stateMode == 5 && timeOn_min > 2)
//	{
//		if(motorStatus)
//		{
//			if(PRess > PRessHold)
//			{
//				PRessHold = PRess;
//			}
//			else if(PRess < ((PRessurePer/100.0)*PRessHold))
//			{
//				flag_PressureUnstable = 1;
//			}
//		}
//	}
//}
//void acn_check_pressureDown()	// this check if pressure is still low after 3 minutes on;
//{
////	if(flag_conf_pressureUnstable && (timeOn_min > 3)
//	if(stateMode == 5 && (timeOn_min > 3))
//	{
//		if(motorStatus)
//		{
//			if(PRess < PRessureRef_Low)
//			{
//				// turn load off;
//			}
//		}
//	}
//}
//void acn_check_timeMatch()
//{
//	uint8_t i, nTM_var=1;
//
//	// matching time verify
//	if(!motorStatus)
//	{
//		if(stateMode)
//		{
//			switch (stateMode)
//			{
//			case 1:
//				nTM_var = 1;
//				break;
//
//			case 2:
//				nTM_var = nTM;
//				break;
//
//			case 3:
//				nTM_var = nTM;
//				break;
//			}
//
//			for(i=0;i<nTM_var;i++)
//			{
//				if((tm.Hour == HourOnTM[i]) && (tm.Minute == MinOnTM[i]))
//				{
//					flag_timeMatch = 1;	// IF conditions are OK, SET (flag_timeMatch) variable;
//				}
//			}
//		}
//	}
//}
//void acn_check_thermalSafe()
//{
//	if(motorStatus)
//	{
//		if(read_Rth())
//		{
//			flag_Th = 1;
//		}
//		else
//		{
//			flag_Th = 0;
//		}
//	}
//}
//void acn_check_TimerVar()
//{
//	if(motorStatus)	// Load is ON;
//	{
//		if(timeOn_sec > 59)
//		{
//			timeOn_sec = 0;
//			timeOn_min++;
//		}
//		else
//		{
//			timeOn_sec++;
//		}
//	}
//	else			// Load is OFF;
//	{
//		if(timeOff_sec > 59)
//		{
//			timeOff_sec = 0;
//			timeOff_min++;
//		}
//		else
//		{
//			timeOff_sec++;
//		}
//	}
//
//	if(flag_waitPowerOn)
//	{
//		if(waitPowerOn_sec == 0)
//		{
//			if(waitPowerOn_min == 0)
//			{
//				flag_waitPowerOn = 0;
//			}
//			else
//			{
//				waitPowerOn_sec = 59;
//				waitPowerOn_min--;
//			}
//		}
//		else
//		{
//			waitPowerOn_sec--;
//		}
//	}
//}
//void acn_driveMotor_OFF()
//{
//	k1_off();
//	k2_off();
//	k3_off();
//}
//uint8_t acn_read_Rth()
//{
//	return !gateRead(pin_in_Rth, 0);
//}
//uint8_t acn_read_k1()
//{
//	return 	!gateRead(pin_in_k1, 0);
//}
//uint8_t acn_read_k2()
//{
//	return !gateRead(pin_in_k2, 0);
//}
//uint8_t acn_read_k3()
//{
//	return !gateRead(pin_in_k3, 0);
//}
//uint8_t acn_readPin_k1()
//{
//	return gateRead(pin_out_k1, 1);
//}
//void acn_k1_on()
//{
//	gateSet(pin_out_k1, 1);
//}
//void acn_k1_off()
//{
//	gateSet(pin_out_k1, 0);
//}
//void acn_k2_on()
//{
//	gateSet(pin_out_k2, 1);
//}
//void acn_k2_off()
//{
//	gateSet(pin_out_k2, 0);
//}
//void acn_k3_on()
//{
//	gateSet(pin_out_k3, 1);
//}
//void acn_k3_off()
//{
//	gateSet(pin_out_k3, 0);
//}
//void acn_blink_led(uint8_t qnt, uint8_t time)
//{
//	int i=0;
//	for(i=1;i<2*qnt+1;i++)
//	{
//		drive_led_toggle();
//		_delay_ms(time);
//	}
//}
//void acn_motor_stop(uint8_t reason)
//{
//	int i;
//	for(i=(nLog-1);i>0;i--)
//	{
//		hourLog_OFF[i] = hourLog_OFF[i-1];
//		minuteLog_OFF[i] = minuteLog_OFF[i-1];
//
//		dayLog_OFF[i] = dayLog_OFF[i-1];
//		monthLog_OFF[i] = monthLog_OFF[i-1];
//	}
//
//	reasonV[1] = reasonV[0];
//	reasonV[0] = reason;
//
//	hourLog_OFF[0] = tm.Hour;
//	minuteLog_OFF[0] = tm.Minute;
//
//	dayLog_OFF[0] = tm.Day;
//	monthLog_OFF[0] = tm.Month;
//
//	timeOff_min = 0;
//	timeOff_sec = 0;
//
//	flag_waitPowerOn = 1;
//	waitPowerOn_min = waitPowerOn_min_standBy;
//
//	driveMotor_OFF();
//	drive_led_off();
////	_delay_ms(1);
//
//	motorStatus = readPin_k1();
////	sprintf(Serial.buffer,"offR:%d", reason);
////	Serial.println(Serial.buffer);
//}
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
//void acn_driveMotor_ON(uint8_t startType)
//{
//	switch (startType)
//	{
//		case 1:	// Partida direta: monofásico
//			k1_on();
//			break;
//
//		case 2: // Partida direta: trifásico
//			k1_on();
//			k2_on();
//			break;
//
//		case 3:	// Partida estrela/triangulo
//			k1_on();
//			k3_on();
////			_delay_ms(2000);
//			//wdt_reset();
//			_delay_ms(((double) 100.0*motorTimerStart1));
//			//wdt_reset();
//
//			k3_off();
////			_delay_ms(500);
//			uint32_t countK = 0;
//			while(read_k3())
//			{
//				countK++;
//				if(countK>=250000)
//				{
//					k1_off();
//					k2_off();
//					k3_off();
//					return;
//				}
//			}
////			Serial.println(count);
//			_delay_ms(motorTimerStart2);
//			k2_on();
//			break;
//	}
//}
//void acn_process_valveControl()
//{
//	if(flag_timeMatch && (PRess >= PRessureRef_Valve))
//	{
//		flag_timeMatch &= 0;
//
//		if(!motorStatus)
//		{
//			motor_start();
//		}
//	}
//}
//void acn_process_waterPumpControl()
//{
//	/*
//	 * 0x01 - pressure
//	 * 0x02 - level
//	 * 0x03 - thermal
//	 * 0x04 - time out
//	 * 0x05 - red time
//	 * 0x06 - command line
//	 * 0x07 - broke pressure
//	 * */
//
////	if(!levelSensorLL)
////	{
////		if(motorStatus)
////		{
////			motor_stop(0x02);
////		}
////	}
////
////	if(levelSensorHL && (stateMode == 4) && levelSensorLL)
////	{
////		if(!motorStatus)
////		{
////			motor_start();
////		}
////	}
//
//	if(flag_timeMatch && (stateMode != 0) && (stateMode != 4))
//	{
//		flag_timeMatch = 0;
//
//		if(!motorStatus)
//		{
//			motor_start();
//		}
//	}
//
////	if(PRessureRef)					// Has a valid number mean this function is activated
////	{
////		if(PRess >= PRessureRef)
////		{
////			if(motorStatus)
////			{
////				motor_stop(0x01);
////			}
////		}
////	}
////
////	if(stateMode == 5)
////	{
////		if(motorStatus && flag_PressureUnstable)
////		{
////			flag_PressureUnstable = 0;
////			motor_stop(0x07);
////		}
////	}
//
//	if(flag_Th)
//	{
//		flag_Th = 0;
//
//		motor_stop(0x03);
//		stateMode = 0;
//	}
//}
//void acn_process_motorPeriodDecision()
//{
//	switch (periodo)
//	{
//	case redTime:
//		if(motorStatus)
//		{
//			motor_stop(0x05);
//		}
//		break;
//
//	case greenTime:
//		process_waterPumpControl();
//		break;
//	}
//}
//void acn_process_Mode()
//{
//	switch (stateMode)
//	{
//		case 0:	// System Down!
//			break;
//
//		case 1:	// Night Working;
//			process_motorPeriodDecision();
//			break;
//
//		case 2:	// For irrigation mode. Start in a programmed time.
//			process_waterPumpControl();
//			break;
//
//		case 3:	// For reservoir only. Works in a inverted pressured! Caution!
//			process_valveControl();
//			break;
//
//		case 4:	// Is that for a water pump controlled by water sensors. Do not use programmed time.
//			process_waterPumpControl();
//			break;
//
//		case 5:	// For irrigation mode and instantly low pressure turn motor off.
//			process_waterPumpControl();
//			break;
//
//		default:
//			stateMode = 0;
//			Serial.println("Standby");
//			break;
//	}
//
//	// maximum time drive keeps turned ON
//	if(motorTimerE)
//	{
//		if(motorStatus)
//		{
//			if(timeOn_min >= motorTimerE)
//			{
//				motor_stop(0x04);
//			}
//		}
//	}
//}
//void acn_summary_Print(uint8_t opt)
//{
//	switch (opt)
//	{
//		case 0:
//			sprintf(Serial.buffer,"Time:%.2d:%.2d:%.2d %.2d/%.2d/%.4d",tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year+1970);
//			Serial.print(Serial.buffer);
//
////			sprintf(Serial.buffer," UP:%.2d:%.2d:%.2d, d:%d m:%d", hour(), minute(), second(), day()-1, month()-1);
//			sprintf(Serial.buffer," UP:%.2d:%.2d:%.2d, d:%d m:%d", rtc.rtc0->tm_hour, rtc.rtc0->tm_min, rtc.rtc0->tm_sec, rtc.rtc0->tm_mday-1, rtc.rtc0->tm_mon);
//			Serial.println(Serial.buffer);
//
//			sprintf(Serial.buffer,"P:%d k1Pin:%d Rth:%d k1:%d k2:%d k3:%d",periodo, motorStatus, Rth_status, k1_status, k2_status, k3_status);
//			Serial.println(Serial.buffer);
//
//			switch (stateMode)
//			{
//				case 0:
//					strcpy(Serial.buffer," Modo:Desligado");
//					break;
//
//				case 1:
//					sprintf(Serial.buffer," Modo:Liga Noite");
//					break;
//
//				case 2:
//					if(nTM == 1)
//					{
//						sprintf(Serial.buffer," Irrig: Liga %dx as %2.d:%.2d", nTM, HourOnTM[0], MinOnTM[0]);
//					}
//					else
//					{
//						sprintf(Serial.buffer," Irrig: Liga %dx/dia",nTM);
//					}
//					break;
//
//				case 3:
//					if(nTM == 1)
//					{
//						sprintf(Serial.buffer," Valve: Liga %dx as %2.d:%.2d", nTM, HourOnTM[0], MinOnTM[0]);
//					}
//					else
//					{
//						sprintf(Serial.buffer," Valve: Liga %dx/dia",nTM);
//					}
//					break;
//
//				case 4:
//					sprintf(Serial.buffer," Modo: Auto HL");
//					break;
//
//				case 5:
//					if(nTM == 1)
//					{
//						sprintf(Serial.buffer," IrrigLow: Liga %dx as %2.d:%.2d", nTM, HourOnTM[0], MinOnTM[0]);
//					}
//					else
//					{
//						sprintf(Serial.buffer," IrrigLow: Liga %dx/dia",nTM);
//					}
//					break;
//					break;
//
//
//				default:
//					strcpy(Serial.buffer,"sMode Err");
//					break;
//			}
//			Serial.println(Serial.buffer);
//			break;
//
//		case 1:
//			int i;
//			if(motorStatus)
//			{
//				for(i=(nLog-1);i>=0;i--)
//				{
//					memset(Serial.buffer,0,sizeof(Serial.buffer));
//					sprintf(Serial.buffer,"OFF_%.2d: %.2d:%.2d, %.2d/%.2d ",(i+1),hourLog_OFF[i], minuteLog_OFF[i], dayLog_OFF[i], monthLog_OFF[i]);
//					Serial.println(Serial.buffer);
//					_delay_ms(20);
//
//					memset(Serial.buffer,0,sizeof(Serial.buffer));
//					sprintf(Serial.buffer,"ON__%.2d: %.2d:%.2d, %.2d/%.2d ",(i+1),hourLog_ON[i], minuteLog_ON[i], dayLog_ON[i], monthLog_ON[i]);
//					Serial.println(Serial.buffer);
//					_delay_ms(20);
//				}
//			}
//			else
//			{
//				for(i=(nLog-1);i>=0;i--) //	for(i=0;i<nLog;i++)
//				{
//					memset(Serial.buffer,0,sizeof(Serial.buffer));
//					sprintf(Serial.buffer,"ON__%.2d: %.2d:%.2d, %.2d/%.2d " ,(i+1),hourLog_ON[i], minuteLog_ON[i], dayLog_ON[i], monthLog_ON[i]);
//					Serial.println(Serial.buffer);
//					_delay_ms(20);
//
//					memset(Serial.buffer,0,sizeof(Serial.buffer));
//					sprintf(Serial.buffer,"OFF_%.2d: %.2d:%.2d, %.2d/%.2d ",(i+1),hourLog_OFF[i], minuteLog_OFF[i], dayLog_OFF[i], monthLog_OFF[i]);
//					Serial.println(Serial.buffer);
//					_delay_ms(20);
//				}
//			}
//			sprintf(Serial.buffer,"r0:%d, r1:%d ",reasonV[0], reasonV[1]);
//			Serial.println(Serial.buffer);
//			break;
//
//		case 2:
//			sprintf(Serial.buffer,"f:%d t1:%.2d:%.2d c%dmin t2:%.2d:%.2d s:%dmin ", flag_waitPowerOn, waitPowerOn_min, waitPowerOn_sec, waitPowerOn_min_standBy, timeOn_min, timeOn_sec, motorTimerE);
//			Serial.println(Serial.buffer);
//			break;
//
//		case 3:
//			sprintf(Serial.buffer,"Motor:%d Fth:%d Rth:%d Pr:%d ", motorStatus, flag_Th, read_Rth(), PRess);
//			Serial.print(Serial.buffer);
//			switch (stateMode)
//			{
//				case 3:
//					sprintf(Serial.buffer,"Pref:%d Ptec: %d ", PRessureRef_Valve, PRessureMax_Sensor);
//					Serial.println(Serial.buffer);
//					break;
//
//				case 5:
//					sprintf(Serial.buffer,"Pref:%d, Pper:%d ", PRessureRef, PRessurePer);
//					Serial.println(Serial.buffer);
//					break;
//
//				default:
//					sprintf(Serial.buffer,"Pref:%d, Ptec: %d, Pper:%d ", PRessureRef, PRessureMax_Sensor, PRessurePer);
//					Serial.println(Serial.buffer);
//					break;
//
//			}
//			break;
//
//		case 4:
//			sprintf(Serial.buffer,"LL:%d ML:%d HL:%d ",levelSensorLL, levelSensorML, levelSensorHL);
//			Serial.println(Serial.buffer);
//
//			sprintf(Serial.buffer,"LL:%d ML:%d HL:%d",levelSensorLL_d, levelSensorML_d, levelSensorHL_d);
//			Serial.println(Serial.buffer);
//
//			sprintf(Serial.buffer,"Rth:%d k1:%d k2:%d k3:%d", Rth_status, k1_status, k2_status, k3_status);
//			Serial.println(Serial.buffer);
//			break;
//
//		case 5:
//			for(i=0;i<nTM;i++)
//			{
//				sprintf(Serial.buffer,"h%d: %.2d:%.2d",i+1, HourOnTM[i], MinOnTM[i]);
//				Serial.println(Serial.buffer);
//			}
//			break;
//
//		case 6:
//			sprintf(Serial.buffer,"tON:%.2d:%.2d ",timeOn_min, timeOn_sec);
//			Serial.println(Serial.buffer);
//			sprintf(Serial.buffer,"tOFF:%.2d:%.2d",timeOff_min, timeOff_sec);
//			Serial.println(Serial.buffer);
//			break;
//
//		case 7:
//			sprintf(Serial.buffer,"P:%d Fth:%d Rth:%d Ftm:%d k1:%d k2:%d k3:%d", PRess, flag_Th, read_Rth(), flag_timeMatch, motorStatus, read_k2(), read_k3());
//			Serial.println(Serial.buffer);
//
//			sprintf(Serial.buffer,"LL:%d ML:%d HL:%d  ",levelSensorLL, levelSensorML, levelSensorHL);
//			Serial.println(Serial.buffer);
//
//			sprintf(Serial.buffer,"LL:%d ML:%d HL:%d  ",levelSensorLL_d, levelSensorML_d, levelSensorHL_d);
//			Serial.println(Serial.buffer);
//			break;
//
//		case 8:
//			sprintf(Serial.buffer,"%.2d:%.2d:%.2d %.2d/%.2d/%.4d",tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year+1970);
//			Serial.println(Serial.buffer);
////			sprintf(Serial.buffer,"WDRF:%d BORF:%d EXTRF:%d PORF:%d", flag_WDRF, flag_BORF, flag_EXTRF, flag_PORF);
////			Serial.println(Serial.buffer);
//			break;
//
//		case 9:
//			sprintf(Serial.buffer,"Err");
//			Serial.println(Serial.buffer);
//			break;
//
//		default:
//			sprintf(Serial.buffer,"not implemented");
//			Serial.println(Serial.buffer);
//			break;
//	}
//}
//void acn_update_inputs()
//{
//	motorStatus = readPin_k1();
//	k1_status = read_k1();
//	k2_status = read_k2();
//	k3_status = read_k3();
//	Rth_status = read_Rth();
//}
//void acn_update_RTC()
//{
//	rtc.getUptime();
//	rtc.getTime();
//	tm.Year 	= rtc.timeinfo->tm_year;
//	tm.Month 	= rtc.timeinfo->tm_mon+1;
//	tm.Day 		= rtc.timeinfo->tm_mday;
//	tm.Hour 	= rtc.timeinfo->tm_hour;
//	tm.Minute 	= rtc.timeinfo->tm_min;
//	tm.Second 	= rtc.timeinfo->tm_sec;
//}
//void acn_RTC_update()
//{
//	if(tm.Second == 59)
//	{
//		tm.Second = 0;
//		if(tm.Minute == 59)
//		{
//			tm.Minute = 0;
//			if(tm.Hour == 23)
//			{
//				tm.Hour = 0;
//				uint8_t month31, month30;
//				if((tm.Month == 1) || (tm.Month == 3) || (tm.Month == 5) || (tm.Month == 7) || (tm.Month == 8) || (tm.Month == 10) || (tm.Month == 12))
//				{
//					month31 = 1;
//				}
//				else
//				{
//					month30 = 1;
//				}
//
//				if((tm.Day == 30 && month30) || (tm.Day == 31 && month31))
//				{
//					tm.Day = 1;
//					if(tm.Month == 12)
//					{
//						tm.Month = 1;
//						tm.Year++;
//					}
//					else
//					{
//						tm.Month++;
//					}
//				}
//				else
//				{
//					tm.Day++;
//				}
//			}
//			else
//			{
//				tm.Hour++;
//			}
//		}
//		else
//		{
//			tm.Minute++;
//		}
//	}
//	else
//	{
//		tm.Second++;
//	}
//}
//void acn_refreshVariables()
//{
//	if (flag_1s)
//	{
//		flag_1s = 0;
////		gateToggle(1);
//
//		update_RTC();
//		update_inputs();
////		RTC.read(tm);
//
//		check_thermalSafe();	// thermal relay check;
//		check_period();			// Period verify;
//		check_timeMatch();		// time matches flag;
//		check_TimerVar();		// drive timers
//
////		check_pressure();		// get and check pressure system;
//
////		check_levelSensors();	// level sensors;
//
////		check_pressureDown();
//
//		if(flag_debug)
//		{
//			summary_Print(7);
//		}
//
//		if(flag_debug_time)
//		{
//			summary_Print(8);
//		}
//
//	}
//}
//void acn_refreshStoredData()
//{
//	stateMode = eeprom.read(eeprom.pageSet, eeprom.addr_stateMode);	//eeprom_read_byte((uint8_t *)(addr_stateMode));
//
//	if(stateMode == 0xFF)
//	{
//		stateMode = 0;
//		eeprom.write(eeprom.pageSet, eeprom.addr_stateMode, stateMode);
//		eeprom.write(eeprom.pageSet, eeprom.addr_LevelRef, 1023);
//		eeprom.write(eeprom.pageSet, eeprom.addr_standBy_min, 0);
//		eeprom.write(eeprom.pageSet, eeprom.addr_PRessureRef, 60);
//		eeprom.write(eeprom.pageSet, eeprom.addr_PRessureRef_Valve, 40);
//		eeprom.write(eeprom.pageSet, eeprom.addr_PRessureMax_Sensor, 100);
//		eeprom.write(eeprom.pageSet, eeprom.addr_motorTimerE, 60);
//		eeprom.write(eeprom.pageSet, eeprom.addr_HourOnTM, 21);
//		eeprom.write(eeprom.pageSet, eeprom.addr_MinOnTM, 40);
//		eeprom.write(eeprom.pageSet, eeprom.addr_nTM, 1);
//		eeprom.write(eeprom.pageSet, eeprom.addr_standBy_min, 10);
//
//
//		eeprom.write(eeprom.pageSet, eeprom.addr_PRessurePer, 85);
//		eeprom.write(eeprom.pageSet, eeprom.addr_rtc_PRL, 40000);
//		eeprom.write(eeprom.pageSet, eeprom.addr_rtc_clkSource, 0);
//
//		Serial.println("default values");
//	}
//
//	rtc.rtc_PRL = eeprom.read(eeprom.pageSet, eeprom.addr_rtc_PRL);
//	rtc.rtc_clkSource = eeprom.read(eeprom.pageSet, eeprom.addr_rtc_clkSource);
//
//	waitPowerOn_min_standBy = eeprom.read(eeprom.pageSet, eeprom.addr_standBy_min); //eeprom_read_byte((uint8_t *)(addr_standBy_min));
//	waitPowerOn_min = waitPowerOn_min_standBy;		// reset timer
//
////	uint8_t lbyte, hbyte;
////	hbyte = eeprom_read_byte((uint8_t *)(addr_LevelRef+1));
////	lbyte = eeprom_read_byte((uint8_t *)(addr_LevelRef));
////	levelRef_10bit = ((hbyte << 8) | lbyte);
//	levelRef_10bit = eeprom.read(eeprom.pageSet, eeprom.addr_LevelRef);
//
////	hbyte = eeprom_read_byte((uint8_t *)(addr_motorTimerE+1));
////	lbyte = eeprom_read_byte((uint8_t *)(addr_motorTimerE));
////	motorTimerE = ((hbyte << 8) | lbyte);
//	motorTimerE = eeprom.read(eeprom.pageSet, eeprom.addr_motorTimerE);
//
//	PRessureRef = eeprom.read(eeprom.pageSet, eeprom.addr_PRessureRef); //eeprom_read_byte((uint8_t *)(addr_PRessureRef));
//	PRessureRef_Valve = eeprom.read(eeprom.pageSet, eeprom.addr_PRessureRef_Valve); //eeprom_read_byte((uint8_t *)(addr_PRessureRef_Valve));
//	PRessurePer = eeprom.read(eeprom.pageSet, eeprom.addr_PRessurePer); //eeprom_read_byte((uint8_t *)(addr_PREssurePer));
//	PRessureMax_Sensor = eeprom.read(eeprom.pageSet, eeprom.addr_PRessureMax_Sensor); //eeprom_read_byte((uint8_t *)(addr_PRessureMax_Sensor));
//
//	nTM = eeprom.read(eeprom.pageSet, eeprom.addr_nTM); //eeprom_read_byte((uint8_t *)(addr_nTM));
//
//	int i;
//	for(i=0;i<9;i++)
//	{
//		HourOnTM[i] = eeprom.read(eeprom.pageSet, eeprom.addr_HourOnTM+i); //eeprom_read_byte((uint8_t *)(addr_HourOnTM+i));
//		MinOnTM[i] = eeprom.read(eeprom.pageSet, eeprom.addr_MinOnTM+i);//eeprom_read_byte((uint8_t *)(addr_MinOnTM+i));
//	}
//}
//void acn_handleMessage()
//{
///*
//$0X;				Verificar detalhes - Detalhes simples (tempo).
//	$00;			- Detalhes simples (tempo).
//	$01;			- Verifica histórico de quando ligou e desligou;
//	$02;			- Mostra tempo que falta para ligar;
//		$02:c;		- Zera o tempo;
//		$02:c:30;	- Ajusta novo tempo para 30 min;
//		$02:s:090;	- Tempo máximo ligado para 90 min. Para não utilizar, colocar zero;
//	$03;			- Verifica detalhes do motor, pressão e sensor termico;
//		$03:s:72;	- Set pressure ref [m.c.a.];
//		$03:v:32;	- Set pressure for valve load turn on and fill reservoir;
//		$03:p:150;	- Set sensor max pressure ref to change the scale [psi];
//		$03:b:85;	- Set to 85% the pressure min bellow the current pressure to avoid pipe broken;
//		$03:m:3;	- Set 3 seconds while K1 and K3 are ON into delta tri start;
//		$03:t:500;	- Set 500 milliseconds to wait K3 go off before start K2;
//	$04;			- Verifica detalhes do nível de �gua no po�o e referência 10 bits;
//		$04:0;		- Interrompe o envio continuo das vari�veis de press�o e n�vel;
//		$04:1;		- Envia continuamente valores de press�o e n�vel;
//		$04:s:0900;	- Adiciona nova referência para os sensores de nível. Valor de 0 a 1023;
//	$05;			- Mostra os hor�rios que liga no modo $62;
//	$06;			- Tempo ligado e tempo desligado;
//	$07:x;			- ADC reference change.
//		$07:0;		- AREF
//		$07:1;		- AVCC with external cap at AREF pin
//		$07:2;		- Internal 1.1 Voltage reference.
//	$08;			- Motivo do reboot.
//	$09;			- Reinicia o sistema.
//
//$1:h:HHMMSS;		- Ajustes do calend�rio;
//	$1:h:HHMMSS;	- Ajusta o hor�rio do sistema;
//	$1:h:123040;	- E.g. ajusta a hora para 12:30:40
//	$1:d:DDMMAAAA;	- Ajusta a data do sistema no formato dia/m�s/ano(4 d�gitos);
//	$1:d:04091986;	- E.g. Altera a data para 04/09/1986;
//	$1:c;			- Shows the LSI current prescaler value;
//	$1:c:40123;		- Set new prescaler value;
//
//$2:DevName;			- Change bluetooth name;
//	$2:Vassalo;		- Altera o nome do bluetooth para "Vassalo";
//
//$4:x:				- Is this applied fo stm32f10xxx series only;
//	$4:r:07;		- Read address 0x07 * 2 of currently page;
//	$4:w:07:03;		- Write variable 3 on address 7 of currently page;
//	$4:f:64:03;		- fill page 64 with 3 value;
//	$4:e:64;		- erase page 64;
//
//$3X;				- Acionamento do motor;
//	$31;			- liga o motor;
//	$30;			- desliga o motor;
//
//$5:n:X; ou $5:hX:HHMM;
//	$5:n:9;			- Configura para acionar 9 vezes. Necess�rio configurar 9 hor�rios;
//	$5:n:9;			- Configura o sistema para acionar uma �nica vez �s 21:30 horas;
//	$5:h1:2130;		- Configura o primeiro hor�rio como 21:30 horas;
//	$5:h8:0437;		- Configura o oitavo hor�rio como 04:37 horas;
//
//$6X;				- Modos de funcionamento;
//	$60; 			- Sistema Desligado (nunca ligar�);
//	$61;			- Liga somente à noite. Sensor superior;
//	$62;			- Liga nos determinados hor�rios estipulados;
//	$63;			- Função para válvula do reservat�rio;
//	$64;			- Função para motobomba do reservat�rio;
//*/
//	// Tx - Transmitter
//	if(enableDecode)
//	{
//		enableDecode = 0;
//
////		int i;
////		for(i=0;i<rLength;i++)
////		{
////			Serial1.println(sInstr[i]);
////		}
////		for(i=0;i<rLength;i++)
////		{
////			Serial1.println(sInstr[i],HEX);
////		}
//
//		// Getting the opcode
//		aux[0] = '0';
//		aux[1] = sInstr[0];
//		aux[2] = '\0';
//		opcode = (uint8_t) atoi(aux);
////		Serial.println("Got!");
////		uint8_t statusCommand = 0;
//
//		switch (opcode)
//		{
//// -----------------------------------------------------------------
//			case 0:		// Check status
//			{
////				aux[0] = '0';
////				aux[1] = sInstr[1];
////				aux[2] = '\0';
////				statusCommand = (uint8_t) atoi(aux);
////
////				switch (statusCommand)
////				{
////					// ----------
////					// $02:c;  -> clear time counter;
////					// $02:c:mm;  -> set time counter ref;
////					case 0:	// $00:1; or $00;
////					{
////						if(sInstr[2]==':' && sInstr[4]==';')
////						{
////							aux[0] = '0';
////							aux[1] = sInstr[3];
////							aux[2] = '\0';
////							flag_debug_time = (uint8_t) atoi(aux);
////						}
////						else
////						{
////							summary_Print(statusCommand);
////						}
////					}
////					break;
////
////					case 2:
////					{
////						if(sInstr[2]==':' && sInstr[3]=='c')
////						{
////							if(sInstr[4] == ';')
////							{
////								flag_waitPowerOn = 0;
////								waitPowerOn_min = 0;
////								waitPowerOn_sec = 0;
////							}
////							else if(sInstr[4] ==':' && sInstr[7] == ';')
////							{
////								aux[0] = sInstr[5];
////								aux[1] = sInstr[6];
////								aux[2] = '\0';
////								waitPowerOn_min_standBy = (uint8_t) atoi(aux);
////
////								eeprom.write(eeprom.pageSet, eeprom.addr_standBy_min, waitPowerOn_min_standBy);
////								//eeprom_write_byte((uint8_t *)(addr_standBy_min), waitPowerOn_min_standBy);
////
//////								Serial.print("powerOn min:");
//////								Serial.println(powerOn_min_Standy);
////							}
////						}//$02:s:129;
////						else if(sInstr[2] == ':' && sInstr[3] == 's' && sInstr[4] == ':' && sInstr[8] == ';')
////						{
////							aux2[0] = '0';
////							aux2[1] = sInstr[5];
////							aux2[2] = sInstr[6];
////							aux2[3] = sInstr[7];
////							aux2[4] = '\0';
////							motorTimerE = (uint16_t) atoi(aux2);
////
////							eeprom.write(eeprom.pageSet, eeprom.addr_motorTimerE, motorTimerE);
////
//////							uint8_t lbyteRef = 0, hbyteRef = 0;
//////							lbyteRef = motorTimerE;
//////							hbyteRef = (motorTimerE >> 8);
////
//////							eeprom_write_byte((uint8_t *)(addr_motorTimerE+1), hbyteRef);
//////							eeprom_write_byte((uint8_t *)(addr_motorTimerE), lbyteRef);
////						}
////						summary_Print(statusCommand);
////					}
////					break;
////					// ------------------------------
////					case 3:// $03:s:68;
////						if(sInstr[2]==':' && sInstr[3] == 's' && sInstr[4] == ':' && sInstr[7] == ';')
////						{
////							aux[0] = sInstr[5];
////							aux[1] = sInstr[6];
////							aux[2] = '\0';
////							PRessureRef = (uint8_t) atoi(aux);
////							eeprom.write(eeprom.pageSet, eeprom.addr_PRessureRef, PRessureRef);
////							//eeprom_write_byte((uint8_t *)(addr_PRessureRef), PRessureRef);
////
////							sprintf(Serial.buffer,"PRessRef: %d", PRessureRef);
////							Serial.println(Serial.buffer);
////						}
////						else if(sInstr[2]==':' && sInstr[3] == 'v' && sInstr[4] == ':' && sInstr[7] == ';')
////						{
////							aux[0] = sInstr[5];
////							aux[1] = sInstr[6];
////							aux[2] = '\0';
////							PRessureRef_Valve = (uint8_t) atoi(aux);
////							eeprom.write(eeprom.pageSet, eeprom.addr_PRessureRef_Valve, PRessureRef_Valve);
////							//eeprom_write_byte((uint8_t *)(addr_PRessureRef_Valve), PRessureRef_Valve);
////
////							sprintf(Serial.buffer,"PRessRef_Valve: %d", PRessureRef_Valve);
////							Serial.println(Serial.buffer);
////						}
////						else if(sInstr[2]==':' && sInstr[3] == 'p' && sInstr[4] == ':' && sInstr[8] == ';')
////						{// $03:p:150;
////							aux2[0] = '0';
////							aux2[1] = sInstr[5];
////							aux2[2] = sInstr[6];
////							aux2[3] = sInstr[7];
////							aux2[4] = '\0';
////							PRessureMax_Sensor = (uint8_t) atoi(aux2);
////
////							eeprom.write(eeprom.pageSet, eeprom.addr_PRessureMax_Sensor, PRessureMax_Sensor);
//////							eeprom_write_byte((uint8_t *)(addr_PRessureMax_Sensor), PRessureMax_Sensor);
////						}
////						else if(sInstr[2]==':' && sInstr[3] == 'b' && sInstr[4] == ':' && sInstr[7] == ';')
////						{
////							aux[0] = sInstr[5];
////							aux[1] = sInstr[6];
////							aux[2] = '\0';
////							PRessurePer = (uint8_t) atoi(aux);
////							eeprom.write(eeprom.pageSet, eeprom.addr_PRessurePer, PRessurePer);
////							//eeprom_write_byte((uint8_t *)(addr_PREssurePer), PRessurePer);
////						}
////						else
////							summary_Print(statusCommand);
////						break;
////					// ------------------------------
////					case 4: // $04:1;
////						if(sInstr[2]==':' && sInstr[4]==';')
////						{
////							aux[0] = '0';
////							aux[1] = sInstr[3];
////							aux[2] = '\0';
////							flag_debug = (uint8_t) atoi(aux);
////						}//$04:s:0900;
////						else if(sInstr[2]==':' && sInstr[3]=='s' && sInstr[4]==':' && sInstr[9]==';')
////						{
////							aux2[0] = sInstr[5];
////							aux2[1] = sInstr[6];
////							aux2[2] = sInstr[7];
////							aux2[3] = sInstr[8];
////							aux2[4] = '\0';
////
////							levelRef_10bit = (uint16_t) atoi(aux2);
////
//////							uint8_t lbyteRef = 0, hbyteRef = 0;
//////							lbyteRef = levelRef_10bit;
//////							hbyteRef = (levelRef_10bit >> 8);
////
////							eeprom.write(eeprom.pageSet, eeprom.addr_LevelRef, levelRef_10bit);
////
////							//eeprom_write_byte((uint8_t *)(addr_LevelRef+1), hbyteRef);
////							//eeprom_write_byte((uint8_t *)(addr_LevelRef), lbyteRef);
////						}
////						summary_Print(statusCommand);
////						sprintf(Serial.buffer,"Ref: %d", levelRef_10bit);
////						Serial.println(Serial.buffer);
////						break;
////					// ------------------------------
////					case 7:
////						if(sInstr[2]==':' && sInstr[4]==';')
////						{
////							aux[0] = '0';
////							aux[1] = sInstr[3];
////							aux[2] = '\0';
////							uint8_t adcCommand = (uint8_t) atoi(aux);
////
////							switch (adcCommand)
////							{
//////								case 0:
//////									ADMUX &=  ~(1<<REFS1);		// AREF, Internal Vref turned off
//////									ADMUX &=  ~(1<<REFS0);
//////									Serial.println("AREF");
//////									break;
//////
//////								case 1:
//////									ADMUX &=  ~(1<<REFS1);		// AVCC with external capacitor at AREF pin
//////									ADMUX |=   (1<<REFS0);
//////									Serial.println("AVCC");
//////									break;
//////
//////								case 2:
//////									ADMUX |=   (1<<REFS1);		// Internal 1.1V Voltage Reference with external capacitor at AREF pin
//////									ADMUX |=   (1<<REFS0);
//////									Serial.println("1.1V");
//////									break;
////							}
////						}
////						break;
////					// ------------------------------
////					case 9:
////						Serial.println("Rebooting...");
////						NVIC_SystemReset();
////						//wdt_enable(WDTO_15MS);
//////						flag_reset = 1;
////						break;
////
////					default:
////						summary_Print(statusCommand);
////						break;
////				}
//			}
//			break;
//// -----------------------------------------------------------------
//			case 1: 	// Setup calendar
//			{
////				// Set-up clock -> $1:h:HHMMSS;
////				if(sInstr[1]==':' && sInstr[2]=='h' && sInstr[3]==':' && sInstr[10]==';')
////				{
////					aux[0] = sInstr[4];
////					aux[1] = sInstr[5];
////					aux[2] = '\0';
////					tm.Hour = (uint8_t) atoi(aux);
////
////					aux[0] = sInstr[6];
////					aux[1] = sInstr[7];
////					aux[2] = '\0';
////					tm.Minute = (uint8_t) atoi(aux);
////
////					aux[0] = sInstr[8];
////					aux[1] = sInstr[9];
////					aux[2] = '\0';
////					tm.Second = (uint8_t) atoi(aux);
////
//////					RTC.write(tm);
////					rtc.write(tm);
////					summary_Print(0);
////				}
////				// 	Set-up date -> $1:d:DDMMAAAA;
////				else if(sInstr[1]==':' && sInstr[2]=='d' && sInstr[3]==':' && sInstr[12]==';')
////				{
////					// Getting the parameters
////					aux[0] = sInstr[4];
////					aux[1] = sInstr[5];
////					aux[2] = '\0';
////					tm.Day = (uint8_t) atoi(aux);
////
////					aux[0] = sInstr[6];
////					aux[1] = sInstr[7];
////					aux[2] = '\0';
////					tm.Month = (uint8_t) atoi(aux);
////
////					char aux2[5];
////					aux2[0] = sInstr[8];
////					aux2[1] = sInstr[9];
////					aux2[2] = sInstr[10];
////					aux2[3] = sInstr[11];
////					aux2[4] = '\0';
////					tm.Year = (uint8_t) (atoi(aux2)-1970);
////
//////					RTC.write(tm);
////					rtc.write(tm);
////
////					summary_Print(0);
////
////				}
////				// Set-up date -> $1:d:DDMMAAAA;
////				else if(sInstr[1]==':' && sInstr[2]=='c' && sInstr[3]==':' && sInstr[9]==';')
////				{	// $1:c:40123;
////					char aux3[6];
////					aux3[0] = sInstr[4];
////					aux3[1] = sInstr[5];
////					aux3[2] = sInstr[6];
////					aux3[3] = sInstr[7];
////					aux3[4] = sInstr[8];
////					aux3[5] = '\0';
////					rtc.rtc_PRL = (uint32_t) atoi(aux3);
////
////					rtc.setRTC_DIV(rtc.rtc_PRL);
////					sprintf(Serial.buffer,"PRLw: %ld", rtc.rtc_PRL);
////					Serial.println(Serial.buffer);
////
////					eeprom.write(eeprom.pageSet, eeprom.addr_rtc_PRL, (uint16_t) rtc.rtc_PRL);
////
////					sprintf(Serial.buffer,"PRLr: %ld", rtc.getRTC_DIV());
////					Serial.println(Serial.buffer);
////				}
////				else if(sInstr[1]==':' && sInstr[2]=='c' && sInstr[3]==';')
////				{
////					sprintf(Serial.buffer,"PRLreg: %lu, PRLflash: %u", rtc.getRTC_DIV(), eeprom.read(eeprom.pageSet, eeprom.addr_rtc_PRL));
////					Serial.println(Serial.buffer);
////				}
////				// $1:s:S;
////				else if(sInstr[1]==':' && sInstr[2]=='s' && sInstr[3]==':' && sInstr[5]==';')
////				{
////					aux[0] = '0';
////					aux[1] = sInstr[4];
////					aux[2] = '\0';
////					rtc.rtc_clkSource = (uint8_t) atoi(aux);
////
////					eeprom.write(eeprom.pageSet, eeprom.addr_rtc_clkSource, rtc.rtc_clkSource);
////					rtc.bkpDomainReset();
////					rtc.begin_rtc(rtc.rtc_clkSource, rtc.rtc_PRL);
////
////					sprintf(Serial.buffer,"rtcSource: %u", rtc.rtc_clkSource);
////					Serial.println(Serial.buffer);
////				}
////				else if(sInstr[1]==':' && sInstr[2]=='s' && sInstr[3]==';')
////				{
////					eeprom.read(eeprom.pageSet, eeprom.addr_rtc_clkSource);
////
////					summary_Print(0);
////					sprintf(Serial.buffer,"rtcSource: %u", rtc.rtc_clkSource);
////					Serial.println(Serial.buffer);
////				}
//			}
//			break;
//// -----------------------------------------------------------------
//			case 2:		// Setup Bluetooth device name
//			{
////				// Setup clock -> $2:BluetoothName;
////				char aux_str[sInstr_SIZE], aux_str2[sInstr_SIZE+7];
////				uint8_t k1=0;
////				if(sInstr[1]==':')
////				{
////					// 3 because 2 and : count 2 characters and one more for '\0'
////					while((k1<sInstr_SIZE-3) && sInstr[k1] != ';')
////					{
////						aux_str[k1] = sInstr[k1+2];
////						k1++;
//////						Serial.println(k1);
////					}
////
////					aux_str[k1-2] = '\0';
////					Serial.println("Disconnect!");
//////					wdt_reset();
//////					_delay_ms(3000);
//////					wdt_reset();
//////					_delay_ms(3000);
//////					wdt_reset();
//////					_delay_ms(3000);
//////					strcpy(aux_str2,"AT");
//////					Serial.print(aux_str2);
////					//wdt_reset();
////					_delay_ms(3000);
////					strcpy(aux_str2,"AT+NAME");
////					strcat(aux_str2,aux_str);
////					Serial.print(aux_str2);
////					//wdt_reset();
////					_delay_ms(1000);
////				}
//			}
//			break;
//// -----------------------------------------------------------------
//			case 3:		// Set motor ON/OFF
//			{
////				uint8_t motorCommand;
////				aux[0] = '0';
////				aux[1] = sInstr[1];
////				aux[2] = '\0';
////				motorCommand = (uint8_t) atoi(aux);
////
////				if (motorCommand && (!motorStatus))
////				{
////					motor_start();
////					Serial.println("value");
////				}
////				else
////				{
////					motor_stop(0x06);
////				}
////
////				summary_Print(3);
//			}
//			break;
//// -----------------------------------------------------------------
//			case 4: 	// FLASH store test, //$4:r:23; and $4:w:23:03;
//			{
////				if(sInstr[1]==':' && sInstr[2]=='r' && sInstr[3]==':' && sInstr[6]==';')
////				{	// $4:r:05; read 05*2 address of page block
////					// Getting the parameters
////					aux[0] = sInstr[4];
////					aux[1] = sInstr[5];
////					aux[2] = '\0';
////					uint8_t addrt = (uint8_t) atoi(aux);
////					uint8_t var = eeprom.read(eeprom.pageSet, addrt);
////					sprintf(Serial.buffer,"EE read: %d ", var);
////					Serial.println(Serial.buffer);
////
////					sprintf(Serial.buffer,"Page %d ", eeprom.pageSet);
////					Serial.println(Serial.buffer);
////
////					sprintf(Serial.buffer,"Address %X ", (unsigned int) eeprom.pageAddress(eeprom.pageSet));
////					Serial.println(Serial.buffer);
////				}
////				if(sInstr[1]==':' && sInstr[2]=='w' && sInstr[3]==':' && sInstr[6]==':' && sInstr[9]==';')
////				{	// $4:w:03:07;
////					// Getting the parameters
////					aux[0] = sInstr[4];
////					aux[1] = sInstr[5];
////					aux[2] = '\0';
////					uint8_t addrt = (uint8_t) atoi(aux);
////
////					aux[0] = sInstr[7];
////					aux[1] = sInstr[8];
////					aux[2] = '\0';
////					uint8_t var = (uint8_t) atoi(aux);
////
////					eeprom.write(eeprom.pageSet, addrt, var);
////					sprintf(Serial.buffer,"EE write: %d ", var);
////					Serial.println(Serial.buffer);
////
////					var = eeprom.read(eeprom.pageSet, addrt);
////					sprintf(Serial.buffer,"EE read2: %d ", var);
////					Serial.println(Serial.buffer);
////
////					sprintf(Serial.buffer,"Page %d ", eeprom.pageSet);
////					Serial.println(Serial.buffer);
////
////					sprintf(Serial.buffer,"Address %X ", (unsigned int) eeprom.pageAddress(eeprom.pageSet));
////					Serial.println(Serial.buffer);
////				}
////				if(sInstr[1]==':' && sInstr[2]=='f' && sInstr[3]==':' && sInstr[6]==':' && sInstr[9]==';')
////				{	// $4:f:64:07;	fill page 64 with 07 value;
////					// Getting the parameters
////					aux[0] = sInstr[4];
////					aux[1] = sInstr[5];
////					aux[2] = '\0';
////					uint8_t page = (uint8_t) atoi(aux);
////
////					aux[0] = sInstr[7];
////					aux[1] = sInstr[8];
////					aux[2] = '\0';
////					uint8_t var = (uint8_t) atoi(aux);
////
////
////
////					eeprom.writePage(page, (var << 8 | var));
////					sprintf(Serial.buffer,"Filled page %d ", page);
////					Serial.println(Serial.buffer);
////
////					sprintf(Serial.buffer,"Address %X ", (unsigned int) eeprom.pageAddress(page));
////					Serial.println(Serial.buffer);
////				}
////				if(sInstr[1]==':' && sInstr[2]=='e' && sInstr[3]==':' && sInstr[6]==';')
////				{	// $4:e:64;	erase page 64
////					// Getting the parameters
////					aux[0] = sInstr[4];
////					aux[1] = sInstr[5];
////					aux[2] = '\0';
////					uint8_t page = (uint8_t) atoi(aux);
////
////					eeprom.erasePage(page);
////					sprintf(Serial.buffer,":Page %d erased!", page);
////					Serial.println(Serial.buffer);
////
////					sprintf(Serial.buffer,"Address %X ", (unsigned int) eeprom.pageAddress(page));
////					Serial.println(Serial.buffer);
////				}
////				break;
//			}
//			break;
//// -----------------------------------------------------------------
//			case 5: 	// Command is $5:h1:2130;
//			{
////				if(sInstr[1]==':' && sInstr[2]=='h' && sInstr[4]==':' && sInstr[9]==';')
////				{
////					aux[0] = '0';
////					aux[1] = sInstr[3];
////					aux[2] = '\0';
////					uint8_t indexV = (uint8_t) atoi(aux);
////
////					aux[0] = sInstr[5];
////					aux[1] = sInstr[6];
////					aux[2] = '\0';
////					HourOnTM[indexV-1] = (uint8_t) atoi(aux);
////					eeprom.write(eeprom.pageSet, eeprom.addr_HourOnTM+indexV-1, HourOnTM[indexV-1]);
////					//eeprom_write_byte(( uint8_t *)(addr_HourOnTM+indexV-1), HourOnTM[indexV-1]);
////
////					aux[0] = sInstr[7];
////					aux[1] = sInstr[8];
////					aux[2] = '\0';
////					MinOnTM[indexV-1] = (uint8_t) atoi(aux);
////					eeprom.write(eeprom.pageSet, eeprom.addr_MinOnTM+indexV-1, MinOnTM[indexV-1]);
//////					eeprom_write_byte(( uint8_t *)(addr_MinOnTM+indexV-1), MinOnTM[indexV-1]);
////
////					summary_Print(5);
////				}
////				else if(sInstr[1]==':' && sInstr[2]=='n' && sInstr[3]==':' && sInstr[5]==';')
////				{
////					aux[0] = '0';
////					aux[1] = sInstr[4];
////					aux[2] = '\0';
////
////					nTM = (uint8_t) atoi(aux);
////					eeprom.write(eeprom.pageSet, eeprom.addr_nTM, nTM);
////					//eeprom_write_byte(( uint8_t *)(addr_nTM), nTM);
////
////					summary_Print(5);
////				}
////				else if(sInstr[1]==';')
////				{
////					summary_Print(5);
////				}
//			}
//			break;
//// ----------------------------------------------------------------
//			case 6:		// Set working mode
//			{
////				aux[0] = '0';
////				aux[1] = sInstr[1];
////				aux[2] = '\0';
////				stateMode = (uint8_t) atoi(aux);
////				eeprom.write(eeprom.pageSet, eeprom.addr_stateMode, stateMode);
//////				//eeprom_write_byte(( uint8_t *)(addr_stateMode), stateMode);
////
////				summary_Print(0);
//			}
//			break;
//// -----------------------------------------------------------------
//			case 7:		// nRF24L01p test functions;
//			{
////				// $7:s:
////				uint8_t state =9;
////				if(sInstr[1]==':' && sInstr[2]=='s' && sInstr[3]==';')
////				{
////					radio.begin_nRF24L01p();
////				}
////				else if(sInstr[1]==':' && sInstr[2]=='g' && sInstr[3]==';')
////				{
////					state = radio.get_stateMode();
////				}
////				else if(sInstr[1]==':' && sInstr[2]=='t' && sInstr[3]==';')
////				{
////					state = radio.set_mode_tx(ENABLE);
////				}
////				else if(sInstr[1]==':' && sInstr[2]=='r' && sInstr[3]==';')
////				{
////					state = radio.set_mode_rx(ENABLE);
////				}
////				else if(sInstr[1]==':' && sInstr[2]=='b' && sInstr[3]==';')
////				{
////					state = radio.set_mode_standbyI();
////				}
////				else if(sInstr[1]==':' && sInstr[2]=='p' && sInstr[3]==';')
////				{
////					state = radio.set_mode_powerDown();
////				}
////
////				sprintf(Serial.buffer,"state: %u", state);
////				Serial.println(Serial.buffer);
//
////				radio.begin_nRF24L01p();
////				radio.set_250kbps();
////				if(sInstr[1]==':' && sInstr[2]=='h' && sInstr[4]==':' && sInstr[9]==';')
////				{
////					aux[0] = '0';
////					aux[1] = sInstr[3];
////					aux[2] = '\0';
////					uint8_t indexV = (uint8_t) atoi(aux);
////
////					aux[0] = sInstr[5];
////					aux[1] = sInstr[6];
////					aux[2] = '\0';
////					HourOnTM[indexV-1] = (uint8_t) atoi(aux);
////					eeprom.write(eeprom.pageSet, eeprom.addr_HourOnTM+indexV-1, HourOnTM[indexV-1]);
////					//eeprom_write_byte(( uint8_t *)(addr_HourOnTM+indexV-1), HourOnTM[indexV-1]);
////
////					aux[0] = sInstr[7];
////					aux[1] = sInstr[8];
////					aux[2] = '\0';
////					MinOnTM[indexV-1] = (uint8_t) atoi(aux);
////					eeprom.write(eeprom.pageSet, eeprom.addr_MinOnTM+indexV-1, MinOnTM[indexV-1]);
//////					eeprom_write_byte(( uint8_t *)(addr_MinOnTM+indexV-1), MinOnTM[indexV-1]);
////
////					summary_Print(5);
//			}
//			break;
//// -----------------------------------------------------------------
//			default:
////				summary_Print(10);
//				break;
//// -----------------------------------------------------------------
//		}
//		memset(sInstr,0,sizeof(sInstr));	// Clear all vector;
//	}
//}
//void acn_comm_Bluetooth()
//{
//	// Rx - Always listening
////	uint8_t j2 =0;
//	while((Serial.available()>0))	// Reading from serial
//	{
//		inChar = Serial.read();
//
//		if(inChar=='$')
//		{
//			j2 = 0;
//			flag_frameStartBT = 1;
////			Serial.println("Frame Start!");
//		}
//
//		if(flag_frameStartBT)
//			sInstrBluetooth[j2] = inChar;
//
////		sprintf(Serial.buffer,"J= %d",j2);
////		Serial.println(Serial.buffer);
//
//		j2++;
//
//		if(j2>=sizeof(sInstrBluetooth))
//		{
//			memset(sInstrBluetooth,0,sizeof(sInstrBluetooth));
//			j2=0;
////			Serial.println("ZEROU! sIntr BLuetooth Buffer!");
//		}
//
//		if(inChar==';')
//		{
////			Serial.println("Encontrou ; !");
//			if(flag_frameStartBT)
//			{
////				Serial.println("Frame Stop!");
//				flag_frameStartBT = 0;
//				rLength = j2;
//				j2 = 0;
//				enableTranslate_Bluetooth = 1;
//			}
//		}
//	}
////	flag_frameStart = 0;
//
//	if(enableTranslate_Bluetooth)
//	{
////		Serial.println("enableTranslate_Bluetooth");
//		enableTranslate_Bluetooth = 0;
//
//		char *pi0, *pf0;
//		pi0 = strchr(sInstrBluetooth,'$');
//		pf0 = strchr(sInstrBluetooth,';');
//
//		if(pi0!=NULL)
//		{
//			uint8_t l0=0;
//			l0 = pf0 - pi0;
//
//			int i;
//			for(i=1;i<=l0;i++)
//			{
//				sInstr[i-1] = pi0[i];
////				Serial.write(sInstr[i-1]);
//			}
//			memset(sInstrBluetooth,0,sizeof(sInstrBluetooth));
//	//		Serial.println(sInstr);
//
//			enableDecode = 1;
//		}
//		else
//		{
////			Serial.println("Err");
////			Serial.write(pi0[0]);
////			Serial.write(pf0[0]);
//		}
//	}
//}

//#endif /* ACIONNA_HPP_ */



