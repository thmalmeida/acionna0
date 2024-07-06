#include "adc.hpp"

static const char *TAG_ADC = "ADC";

ADC_Driver::ADC_Driver(adc_mode mode = adc_mode::oneshot) : mode_{mode} {
	init();
}
ADC_Driver::~ADC_Driver(void) {
	if(mode_ == adc_mode::oneshot) {
		oneshot_deinit_();
	} else {
		stream_deinit_();
	}
}

// Common member functions (public)
void ADC_Driver::init(void) {
	switch (mode_) {
		case adc_mode::oneshot:
			oneshot_init_();
			break;
		case adc_mode::stream:
			stream_init_();
			break;
	}
}
void ADC_Driver::channel_config(int channel, int attenuation, int bitwidth) {
		pattern_table_current_index++;
		pattern_table_[pattern_table_current_index].channel = static_cast<adc_channel_t>(channel);
		pattern_table_[pattern_table_current_index].atten = static_cast<adc_atten_t>(attenuation);
		pattern_table_[pattern_table_current_index].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
		pattern_table_[pattern_table_current_index].unit = ADC_UNIT_1;
		
		if(mode_ == adc_mode::oneshot) {
			oneshot_channel_config_(channel, attenuation, bitwidth);

		} else if(mode_ == adc_mode::stream) {
			pattern_table_current_index++;
			pattern_table_[pattern_table_current_index].channel = static_cast<adc_channel_t>(channel);
			pattern_table_[pattern_table_current_index].atten = static_cast<adc_atten_t>(attenuation);
			pattern_table_[pattern_table_current_index].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
			pattern_table_[pattern_table_current_index].unit = ADC_UNIT_1;
		}
}
void ADC_Driver::channel_config(int channel, int attenuation) {
	channel_config(channel, attenuation, 12);
}
void ADC_Driver::set_channel(int channel) {
    channel_ = static_cast<adc_channel_t>(channel);
}
int ADC_Driver::read(int channel, int num_samples) {
/*
 * @brief stream read using oneshot method
 * 
 * Copy adc raw values on specific frequency using single shot to it's pointer
 * 
 * @param channel channel desired
 * @param v pointer to receive the copy
 * @param length the length of array
 * @param frequency the signal frequency
 * 
 * @return nothing, just copy the samples to v pointer
*/
	int adc_raw = read(channel);
	int filtered = static_cast<long int>(adc_raw);

	for(int i=1; i<num_samples; i++) {
		// v[i] = 0.8*v[i-1] + 0.2*read(channel);
		adc_raw = 0.8*adc_raw + 0.2*read(channel);
		filtered += (adc_raw + 1);
		filtered >>= 1;
	}
	return filtered;
}
void ADC_Driver::read(int channel, int* v, int length, int frequency) {
	switch (mode_) {
		case adc_mode::oneshot: {
			int Ts = static_cast<int>(1.0/static_cast<double>(frequency)*1000000.0);
			for(int i=0; i<length; i++) {
				v[i] = read(channel);
				delay_us(Ts);
			}
			break;
		}
		case adc_mode::stream: {
			// stream_read_(channel, v, length);
		}
	}
}
void ADC_Driver::read(int channel, uint16_t *v, int length) {
	stream_read_(channel, v, length);
}
int ADC_Driver::read(int channel) {
/*
* @brief makes n reads and return an average 
*
* @return the average of n reads
*/	
	int data_adc_raw;
	ESP_ERROR_CHECK(adc_oneshot_read(oneshot_handle_, static_cast<adc_channel_t>(channel), &data_adc_raw));

	return data_adc_raw;
}

