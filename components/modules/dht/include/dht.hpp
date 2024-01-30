#ifndef __DHT_HPP__
#define __DHT_HPP__
/**
 * Leitor de humidade e temperatura DTH
 * Só testado para o DTH11
 * Em teoria serve para DTH22 e outros
 * Based: https://github.com/adafruit/DHT-sensor-library
 *
 ***********************************
 * Modo de usar:
 *
 * if(requestRead){
 * 	float t = getTempCelsius();
 * 	float h = getHumidity();
 * } else
 *  printf("Erro na leitura");
 ***********************************
 */


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#define TAG_DHT "DHT"


#include "gpio_driver.h"
#include "hardware_defs.h"


class DHT{
	public:
		DHT(GPIO_DRIVER* gpio);

		void begin();
		bool requestRead();
		bool read2();
		int16_t getTempCelsius(int sensor_DHT11);
		uint16_t getHumidity(int sensor_DHT11);

		void set_low();
		void set_high();

	private:
		GPIO_DRIVER* _gpio;
		
		uint8_t _data[5];
		bool checkSum();
		bool read();
		int count_pulse_us(int pulse, int timeout, int transition_edge);
		uint8_t waitPulse(uint8_t pulse, uint8_t us = 80);
};

#endif /* __DTH_H__ */
