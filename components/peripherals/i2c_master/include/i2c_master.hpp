#ifndef I2C_MASTER_HPP__
#define I2C_MASTER_HPP__

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "delay.hpp"

#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

#include "pinout.hpp"

#define I2C_ERR_OK		1
#define I2C_ERR_WRITE	-1
#define I2C_ERR_READ	-2
#define I2C_ERR_ACK		-3

#define I2C_FAST_SPEED_HZ	400000
#define I2C_NORMAL_SPEED_HZ	100000

#ifdef CONFIG_I2C_COMMAND_WAIT
#define I2C_COMMAND_WAIT_MS	CONFIG_I2C_COMMAND_WAIT
#else
#define I2C_COMMAND_WAIT_MS	1000
#endif /* CONFIG_I2C_COMMAND_WAIT */

class I2C_Master{
	public:
		// I2C_Master(i2c_port_t port, int scl, int sda, uint32_t freq, bool pull_up = false);
		I2C_Master(int port, int scl, int sda, uint32_t freq, bool pull_up /* = false */);
		~I2C_Master();

		// void init(int intr_alloc_flags = 0);
		// void deinit();

		int write(uint8_t slave_addr, uint8_t reg, uint8_t* data, size_t len, bool ack_check);
		int write(uint8_t slave_addr, uint8_t reg, uint8_t data, bool ack_check);
		int write(uint8_t slave_addr, uint8_t reg, bool ack_check);
		int write(uint8_t slave_addr, uint8_t *data, uint8_t data_len);

		int set_mask(uint8_t slave_addr, uint8_t reg, uint8_t data, uint8_t mask, bool ack_check = true);

		int read(uint8_t slave_addr, uint8_t reg, uint8_t* data, size_t len, bool ack_check);
		int read(uint8_t slave_addr, uint8_t reg, uint8_t* data, bool ack_check = true);
		int read(uint8_t slave_address, const uint8_t *write_buffer, size_t write_buffer_len, uint8_t *read_buffer, size_t read_buffer_len);
		int read_only(uint8_t slave_addr, uint8_t* data, size_t data_len, bool ack_check = true);

		bool probe(uint8_t addr) noexcept;
		uint8_t probe_find(uint8_t addr_init = 0);
	protected:
		int	i2c_master_port_;
};

#endif /* I2C_MASTER_H__ */
