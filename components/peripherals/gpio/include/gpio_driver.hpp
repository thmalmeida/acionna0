#ifndef GPIO_DRIVER_H__
#define GPIO_DRIVER_H__

#include "driver/gpio.h"

/**
 * TODO fazer melhor controle da variavel driver_instaled, que contém quantos dispositivos
 * 	estao se utilizando do driver de interrucao
 */

class GPIO_DRIVER{
	public:
		// GPIO_DRIVER(int pin_number, int direction = 0);
		GPIO_DRIVER(int pin_number, int direction = 0, bool remap = false);

		void mode(int direction);
		int read(void);
		void write(int level);
		void toggle(void);
		void reset(void) noexcept;

		~GPIO_DRIVER();

		// uC specifics - ESP32
		void pull(int mode);

		void register_interrupt(gpio_isr_t handler, void* isr_args);
		void unregister_interrupt();

		void enable_interrupt(gpio_int_type_t type);
		void disable_interrupt();

		void strength(gpio_drive_cap_t cap);
		void hold(bool hold);
		static void deep_sleep_hold(bool hold);

		void* get_isr_args();

	private:
		int pin_number_;
		int direction_;
		int level_ = 0;

		// uC specifics - ESP32
		gpio_num_t pin_number_t_;
		void* isr_args = NULL;
		static unsigned int driver_instaled;

};

typedef struct {
	GPIO_DRIVER* esse;
	void* args;
}gpio_isr_args_t;

#endif /* GPIO_H__ */
