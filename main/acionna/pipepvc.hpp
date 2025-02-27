#ifndef PIPEPVC_HPP
#define PIPEPVC_HPP

#include "sensor_pressure.hpp"
#include "helper.hpp"

enum class air_detect_states {
	pressure_low_idle = 0,
	pressure_increasing,
	pressure_stable,
	pressure_slope
};

class Pipepvc {
public:
	
	int pressure_max = 56;							// max supported pressure by pipe [m.c.a];
	int pressure_min = 30;							// min threshold pressure for indicate some problem;
	int pressure_mca_fi = 0;						// pressure value after dig low pass filter

	// Variables for low pressure detection algorithm
	int air_detect_pressure_low_ref = 10;			// after stable, min threshold pressure to find air intake;
	uint32_t air_detect_timer_increase = 0;
	uint32_t air_detect_timer_increase_ref = 60;
	uint32_t air_detect_count_increase = 0;
	uint32_t air_detect_count_increase_ref = 10;
	uint32_t air_detect_timer_stable = 0;
	int pressure_mca_previous = 0;					// for low press dectection algoritm;
	int pressure_mca_avg = 0;
	air_detect_states air_detect_state = air_detect_states::pressure_low_idle;

	int n_samples = 10;

//	int PRessHold=0;					// max pressure converted on turned on period;
//	uint8_t PRessureRef = 0;			// max threshold pressure;
//	uint8_t PRessureRef_Valve = 0;		// max threshold valve pressure to turn open;
//	uint8_t PRessureMax_Sensor = 100; 	// sensor max pressure [psi]
//	uint8_t PRessurePer = 85;			// percent pressure bellow nominal to turn load off;
//	uint8_t PRessureRef_Low = 10;
//	uint16_t levelRef_10bit = 0;		// digital 10 bit number to threshold level sensor;
//	uint8_t flag_PressureUnstable = 1;
//	uint8_t flag_PressureDown = 0;		// flag for pressure down occurrence;

	Pipepvc(ADC_Driver *adc, int channel, int press_psi_factory, uint32_t* epoch_time) : sensor0(adc, channel, press_psi_factory), epoch_time_(epoch_time) {
	}
	void update(void) {
		pressure_mca_ = sensor0.pressure_mca();

		make_log();
	}
	// Return the last current pipe pressure found
	int pressure_mca(void) {
		return pressure_mca_;
	}
	// int sensor_pressure_ref;						// sensor max pressure [psi];
	int sensor_pressure_ref(void) {
		return sensor0.pressure_psi_max();
	}
	void sensor_pressure_ref(int value) {
		sensor0.pressure_psi_max(value);
	}

	void make_log(void) {
		// convert unix time to date time;
		// compare interval;
		// or
		// just make log at some interval predefined;
		// make log writing some array shifting values;
	}
	
	int air_intake_detect(states_motor state_motor, states_motor state_motor_ref, int pressure_expected) {
		switch (air_detect_state) 
		{
			case air_detect_states::pressure_low_idle: {
				if(state_motor == state_motor_ref) {
					air_detect_state = air_detect_states::pressure_increasing;
					air_detect_timer_increase = 0;
					air_detect_timer_stable = 0;
					pressure_mca_previous = pressure_mca_;
					pressure_mca_avg = pressure_mca_;
					pressure_mca_fi = pressure_mca_;
				}
				break;
			}
			case air_detect_states::pressure_increasing: {
				if(state_motor == state_motor_ref) {
					
					// digital low pass filter
					// pressure_mca_fi = beta*pressure_mca_ + pressure_mca_fi - beta*pressure_mca_fi;
					// pressure_mca_avg = 
					// Weight = beta*WeightTemp + Weight - beta*Weight;

					air_detect_timer_increase++;

					if(air_detect_timer_increase > 5) {

						// if new read pressure is bellow 70% of previous one, slope is detected.
						if(pressure_mca_ < 0.70*pressure_mca_previous) {
							air_detect_state = air_detect_states::pressure_slope;
							break;
						}

						// 
						if(!(pressure_mca_ - pressure_mca_avg))
						{
							air_detect_count_increase++;
							if(air_detect_count_increase >= air_detect_count_increase_ref) {
								air_detect_timer_stable = 0;
								air_detect_state = air_detect_states::pressure_stable;
							}
							break;
						}
					}

					if(air_detect_timer_increase >= air_detect_timer_increase_ref) {
						// never achieve stability?
					}

					pressure_mca_previous = pressure_mca_;
					
					// Refresh pressure average
					pressure_mca_avg++;					// Remove the .5 value
					pressure_mca_avg += pressure_mca_;	// add old value with newone
					pressure_mca_avg >>= 1;				// divide by 2 finding new average
				}
				else {
					air_detect_state = air_detect_states::pressure_low_idle;
				}
				break;
			}
			case air_detect_states::pressure_stable: {
				if(state_motor == state_motor_ref) {
					air_detect_timer_stable++;

					// if(pressure_mca_ < 0.75*pressure_mca_previous) {
					if((pressure_mca_ < 0.70*pressure_mca_avg) || (pressure_mca_ < air_detect_pressure_low_ref)) {

						air_detect_state = air_detect_states::pressure_slope;
						break;
					}

					pressure_mca_previous = pressure_mca_;

					// Refresh pressure average
					pressure_mca_avg++;
					pressure_mca_avg += pressure_mca_;
					pressure_mca_avg >>= 1;
				} else {
					air_detect_state = air_detect_states::pressure_low_idle;
				}
				break;
			}
			case air_detect_states::pressure_slope: {
				air_detect_state = air_detect_states::pressure_low_idle;
				return 1;
				break;
			}
			default: {
				air_detect_state = air_detect_states::pressure_low_idle;
				break;
			}
		}

		return 0;
	}
	int air_intake_detect_state(void) {
		return static_cast<int>(air_detect_state);
	}
	int broke_pipe_detect(int pump_state) {
		return 0;
	}

private:
	// ADC_Driver *adc_;
	Sensor_Pressure sensor0;

	uint32_t *epoch_time_;
	// int channel_;
	int pressure_mca_ = 0;					// converted value [m.c.a.];
};
#endif /* PIPEPVC_H__ */
