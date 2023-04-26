#ifndef PWM_LEDC_HPP__
#define PWM_LEDC_HPP__

// #include <stdio.h>
#include "driver/ledc.h"

// For debug purpose
#include "esp_log.h"
static const char *TAG_PWM = "PWM_LEDC";

/*  ledc pwm peripheral are dividided into 2 groups of 8 channels.
    1- group uses hardware;
    2- group uses software.

    Each group is able to use different clock sources.
*/

class pwm_ledc {

public:

	pwm_ledc(int gpio_number, uint32_t frequency, uint32_t duty_percent, uint32_t invert)
    {
        _gpio_number = gpio_number;
        _frequency = frequency;
        _duty = _duty_convert(duty_percent);

        // 1- Timer configuration
        ledc_timer_config_t led0_timer_cfg;

        led0_timer_cfg.speed_mode = LEDC_LOW_SPEED_MODE;
        led0_timer_cfg.timer_num = LEDC_TIMER_0;
        led0_timer_cfg.freq_hz = _frequency;
        led0_timer_cfg.duty_resolution = LEDC_TIMER_10_BIT;
        led0_timer_cfg.clk_cfg = LEDC_AUTO_CLK;
        ledc_timer_config(&led0_timer_cfg);

        // 2- Channel's configuration
        ledc_channel_config_t led0_channel_cfg;

        led0_channel_cfg.gpio_num = _gpio_number;
        led0_channel_cfg.speed_mode = LEDC_LOW_SPEED_MODE;
        led0_channel_cfg.channel = LEDC_CHANNEL_0;
        led0_channel_cfg.intr_type = LEDC_INTR_DISABLE;
        led0_channel_cfg.timer_sel = LEDC_TIMER_0;
        led0_channel_cfg.duty = _duty;
        led0_channel_cfg.hpoint = 0;	// max of 0xfffff from 20 bit of counter register;
        led0_channel_cfg.flags.output_invert = invert;
        ledc_channel_config(&led0_channel_cfg);

        ESP_LOGI(TAG_PWM, "pwm_ledc init");
    }

 	void pwm_ledc_stop(uint32_t level) {
        ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, level);
        ESP_LOGI(TAG_PWM, "pwm_ledc stop");
    }
    void pwm_ledc_set_duty(uint32_t duty_percent)
    {
        _duty = _duty_convert(duty_percent);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, _duty);
        
        // must be called to update;
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        ESP_LOGI(TAG_PWM, "pwm_ledc duty change");
    }
    void pwm_ledc_set_frequency(uint32_t frequency)
    {
        this->_frequency = frequency;
        ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, _frequency);

        ESP_LOGI(TAG_PWM, "pwm_ledc freq change");
    }

    private:
        int _gpio_number;
        uint32_t _frequency;
        uint32_t _duty;

        uint32_t _duty_convert(uint32_t duty_percent)
        {
            return uint32_t(duty_percent/100.0*1024.0);;
        }
};

#endif