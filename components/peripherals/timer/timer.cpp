#include "timer.hpp"

static const char *TAG_TIMER = "TIMER";

TIMER_driver::TIMER_driver(int timer_num, int frequency) {
}
TIMER_driver::~TIMER_driver(void) {
	// deinit_driver_oneshot();
}
