#include "adc2.hpp"

static const char *TAG_ADC = "ADC driver";

// static TaskHandle_t s_task_handle;
// static bool IRAM_ATTR s_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data)
// {
// 	BaseType_t mustYield = pdFALSE;
// 	//Notify that ADC continuous driver has done enough number of conversions
// 	vTaskNotifyGiveFromISR(s_task_handle, &mustYield);

// 	return (mustYield == pdTRUE);
// }

// ADC_driver::ADC_driver(int channel) : channel_{static_cast<adc_channel_t>(channel)} {
// 	init_driver_oneshot();
// }
ADC_driver::ADC_driver(void) {
	init_driver_oneshot();
}
ADC_driver::~ADC_driver(void) {
	deinit_driver_oneshot();
}
void ADC_driver::init_driver_oneshot(void) {
	// 1 - Resource Allocation (init)
	// adc_oneshot_unit_handle_t adc1_handle_;
	adc_oneshot_unit_init_cfg_t init_config1 = {
		.unit_id = unit_,							// ADC select
		.ulp_mode = ADC_ULP_MODE_DISABLE,
	};

	// 1.1 - install driver and ADC instance
	ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle_));

	// Config channel to read
	// channel_config_oneshot();
}
void ADC_driver::set_channel(int channel) {
    channel_ = static_cast<adc_channel_t>(channel);
}
void ADC_driver::deinit_driver_oneshot(void) {

	ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle_));
	if (do_calibration1) {
		adc_calibration_deinit(adc1_cali_handle_);
	}
}
void ADC_driver::channel_config_oneshot(int channel) {
	// 2 - Unit configuration of ADC1
	adc_oneshot_chan_cfg_t config = {
		.atten = attenuation_,
		.bitwidth = width_,
	};
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle_, static_cast<adc_channel_t>(channel), &config));
}
int ADC_driver::read(int channel) {

	int data_adc_raw;
	ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle_, static_cast<adc_channel_t>(channel), &data_adc_raw));
	// ESP_LOGI(TAG_ADC, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC1_CHAN0, data_adc_raw);

	// ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle_, ADC1_CHAN1, &adc_raw[0][1]));
	// ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC1_CHAN1, adc_raw[0][1]);

	return data_adc_raw;
}

bool ADC_driver::adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle)
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
void ADC_driver::adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG_ADC, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    ESP_LOGI(TAG_ADC, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}
void ADC_driver::calibrate(void) {

	if (do_calibration1) {
		ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle_, adc_raw[0][0], &voltage[0][0]));
		ESP_LOGI(TAG_ADC, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC1_CHAN0, voltage[0][0]);

		ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle_, adc_raw[0][1], &voltage[0][1]));
		ESP_LOGI(TAG_ADC, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, ADC1_CHAN1, voltage[0][1]);
	}
}
















// void ADC_driver::init_continuous(void) {
	
// 	esp_err_t ret;
//     uint32_t ret_num = 0;
//     uint8_t result[EXAMPLE_READ_LEN] = {0};
//     memset(result, 0xcc, EXAMPLE_READ_LEN);

//     s_task_handle = xTaskGetCurrentTaskHandle();

//     adc_continuous_handle_t handle = NULL;
//     config_continuous(channel, sizeof(channel) / sizeof(adc_channel_t), &handle);

//     adc_continuous_evt_cbs_t cbs = {
//         .on_conv_done = s_conv_done_cb,
//     };
//     ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(handle, &cbs, NULL));
//     ESP_ERROR_CHECK(adc_continuous_start(handle));
// }
// void ADC_driver::config_continuous(adc_channel_t *channel, uint8_t channel_num, adc_continuous_handle_t *out_handle)
// {
// 	adc_continuous_handle_t handle = NULL;

// 	adc_continuous_handle_cfg_t adc_config_continuous = {
// 		.max_store_buf_size = 1024,
// 		.conv_frame_size = EXAMPLE_READ_LEN,
// 	};
// 	ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config_continuous, &handle));

// 	adc_continuous_config_t dig_cfg = {
// 		.sample_freq_hz = 20 * 1000,
// 		.conv_mode = ADC_CONV_MODE,
// 		.format = ADC_OUTPUT_TYPE,
// 	};

// 	adc_digi_pattern_config_t adc_pattern[SOC_ADC_PATT_LEN_MAX] = {0};
// 	dig_cfg.pattern_num = channel_num;
// 	for (int i = 0; i < channel_num; i++) {
// 		uint8_t unit = GET_UNIT(channel[i]);
// 		uint8_t ch = channel[i] & 0x7;
// 		adc_pattern[i].atten = ADC_ATTEN_DB_0;
// 		adc_pattern[i].channel = ch;
// 		adc_pattern[i].unit = unit;
// 		adc_pattern[i].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

// 		ESP_LOGI(TAG, "adc_pattern[%d].atten is :%x", i, adc_pattern[i].atten);
// 		ESP_LOGI(TAG, "adc_pattern[%d].channel is :%x", i, adc_pattern[i].channel);
// 		ESP_LOGI(TAG, "adc_pattern[%d].unit is :%x", i, adc_pattern[i].unit);
// 	}
// 	dig_cfg.adc_pattern = adc_pattern;
// 	ESP_ERROR_CHECK(adc_continuous_config(handle, &dig_cfg));

// 	*out_handle = handle;
// }
// void ADC_driver::read_stream(uint8_t *data, int len) {
// }
