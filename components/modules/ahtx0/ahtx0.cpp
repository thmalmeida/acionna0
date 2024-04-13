#include "ahtx0.hpp"

const char* TAG_AHT10 = "AHTX0";

AHTX0::AHTX0(I2C_Driver *i2c, ahtx0_model model) : i2c_(i2c) {
	// wait stablish the power connections
	switch (model) {
		default:
			ahtx_reg_init_addr_ = AHT10_REG_INIT;
			data_len_ = 6;	// don't use CRC
			break;
		case ahtx0_model::aht20:
			ahtx_reg_init_addr_ = AHT20_REG_INIT;
			data_len_ = 7;	// last byte is the CRC
			break;
	}
}
void AHTX0::init(ahtx0_mode mode) {
	if(first_init_) {
		first_init_ = 0;
		// Power on delay
		delay_ms(AHTX0_DELAY_POWER_ON);

		// Soft reset before init
		#ifdef AHTX_DEBUG
		printf("AHTX0 RESET!\n");
		#endif
		reset();

		// Power on delay
		delay_ms(AHTX0_DELAY_POWER_ON);
	}

	// sequence data to initialize register command
	uint8_t data_cmd[2];
	uint8_t data_cmd_mode = 0x00;	// suppose to be 0x48 ?
	
	switch(mode) {
		default:
			#ifdef AHTX0_DEBUG
			printf("Init Normal Mode\n");
			#endif
			data_cmd_mode = (AHT10_INIT_CTRL_NORMAL_MODE | AHTX0_INIT_CTRL_CAL_ON);
			break;		

		case ahtx0_mode::CYCLE_MODE:
			#ifdef AHTX0_DEBUG
			printf("Init Cycle mode\n");
			#endif
			data_cmd_mode = (AHT10_INIT_CTRL_CYCLE_MODE | AHTX0_INIT_CTRL_CAL_ON);
			break;

		case ahtx0_mode::COMMAND_MODE:
			#ifdef AHTX0_DEBUG
			printf("Init Command Mode\n");
			#endif
			data_cmd_mode = (AHT10_INIT_CTRL_CMD_MODE | AHTX0_INIT_CTRL_CAL_ON);
			break;

		case ahtx0_mode::CALIBRATE_ONLY_MODE:
			#ifdef AHTX0_DEBUG
			printf("Init Calibrate only\n");
			#endif
			data_cmd_mode = AHTX0_INIT_CTRL_CAL_ON;
			break;
	
		case ahtx0_mode::A8_MODE:
			#ifdef AHTX0_DEBUG
			printf("Init mode 0xA8\n");
			#endif
			data_cmd_mode = 0xA8; // some codes show this way
			break;
	}

	data_cmd[0] = data_cmd_mode;
	data_cmd[1] = AHTX0_NOP_CTRL;

	uint8_t cal_en_bit = calibrated_();
	#ifdef AHTX0_DEBUG
	printf("CAL Enable bit: %u\n", cal_en_bit);
	#endif

	if(!cal_en_bit) {
		// small delay before any command;
		delay_ms(AHTX0_DELAY_CMD);
		i2c_->write(AHTX0_ADDR, ahtx_reg_init_addr_, &data_cmd[0], 2);
		delay_ms(AHTX0_DELAY_CMD);
	}
}
bool AHTX0::probe(void) {
	
	delay_ms(AHTX0_DELAY_CMD);

	bool alive = i2c_->probe(AHTX0_ADDR);
	// printf("probe: %d", static_cast<int>(alive));
	return alive;
}
void AHTX0::reset(void) {
	#ifdef AHTX0_DEBUG
	printf("cmd soft reset\n");
	#endif
	i2c_->write(AHTX0_ADDR, AHTX0_REG_SOFT_RST);
	delay_ms(AHTX0_DELAY_SOFT_RESET);
}
float AHTX0::humidity(void) {
	uint32_t humidity   = data_raw_[1];                          //20-bit raw humidity data
	humidity <<= 8;
	humidity  |= data_raw_[2];
	humidity <<= 4;
	humidity  |= data_raw_[3] >> 4;

	if (humidity > 0x100000) {humidity = 0x100000;}             //check if RH>100, no need to check for RH<0 since "humidity" is "uint"

	return ((float)humidity / 0x100000) * 100;
}
float AHTX0::temperature(void) {
	uint32_t temperature   = data_raw_[3] & 0x0F;                //20-bit raw temperature data
	temperature <<= 8;
	temperature  |= data_raw_[4];
	temperature <<= 8;
	temperature  |= data_raw_[5];

	return ((float)temperature / 0x100000) * 200 - 50;
}
void AHTX0::fetch(void) {
	
	// Delay before execute some command
	delay_ms(AHTX0_DELAY_CMD);

	// Clear vector
	memset(&data_raw_[0], 0, sizeof(data_raw_));

	uint8_t data_cmd[2];
	data_cmd[0] = AHTX0_START_MEAS_CTRL;
	data_cmd[1] = AHTX0_NOP_CTRL;

	if(i2c_->write(AHTX0_ADDR, AHTX0_REG_TRIG_MEAS, &data_cmd[0], 2) != i2c_ans::ok) {
		#ifdef AHTX0_DEBUG
		printf("Error trig write!\n");
		#endif
	}

	// wait at least 75 ms
	delay_ms(AHTX0_DELAY_MEASUREMENT);

	// Read only after write. There are six bytes to read
	if(i2c_->read(AHTX0_ADDR, &data_raw_[0], data_len_) != i2c_ans::ok) {
		#ifdef AHTX0_DEBUG
		printf("Error trig read!\n");
		#endif
	}

	// refresh status byte
	status_byte_ = data_raw_[0];
	// printf("Trig meas with data_raw_[0]= 0x%02x", data_raw_[0]);
}
uint8_t AHTX0::status_(void) {

	delay_ms(AHTX0_DELAY_CMD);

	// way 1
	i2c_->read(AHTX0_ADDR, AHTX0_REG_STATUS, &status_byte_);
	
	// -----

	// way 2
	// uint8_t data_cmd = AHT10_NOP_CTRL;
	// i2c_->write(AHTX0_ADDR, AHTX0_REG_STATUS, &data_cmd, 1, true);
	// delay_ms(75);
	// i2c_->read_only(AHTX0_ADDR, &status_byte_, 1, true);
	// -----

	return status_byte_;
}
uint8_t AHTX0::busy_(void) {
	return (status_() >> 7) & 0x01;	
}
uint8_t AHTX0::calibrated_(void) {
	return (status_() >> 3) & 0x01;
}
void AHTX0::crc_check_(void) {

}

// Debug functions
void AHTX0::print_status_bits(void) {
	int i, n, status_byte[8];
	memset(&status_byte[0], 0, sizeof(status_byte));
	// n = data_raw_[0];

	n = status_byte_;
	for(i=0; n>0; i++) {
		status_byte[i] = n % 2;
		n = n / 2;
	}

	char buffer[40], buffer_temp[2];
	sprintf(buffer, "status: %d, 0b", status_byte_);
	for(i=7; i>=0; i--) {
		sprintf(buffer_temp, "%d", status_byte[i]);
		strcat(buffer, buffer_temp);
	}
	printf("%s", buffer);
}
void AHTX0::print_raw_data(void) {

	for(int i=0; i<data_len_; i++) {
		printf("data[%d]= %d",i, data_raw_[i]);
	}

	printf("humidity: %f, temperature: %f", humidity(), temperature());
}
// bool AHTX0::get_status_bit(uint8_t bit_select, bool new_read) {

// 	if(new_read) {
// 		status_();
// 	}

// 	return (status_byte_ & (1 << bit_select));
// }
