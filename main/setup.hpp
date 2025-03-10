/*
 * ws_setup.hpp
 *
 *  Created on: 17 de jul. de 2022
 *      Author: thmalmeida
 */
#ifndef __SETUP_HPP__
#define __SETUP_HPP__

#include "acionna/acionna.hpp"
  
// extern ADC_driver adc0;
// extern Acionna acionna0;

// For test purpose only
void test_adc(void *pvParameter);
void test_adc_dma(void *pvParameter);
void test_i2c_to_gpio(void *pvParameter);
void test_sensors(void *pvParameter);
void test_timer(void *pvParameter);
void test_wifi(void *pvParameter);


// Acionna work machine
void isr_1sec(void *pvParameter);
void isr_100ms(void *pvParameter);
void machine_run(void *pvParameter);
#endif
