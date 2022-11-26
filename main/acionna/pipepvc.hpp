#ifndef PIPEPVC_HPP
#define PIPEPVC_HPP

#include <adc.hpp>
#include <gpio.hpp>
#include <pinout.hpp>
#include <cmath>

class Pipepvc {
public:
	
	int pressure_max = 70;				// max supported pressure by pipe [m.c.a];
	int pressure_min = 30;				// min threshold pressure for indicate some problem;
	int sensor_pressure_ref = 150;		// sensor max pressure [psi];
	uint16_t sensor_data_dig = 0;		// readed value from ADC peripheral;
	int channel_adc = 4;				// ADC channel;
	
	int pressure_mca_previous = 0;		// for low press dectection algoritm;

//	int PRessHold=0;					// max pressure converted on turned on period;
//	uint8_t PRessureRef = 0;			// max threshold pressure;
//	uint8_t PRessureRef_Valve = 0;		// max threshold valve pressure to turn open;
//	uint8_t PRessureMax_Sensor = 100; 	// sensor max pressure [psi]
//	uint8_t PRessurePer = 85;			// percent pressure bellow nominal to turn load off;
//	uint8_t PRessureRef_Low = 10;
//	uint16_t levelRef_10bit = 0;		// digital 10 bit number to threshold level sensor;
//
//	uint8_t flag_PressureUnstable = 1;
//	uint8_t flag_PressureDown = 0;		// flag for pressure down occurrence;

	Pipepvc() : adc_{4} {}
	void update()
	{
		sensor_data_dig = adc_.read();
		convert_pressure(sensor_data_dig);
	}
	int pressure_mca()
	{
		return pressure_mca_;
	}
	int air_intake_detect(int pump_state)
	{
		if(pressure_mca_ > pressure_mca_previous)
		{
			// pressure_current = 
		}
		// if(pump_state)
		// {
			
		// }
		return 0;
	}
	int broke_pipe_detect(int pump_state)
	{
		return 0;
	}

private:
	ADC_Basic adc_;
	int pressure_mca_ = 0;					// converted value [m.c.a.];
	int pressure_psi_ = 0;					// converted value [psi];

	// Sensor functions
	/*
	  4.5 V___	    922___	1.2 MPa___	 12 Bar___	 120 m.c.a.___		  4096 ___       3.16 V___
			|			|			|			|				|				|				|
			|			|			|			|				|				|				|
			|			|			|			|				|				|				|
		 Vo_|		Do__|		Po__|			|			Pa__|		   d12__|			 v__|
			|			|			|			|				|				|				|
			|			|			|			|				|				|				|
			|			|			|			|				|				|				|
			|	_	   _|_		   _|_		   _|_			   _|_			   _|_			   _|_
		0.5 V		103			0 MPa		0 Bar		0 m.c.a.			0				0 V

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
	void convert_pressure(uint16_t data_12bits)
	{
		const float K_psi_mca = 0.703089;
		float sensor_press_max = 150.0;
		const float d_max = 3878.0;	// 4500 mV
		const float d_min = 124.0; 	// 502 mV
		pressure_psi_ = static_cast<int>(sensor_press_max*(data_12bits - d_min)/(d_max-d_min));
		pressure_mca_ = static_cast<int> (pressure_psi_*K_psi_mca);
	}

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
};
#endif /* PUMP_H__ */
