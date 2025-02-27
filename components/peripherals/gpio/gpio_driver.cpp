#include "gpio_driver.hpp"

#ifndef GPIO_INTERRUPT_FLAGS
#define GPIO_INTERRUPT_FLAGS		(ESP_INTR_FLAG_LEVEL3|ESP_INTR_FLAG_IRAM)
#endif

unsigned int GPIO_DRIVER::driver_instaled = 0;

// GPIO_DRIVER::GPIO_DRIVER(int pin_number, int direction) : pin_number_(pin_number), direction_(direction) {
// 	// this->num = num;
// 	pin_number_t_ = static_cast<gpio_num_t>(pin_number_);
// 	mode(direction);
// }
GPIO_DRIVER::GPIO_DRIVER(int pin_number, int direction, bool remap) : pin_number_(pin_number), direction_(0) {
	// this->num = num;
	pin_number_t_ = static_cast<gpio_num_t>(pin_number);
	
	mode(direction);

	// JTAG shared ports must redirect to GPIO using iomux matrix
	if(remap == true)
		gpio_iomux_out(pin_number_t_, 2, false);
}
GPIO_DRIVER::~GPIO_DRIVER(){
	unregister_interrupt();
}
void GPIO_DRIVER::mode(int direction) {

	direction_ = direction;

	gpio_mode_t direction_t_;
	switch (direction) {
		case 0:
			direction_t_ = GPIO_MODE_INPUT;
			break;
		case 1:
			direction_t_ = GPIO_MODE_INPUT_OUTPUT;
			break;
		default:
			direction_t_ = GPIO_MODE_INPUT;
			break;
	}

	gpio_set_direction(pin_number_t_, direction_t_);

	if(!direction) {
		pull(0);		// set pull up if input mode;
	}
}
int GPIO_DRIVER::read(void){
	level_ = gpio_get_level(pin_number_t_);
	return level_;
}
void GPIO_DRIVER::write(int level){
	gpio_set_level(pin_number_t_, level);
}
void GPIO_DRIVER::toggle(void){
	level_ = !level_;
	write(level_);
}
void GPIO_DRIVER::reset(void) noexcept {
	gpio_reset_pin(pin_number_t_);
}

// uC specifics - ESP32
void GPIO_DRIVER::pull(int mode) {

	gpio_pull_mode_t pull_mode_t_ = static_cast<gpio_pull_mode_t>(mode);
	switch (mode) {
		case 0:
			pull_mode_t_ = GPIO_PULLUP_ONLY;
			break;
		case 1:
			pull_mode_t_ = GPIO_PULLDOWN_ONLY;
			break;
		case 2:
			pull_mode_t_ = GPIO_PULLUP_PULLDOWN;
			break;
		case 3:
			pull_mode_t_ = GPIO_FLOATING;
			break;
		default:
			pull_mode_t_ = GPIO_PULLUP_ONLY;
			break;
	}
	gpio_set_pull_mode(pin_number_t_, pull_mode_t_);
}
void GPIO_DRIVER::strength(gpio_drive_cap_t cap){
	gpio_set_drive_capability(pin_number_t_, cap);
}
void GPIO_DRIVER::hold(bool hold){
	if(hold){
		gpio_hold_en(pin_number_t_);
	} else {
		gpio_hold_dis(pin_number_t_);
	}
}
void GPIO_DRIVER::deep_sleep_hold(bool hold){
	if(hold){
		gpio_deep_sleep_hold_en();
	} else {
		gpio_deep_sleep_hold_dis();
	}
}
void GPIO_DRIVER::register_interrupt(gpio_isr_t handler, void* isr_args){
	if(driver_instaled == 0){
		gpio_install_isr_service(GPIO_INTERRUPT_FLAGS);
		driver_instaled++;
	}

	this->isr_args = isr_args;
	gpio_isr_handler_add(pin_number_t_, handler,this);
}
void GPIO_DRIVER::unregister_interrupt(){
	if(driver_instaled == 0){
		return;
	}

	gpio_isr_handler_remove(pin_number_t_);
	disable_interrupt();

	if(--driver_instaled){
		gpio_uninstall_isr_service();
	}
}

void GPIO_DRIVER::enable_interrupt(gpio_int_type_t type){
	gpio_set_intr_type(pin_number_t_, type);
	gpio_intr_enable(pin_number_t_);
}

void GPIO_DRIVER::disable_interrupt(){
	gpio_intr_disable(pin_number_t_);
}

void* GPIO_DRIVER::get_isr_args(){
	return isr_args;
}
