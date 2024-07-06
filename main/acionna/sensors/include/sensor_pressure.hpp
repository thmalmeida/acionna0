#ifndef SENSOR_PRESSURE_HPP__
#define SENSOR_PRESSURE_HPP__

#include "adc.hpp"
// #include <gpio_driver.h>
// #include <pinout.hpp>

class sensor_pressure {
public:
	int PRess;								// last pressure converted;
	int PRessHold;							// max pressure converted on turned on period;

	uint8_t PRessureRef = 0;				// max threshold pressure;
	uint8_t PRessureRef_Valve = 0;			// max threshold valve pressure to turn open;
	uint8_t PRessureMax_Sensor = 100; 		// sensor max pressure [psi]
	uint8_t PRessurePer = 85;				// percent pressure bellow nominal to turn load off;
	uint8_t PRessureRef_Low = 10;
	uint16_t levelRef_10bit = 0;			// digital 10 bit number to threshold level sensor;

	uint8_t flag_PressureUnstable = 1;
	uint8_t flag_PressureDown = 0;			// flag for pressure down occurrence;
	uint8_t flag_timeMatch = 0;				// flag when turn on time occurs;

	// functions
	double pressure(void) {

		return 0.0;
	}

private:
	ADC_Driver *adc_;
};

#endif /* P_SENSOR_HPP__ */