// Oneshot functions - ADC single mode
void ADC_Driver::oneshot_init_(void) {
	// 1 - Resource Allocation (init)
	adc_oneshot_unit_init_cfg_t init_config1 = {};
	init_config1.unit_id = unit_;
	init_config1.ulp_mode = ADC_ULP_MODE_DISABLE;
	init_config1.clk_src = ADC_RTC_CLK_SRC_DEFAULT;

	// 1.1 - install driver and ADC instance
	ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &oneshot_handle_));

	// Config channel to read
	// channel_config_oneshot();
}
void ADC_Driver::oneshot_deinit_(void) {
	ESP_ERROR_CHECK(adc_oneshot_del_unit(oneshot_handle_));
	if (do_calibration1) {
		calibration_deinit_(adc1_cali_handle_);
	}
}
void ADC_Driver::oneshot_channel_config_(int channel) {
	// attenuation to 0 dB and bit width to 12-bit;
	oneshot_channel_config_(channel, 0, 12);
}
void ADC_Driver::oneshot_channel_config_(int channel, int attenuation, int bitwidth) {
	/* This function setup:
	- channel number;
	- attenuation;
	- bit width.
	*/
	switch(attenuation) {
		case 0: {
			attenuation_ = ADC_ATTEN_DB_0;
			break;
		}
		case 1: {
			attenuation_ = ADC_ATTEN_DB_2_5;
			break;
		}
		case 2: {
			attenuation_ = ADC_ATTEN_DB_6;
			break;
		}
		case 3: {
			attenuation_ = ADC_ATTEN_DB_12;
			break;
		}
		default: {
			attenuation_ = ADC_ATTEN_DB_0;
			break;
		}
	}
	switch(bitwidth) {
		case 9: {
			width_ = ADC_BITWIDTH_9;
			break;
		}
		case 10: {
			width_ = ADC_BITWIDTH_10;
			break;
		}
		case 11: {
			width_ = ADC_BITWIDTH_11;
			break;
		}
		case 12: {
			width_ = ADC_BITWIDTH_12;			
			break;
		}
		case 13: {
			width_ = ADC_BITWIDTH_13;
			break;
		}
		default: {
			width_ = ADC_BITWIDTH_12;
			break;
		}
	}
	// 2 - Unit configuration of ADC1
	adc_oneshot_chan_cfg_t config = {
		.atten = attenuation_,
		.bitwidth = width_,
	};
	ESP_ERROR_CHECK(adc_oneshot_config_channel(oneshot_handle_, static_cast<adc_channel_t>(channel), &config));
}

