#include "pcy8575.hpp"

const char* TAG_PCY8575 = "PCY8575";

pcy8575::pcy8575(I2C_Master *i2c) : i2c_(i2c) {
	ESP_LOGI(TAG_PCY8575, "PCY8575 initialized!");
	// ESP_LOGI(TAG_PCY8575, "output_ 0x%04x", output_);
}
void pcy8575::init(uint8_t mode) {
}
bool pcy8575::probe(void) {
	// bool alive = i2c_->probe(PCY8575_ADDR);
	// ESP_LOGI(TAG_PCY8575, "probe: %d", static_cast<int>(alive));
	static int n_bytes = 1;
	uint8_t data[n_bytes];
	int ret = i2c_->read(PCY8575_ADDR, PCY8575_REG_PROBE, &data[0], n_bytes, true);

	for(int i=0; i<n_bytes; i++) {
		ESP_LOGI(TAG_PCY8575, "Probe byte[%d]: 0x%02x",i, data[i]);
	}

	if(ret == 1)
		return true;
	else
		return false;
}
void pcy8575::soft_reset(void) {
	ESP_LOGI(TAG_PCY8575, "cmd soft reset");
	uint8_t data0 = 0x00;
	i2c_->write(PCY8575_ADDR, PCY8575_REG_SOFT_RESET, &data0, 1, true);
	// delay_ms(AHT10_DELAY_SOFT_RESET);
}
void pcy8575::write(int pin, int level) {
	// ESP_LOGI(TAG_PCY8575, "output_0: 0x%04x", output_);
	if(pin) {
		if(level) {
			output_ |=  (1 << (pin-1));
		}
		else {
			output_ &= ~(1 << (pin-1));
		}
	} else {
		if(level) {
			output_ = 0xFFFF;
		} else {
			output_ = 0x0000;
		}
	}
	// ESP_LOGI(TAG_PCY8575, "output_1: 0x%04x", output_);
	put(output_);
}
int pcy8575::read(int pin) {

	// ESP_LOGI(TAG_PCY8575, "READ FUNCTION!");
	output_ = get();

	return (output_ >> (pin-1)) & 1;
	// return (((data[1] << 8) | data[0]) >> (pin-1)) & 1;
}
void pcy8575::put(uint16_t word) {
	
	uint8_t data[2];
	data[0] = word & 0x00FF;
	data[1] = (word >> 8) & 0x00FF;
	output_ = word;

	i2c_->write(PCY8575_ADDR, PCY8575_REG_PUT, &data[0], 2, true);

	// debug
	// ESP_LOGI(TAG_PCY8575, "put: 0x%04x", ((data[1] << 8) | data[0]));
	// for(int i=0; i<2; i++) {
	// 	ESP_LOGI(TAG_PCY8575, "data_tx[%d]: 0x%02x", i, data[i]);
	// }
}
uint16_t pcy8575::get(void) {

	uint8_t data[2];
	i2c_->read(PCY8575_ADDR, PCY8575_REG_GET, &data[0], 2, true);

	// for(int i=0; i<2; i++) {
	// 	ESP_LOGI(TAG_PCY8575, "data_rx[%d]: 0x%02x", i, data[i]);
	// }
	output_ = static_cast<uint16_t>((data[1] << 8) | data[0]);
	return output_;
}
uint16_t pcy8575::temperature(void) {
	
	uint8_t data[2];
	i2c_->read(PCY8575_ADDR, PCY8575_REG_TEMPERATURE, &data[0], 2, true);

	// for(int i=0; i<2; i++) {
	// 	ESP_LOGI(TAG_PCY8575, "data_temp[%d]: 0x%02x", i, data[i]);
	// }
	return static_cast<uint16_t>((data[1] << 8) | data[0]);
}
uint32_t pcy8575::uptime(void) {
	
	uint8_t data[4];
	i2c_->read(PCY8575_ADDR, PCY8575_REG_UPTIME, &data[0], 4, true);

	// for(int i=0; i<sizeof(data); i++) {
	// 	ESP_LOGI(TAG_PCY8575, "data_temp[%d]: 0x%02x", i, data[i]);
	// }
	return static_cast<uint32_t>((data[3] << 24) | (data[2] << 16) | (data[1] << 8) | (data[0]));
}
void pcy8575::i_process(uint8_t mode) {
	uint8_t data = static_cast<uint8_t>(mode);
	i2c_->write(PCY8575_ADDR, PCY8575_REG_I_PROCESS, &data, 1, true);
	// delay_ms(AHT10_DELAY_SOFT_RESET);
}
uint16_t pcy8575::irms(void) {
	uint8_t data[2];
	i2c_->read(PCY8575_ADDR, PCY8575_REG_IRMS, &data[0], 2, true);

	// for(int i=0; i<sizeof(data); i++) {
	// 	ESP_LOGI(TAG_PCY8575, "data_temp[%d]: 0x%02x", i, data[i]);
	// }
	return static_cast<uint16_t>((data[1] << 8) | (data[0]));
}
void pcy8575::i_data(void) {
	int len = N_SAMPLES*2;
	uint8_t data[len];
	i2c_->read(PCY8575_ADDR, PCY8575_REG_I_DATA, &data[0], len, true);

	for(int i=0; i<N_SAMPLES; i++) {
		stream_array_raw[i] = (data[2*i+1] << 8) | (data[2*i]);
	}

	// memcpy(&stream_array_raw[0], (uint16_t*)&data[0], len/2);

	// printf("adc_array_raw[%d] ", len/2);
	// for(int i=0; i<len/2; i++) {
	// 	printf("%u, ", stream_array_raw[i]);
	// }
}
void i_n_points(int length) {

	uint8_t data[2];
	data[0] = length & 0x00FF;
	data[1] = (length >> 8) & 0x00FF;

	i2c_->write(PCY8575_ADDR, PCY8575_REG_I_SET_NP, &data[0], 2, true);
}
uint16_t i_n_points(void) {
	uint8_t data[2];
	i2c_->read(PCY8575_ADDR, PCY8575_REG_I_GET_NP, &data[0], 2, true);
	// for(int i=0; i<2; i++) {
	// 	ESP_LOGI(TAG_PCY8575, "data_temp[%d]: 0x%02x", i, data[i]);
	// }
	return static_cast<uint16_t>((data[1] << 8) | data[0]);
}
void pcy8575::data_test(void) {
	uint8_t data[8];
	uint16_t data16[4];

	i2c_->read(PCY8575_ADDR, PCY8575_REG_TEST, &data[0], 8, true);


	memcpy(&data16[0], (uint16_t*)&data[0], 4);

	for(int i=0; i<4; i++) {
		data16[i] = (data[2*i+1] << 8) | data[2*i];
	}

	for(int i=0; i<8; i++) {
		printf("0x%02x, ", data[i]);
	}
	printf("\n");

	for(int i=0; i<4; i++) {
		printf("0x%04x, ", data16[i]);
	}
	printf("\n");

	// return static_cast<uint16_t>((data[1] << 8) | data[0]);
}
uint8_t pcy8575::reset_reason(void) {
	uint8_t data;

	i2c_->read(PCY8575_ADDR, PCY8575_REG_IRMS, &data);

	return static_cast<uint8_t>(data);
}
