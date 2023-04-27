#ifndef __TIMER_HPP__
#define __TIMER_HPP__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "driver/gptimer.h"


enum class timer_states {
	disabled = 0,
	stopped,				// enable and stopped;
	running					// enable and running;
};

class TIMER_driver {
public:
	TIMER_driver(int timer_num, int direction, uint32_t frequency) : frequency_(frequency), direction_(direction) {
		init();
	}
	~TIMER_driver(void) {
		deinit();
	}
	void init(void) {

		config_.clk_src = GPTIMER_CLK_SRC_DEFAULT;
		config_.direction = direction_ ? GPTIMER_COUNT_DOWN : GPTIMER_COUNT_UP;
		config_.resolution_hz = frequency_;	// 1MHz, 1 tick=1us

		ESP_ERROR_CHECK(gptimer_new_timer(&config_, &handler_));

	}
	void enable(void) {
		gptimer_enable(handler_);
	}
	void start(void) {
		gptimer_start(handler_);
	}
	void stop(void) {
		gptimer_stop(handler_);
	}
	uint32_t get_frequency(void) {
		uint32_t freq = 0;
		gptimer_get_resolution(handler_, &freq);

		return freq;
	}
	void disable(void) {
		gptimer_disable(handler_);
	}
	uint64_t get_count(void) {
		uint64_t count = 0;
		gptimer_get_raw_count(handler_, &count);

		return count;
	}
	void set_count(uint64_t value) {
		gptimer_set_raw_count(handler_, value);
	}
	void deinit(void) {
		stop();
		disable();
		gptimer_del_timer(handler_);
	}

private:
    gptimer_handle_t handler_ = NULL;
	gptimer_config_t config_;
	int frequency_ = 0;
	int direction_ = 0;
	timer_states timer_state = timer_states::disabled;
};

#endif /* ADC_HPP__ */
