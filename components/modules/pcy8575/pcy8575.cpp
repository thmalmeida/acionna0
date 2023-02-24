#include "pcy8575.hpp"

/*
write mode

There are 16 controlled pins

____________.________.________.
slave_addr|0| opcode | byte 1

opcode:
	- Ok:		0x00
	- set pin:	0x01
	- set out: 	0x03
	- set in:	0x04
	- reset: 	0x07

read mode
	- get pin:	0x02 
	- get temp:	0x06


*/

const char* TAG_PCY8575 = "PCY8575";

pcy8575::pcy8575(I2C_Master *i2c) : i2c_(i2c) {

}
void pcy8575::init(uint8_t mode) {

}
// void pcy8575::set_pin(uint8_t pin_number) {

	// uint16_t 
	// i2c_.write(PCY8575_ADDR, PCY8587_SET_PIN, &pin_num, 1)
// }
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
