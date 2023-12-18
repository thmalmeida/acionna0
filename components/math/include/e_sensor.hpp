#ifndef E_SENSOR_HPP__
#define E_SENSOR_HPP__

#include <stdio.h>

#include "adc.hpp"
#include "dsp.hpp"

/* Electrical sensor class calculates electrical parameters based on analogic samples collected by ADC peripheral drive;
 *
 * Here we need:
 *	- ADC class and it's samples;
 *	- some DSP functions;
 *
 *
 *
 */

class E_Sensor
{
public:
	E_Sensor(ADC_driver *adc, int channel) : channel_{channel}
	{
		init();
	}
	~E_Sensor(void)
	{
		delete[] i_k;
		delete[] v_k;
	}

	void init(void)
	{

		// memory allocation for arrays
		i_k = new int[length_];
		i_t = new double[length_];

		// configure adc channel
		adc_->oneshot_channel_config(channel_, 0, 12);
		// adc_.channel_config(1, 0, 12);
	}
	double i_rms(void)
	{

		// read analog samples and convert to digital
		adc_.read(channel_, i_k, length_, sample_rate_);

		// convert to current signal based on electric parameters configured on dsp class
		dsp_.calc_iL_t(i_k, i_t, length_);

		// remove offset component
		dsp_.dc_remove(i_t, length_);

		// return rms value
		return dsp_.calc_rms(i_t, length_);
	}
	/* @brief Calculate the v_rms
	 *
	 */
	double v_rms(void)
	{
		// dsp_.
	}

	void set_channel(int channel)
	{
	}

private:
	void set_frequency_(int freq)
	{
	}
	void set_length(int len)
	{
	}

	ADC_driver adc_;
	DSP dsp_;

	double *i_t;
	int *i_k;
	int *v_k;
	int length_ = 100;
	int sample_rate_ = 1000;

	int channel_;
}

/* Sensors mapping
 * iSensor_1: GPIO36/ADC1_0
 * iSensor_2: GPIO39/ADC1_3
 * pSensor_1: GPIO32/ADC1_4
 * vSensor_R: GPIO34/ADC1_6
 * pSensor_2: GPIO35/ADC1_7
 */
#endif
