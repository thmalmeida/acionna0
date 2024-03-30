#include "ssd1306.hpp"

const char* TAG_SSD1306 = "SSD1306";

SSD1306::SSD1306(I2C_Driver *i2c) : i2c_(i2c) {

}
void SSD1306::init(void) {

	// 01- Set mux ratio for ...?
	mux_ratio_(SSD1306_CMD_MUX_RATIO_RESET);

	// 02- Set display offset
	set_display_offset_(SSD1306_CMD_DISPLAY_OFFSET_RST);

	// 03- Set display start line
	set_display_start_line_(0);

	// 04- Set segment re-map
	set_segment_remap_(0);

	// 05- set COM Output scan direction
	scan_direction_(0);

	// 06- Set COM Pins hardware configuration
	hardware_config_(1, 1);

	// 07- set contrast control
	set_contrast_(50);

	// 08- disable entire display on
	entire_display_on_(0);

	// 09- set normal display
	set_normal_display_(0);

	// 10- set osc frequency
	set_osc_frequency_(1, 8);

	// 11- charge pump set
	charge_pump_en_(0);

	// 12- display on
	power(1);

}
bool SSD1306::probe(void) {
	bool alive = i2c_->probe(SSD1306_ADDR);
	return alive;
}
void SSD1306::soft_reset(void) {
	// printf("cmd soft reset\n");
	i2c_->write(SSD1306_ADDR, SSD1306_RESET);
	// delay_ms(SSD1306_DELAY_SOFT_RESET);
}
void SSD1306::power(uint8_t x0) {
	uint8_t data[2];
	data[0] = ctrl_byte_(1, 0);
	data[1] = SSD1306_CMD_DISPLAY_POWER | (x0 << 0);

	i2c_->write(SSD1306_ADDR, &data[0], 2);
}
void SSD1306::list_addr(void) {
	// i2c_->probe_find();
	i2c_->probe_list();
}

// Hardware Configuration commands
void SSD1306::mux_ratio_(uint8_t cmd) {
	uint8_t data[3];
	data[0] = ctrl_byte_(1, 0);
	data[1] = SSD1306_REG_MUX_RATIO;
	data[2] = cmd;

	i2c_->write(SSD1306_ADDR, &data[0], 3);
}
void SSD1306::set_display_offset_(uint8_t cmd) {
	uint8_t data[3];
	data[0] = ctrl_byte_(1, 0);
	data[1] = SSD1306_REG_SET_DISPLAY_OFFSET;
	data[2] = cmd;

	i2c_->write(SSD1306_ADDR, &data[0], 3);
}
void SSD1306::set_display_start_line_(uint8_t line) {
	uint8_t data[3];
	data[0] = ctrl_byte_(1, 0);
	data[1] = SSD1306_REG_DISPLAY_START_LINE;
	data[2] = line;

	i2c_->write(SSD1306_ADDR, &data[0], 3);
}
void SSD1306::scan_direction_(uint8_t x3) {
	uint8_t data[2];
	data[0] = ctrl_byte_(1, 0);
	data[1] = SSD1306_REG_SCAN_DIRECTION | (x3 << 3);

	i2c_->write(SSD1306_ADDR, &data[0], 2);
}
void SSD1306::set_segment_remap_(uint8_t x0) {
	uint8_t data[2];
	data[0] = ctrl_byte_(1, 0);
	data[1] = SSD1306_CMD_SEGMENT_REMAP | x0;

	i2c_->write(SSD1306_ADDR, &data[0], 2);
}

void SSD1306::hardware_config_(uint8_t a4, uint8_t a5) {
	uint8_t data[3];
	data[0] = ctrl_byte_(1, 0);
	data[1] = SSD1306_REG_HARDWARE_CONFIG;
	data[2] = 0x02 | (a5 << 1) | (a4 << 4);

	i2c_->write(SSD1306_ADDR, &data[0], 3);
}

