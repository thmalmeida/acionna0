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
// #include "esp32/rom/ets_sys.h"      // include for ets_delay_us()
#include "delay.hpp"					// include for delay us

// Continuous macros
#define POINTS_PER_CYCLE	350
#define N_CYCLES			2
#define READ_LENGTH 		POINTS_PER_CYCLE*N_CYCLES*SOC_ADC_DIGI_RESULT_BYTES			// 2 bytes of conversion
// #define READ_LENGTH 		POINTS_PER_CYCLE*N_CYCLES*SOC_ADC_DIGI_DATA_BYTES_PER_CONV	// 4 bytes of conversion
#define MAX_NUM_CHANNELS	6		// Max ADC1 channels available on ESP32 with std_board

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

	// ----- For any mode ------
	void channel_config(int channel, int attenuation, int bitwidth) {
		pattern_table_current_index++;
		pattern_table_[pattern_table_current_index].channel = static_cast<adc_channel_t>(channel);
		pattern_table_[pattern_table_current_index].atten = static_cast<adc_atten_t>(attenuation);
		pattern_table_[pattern_table_current_index].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
		pattern_table_[pattern_table_current_index].unit = ADC_UNIT_1;
		
		if(mode_ == adc_mode::oneshot) {
			oneshot_channel_config(channel, attenuation, bitwidth);

		} else if(mode_ == adc_mode::stream) {
			pattern_table_current_index++;
			pattern_table_[pattern_table_current_index].channel = static_cast<adc_channel_t>(channel);
			pattern_table_[pattern_table_current_index].atten = static_cast<adc_atten_t>(attenuation);
			pattern_table_[pattern_table_current_index].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
			pattern_table_[pattern_table_current_index].unit = ADC_UNIT_1;
		}
	}

	// ----- Single mode setup -----
	void oneshot_init(void);
	void oneshot_deinit(void);
	void oneshot_channel_config(int channel);
	void oneshot_channel_config(int channel, int attenuation, int bitwidth);
	void set_channel(int channel);
	int read(int channel);
	int read(int channel, int length);
	void read(int channel, int* v, int length, int frequency);

	// ----- Continuous mode setup -----
	// Parameters initizalization
	// adc_channel_t channels_list[ADC_CHANNELS_NUMBER];	// channels list		
	// adc_continuous_evt_cbs_t stream_callback;		// Callback structure

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

	// void delay_us_(uint32_t microseconds)
	// {
	// 	ets_delay_us(microseconds);
	// }
	
	// adc parameters
	adc_mode mode_ = adc_mode::noption;
	adc_digi_pattern_config_t pattern_table_[MAX_NUM_CHANNELS];
	int pattern_table_current_index = -1;

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
