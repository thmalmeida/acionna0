#ifndef ADC2_HPP__
#define ADC2_HPP__

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// Includes for one shot read
#include <stdlib.h>
#include "soc/soc_caps.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

// Includes for continuous mode
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

	void init_driver_oneshot(void);
	void deinit_driver_oneshot(void);
	void channel_config_oneshot(int channel, int attenuation, int bitwidth);
	void set_channel(int channel);
	int read(int channel);

	// void init_continuous(void);
	// void read_stream(uint8_t *data, int len);

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