// Fundamental commands
void SSD1306::set_contrast_(uint8_t value) {
	uint8_t data[3];
	data[0] = ctrl_byte_(1, 0);
	data[1] = SSD1306_REG_CONTRAST_CONTROL;
	data[2] = value;

	i2c_->write(SSD1306_ADDR, &data[0], 3);
}
void SSD1306::entire_display_on_(uint8_t x0) {
	uint8_t data[2];
	data[0] = ctrl_byte_(1, 0);
	data[1] = SSD1306_REG_ENTIRE_DISPLAY_ON | x0;

	i2c_->write(SSD1306_ADDR, &data[0], 2);
}
void SSD1306::set_normal_display_(uint8_t x0) {
	uint8_t data[2];
	data[0] = ctrl_byte_(1, 0);
	data[1] = SSD1306_CMD_NORMAL_DISPLAY | x0;

	i2c_->write(SSD1306_ADDR, &data[0], 2);
}
uint8_t SSD1306::ctrl_byte_(uint8_t co, uint8_t dc) {
	return ((co << 7) | (dc << 6)) & 0xC0;
}

// Timing & Driving Scheme Setting Command Table
void SSD1306::set_osc_frequency_(uint8_t div, uint8_t freq_type) {
	uint8_t data[3];
	data[0] = ctrl_byte_(1, 0);
	data[1] = SSD1306_CMD_CLK_DIV;
	data[2] = (freq_type << 4) | (div << 0);

	i2c_->write(SSD1306_ADDR, &data[0], 3);
}

void SSD1306::charge_pump_en_(uint8_t a2) {
	uint8_t data[3];
	data[0] = ctrl_byte_(1, 0);
	data[1] = SSD1306_CMD_CHARGE_PUMP;
	data[2] = 0x10 | (a2 << 2);

	i2c_->write(SSD1306_ADDR, &data[0], 3);
}

// 1000 8
// 1001 9
// 1010 A
// 1011 B
// 1100 C
// 1101 D
// 1110 E
// 1111 F


// uint8_t aht10::read_status_register(void) {

// 	delay_ms(AHT10_DELAY_CMD);

// 	printf("cmd status register\n");
// 	// way 1
// 	i2c_->read(AHT10_ADDR, AHT10_REG_READ_STATUS, &status_byte_);
// 	// -----

// 	// way 2
// 	// uint8_t data_cmd = AHT10_NOP_CTRL;
// 	// i2c_->write(AHT10_ADDR, AHT10_REG_READ_STATUS, &data_cmd, 1, true);
// 	// delay_ms(75);
// 	// i2c_->read_only(AHT10_ADDR, &status_byte_, 1, true);
// 	// -----

// 	return status_byte_;
// }
// void aht10::trig_meas(void) {
	
// 	// Delay before execute some command
// 	delay_ms(AHT10_DELAY_CMD);

// 	// Clear vector
// 	memset(&data_raw_[0], 0, sizeof(data_raw_));

// 	uint8_t data_cmd[2];
// 	data_cmd[0] = AHT10_START_MEAS_CTRL;
// 	data_cmd[1] = AHT10_NOP_CTRL;

// 	i2c_->write(AHT10_ADDR, AHT10_REG_TRIG_MEAS, &data_cmd[0], 2);

// 	// wait at least 75 ms
// 	delay_ms(75);

// 	// Read only after write. There are six bytes to read
// 	i2c_->read_only(AHT10_ADDR, &data_raw_[0], 6);

// 	// refresh status byte
// 	status_byte_ = data_raw_[0];
// 	// printf("Trig meas with data_raw_[0]= 0x%02x", data_raw_[0]);
// }
// bool aht10::get_status_bit(uint8_t bit_select, bool new_read) {

// 	if(new_read) {
// 		read_status_register();
// 	}

// 	return (status_byte_ & (1 << bit_select));
// }

// 	const int data_len = 6;

// 	for(int i=0; i<data_len; i++) {
// 		printf("data[%d]= %d",i, data_raw_[i]);
// 	}

// 	printf("humidity: %f, temperature: %f", get_humidity(), get_temperature());
// }