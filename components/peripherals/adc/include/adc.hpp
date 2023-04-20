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
#include "freertos/semphr.h"

#include "sdkconfig.h"

#define ADC1_CHAN0			ADC_CHANNEL_4
#define ADC1_CHAN1			ADC_CHANNEL_7

// Continuous macros
#define READ_LEN   256


// #define DEFAULT_VREF    1110        //Use adc2_vref_to_gpio() to obtain a better estimate
// #define NO_OF_SAMPLES   16          //Multisampling

// #define TIMES              256
// #define GET_UNIT(x)        ((x>>3) & 0x1)

// #define ADC_RESULT_BYTE     2
// #define ADC_CONV_LIMIT_EN   1                       //For ESP32, this should always be set to 1
// #define ADC_CONV_MODE       ADC_CONV_SINGLE_UNIT_1  //ESP32 only supports ADC1 DMA mode
// #define ADC_OUTPUT_TYPE     ADC_DIGI_OUTPUT_FORMAT_TYPE1

//static uint16_t adc1_chan_mask = BIT(7);
//static uint16_t adc2_chan_mask = 0;
//static adc_channel_t channels[1] = {ADC_CHANNEL_4};
//static uint16_t adc1_chan_mask = BIT(7);
//static uint16_t adc2_chan_mask = 0;
//static adc_channel_t channels[1] = {static_cast<adc_channel_t>(ADC1_CHANNEL_4)};

/* Sensors mapping
 * iSensor_1: GPIO36/ADC1_0
 * iSensor_2: GPIO39/ADC1_3
 * pSensor_1: GPIO32/ADC1_4
 * vSensor_R: GPIO34/ADC1_6
 * pSensor_2: GPIO35/ADC1_7
 */

#define ADC_CHANNELS_NUMBER 1
#define READ_LENGTH 256
static const char *TAG_ADC = "ADC";

class ADC_driver{
public:
	// single read
	// esp_adc_cal_characteristics_t *adc_chars;	// outside the class use static

	// dma read
	//	static const uint8_t n_channels = 1;					// number of channels to scanning on DMA conversion
	//	static const uint32_t n_points = 256;

	int adc_raw[2][10];
	int voltage[2][10];

	bool do_calibration1 = false;
	adc_digi_pattern_config_t a;

	ADC_driver(void);
	~ADC_driver(void);

	// ----- Single mode setup -----
	void init_driver_oneshot(void);
	void deinit_driver_oneshot(void);
	void channel_config_oneshot(int channel, int attenuation, int bitwidth);
	void set_channel(int channel);
	int read(int channel);


	// ----- Continuous mode setup -----
	// Parameters initizalization
	// const int channels_number = 1;
	adc_channel_t channels_list[ADC_CHANNELS_NUMBER];	// channels list		
	adc_continuous_handle_t stream_handle = NULL;	// ADC handle
	// adc_continuous_evt_cbs_t stream_callback;		// Callback structure
	// const int read_length = 256;
	uint8_t result[READ_LENGTH] = {0};

	void stream_init(void) {

		memset(result, 0xcc, READ_LENGTH);

		stream_adc_config();
	}
	void stream_callback_config(void) {
		// callback config
		// stream_callback.on_conv_done = 
		// ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(stream_handle))
	}
	void stream_adc_config(void) {
		// ----- Resource Allocation -----

		// Driver initialize on ADC Continuous Mode
		adc_continuous_handle_cfg_t stream_config;
		stream_config.conv_frame_size = 1024;
		stream_config.conv_frame_size = 100;
		ESP_ERROR_CHECK(adc_continuous_new_handle(&stream_config, &stream_handle));

		// Configurations of ADC
		adc_digi_pattern_config_t pattern_table[ADC_CHANNELS_NUMBER];

		pattern_table[0].atten = ADC_ATTEN_DB_0;
		pattern_table[0].channel = ADC_CHANNEL_0;
		pattern_table[0].unit = ADC_UNIT_1;
		pattern_table[0].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

		int i = 0;
		ESP_LOGI(TAG_ADC, "pattern_table[%d].atten is :%x", i, pattern_table[i].atten);
		ESP_LOGI(TAG_ADC, "pattern_table[%d].channel is :%x", i, pattern_table[i].channel);
		ESP_LOGI(TAG_ADC, "pattern_table[%d].unit is :%x", i, pattern_table[i].unit);
		ESP_LOGI(TAG_ADC, "pattern_table[%d].bit_width is :%u", i, pattern_table[i].unit);

		adc_continuous_config_t stream_dig_config;
		stream_dig_config.pattern_num = 1;							// number of ADC channel;
		stream_dig_config.adc_pattern = &pattern_table[0];			// list of configs for each ADC channel
		stream_dig_config.sample_freq_hz = 1000;					// Sampling frequency [Samples/s]
		stream_dig_config.conv_mode = ADC_CONV_SINGLE_UNIT_1;		// ADC digital controller (DMA mode) working mode. Only ADC1 for conversion
		stream_dig_config.format = ADC_DIGI_OUTPUT_FORMAT_TYPE1;	// output data format?

		ESP_ERROR_CHECK(adc_continuous_config(stream_handle, &stream_dig_config));
	}
	void stream_start(void) {
		adc_continuous_start(stream_handle);
	}
	void stream_stop(void) {
		ESP_ERROR_CHECK(adc_continuous_stop(stream_handle));
	}
	void stream_read(int channel, uint16_t* buffer, int length) {

		uint32_t read_len_out = 0;
		adc_continuous_read(stream_handle, result, READ_LENGTH, &read_len_out, 0);
		for(int i=0; i<read_len_out; i += SOC_ADC_DIGI_RESULT_BYTES) {
			adc_digi_output_data_t *p = (adc_digi_output_data_t*)&result[i];
			buffer[i] = p->type1.data;
		}
	}
	// Recycle the ADC Unit
	void stream_deinit(void) {
		adc_continuous_deinit(stream_handle);
	}

	bool adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle);
	void adc_calibration_deinit(adc_cali_handle_t handle);
	void calibrate(void);

private:
	// one shot configuration
	adc_oneshot_unit_handle_t adc1_handle_;
    adc_cali_handle_t adc1_cali_handle_ = NULL;

	// used for single read
	adc_channel_t channel_;						// ADC channel
	adc_bitwidth_t width_ = ADC_BITWIDTH_12;	// bits for resolution conversion
	adc_atten_t attenuation_ = ADC_ATTEN_DB_0;	// attenuation for the channel
	adc_unit_t unit_ = ADC_UNIT_1;				// unit conversion

	// continuous DMA read
	// static adc_channel_t channel[1] = {ADC_CHANNEL_4};
	// adc_channel_t channels[5] = {ADC_CHANNEL_0, ADC_CHANNEL_3, ADC_CHANNEL_4, ADC_CHANNEL_6, ADC_CHANNEL_7};
	// uint8_t n_channels = sizeof(channels) / sizeof(adc_channel_t);
	// uint32_t n_points = 120;
	// uint16_t adc1_chan_mask = (BIT(0) | BIT(3) | BIT(4) | BIT(6) | BIT(7));
//		uint16_t *channel_0;
//		uint16_t *channel_3;
//		uint16_t *channel_4;
//		uint16_t *channel_6;
//		uint16_t *channel_7;

//		// used for fixed sample rate read to DMA
//		uint8_t list_channels_[n_channels] = {4};
//
};

#endif /* ADC_HPP__ */
