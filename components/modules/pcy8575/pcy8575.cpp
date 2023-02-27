#include "pcy8575.hpp"

const char* TAG_PCY8575 = "PCY8575";

pcy8575::pcy8575(I2C_Master *i2c) : i2c_(i2c) {

}
void pcy8575::init(uint8_t mode) {
}
bool pcy8575::probe(void) {

	bool alive = i2c_->probe(PCY8575_ADDR);
	ESP_LOGI(TAG_PCY8575, "probe: %d", static_cast<int>(alive));
	return alive;
}
void pcy8575::soft_reset(void) {
	// ESP_LOGI(TAG_AHT10, "cmd soft reset");
	// i2c_->write(AHT10_ADDR, AHT10_REG_SOFT_RST, true);
	// delay_ms(AHT10_DELAY_SOFT_RESET);
}
void pcy8575::write(int pin, int level) {

	if(level) {
		output_ |= (1 << (pin-1));
	}
	else {
		output_ &= ~(1 << (pin-1));
	}

	uint8_t data[2];
	data[0] = output_;
	data[1] = output_ >> 8;

	i2c_->write(PCY8575_ADDR, PCY8575_REG_PUT, &data[0], 2, true);
}
int pcy8575::read(int pin) {
	return 0;
}
void pcy8575::put(uint16_t word) {
	
	output_ = word;

	uint8_t data[2];
	data[0] = output_;
	data[1] = output_ >> 8;

	i2c_->write(PCY8575_ADDR, PCY8575_REG_PUT, &data[0], 2, true);
}
uint16_t pcy8575::get(void) {
	return 0;
}