#ifndef SPI_DRIVER_HPP__
#define SPI_DRIVER_HPP__

// #include "driver/spi_master.h"
// #include "driver/gpio.h"

// #include "driver/spi_common.h"
#include <cstring>
#include "driver/spi_master.h"

// Message returns for read, write errors and ok communication

class SPI_Driver{

public:
	// GPIO_DRIVER(gpio_num_t num) : num(num) {}
	SPI_Driver(void);

	void init(void);

	uint8_t transfer(uint8_t data);
	uint8_t transfer(uint8_t *data, size_t length);
	// spi_ans write(uint8_t *data, size_t length);
	// spi_ans read(uint8_t *data, size_t length);

private:
	spi_device_handle_t spi_handle_;

};

#endif /* GPIO_H__ */
