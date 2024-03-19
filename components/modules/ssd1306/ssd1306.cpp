#include "ssd1306.h"

const char* TAG_SSD1306 = "SSD1306";

ssd1306::ssd1306(I2C_Master *i2c) : i2c_(i2c) {

}
void ssd1306::init(aht10_mode mode) {

	if(first_init_) {
		first_init_ = 0;
		// Power on delay
		delay_ms(AHT10_DELAY_POWER_ON);

		// Soft reset before init
		printf("AHT10 RESET!\n");
		soft_reset();	
	}

	// small delay before any command;
	delay_ms(AHT10_DELAY_CMD);

	// Command to initialize
	uint8_t data_cmd[2];
	uint8_t data_cmd_mode = 0x00;	// suppose to be 0x48 ?
	
	switch(mode) {
		default:
			printf("Init Normal Mode\n");
			data_cmd_mode = (AHT10_INIT_CTRL_NORMAL_MODE | AHT10_INIT_CTRL_CAL_ON);
			break;		

		case aht10_mode::CYCLE_MODE:
			printf("Init Cycle mode\n");
			data_cmd_mode = (AHT10_INIT_CTRL_CYCLE_MODE | AHT10_INIT_CTRL_CAL_ON);
			break;

		case aht10_mode::COMMAND_MODE:
			printf("Init Command Mode\n");
			data_cmd_mode = (AHT10_INIT_CTRL_CMD_MODE | AHT10_INIT_CTRL_CAL_ON);
			break;

		case aht10_mode::CALIBRATE_ONLY_MODE:
			printf("Init Calibrate only\n");
			data_cmd_mode = AHT10_INIT_CTRL_CAL_ON;
			break;
	
		case aht10_mode::A8_MODE:
			printf("Init mode 0xA8\n");
			data_cmd_mode = 0xA8; // some codes show this way
			break;
	}

	data_cmd[0] = data_cmd_mode;
	data_cmd[1] = AHT10_NOP_CTRL;

	i2c_->write(AHT10_ADDR, AHT10_REG_INIT, &data_cmd[0], 2);
}
bool ssd1306::probe(void) {
	bool alive = i2c_->probe(SSD1306_ADDR);
	return alive;
}
void ssd1306::soft_reset(void) {
	// printf("cmd soft reset\n");
	i2c_->write(SSD1306_ADDR, SSD1306_RESET);
	// delay_ms(SSD1306_DELAY_SOFT_RESET);
}
void ssd1306::power(bool state) {
	if(state) {
		i2c_->write(SSD1306_ADDR, SSD1306_DISPLAY_ON);
	} else {
		i2c_->write(SSD1306_ADDR, SSD1306_DISPLAY_OFF);
	}
}



uint8_t aht10::read_status_register(void) {

	delay_ms(AHT10_DELAY_CMD);

	printf("cmd status register\n");
	// way 1
	i2c_->read(AHT10_ADDR, AHT10_REG_READ_STATUS, &status_byte_);
	// -----

	// way 2
	// uint8_t data_cmd = AHT10_NOP_CTRL;
	// i2c_->write(AHT10_ADDR, AHT10_REG_READ_STATUS, &data_cmd, 1, true);
	// delay_ms(75);
	// i2c_->read_only(AHT10_ADDR, &status_byte_, 1, true);
	// -----

	return status_byte_;
}
void aht10::trig_meas(void) {
	
	// Delay before execute some command
	delay_ms(AHT10_DELAY_CMD);

	// Clear vector
	memset(&data_raw_[0], 0, sizeof(data_raw_));

	uint8_t data_cmd[2];
	data_cmd[0] = AHT10_START_MEAS_CTRL;
	data_cmd[1] = AHT10_NOP_CTRL;

	i2c_->write(AHT10_ADDR, AHT10_REG_TRIG_MEAS, &data_cmd[0], 2);

	// wait at least 75 ms
	delay_ms(75);

	// Read only after write. There are six bytes to read
	i2c_->read_only(AHT10_ADDR, &data_raw_[0], 6);

	// refresh status byte
	status_byte_ = data_raw_[0];
	// printf("Trig meas with data_raw_[0]= 0x%02x", data_raw_[0]);
}
bool aht10::get_status_bit(uint8_t bit_select, bool new_read) {

	if(new_read) {
		read_status_register();
	}

	return (status_byte_ & (1 << bit_select));
}

	const int data_len = 6;

	for(int i=0; i<data_len; i++) {
		printf("data[%d]= %d",i, data_raw_[i]);
	}

	printf("humidity: %f, temperature: %f", get_humidity(), get_temperature());
}