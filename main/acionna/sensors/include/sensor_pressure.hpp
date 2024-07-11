#ifndef SENSOR_PRESSURE_HPP__
#define SENSOR_PRESSURE_HPP__

#define K_PSI_MCA 0.703089

#include "adc.hpp"
// #include <gpio_driver.h>
// #include <pinout.hpp>


		/* Sensor functions
	  4.5 V___	 1.1 V__	922___	1.2 MPa___	 12 Bar___	 120 m.c.a.___		  4096 ___       3.16 V___
			|		  | 		|			|			|				|				|				|
			|		  |			|			|			|				|				|				|
			|		  |			|			|			|				|				|				|
		 Vo_|		  |		Do__|		Po__|			|			Pa__|		   d12__|			 v__|
			|		  |			|			|			|				|				|				|
			|		  |			|			|			|				|				|				|
			|		  |			|			|			|				|				|				|
		   _|_		 _|_	   _|_		   _|_		   _|_			   _|_			   _|_			   _|_
		0.5 V	 0.1 V			103			0 MPa		0 Bar		0 m.c.a.			0				0 V

	Vo  = Do
	4.5	  2^n

	with n = 10, Vmax = 5.0 V
	Do = Vo*2^n
	      5.0


	we are trying to convert
	  1.1 V___	   2^n_bits___	  	  5.0 V___	  	  4.5 V___	 120 m.c.a.___		  4096 ___       3.16 V___
			|				|				|				|				|				|				|
			|				|				|				|				|				|				|
			|				|				|				|				|				|				|
	 V1_out_|			Pd__|		 V2_out_|				|			Pa__|		   d12__|			 v__|
			|				|				|				|				|				|				|
			|				|				|				|				|				|				|
			|				|				|	 			|				|				|				|
		   _|_			  0_|_			0 V_|_		 0.5 V _|_			   _|_			   _|_			   _|_
		0.0 V											0 m.c.a.			0				0 V

	 using 12 bits and 0 attenuation, experimentally we have found the following relationship on 2022-04-29

	  4.5 V___	   	1.016 V___	  	   3890___	  	150 psi___	  x m.c.a. ___
			|				|				|				|				|
			|				|				|				|				|
			|				|				|				|				|
	 V1_out_|			Pd__|		 V2_out_|		 P_psi__|		 P_mca__|
			|				|				|				|				|
			|				|				|				|				|
			|				|				|	 			|				|
	  0.5 V_|_		 113 mV_|_			127_|_		  0 psi_|_	   0 m.c.a._|_

	1 psi = 0,703089 m.c.a.
	(V2_out-127)/(3890-127) = P_psi/150

	P_psi = 150*(V2_out - 127)/3763
	P_mca = P_psi*0.703089

	 */


class Sensor_Pressure {
public:
	Sensor_Pressure(ADC_Driver *adc, int channel, int pressure_psi_max) : adc_(adc), channel_(channel), pressure_psi_max_(pressure_psi_max) {
		adc_->channel_config(channel_);
	}

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
	double pressure_psi(void) {

		int data_12bits = adc_->read(channel_);
		return static_cast<double>(pressure_psi_max_)*(data_12bits - d_min_)/(d_max_-d_min_);
	}
	double pressure_mca(void) {
		return pressure_psi()*K_PSI_MCA;
	}
	void pressure_psi_max(int value) {
		pressure_psi_max_ = value;
	}
	int pressure_psi_max(void) {
		return pressure_psi_max_;
	}
	
private:
	ADC_Driver *adc_;
	int channel_;
	int pressure_psi_max_;
	const double d_max_ = 3878.0;	// 4500 mV
	const double d_min_ = 124.0; 	// 502 mV
};

#endif /* P_SENSOR_HPP__ */




//	uint32_t voltage_converter(uint32_t d12)
//	{
//	/*Sens output
//		(out-0.5)/(4.5-0.5) = 1024
//
//		(out-0.0)/(5-0) = (x-0)/(1024-0)
//
//		(Pd - 103)/(922-103) = (Pa - 0)/(120 - 0)
//		Pa = 120.0*Pd/(1024.0);
//
//		(xs - 0) = temp - (0)
//		(255 - 0)  +50 - (0)
//
//		Direct Conversion
//		xs = 255*(temp+0)/51
//		tempNow_XS = (uint8_t) 255.0*(tempNow+0.0)/51.0;
//
//		Inverse Conversion
//		temp = (TempMax*xs/255) - TempMin
//		tempNow = (uint8_t) ((sTempMax*tempNow_XS)/255.0 - sTempMin);
//
//	    (d12-0)/(4096-0) = (v-0)/(3.16-0)
//	    v = d12/4096*3162
//
//	    */
//
//	//	0,703089
//
//		uint32_t voltage = 0;
//
//	//	voltage = d12*DEFAULT_VREF/2047;
//		printf("Raw0: %d\n", d12);
//		voltage = 1000.0*5.0*(d12/2048.0);
//		printf("Raw0: %d\tVoltage0:%d\n", d12, voltage);
//		return voltage;
//	}