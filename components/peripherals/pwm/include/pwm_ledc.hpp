#ifndef PWM_LEDC_HPP__
#define PWM_LEDC_HPP__

// #include <stdio.h>
#include "driver/ledc.h"

// For debug purpose
#include "esp_log.h"
static const char *TAG_PWM = "PWM_LEDC";

/*  ledc pwm peripheral are dividided into 2 groups of 8 channels. Each group has 4 timers.

Group 1 - High speed: automatic and glitch-free changing of the PWM duty cycle;
Group 2 - Low speed: the PWM duty cycle must be changed by the driver in software.

Each group is able to use different clock sources.

Ref.:
[1] https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html;
[2] 14LED PWM Controller (LEDC), esp32 technical reference manual, v49, 2023;

	channel range	: 0 - 7;
	timer_num		: 0 - 3;
	timer_module	: 0 or 1;		0: slow, 1: fast
*/

class pwm_ledc {

public:

	pwm_ledc(int gpio_number, uint32_t frequency, uint32_t duty_percent, unsigned int invert) {
		init(gpio_number, frequency, duty_percent, invert);
	}
	pwm_ledc(int gpio_number, uint32_t frequency, uint32_t duty_percent, unsigned int invert, int channel, int timer_num, int timer_module) {
		channel_ = channel;
		timer_num_ = timer_num;
		timer_module_ = timer_module;
		init(gpio_number, frequency, duty_percent, invert);
	}
	// ~pwm_ledc(void) {
	// }
	void init(int gpio_number, uint32_t frequency, uint32_t duty_percent, unsigned int invert) {
		_gpio_number = gpio_number;
		_frequency = frequency;
		_duty = _duty_convert(duty_percent);

		// 1- Timer configuration
		ledc_timer_config_t led0_timer_cfg;

		led0_timer_cfg.speed_mode = static_cast<ledc_mode_t>(timer_module_);
		led0_timer_cfg.timer_num = static_cast<ledc_timer_t>(timer_num_);
		led0_timer_cfg.freq_hz = _frequency;
		led0_timer_cfg.duty_resolution = LEDC_TIMER_10_BIT;
		led0_timer_cfg.clk_cfg = LEDC_AUTO_CLK;
		ledc_timer_config(&led0_timer_cfg);

		// 2- Channel's configuration
		ledc_channel_config_t led0_channel_cfg;

		led0_channel_cfg.gpio_num = gpio_number;
		led0_channel_cfg.speed_mode = static_cast<ledc_mode_t>(timer_module_);
		led0_channel_cfg.channel = static_cast<ledc_channel_t>(channel_);
		led0_channel_cfg.intr_type = LEDC_INTR_DISABLE;
		led0_channel_cfg.timer_sel = static_cast<ledc_timer_t>(timer_num_);
		led0_channel_cfg.duty = _duty;
		led0_channel_cfg.hpoint = 0;	// max of 0xfffff from 20 bit of counter register;
		led0_channel_cfg.flags.output_invert = invert;
		ledc_channel_config(&led0_channel_cfg);

		ESP_LOGI(TAG_PWM, "pwm_ledc init");
	}
	void stop(uint32_t level) {
		ledc_stop(static_cast<ledc_mode_t>(timer_module_), static_cast<ledc_channel_t>(channel_), level);
		ESP_LOGI(TAG_PWM, "pwm_ledc stop");
	}
	void pause(void) {
		ledc_timer_pause(static_cast<ledc_mode_t>(timer_module_), static_cast<ledc_timer_t>(timer_num_));
	}
	void resume(void) {
		ledc_timer_resume(static_cast<ledc_mode_t>(timer_module_), static_cast<ledc_timer_t>(timer_num_));
	}
	void set_duty(uint32_t duty_percent) {
		_duty = _duty_convert(duty_percent);
		ledc_set_duty(static_cast<ledc_mode_t>(timer_module_), static_cast<ledc_channel_t>(channel_), _duty);

		// must be called to update;
		ledc_update_duty(static_cast<ledc_mode_t>(timer_module_), static_cast<ledc_channel_t>(channel_));

		ESP_LOGI(TAG_PWM, "pwm_ledc duty change");
	}
	void set_count(uint16_t value) {

	}
	void set_frequency(uint32_t frequency)
	{
		this->_frequency = frequency;
		ledc_set_freq(static_cast<ledc_mode_t>(timer_module_), static_cast<ledc_timer_t>(timer_num_), _frequency);

		ESP_LOGI(TAG_PWM, "pwm_ledc freq change");
	}
	void clear_count(void) {
		ledc_timer_rst(static_cast<ledc_mode_t>(timer_module_), static_cast<ledc_timer_t>(timer_num_));
	}
	void get_htop(void) {

	}

private:
	int _gpio_number;
	uint32_t _frequency;
	uint32_t _duty;

	unsigned int timer_module_ = 0;		// 0: low speed. 1: high speed
	unsigned int timer_num_ = 0;		// ranges from 0 to 3;
	unsigned int channel_ = 0;			// 0 - 7;

	uint32_t _duty_convert(uint32_t duty_percent)
	{
		return uint32_t(duty_percent/100.0*1024.0);;
	}
};

#endif