// Stream functions - ADC DMA Continuous mode
void ADC_Driver::stream_init_(void) {

	// ----- Resource Allocation -----

	/*	_______________________________ One conversion Frame ____________________________________
		| Conversion Result |  Conversion Result |  Conversion Result |	... | Conversion Result  |
	*/

	// Driver initialize on ADC Continuous Mode - set the conversion frame size that contains conversion results
	adc_continuous_handle_cfg_t adc_config;
	adc_config.max_store_buf_size = POINTS_PER_CYCLE*N_CYCLES*SOC_ADC_DIGI_DATA_BYTES_PER_CONV;
	adc_config.conv_frame_size = POINTS_PER_CYCLE*N_CYCLES*SOC_ADC_DIGI_RESULT_BYTES;	//SOC_ADC_DIGI_DATA_BYTES_PER_CONV;
	ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &stream_handle_));

	// stream_config();
}
void ADC_Driver::stream_callback_config_(void) {
	// callback config
	// stream_callback.on_conv_done = 
	// ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(stream_handle_))
}
void ADC_Driver::stream_config_(int channel, int attenuation) {
	int channels_list[1];
	int attenuations_list[1];
	channels_list[0] = channel;
	attenuations_list[0] = attenuation;
	stream_config_(&channels_list[0], &attenuations_list[0], 1);
}
void ADC_Driver::stream_config_(int* channels_list, int* attenuations_list, int n_channels) {
	
	// Configurations of ADC - fill the pattern table
	adc_digi_pattern_config_t pattern_table[n_channels];
	for(int i=0; i<n_channels; i++) {
		pattern_table[i].channel = static_cast<adc_channel_t>(channels_list[i]);	// ADC_CHANNEL_0	
		pattern_table[i].atten = static_cast<adc_atten_t>(attenuations_list[i]);	// ADC_ATTEN_DB_11
		pattern_table[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;
		pattern_table[i].unit = ADC_UNIT_1;

		ESP_LOGI(TAG_ADC, "pattern_table[%d].channel is :%x", i, pattern_table[i].channel);
		ESP_LOGI(TAG_ADC, "pattern_table[%d].atten is :%x", i, pattern_table[i].atten);
		ESP_LOGI(TAG_ADC, "pattern_table[%d].bit_width is :%u", i, pattern_table[i].unit);
		ESP_LOGI(TAG_ADC, "pattern_table[%d].unit is :%x", i, pattern_table[i].unit);
	}

	adc_continuous_config_t stream_dig_config;
	stream_dig_config.pattern_num = n_channels;					// number of ADC channels;
	stream_dig_config.adc_pattern = &pattern_table[0];			// list of configs for each ADC channel
	stream_dig_config.sample_freq_hz = 25600;					// Sampling frequency [Samples/s] 20 kS/s to 2 MS/s.
	stream_dig_config.conv_mode = ADC_CONV_SINGLE_UNIT_1;		// ADC digital controller (DMA mode) working mode. Only ADC1 for conversion
	stream_dig_config.format = ADC_DIGI_OUTPUT_FORMAT_TYPE1;	// output data format?

	ESP_ERROR_CHECK(adc_continuous_config(stream_handle_, &stream_dig_config));
}
void ADC_Driver::stream_start_(void) {
	adc_continuous_start(stream_handle_);
	stream_state_ = adc_stream_states::running;
}
void ADC_Driver::stream_stop_(void) {
	ESP_ERROR_CHECK(adc_continuous_stop(stream_handle_));
	stream_state_ = adc_stream_states::stopped;
}
void ADC_Driver::stream_read_(int channel, uint16_t* buffer, int length) {

	if(stream_state_ == adc_stream_states::stopped) {
		stream_start_();
	}

	uint32_t length_out = 0;
	int i = 0, j = 0;

	// conversion data raw has 12 bits using 16 bit value. that uses 4 bits to channel identify;
	uint32_t length_exp = static_cast<uint32_t>(2*length);

	// result should have the double size of buffer of adc data raw
	uint8_t result[length_exp];
	// memset(result, 0xcc, READ_LENGTH);

	adc_continuous_read(stream_handle_, result, length_exp, &length_out, 0);
	
	// SOC_ADC_DIGI_DATA_BYTES_PER_CONV = 4 and SOC_ADC_DIGI_RESULT_BYTES = 2
	for(i=0; i<length_out; i += SOC_ADC_DIGI_RESULT_BYTES) {
		// adc_digi_output_data_t *p = reinterpret_cast<adc_digi_output_data_t*>(&result[i]);
		adc_digi_output_data_t *p = (adc_digi_output_data_t*)&result[i];
	
		// printf("channel: %d, data:%d\n", p->type1.channel, p->type1.data);
		buffer[i/2] = static_cast<uint16_t>(p->type1.data);
		// printf("%u, ", buffer[i/2]);
		j++;
		// if(data_raw < 4096) {
		// 	if(data_raw > 10) {
		// 		buffer[i] = data_raw;
		// 		j++;
		// 		printf("%u ", data_raw);
		// 	}
		// }
	}
	// ESP_LOGI(TAG_ADC, "RAM free:%lu, min:%lu", esp_get_free_internal_heap_size(), esp_get_minimum_free_heap_size());
	// ESP_LOGI(TAG_ADC, "len_exp: %lu, len_out:%lu, i:%d, j:%d", length_exp, length_out, i, j);
}
void ADC_Driver::stream_deinit_(void) {
	// Recycle the ADC Unit
	adc_continuous_deinit(stream_handle_);
}

// Calibration functions
void ADC_Driver::calibrate(void) {

	// if (do_calibration1) {
	// 	ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle_, adc_raw[0][0], &voltage[0][0]));
	// 	ESP_LOGI(TAG_ADC, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC1_CHAN0, voltage[0][0]);

	// 	ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle_, adc_raw[0][1], &voltage[0][1]));
	// 	ESP_LOGI(TAG_ADC, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC1_CHAN1, voltage[0][1]);
	// }
}
bool ADC_Driver::calibration_init_(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
	adc_cali_handle_t handle = NULL;
	esp_err_t ret = ESP_FAIL;
	bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG_ADC, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

// #if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG_ADC, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
            .default_vref = 1110,
            // adc_cali_line_fitting_config_t::default_vref
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
// #endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG_ADC, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG_ADC, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG_ADC, "Invalid arg or no memory");
    }

    return calibrated;
}
void ADC_Driver::calibration_deinit_(adc_cali_handle_t handle) {
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG_ADC, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG_ADC, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}

// Callback implementation
// static TaskHandle_t s_task_handle;
// static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
// {
// 	BaseType_t mustYield = pdFALSE;
// 	//Notify that ADC continuous driver has done enough number of conversions
// 	vTaskNotifyGiveFromISR(s_task_handle, &mustYield);
// 	return (mustYield == pdTRUE);
// }

// ADC_Driver::ADC_Driver(int channel) : channel_{static_cast<adc_channel_t>(channel)} {
// 	init_driver_oneshot();
// }
