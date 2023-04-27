#ifndef __ADC_HPP__
#define __ADC_HPP__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// Includes for one shot read
#include "soc/soc_caps.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

// Includes for adc continuous mode
#include "esp_adc/adc_continuous.h"

// #include "arch/sys_arch.h"	        // include for delays
#include "esp32/rom/ets_sys.h"      // include for ets_delay_us()

// Continuous macros
#define POINTS_PER_CYCLE	350
#define N_CYCLES			2
#define READ_LENGTH 		POINTS_PER_CYCLE*N_CYCLES*SOC_ADC_DIGI_RESULT_BYTES			// 2 bytes of conversion
// #define READ_LENGTH 		POINTS_PER_CYCLE*N_CYCLES*SOC_ADC_DIGI_DATA_BYTES_PER_CONV	// 4 bytes of conversion

enum class adc_mode {
	noption = 0,			// Do not initialize
	oneshot,
	stream
};

enum class adc_stream_states {
	stopped = 0,
	running
};

class ADC_driver{
public:
	bool do_calibration1 = false;

	ADC_driver(adc_mode mode);
	~ADC_driver(void);

	// ----- Single mode setup -----
	void oneshot_init(void);
	void deinit_driver_oneshot(void);
	void channel_config_oneshot(int channel, int attenuation, int bitwidth);
	void set_channel(int channel);
	int read(int channel);
	int read(int channel, int n_samples) {
		int adc_raw = read(channel);
		int filtered = static_cast<long int>(adc_raw);

		for(int i=1; i<n_samples; i++) {
			// v[i] = 0.8*v[i-1] + 0.2*read(channel);
			adc_raw = 0.8*adc_raw + 0.2*read(channel);
			filtered += (adc_raw + 1);
			filtered >>= 1;
		}
		return filtered;
	}
	void read(int channel, int* v, int length, int frequency) {
		int Ts = static_cast<int>(1.0/static_cast<double>(frequency)*1000000.0);
		for(int i=0; i<length; i++) {
			v[i] = read(channel);
			delay_us_(Ts);	
		}
	}

	// ----- Continuous mode setup -----
	// Parameters initizalization
	// adc_channel_t channels_list[ADC_CHANNELS_NUMBER];	// channels list		
	// adc_continuous_evt_cbs_t stream_callback;		// Callback structure
	uint8_t result[READ_LENGTH] = {0};

	void stream_init(void);
	void stream_callback_config(void); 
	void stream_config(int channel, int attenuation);
	void stream_config(int* channels_list, int* attenuations_list, int n_channels);
	void stream_start(void);
	void stream_stop(void);
	void stream_read(int channel, uint16_t* buffer, int length);
	void stream_deinit(void);

	bool adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle);
	void adc_calibration_deinit(adc_cali_handle_t handle);
	void calibrate(void);

private:

	void delay_us_(uint32_t microseconds)
	{
		ets_delay_us(microseconds);
	}
	// adc parameters
	adc_mode mode_ = adc_mode::noption;

	// one shot configuration
	adc_oneshot_unit_handle_t oneshot_handle_;
    adc_cali_handle_t adc1_cali_handle_ = NULL;
	adc_channel_t channel_;											// ADC channel
	adc_bitwidth_t width_ = ADC_BITWIDTH_12;						// bits for resolution conversion
	adc_atten_t attenuation_ = ADC_ATTEN_DB_0;						// attenuation for the channel
	adc_unit_t unit_ = ADC_UNIT_1;									// unit conversion

	// continuous DMA read
	adc_continuous_handle_t stream_handle_ = NULL;					// ADC handle
	adc_stream_states stream_state_ = adc_stream_states::stopped;
};

#endif /* ADC_HPP__ */
