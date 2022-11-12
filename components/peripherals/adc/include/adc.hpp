#ifndef ADC_HPP__
#define ADC_HPP__

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"

#define DEFAULT_VREF    1110        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   16          //Multisampling


#define TIMES              256
#define GET_UNIT(x)        ((x>>3) & 0x1)

#define ADC_RESULT_BYTE     2
#define ADC_CONV_LIMIT_EN   1                       //For ESP32, this should always be set to 1
#define ADC_CONV_MODE       ADC_CONV_SINGLE_UNIT_1  //ESP32 only supports ADC1 DMA mode
#define ADC_OUTPUT_TYPE     ADC_DIGI_OUTPUT_FORMAT_TYPE1

//static uint16_t adc1_chan_mask = BIT(7);
//static uint16_t adc2_chan_mask = 0;
static const char *TAG_ADC = "ADC DMA";
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

class ADC_Basic{
	public:

		// single read
		// esp_adc_cal_characteristics_t *adc_chars;	// outside the class use static

		// dma read
// 		static const uint8_t n_channels = 1;					// number of channels to scanning on DMA conversion
//		static const uint32_t n_points = 256;

		ADC_Basic(int channel) : channel_{static_cast<adc_channel_t>(channel)}
		{
			init_single_read();				// Single read initialize
		}
		ADC_Basic(int channel, int resolution, int attenuation) : channel_{static_cast<adc_channel_t>(channel)}, width_{static_cast<adc_bits_width_t>(resolution)}, attenuation_{static_cast<adc_atten_t>(attenuation)}
		{
			init_single_read();				// Single read initialize
//			init_continuous_read_dma(_channel, channel_num)		// Continuous read initialize
		}
		// ~ADC_Basic();

		void init_single_read()
		{
			// Check if Two Point or Vref are burned into eFuse
			check_efuse_();

			//Configure ADC
			if (unit_ == ADC_UNIT_1)
			{
				adc1_config_width(width_);
				adc1_config_channel_atten((adc1_channel_t)channel_, attenuation_);
			}
			else
			{
				adc2_config_channel_atten((adc2_channel_t)channel_, attenuation_);
			}

			//Characterize ADC
			// adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
//			esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
		}
//		void continuous_adc_init(uint16_t adc1_chan_mask, uint16_t adc2_chan_mask, adc_channel_t *channel, uint8_t channel_num)
		void init_continuous_read_dma(adc_channel_t *_channel, uint8_t channel_num)
		{
			// ADC DMA driver configuration
			adc_digi_init_config_t adc_dma_config;

			adc_dma_config.max_store_buf_size = 1024;					// Max length of the converted data that driver can store before they are processed.
			adc_dma_config.conv_num_each_intr = n_channels*n_points;	// Amount of bytes of data that can be converted in 1 interrupt.
			adc_dma_config.adc1_chan_mask = adc1_chan_mask;				// Channel list of ADC1 to be initialized.
			adc_dma_config.adc2_chan_mask = 0;

			// Step 1 - ADC driver initialize (also SENS_SAR1_DIG_FORCE = 1?)
			ESP_ERROR_CHECK(adc_digi_initialize(&adc_dma_config));

			// ADC digital controller configuration
			adc_digi_configuration_t adc_digi_ctrl_config;
			adc_digi_ctrl_config.conv_limit_en = ADC_CONV_LIMIT_EN;	//For ESP32, this should always be set to 1
			adc_digi_ctrl_config.conv_limit_num = 4;				// Set the upper limit of the number of ADC conversion triggers. Range: 1 ~ 255.
			adc_digi_ctrl_config.conv_mode = ADC_CONV_MODE; 		//ADC_CONV_SINGLE_UNIT_1; 	// ADC DMA conversion mode, see `adc_digi_convert_mode_t`.
			adc_digi_ctrl_config.format = ADC_OUTPUT_TYPE; 			//ADC_DIGI_OUTPUT_FORMAT_TYPE1;	// ADC DMA conversion output format, see `adc_digi_output_format_t
			adc_digi_ctrl_config.sample_freq_hz = 2040;				// SOC_ADC_SAMPLE_FREQ_THRES_LOW;
			/* The expected ADC sampling frequency in Hz. Range: 611Hz ~ 83333Hz
					Fs = Fd / interval / 2
					Fs: sampling frequency;
					Fd: digital controller frequency,
						no larger than 5M for better performance
						interval: interval between 2 measurement trigger signal,
						the smallest interval should not be smaller than the 						ADC measurement period, the largest interval should not
						be larger than 4095
 	 	 	 */

			// ADC digital controller pattern configuration
		    adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX];// = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 ,0 ,0 ,0 ,0 ,0};
		    adc_digi_ctrl_config.pattern_num = channel_num;		// List of configs for each ADC channel that will be used
		    for (int i = 0; i < channel_num; i++)
		    {
		        uint8_t unit = GET_UNIT(channels[i]);
		        uint8_t ch = channels[i] & 0x07;
		        adc_pattern[i].atten = static_cast<uint8_t>(ADC_ATTEN_DB_2_5);
		        adc_pattern[i].channel = ch;
		        adc_pattern[i].unit = unit;
		        adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

		        ESP_LOGI(TAG_ADC, "adc_pattern[%d].atten is :%x", i, adc_pattern[i].atten);
		        ESP_LOGI(TAG_ADC, "adc_pattern[%d].channel is :%x", i, adc_pattern[i].channel);
		        ESP_LOGI(TAG_ADC, "adc_pattern[%d].unit is :%x", i, adc_pattern[i].unit);
		    }
		    adc_digi_ctrl_config.adc_pattern = adc_pattern;

