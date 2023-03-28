#include "i2c_master.hpp"

//#define I2C_WRITE_FLAG	0
//#define I2C_READ_FLAG		1

static const char *TAG_I2C = "I2C";

I2C_Master::I2C_Master(int port, int scl, int sda, uint32_t freq, bool pull_up /* = false */) : i2c_master_port_(port) {
	// Configuration
	i2c_config_t conf = {};
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = sda;
	conf.sda_pullup_en = pull_up ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
	conf.scl_io_num = scl;
	conf.scl_pullup_en = pull_up ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE;
	conf.master.clk_speed = freq; // I2C_FAST_SPEED_HZ or I2C_NORMAL_SPEED_HZ;

	esp_err_t err = i2c_param_config(static_cast<i2c_port_t>(i2c_master_port_), &conf);
	if(err != ESP_OK) {
		ESP_LOGI(TAG_I2C, "I2C config error [%d/%s]\n", err, esp_err_to_name(err));
	}

	// Install driver
	int intr_alloc_flags = 0;
	err = i2c_driver_install(static_cast<i2c_port_t>(i2c_master_port_), conf.mode, 0, 0, intr_alloc_flags);

	if(err != ESP_OK) { 
		ESP_LOGE(TAG_I2C, "I2C install error [%d/%s]\n", err, esp_err_to_name(err));
	}
}
I2C_Master::~I2C_Master() {
	i2c_driver_delete(static_cast<i2c_port_t>(i2c_master_port_));
}
int I2C_Master::write(uint8_t slave_addr, uint8_t reg, uint8_t* data, size_t len, bool ack_check) {
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
		ESP_LOGI(TAG_I2C, "read: error on write: %s", esp_err_to_name(ret));
		return I2C_ERR_WRITE;
	}

	return I2C_ERR_OK;
}
int I2C_Master::write(uint8_t slave_addr, uint8_t reg, uint8_t data, bool ack_check /* = true */){
	return write(slave_addr, reg, &data, 1, ack_check);
}
int I2C_Master::write(uint8_t slave_addr, uint8_t reg, bool ack_check /* = true */){
	uint8_t data;
	return write(slave_addr, reg, &data, 0, ack_check);
}
int I2C_Master::write(uint8_t slave_addr, uint8_t *data, uint8_t data_len) {
	
	esp_err_t ret = i2c_master_write_to_device(static_cast<i2c_port_t>(i2c_master_port_), slave_addr, data, data_len, I2C_COMMAND_WAIT_MS / portTICK_PERIOD_MS);

	if (ret != ESP_OK) {
		ESP_LOGI(TAG_I2C, "Error on write2");
		return I2C_ERR_WRITE;
	}

	return I2C_ERR_OK;
}
int I2C_Master::set_mask(uint8_t slave_addr, uint8_t reg, uint8_t data, uint8_t mask, bool ack_check /* = true */) {
	uint8_t content;

	int ret = read(slave_addr, reg, &content, ack_check);
	if(ret < 0){
		return ret;
	}

	content &= ~mask;
	content |= data & mask;

	return write(slave_addr, reg, content, ack_check);
}
int I2C_Master::read(uint8_t slave_addr, uint8_t reg, uint8_t* data, size_t len, bool ack_check) {
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
		ESP_LOGI(TAG_I2C, "read: error on write: %s", esp_err_to_name(ret));
		return I2C_ERR_WRITE;
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
		ESP_LOGI(TAG_I2C, "read: error on read: %s", esp_err_to_name(ret));
		return I2C_ERR_READ;
	}

	return I2C_ERR_OK;
}
int I2C_Master::read(uint8_t slave_addr, uint8_t reg, uint8_t* data, bool ack_check /* = true */) {
	return read(slave_addr, reg, data, 1, ack_check);
}
int read(uint8_t slave_addr, uint8_t reg, uint8_t* data) {
	return read(slave_addr, reg, data, 1, true);
}
int I2C_Master::read(uint8_t slave_address, const uint8_t *write_buffer, size_t write_buffer_len, uint8_t *read_buffer, size_t read_buffer_len) {

	esp_err_t ret = i2c_master_write_read_device(static_cast<i2c_port_t>(i2c_master_port_), slave_address, write_buffer, write_buffer_len, read_buffer, read_buffer_len, I2C_COMMAND_WAIT_MS / portTICK_PERIOD_MS);

	if(ret != ESP_OK) {
		ESP_LOGI(TAG_I2C, "read2: error on read: %s", esp_err_to_name(ret));
		return I2C_ERR_READ;
	}

	return I2C_ERR_OK;
}
int I2C_Master::read_only(uint8_t slave_addr, uint8_t* data, size_t data_len, bool ack_check) {

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
		ESP_LOGI(TAG_I2C, "read only: error on read: %s", esp_err_to_name(ret));
		return I2C_ERR_READ;
	}

	return I2C_ERR_OK;
}
bool I2C_Master::probe(uint8_t addr) noexcept
{
	uint8_t data;
	ESP_LOGI(TAG_I2C, "probe addr: 0x%02x\n", addr);
	if(read(addr, 0x00, &data) > 0)
		return true;
	return false;
}
uint8_t I2C_Master::probe_find(uint8_t addr_start /* = 0 */){
	uint8_t data;
	for(uint8_t i = addr_start; i < 127; i++) {
		ESP_LOGI(TAG_I2C, "addr: 0x%02x", i);
		if(read(i, 0x00, &data) > 0) {
			return i;
		}
		delay_ms(20);
	}
	ESP_LOGI(TAG_I2C, "nothing found!");
	return 0xFF;
}
