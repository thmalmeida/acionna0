#include "i2c_driver.hpp"

static const char *TAG_I2C = "I2C";

I2C_Driver::I2C_Driver(int port, int scl, int sda, uint32_t freq) : i2c_master_port_(port) {
	// Bus configuration
	i2c_master_bus_config_t bus_cfg;
	bus_cfg.i2c_port = static_cast<i2c_port_num_t>(port);
	bus_cfg.scl_io_num = static_cast<gpio_num_t>(scl);
	bus_cfg.sda_io_num = static_cast<gpio_num_t>(sda);
	bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
	bus_cfg.intr_priority = 0;
	bus_cfg.trans_queue_depth = 0;
	bus_cfg.glitch_ignore_cnt = 7;
	bus_cfg.flags.enable_internal_pullup = true;
	ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus_handle_));

	dev_cfg_.dev_addr_length = I2C_ADDR_BIT_LEN_7;
	dev_cfg_.scl_speed_hz = freq;
	
	ESP_LOGI(TAG_I2C, "I2C bus init done!");
}
I2C_Driver::~I2C_Driver() {
	// Deinitialize the I2C master bus and delete the handle.
	ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle_));

	// I2C master bus delete device.
	// ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle_));
}

i2c_ans I2C_Driver::write(uint8_t slave_addr, uint8_t *data, uint8_t len) {
	dev_cfg_.device_address = slave_addr;
	ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle_, &dev_cfg_, &dev_handle_));
	esp_err_t ret = i2c_master_transmit(dev_handle_, data, len, I2C_COMMAND_WAIT_MS);
	ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle_));

	if(ret != ESP_OK) {
		// ESP_LOGI(TAG_I2C, "i2c write error");
		return i2c_ans::error_write;
	}
	else
		return i2c_ans::ok;
}
i2c_ans I2C_Driver::write(uint8_t slave_addr, uint8_t reg, uint8_t *data, size_t len) {

	uint8_t frame[len+1];
	frame[0] = reg;
	memcpy(&frame[1], data, len);

	return write(slave_addr, &frame[0], len+1);
}
i2c_ans I2C_Driver::write(uint8_t slave_addr, uint8_t reg, uint8_t data) {
	
	uint8_t frame[2];
	frame[0] = reg;
	frame[1] = data;

	return write(slave_addr, &frame[0], 2);
}
i2c_ans I2C_Driver::write(uint8_t slave_addr, uint8_t reg) {
	return write(slave_addr, &reg, 1);
}

i2c_ans I2C_Driver::read(uint8_t slave_addr, const uint8_t *data_write, size_t len_write, uint8_t *data_read, size_t len_read) {
	dev_cfg_.device_address = slave_addr;
	ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle_, &dev_cfg_, &dev_handle_));
	esp_err_t ret = i2c_master_transmit_receive(dev_handle_, data_write, len_write, data_read, len_read, I2C_COMMAND_WAIT_MS);
	ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle_));

	if(ret != ESP_OK) {
		// ESP_LOGI(TAG_I2C, "i2c read error");
		return i2c_ans::error_read;
	}
	else
		return i2c_ans::ok;
}
i2c_ans I2C_Driver::read(uint8_t slave_addr, uint8_t *data, size_t len) {
	dev_cfg_.device_address = slave_addr;
	ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle_, &dev_cfg_, &dev_handle_));
	esp_err_t ret = i2c_master_receive(dev_handle_, data, len, I2C_COMMAND_WAIT_MS);

	if(ret != ESP_OK) {
		return i2c_ans::error_read;
	}
	else
		return i2c_ans::ok;
}
i2c_ans I2C_Driver::read(uint8_t slave_addr, uint8_t *data) {
	return read(slave_addr, data, 1);
}
i2c_ans I2C_Driver::read(uint8_t slave_addr, uint8_t reg, uint8_t *data, size_t len) {
	return read(slave_addr, &reg, 1, data, len);
}
i2c_ans I2C_Driver::read(uint8_t slave_addr, uint8_t reg, uint8_t *data) {
	return read(slave_addr, &reg, 1, data, 1);
}

bool I2C_Driver::probe(uint8_t addr) noexcept {
	if(i2c_master_probe(bus_handle_, addr, I2C_COMMAND_WAIT_MS) == ESP_OK)
		return true;
	else
		return false;	

	// ESP_LOGI(TAG_I2C, "probe addr: 0x%02x\n", addr);	
	// if(read(addr, 0x00, &data) == i2c_ans::ok)
}
uint8_t I2C_Driver::probe_find(uint8_t addr_start) {
	// uint8_t data;
	uint8_t n_sensors = 0;
	for(uint8_t i = addr_start; i < 128; i++) {

		if(i2c_master_probe(bus_handle_, i, I2C_COMMAND_WAIT_MS) == ESP_OK) {
			printf("addr %u: 0x%02x\n", n_sensors++, i);
			return i;			
		}

		// if(read(i, 0x00, &data) == i2c_ans::ok) {
		// 	printf("addr %u: 0x%02x\n", n_sensors++, i);
		// 	return i;
		// }

	}
	// ESP_LOGI(TAG_I2C, "nothing found!");
	
	return 0x00;
}
void I2C_Driver::probe_list(void) {

	// uint8_t data;
	// uint8_t addr_list[20];

	int i=0;
	uint8_t n_sensors = 0;
	for(i=0; i<128; i++) {

		if(i2c_master_probe(bus_handle_, i, I2C_COMMAND_WAIT_MS) == ESP_OK) {
			printf("addr %u: 0x%02x\n", n_sensors++, i);
		}

		// if(read(i, 0x00, &data) == i2c_ans::ok) {
		// 	addr_list[n_sensors] = i;
		// 	n_sensors++;
		// 	// printf("addr %u: 0x%02x\n", n_sensors++, i);
		// }
	}

	// if(n_sensors) {
	// 	printf("found %d sensors\n", n_sensors);
	// 	for(i=0; i<n_sensors; i++) {
	// 		printf("addr[%d]: 0x%02x\n", i, addr_list[i]);
	// 	}
	// }
}