			// Step 2 - ADC controller initialize
			ESP_ERROR_CHECK(adc_digi_controller_configure(&adc_digi_ctrl_config));

			// Step 3 - Start the ADC continuous reading
			adc_digi_start();
		}
		void continuous_stop()
		{
		    adc_digi_stop();
		    adc_digi_deinitialize();
//		    ret_ = adc_digi_deinitialize();
//		    assert(ret_ == ESP_OK);
		}
		void continuous_read()
		{
			esp_err_t ret;
			uint32_t ret_num = 0;
			uint8_t result[n_channels*n_points] = {0};
			memset(result, 0xcc, n_channels*n_points);

//			init_continuous_read_dma(_channel, channel_num);

			while(1)
			{
				ret = adc_digi_read_bytes(result, n_channels*n_points, &ret_num, ADC_MAX_DELAY);
				if (ret == ESP_OK || ret == ESP_ERR_INVALID_STATE)
				{
					if (ret == ESP_ERR_INVALID_STATE)
					{
						/**
						* @note 1
						* Issue:
						* As an example, we simply print the result out, which is super slow. Therefore the conversion is too
						* fast for the task to handle. In this condition, some conversion results lost.
						*
						* Reason:
						* When this error occurs, you will usually see the task watchdog timeout issue also.
						* Because the conversion is too fast, whereas the task calling `adc_digi_read_bytes` is slow.
						* So `adc_digi_read_bytes` will hardly block. Therefore Idle Task hardly has chance to run. In this
						* example, we add a `vTaskDelay(1)` below, to prevent the task watchdog timeout.
						*
						* Solution:
						* Either decrease the conversion speed, or increase the frequency you call `adc_digi_read_bytes`
						*/
					}

//					uint16_t data0[n_channels][n_points];
//
//					int count[n_channels];
//					for(int j=0; j<n_channels; j++)
//					{
//						count[j] = 0;
//					}

					ESP_LOGI("TASK:", "ret is %x, ret_num is %d", ret, ret_num);
//					adc_digi_output_data_t *p = (adc_digi_output_data_t*) malloc(ret_num*sizeof(adc_digi_output_data_t));
					for (int i = 0; i < ret_num; i += ADC_RESULT_BYTE)
					{
//						adc_digi_output_data_t *p = (void*)&result[i];
						adc_digi_output_data_t *p = reinterpret_cast<adc_digi_output_data_t*>(&result[i]);

//						switch (p->type1.channel)
//						{
//							case 0:
//								count[0]++;
//		                		break;
//
//							case 3:
//								break;
//
//							case 4:
//								break;
//
//							case 6:
//
//							case 7:
//								break;
//
//							default:
//								break;
//						}

						ESP_LOGI(TAG_ADC, "Unit: %d, Channel: %d, Value: %d", 1, p->type1.channel, p->type1.data);
					}
					//See `note 1`
					vTaskDelay(100);
				} else if (ret == ESP_ERR_TIMEOUT)
				{
					/**
					* ``ESP_ERR_TIMEOUT``: If ADC conversion is not finished until Timeout, you'll get this return error.
					* Here we set Timeout ``portMAX_DELAY``, so you'll never reach this branch.
					*/
					ESP_LOGW(TAG_ADC, "No data, increase timeout or reduce conv_num_each_intr");
					vTaskDelay(1000);
				}
			}
			adc_digi_stop();
			ret = adc_digi_deinitialize();
			assert(ret == ESP_OK);
		}
