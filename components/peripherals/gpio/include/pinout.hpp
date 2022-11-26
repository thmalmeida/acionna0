#ifndef PINOUT_HPP__
#define PINOUT_HPP__

#include "hal/gpio_types.h"

#define I2C_SDA				GPIO_NUM_0
#define I2C_SCL				GPIO_NUM_4

#define AC_LOAD1			GPIO_NUM_21		//n-board 21: contator
#define AC_LOAD2			GPIO_NUM_18
#define AC_LOAD3			GPIO_NUM_5

#define DS18B20_DATA		GPIO_NUM_17

#define GPIO_GENERIC1		GPIO_NUM_23		// K1: contator
#define GPIO_GENERIC2		GPIO_NUM_22		// K2
#define GPIO_GENERIC3		GPIO_NUM_19     // K3
#define GPIO_GENERIC4		GPIO_NUM_33		// T1: relé térmico

#define WATER_LEVEL1		GPIO_NUM_26
#define WATER_LEVEL2		GPIO_NUM_14
#define WATER_LEVEL3		GPIO_NUM_12
#define WATER_LEVEL4		GPIO_NUM_27

#define PRESSURE_SENSOR1	ADC1_CHANNEL_4  // (GPIO_NUM_32) Presssure/potenciometro 0V-1.1V - ADC
#define PRESSURE_SENSOR2    ADC1_CHANNEL_7  // (GPIO_NUM_35) For a second pressure sensor - motor 2 (GPIO_NUM_35)
#define	VOLTAGE_SENSOR		GPIO_NUM_34		//Voltage sensor 0V-2.6V - ADC

#define CURRENT_SENSOR1		GPIO_NUM_36		//n-board mounted
#define CURRENT_SENSOR1_ADC	ADC1_CHANNEL_0

#define CURRENT_SENSOR2		GPIO_NUM_39
#define CURRENT_SENSOR2_ADC	ADC1_CHANNEL_3

#define VALVE_01            GPIO_NUM_32      // wrong!!!
#define VALVE_02            GPIO_NUM_25
#define VALVE_03            GPIO_NUM_26
#define VALVE_04            GPIO_NUM_27
#define VALVE_05            GPIO_NUM_14
#define VALVE_06            GPIO_NUM_12
#define VALVE_07            GPIO_NUM_13
#define VALVE_08            GPIO_NUM_17
#define VALVE_09            GPIO_NUM_16
#define VALVE_10            GPIO_NUM_2
#define VALVE_11            GPIO_NUM_15

#define HUMIDITY_SENSOR     GPIO_NUM_13 // with jump J19


#define LED_0				GPIO_NUM_2

#endif /* AGRO_MESH_PINOUT_HPP__ */
