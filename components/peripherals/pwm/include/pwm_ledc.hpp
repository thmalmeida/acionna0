#ifndef PWM_LEDC_HPP__
#define PWM_LEDC_HPP__

// esp32 especifics
#include "driver/ledc.h"	// ledc lib
#include "esp_log.h"		// For debug purpose

static const char *TAG_PWM = "PWM_LEDC";

/*  ledc pwm peripheral are dividided into 2 groups of 8 channels. Each group has 4 timers.

The total is
	- 8 timers (4 high and 4 low);
	- 16 channels (8 for high group and 8 on low group);


Group 1 - High speed: automatic and glitch-free changing of the PWM duty cycle;
Group 2 - Low speed: the PWM duty cycle must be changed by the driver in software.

Each group is able to use different clock sources.

Ref.:
[1] https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html;
[2] 14LED PWM Controller (LEDC), esp32 technical reference manual, v49, 2023;

	channel range			: 0 - 7;
	timer_num				: 0 - 3;
	timer_module (group)	: 0 or 1;		0: slow, 1: fast
*/

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (5) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4095) // Set duty to 50%. ((2 ** 13) - 1) * 50% = 4095
#define LEDC_FREQUENCY          (1) // Frequency in Hertz. Set frequency at 5 kHz

/*
	Parameters:
		gpio_port_number
		frequency [Hz]
		duty_percent [%]
		invert_cycle, 0- non inverted, 1- inverted
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
	~pwm_ledc(void) {
	}
	void init(int gpio_number, uint32_t frequency, uint32_t duty_percent, unsigned int invert) {
		gpio_number_ = gpio_number;
		frequency_ = frequency;
		duty_ = duty_convert_(duty_percent);

		// Prepare and then apply the LEDC PWM timer configuration
		// ledc_timer_config_t *led0_timer_cfg = new ledc_timer_config_t;

		// led0_timer_cfg->speed_mode = static_cast<ledc_mode_t>(timer_module_);
		// led0_timer_cfg->duty_resolution = LEDC_TIMER_10_BIT;
		// led0_timer_cfg->timer_num = static_cast<ledc_timer_t>(timer_num_);
		// led0_timer_cfg->freq_hz = frequency_;
		// led0_timer_cfg->clk_cfg = LEDC_AUTO_CLK;
		// led0_timer_cfg->deconfigure = false;

		// Prepare and then apply the LEDC PWM channel configuration
		// // 1- Timer configuration
		ledc_timer_config_t led0_timer_cfg;

		led0_timer_cfg.speed_mode = static_cast<ledc_mode_t>(timer_module_);
		led0_timer_cfg.duty_resolution = LEDC_TIMER_10_BIT;
		led0_timer_cfg.timer_num = static_cast<ledc_timer_t>(timer_num_);
		led0_timer_cfg.freq_hz = frequency_;
		led0_timer_cfg.clk_cfg = LEDC_AUTO_CLK;
		led0_timer_cfg.deconfigure = false;

		ESP_LOGI(TAG_PWM, "init 2.a");
		ESP_ERROR_CHECK(ledc_timer_config(&led0_timer_cfg));
		ESP_LOGI(TAG_PWM, "init 2.b");

		// delete led0_timer_cfg;

		// 2- Channel's configuration
		ledc_channel_config_t led0_channel_cfg;
		led0_channel_cfg.gpio_num = gpio_number;
		led0_channel_cfg.speed_mode = static_cast<ledc_mode_t>(timer_module_);
		led0_channel_cfg.channel = static_cast<ledc_channel_t>(channel_);
		led0_channel_cfg.intr_type = LEDC_INTR_DISABLE;
		led0_channel_cfg.timer_sel = static_cast<ledc_timer_t>(timer_num_);
		led0_channel_cfg.duty = duty_;
		led0_channel_cfg.hpoint = 0;	// max of 0xfffff from 20 bit of counter register;
		led0_channel_cfg.sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD;
		led0_channel_cfg.flags.output_invert = invert;

		ESP_LOGI(TAG_PWM, "init 2.c");
		ESP_ERROR_CHECK(ledc_channel_config(&led0_channel_cfg));

		ESP_LOGI(TAG_PWM, "init");
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
		duty_ = duty_convert_(duty_percent);
		ledc_set_duty(static_cast<ledc_mode_t>(timer_module_), static_cast<ledc_channel_t>(channel_), duty_);

		// must be called to update;
		ledc_update_duty(static_cast<ledc_mode_t>(timer_module_), static_cast<ledc_channel_t>(channel_));

		ESP_LOGI(TAG_PWM, "pwm_ledc duty change to %lu", duty_percent);
	}
	void set_count(uint16_t value) {

	}
	void set_frequency(uint32_t frequency)
	{
		// this->frequency_ = frequency;
		frequency_ = frequency;
		ledc_set_freq(static_cast<ledc_mode_t>(timer_module_), static_cast<ledc_timer_t>(timer_num_), frequency_);

		ESP_LOGI(TAG_PWM, "pwm_ledc freq change");
	}
	void clear_count(void) {
		ledc_timer_rst(static_cast<ledc_mode_t>(timer_module_), static_cast<ledc_timer_t>(timer_num_));
	}
	void get_htop(void) {

	}

private:
	int gpio_number_;
	uint32_t frequency_;
	uint32_t duty_;

	unsigned int timer_module_ = 0;		// 0: low speed. 1: high speed
	unsigned int timer_num_ = 0;		// ranges from 0 to 3;
	unsigned int channel_ = 0;			// 0 - 7;

	uint32_t duty_convert_(uint32_t duty_percent)
	{
		return uint32_t(duty_percent/100.0*1024.0);;
	}
};

#endif