//		uint32_t voltage_converter(uint32_t d12);
		uint16_t read()
		{
			uint16_t adc_reading = 0;

			for (int i = 0; i < NO_OF_SAMPLES; i++)		//Multisampling
			{
				if (unit_ == ADC_UNIT_1)					// Sample ADC1
				{
					adc_reading += adc1_get_raw((adc1_channel_t)channel_);
				}
				else
				{
					int raw;
					adc2_get_raw((adc2_channel_t)channel_, width_, &raw);
					adc_reading += raw;
				}
			}
			adc_reading /= NO_OF_SAMPLES;

			return adc_reading;
		}

	private:
		// used for single read
		adc_channel_t channel_;						// ADC channel
		adc_bits_width_t width_ = ADC_WIDTH_BIT_12;	// bits for resolution conversion
		adc_atten_t attenuation_ = ADC_ATTEN_DB_0;	// attenuation for the channel
		adc_unit_t unit_ = ADC_UNIT_1;				// unit conversion

		// continuous DMA read
		adc_channel_t channels[5] = {ADC_CHANNEL_0, ADC_CHANNEL_3, ADC_CHANNEL_4, ADC_CHANNEL_6, ADC_CHANNEL_7};
		uint8_t n_channels = sizeof(channels) / sizeof(adc_channel_t);
		uint32_t n_points = 120;
		uint16_t adc1_chan_mask = (BIT(0) | BIT(3) | BIT(4) | BIT(6) | BIT(7));
//		uint16_t *channel_0;
//		uint16_t *channel_3;
//		uint16_t *channel_4;
//		uint16_t *channel_6;
//		uint16_t *channel_7;

//		// used for fixed sample rate read to DMA
//		uint8_t list_channels_[n_channels] = {4};
//
		void check_efuse_(void)
		{
		    //Check if TP is burned into eFuse
		    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
		        printf("eFuse Two Point: Supported\n");
		    } else {
		        printf("eFuse Two Point: NOT supported\n");
		    }
		    //Check Vref is burned into eFuse
		    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
		        printf("eFuse Vref: Supported\n");
		    } else {
		        printf("eFuse Vref: NOT supported\n");
		    }
		}
};

#endif /* ADC_HPP__ */



