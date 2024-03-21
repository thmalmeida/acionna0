#include "i2c_driver.hpp"

static const char *TAG_I2C = "I2C";

I2C_DRIVER::I2C_DRIVER(int port, int scl, int sda, uint32_t freq, bool pull_up /* = false */) : i2c_master_port_(port) {
	// Configuration
	// i2c_master_bus_config_t i2c_mst_config = {
	// 	.clk_source = I2C_CLK_SRC_DEFAULT,
	// 	.i2c_port = TEST_I2C_PORT,
	// 	.scl_io_num = I2C_MASTER_SCL_IO,
	// 	.sda_io_num = I2C_MASTER_SDA_IO,
	// 	.glitch_ignore_cnt = 7,
	// 	.flags.enable_internal_pullup = true,
	// };
	i2c_master_bus_config_t bus_cfg;

	bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT,
	bus_cfg.i2c_port = static_cast<i2c_port_num_t>(i2c_master_port_);
	bus_cfg.scl_io_num = static_cast<gpio_num_t>(scl);
	bus_cfg.sda_io_num = static_cast<gpio_num_t>(sda);
	bus_cfg.glitch_ignore_cnt = 7,
	bus_cfg.flags.enable_internal_pullup = true,

	i2c_master_bus_handle_t bus_handle;
	ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus_handle));

	// i2c_device_config_t dev_cfg = {
	// .dev_addr_length = I2C_ADDR_BIT_LEN_7,
	// .device_address = 0x58,
	// .scl_speed_hz = 100000,
	// };
	i2c_device_config_t dev_cfg;
	dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
	dev_cfg.device_address = I2C_ADDR;
	dev.scl_speed_hz = I2C_SPEED;

	i2c_master_dev_handle_t dev_handle;
	ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));


	// i2c_config_t conf = {};
	// conf.mode = I2C_MODE_MASTER;
	// conf.sda_io_num = sda;
	// conf.sda_pullup_en = pull_up ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
	// conf.scl_io_num = scl;
	// conf.scl_pullup_en = pull_up ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
	// conf.master.clk_speed = freq; // I2C_FAST_SPEED_HZ or I2C_NORMAL_SPEED_HZ;

	// esp_err_t err = i2c_param_config(static_cast<i2c_port_t>(i2c_master_port_), &conf);
	// if(err != ESP_OK) {
	// 	ESP_LOGI(TAG_I2C, "I2C config error [%d/%s]\n", err, esp_err_to_name(err));
	// }

	// // Install driver
	// int intr_alloc_flags = 0;
	// err = i2c_driver_install(static_cast<i2c_port_t>(i2c_master_port_), conf.mode, 0, 0, intr_alloc_flags);

	// if(err != ESP_OK) { 
	// 	ESP_LOGE(TAG_I2C, "I2C install error [%d/%s]\n", err, esp_err_to_name(err));
	// }
}
I2C_DRIVER::~I2C_DRIVER() {
	i2c_driver_delete(static_cast<i2c_port_t>(i2c_master_port_));
}
i2c_ans I2C_DRIVER::write(uint8_t slave_addr, uint8_t reg, uint8_t* data, size_t len, bool ack_check /* = true */) {

	esp_err_t ret;

	i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
	i2c_master_start(cmd_handle);
	i2c_master_write_byte(cmd_handle, slave_addr << 1 | I2C_MASTER_WRITE, ack_check);
	i2c_master_write_byte(cmd_handle, reg, ack_check);
	i2c_master_write(cmd_handle, data, len, ack_check);
	i2c_master_stop(cmd_handle);
	ret = i2c_master_cmd_begin(static_cast<i2c_port_t>(i2c_master_port_), cmd_handle, I2C_COMMAND_WAIT_MS / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd_handle);

	if (ret != ESP_OK) {
		// ESP_LOGI(TAG_I2C, "read: error on write: %s", esp_err_to_name(ret));
		return i2c_ans::error_write;
		// return I2C_ERR_WRITE;
	}

	return i2c_ans::ok;
}
i2c_ans I2C_DRIVER::write(uint8_t slave_addr, uint8_t reg, uint8_t data, bool ack_check /* = true */){
	return write(slave_addr, reg, &data, 1, ack_check);
}
i2c_ans I2C_DRIVER::write(uint8_t slave_addr, uint8_t reg, bool ack_check /* = true */){
	uint8_t data;
	return write(slave_addr, reg, &data, 0, ack_check);
}
i2c_ans I2C_DRIVER::write(uint8_t slave_addr, uint8_t *data, uint8_t data_len) {
	
	esp_err_t ret = i2c_master_write_to_device(static_cast<i2c_port_t>(i2c_master_port_), slave_addr, data, data_len, I2C_COMMAND_WAIT_MS / portTICK_PERIOD_MS);

	if (ret != ESP_OK) {
		// ESP_LOGI(TAG_I2C, "Error on write2");
		// return I2C_ERR_WRITE;
		return i2c_ans::error_write;
	}

	// return I2C_ERR_OK;
	return i2c_ans::ok;
}
i2c_ans I2C_DRIVER::set_mask(uint8_t slave_addr, uint8_t reg, uint8_t data, uint8_t mask, bool ack_check /* = true */) {
	uint8_t content;

	i2c_ans ret = read(slave_addr, reg, &content, ack_check);

	if(ret != i2c_ans::ok)
		return ret;

	content &= ~mask;
	content |= data & mask;

	return write(slave_addr, reg, content, ack_check);
}
i2c_ans I2C_DRIVER::read(uint8_t slave_addr, uint8_t reg, uint8_t* data, size_t len, bool ack_check) {
	esp_err_t ret;

	// Read: write to register first
	i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
	i2c_master_start(cmd_handle);
	i2c_master_write_byte(cmd_handle, slave_addr << 1 | I2C_MASTER_WRITE, ack_check);
	i2c_master_write_byte(cmd_handle, reg, ack_check);
	i2c_master_stop(cmd_handle);
	ret = i2c_master_cmd_begin(static_cast<i2c_port_t>(i2c_master_port_), cmd_handle, I2C_COMMAND_WAIT_MS / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd_handle);

	if (ret != ESP_OK) {
		// ESP_LOGI(TAG_I2C, "read: error on write: %s", esp_err_to_name(ret));
		// return I2C_ERR_WRITE;
		return i2c_ans::error_write;
	}

	// Read: read start
	cmd_handle = i2c_cmd_link_create();
	i2c_master_start(cmd_handle);
	i2c_master_write_byte(cmd_handle, slave_addr << 1 | I2C_MASTER_READ, ack_check);
	if(len > 1) {
		i2c_master_read(cmd_handle, data, len - 1, I2C_MASTER_ACK);
	}
	i2c_master_read_byte(cmd_handle, data + len - 1, I2C_MASTER_NACK);
	i2c_master_stop(cmd_handle);
	ret = i2c_master_cmd_begin(static_cast<i2c_port_t>(i2c_master_port_), cmd_handle, I2C_COMMAND_WAIT_MS / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd_handle);

	if(ret != ESP_OK) {
		// ESP_LOGI(TAG_I2C, "read: error on read: %s", esp_err_to_name(ret));
		// return I2C_ERR_READ;
		return i2c_ans::error_read;
	}

	// return I2C_ERR_OK;
	return i2c_ans::ok;
}
i2c_ans I2C_DRIVER::read(uint8_t slave_addr, uint8_t reg, uint8_t* data, bool ack_check /* = true */) {
	return read(slave_addr, reg, data, 1, ack_check);
}
i2c_ans I2C_DRIVER::read(uint8_t slave_address, const uint8_t *write_buffer, size_t write_buffer_len, uint8_t *read_buffer, size_t read_buffer_len) {

	esp_err_t ret = i2c_master_write_read_device(static_cast<i2c_port_t>(i2c_master_port_), slave_address, write_buffer, write_buffer_len, read_buffer, read_buffer_len, I2C_COMMAND_WAIT_MS / portTICK_PERIOD_MS);

	if(ret != ESP_OK) {
		// ESP_LOGI(TAG_I2C, "read2: error on read: %s", esp_err_to_name(ret));
		// return I2C_ERR_READ;
		return i2c_ans::error_read;
	}

	// return I2C_ERR_OK;
	return i2c_ans::ok;
}
i2c_ans I2C_DRIVER::read_only(uint8_t slave_addr, uint8_t* data, size_t data_len, bool ack_check /* = true */) {

	// Read only: read start	
	i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
	i2c_master_start(cmd_handle);
	i2c_master_write_byte(cmd_handle, slave_addr << 1 | I2C_MASTER_READ, ack_check);
	if(data_len > 1) {
		i2c_master_read(cmd_handle, data, data_len - 1, I2C_MASTER_ACK);
	}
	i2c_master_read_byte(cmd_handle, data + data_len - 1, I2C_MASTER_NACK);
	i2c_master_stop(cmd_handle);
	esp_err_t ret = i2c_master_cmd_begin(static_cast<i2c_port_t>(i2c_master_port_), cmd_handle, I2C_COMMAND_WAIT_MS / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd_handle);

	if(ret != ESP_OK) {
		// ESP_LOGI(TAG_I2C, "read only: error on read: %s", esp_err_to_name(ret));
		// return I2C_ERR_READ;
		return i2c_ans::error_read;
	}

	// return I2C_ERR_OK;
	return i2c_ans::ok;
}
bool I2C_DRIVER::probe(uint8_t addr) noexcept {
	uint8_t data;
	ESP_LOGI(TAG_I2C, "probe addr: 0x%02x\n", addr);
	if(read(addr, 0x00, &data) == i2c_ans::ok)
		return true;
	return false;
}
uint8_t I2C_DRIVER::probe_find(uint8_t addr_start /* = 0 */){
	uint8_t data;
	uint8_t n_sensors = 0;
	for(uint8_t i = addr_start; i < 127; i++) {
		if(read(i, 0x00, &data) == i2c_ans::ok) {
			printf("addr %u: 0x%02x\n", n_sensors++, i);
			return i;
		}
		delay_ms(20);
	}
	ESP_LOGI(TAG_I2C, "nothing found!");
	return 0xFF;
}
void I2C_DRIVER::seek_addr(void) {
	uint8_t data;
	uint8_t n_sensors = 0;
	for(auto i=0; i<127; i++) {
		if(read(i, 0x00, &data) == i2c_ans::ok) {
			printf("addr %u: 0x%02x\n", n_sensors++, i);
		}
	}
}
