/*
 * load_cell.hpp
 *  Started creation on 2017-01-19
 *  Created on: 2024-01-19
 *      Author: thmalmeida@gmail.com
 * 	Modified: 24/04/2024
 */

#ifndef HX711_HPP__
#define HX711_HPP__

#include "gpio_driver.hpp"


/* This class is used to control the HX711 module.
* 
* The HX711 digital communication is made in a serial way using two pins.
* We need one pin for data and another for the clock.
*
* Pinout reference
* 
* Analog part
* E+    supply voltage to load cell. 5 V applied to AVDD.
* E-    supply negative voltage to load cell
* A+    Channel A programmed with a gain of 128 or 64 corresponding to a
* A-    full scale differential input voltage of 20 mV or 40 mV.
* B+    Channel B has a fixed gain of 32.
* B-
*
* Digital part
* SCK
* SDA
*
* Ref. HX711 - 24-Bit Analog-to-Digital Converter (ADC) for Weigh Scales (AVIA Semicondutor)
*/

#define HX711_TIME_DELAY_PROTOCOL	25     // time delay (frequency) protocol of HX711 interface [us];
#define DEBUG_HX711					1

class HX711 {
public:
	/* 
	* @param id some identification to find faults on working task
	*/
	HX711(int pin_dout, int pin_pd_sck, int id) : pin_{{pin_dout, 0},{pin_pd_sck, 1}}, id_{id} {
		pin_[0].mode(0);	// set pin_dout as input
		pin_[1].mode(1);	// set pin sck as output

		// gateConfig(pin_data, 0);		// data pin
		// gateConfig(pin_sck, 1);			// sck pin
		// gateConfig(pin_tare_HX711, 0);	// data pin
		// gateConfig(pin_led, 1);			// led
		// gateConfig(pin_beep, 1);		// beep
		// pin_data_HX711 = pin_data;
		// pin_sck_HX711 = pin_sck;
		#ifdef DEBUG_HX711
		printf("HX711: init\n");
		#endif
	}
	uint32_t read(void) {
		uint32_t value = 0;

		// set pin data to 1 and wait pin data go low after set sck to 0;
		pin_data_(1);
		pin_sck_(0);

		int fault = 200;	// wait 200 ms before return error
		while(pin_data_()) {
			delay_ms(1);
			fault--;
			if(!fault) {
				#ifdef DEBUG_HX711
				printf("HX711_%d: pin data lock high\n", id_);
				#endif
				break;
			}
		}

		// 24 bit read size. Should run 24 cycles
		for(int i=0; i<24; i++) {

			// start new wave cycle
			pin_sck_(1);
			delay_us(HX711_TIME_DELAY_PROTOCOL);

			// shift buffer left before add the new bit
			value <<= 1;

			pin_sck_(0);
			delay_us(HX711_TIME_DELAY_PROTOCOL);

			// OR operation between new pin read and existing buffer value
			if(pin_data_())
				value |= 1;
		}

		// XOR operatinon on 24 th bit recommended by datasheet
		// value ^= 0x800000;

		pin_sck_(1);
		delay_us(HX711_TIME_DELAY_PROTOCOL);

		pin_sck_(0);
		delay_us(HX711_TIME_DELAY_PROTOCOL);

		return value;
	}

	void power_down(void) {
		pin_sck_(1);
		delay_us(65);
	}
	void reset(void) {
		power_down();
		pin_sck_(0);
	}

private:

	GPIO_Driver pin_[2];

	void pin_sck_(int status) {
		pin_[1].write(status);
	}
	void pin_data_(int status) {
		pin_[0].write(status);
	}
	int pin_data_(void) {
		return pin_[0].read();
	}

	// For debug purpose
	int id_;
};

#endif