//		void read_stream(uint32_t* buffer, std::size_t buffer_len) const
//		void read_stream()
//		{
//			esp_err_t ret;
//			uint32_t ret_num = 0;			// Real length of data read from the ADC via this API.
//			uint8_t result[n_points];
////			memset(result, 0xcc, n_points);
//
//			/**
//			 * @brief Read bytes from Digital ADC through DMA.
//			 *
//			 * @param[out] buf                 Buffer to read from ADC.
//			 * @param[in]  length_max          Expected length of data read from the ADC.
//			 * @param[out] out_length          Real length of data read from the ADC via this API.
//			 * @param[in]  timeout_ms          Time to wait for data via this API, in millisecond.
//			 *
//			 * @return
//			 *         - ESP_ERR_INVALID_STATE Driver state is invalid. Usually it means the ADC sampling rate is faster than the task processing rate.
//			 *         - ESP_ERR_TIMEOUT       Operation timed out
//			 *         - ESP_OK                On success
//			 */
//			ret = adc_digi_read_bytes(result, n_points, &ret_num, ADC_MAX_DELAY);
//
//
////			adc_digi_output_data_t *p;// = (adc_digi_output_data_t*) malloc(ret_num*sizeof(adc_digi_output_data_t));
//
//			if(ret == ESP_OK)
//			{
//				for (int i = 0; i < ret_num; i += ADC_RESULT_BYTE)
//				{
////					adc_digi_output_data_t *p = reinterpret_cast<void*>(&result[i]);
//					adc_digi_output_data_t *p = reinterpret_cast<adc_digi_output_data_t*>(&result[i]);
//					ESP_LOGI("Tag", "Unit: %d, Channel: %d, Value: %x", 1, p->type1.channel, p->type1.data);
//					//*p = (void*)&result[i];
//					//p->type1[i] = static_cast<adc_digi_output_data_t>(uint16_t(result[i] << 8) | result[i+1]);
//
//				}
//
////				p = static_cast<adc_digi_output_data_t>(result);
////				ESP_LOGI("TASK:", "ret is %x, ret_num is %d", ret, ret_num);
////				for (int i = 0; i < ret_num; i+=2)
////				{
////	                p = &result[i];
////	                printf(p)
////					*(p+i) = (void*)&result[i];
////					ESP_LOGI("Report", "Unit: %d, Channel: %d, Value: %x, another: %x", 1, (p+i)->type1.channel, (p+i)->type1.data, (p+i)->val);
////				}
//				//See `note 1`
//				vTaskDelay(1);
//			}
//			else if (ret == ESP_ERR_INVALID_STATE)
//			{
//				/**
//				* @note 1
//				* Issue:
//				* As an example, we simply print the result out, which is super slow. Therefore the conversion is too
//				* fast for the task to handle. In this condition, some conversion results lost.
//				*
//				* Reason:
//				* When this error occurs, you will usually see the task watchdog timeout issue also.
//				* Because the conversion is too fast, whereas the task calling `adc_digi_read_bytes` is slow.
//				* So `adc_digi_read_bytes` will hardly block. Therefore Idle Task hardly has chance to run. In this
//				* example, we add a `vTaskDelay(1)` below, to prevent the task watchdog timeout.
//				*
//				* Solution:
//				* Either decrease the conversion speed, or increase the frequency you call `adc_digi_read_bytes`
//				*/
//			}
//			else if(ret == ESP_ERR_TIMEOUT)
//			{
//				/**
//				* ``ESP_ERR_TIMEOUT``: If ADC conversion is not finished until Timeout, you'll get this return error.
//				* Here we set Timeout ``portMAX_DELAY``, so you'll never reach this branch.
//				*/
//				ESP_LOGW("warning","No data, increase timeout or reduce conv_num_each_intr");
//				vTaskDelay(1000);
//			}
//
////            int i = 0;
////            while(buffer_len--)
////            {
////                buffer[i++] = adc1_get_raw((adc1_channel_t)channel_);;
////            }
//            //			uint32_t adc_reading = 0;
//            //
//            //			for (int i = 0; i < NO_OF_SAMPLES; i++)		//Multisampling
//            //			{
//            //				if (unit == ADC_UNIT_1)					// Sample ADC1
//            //				{
//            //					adc_reading += adc1_get_raw((adc1_channel_t)channel_);
//            //				}
//            //				else
//            //				{
//            //					int raw;
//            //					adc2_get_raw((adc2_channel_t)channel_, width_, &raw);
//            //					adc_reading += raw;
//            //				}
//            //			}
//            //			adc_reading /= NO_OF_SAMPLES;
//            //
//            //			return adc_reading;
//        }
