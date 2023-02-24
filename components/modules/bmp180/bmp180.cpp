#include "bmp180.hpp"

const char* TAG_BMP180 = "BMP180";

void bmp180::probe(void) {
	bool alive = i2c_->probe(BMP180_ADDR);
	// uint8_t data;
	// uint8_t reg_address = 0xD0;
	
	// i2c_->read(slave_address, reg_address, &data, true);

	ESP_LOGI(TAG_BMP180, "BMP180 status: %d\n", alive);
}
void bmp180::init(void) {
	probe();

	uint8_t data_MSB, data_LSB;

	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC1_LSB, &data_LSB, true);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC1_MSB, &data_MSB, true);
	AC1 = (data_MSB << 8) | data_LSB;
	ESP_LOGI(TAG_BMP180, "AC1= %d", AC1);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC2_LSB, &data_LSB, true);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC2_MSB, &data_MSB, true);
	AC2 = (data_MSB << 8) | data_LSB;
	ESP_LOGI(TAG_BMP180, "AC2= %d", AC2);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC3_LSB, &data_LSB, true);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC3_MSB, &data_MSB, true);
	AC3 = (data_MSB << 8) | data_LSB;
	ESP_LOGI(TAG_BMP180, "AC3= %d", AC3);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC4_LSB, &data_LSB, true);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC4_MSB, &data_MSB, true);
	AC4 = (data_MSB << 8) | data_LSB;
	ESP_LOGI(TAG_BMP180, "AC4= %d", AC4);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC5_LSB, &data_LSB, true);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC5_MSB, &data_MSB, true);
	AC5 = (data_MSB << 8) | data_LSB;
	ESP_LOGI(TAG_BMP180, "AC5= %d", AC5);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC6_LSB, &data_LSB, true);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC6_MSB, &data_MSB, true);
	AC6 = (data_MSB << 8) | data_LSB;
	ESP_LOGI(TAG_BMP180, "AC6= %d", AC6);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_B1_LSB, &data_LSB, true);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_B1_MSB, &data_MSB, true);
	B1 = (data_MSB << 8) | data_LSB;
	ESP_LOGI(TAG_BMP180, "B1= %d", B1);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_B2_LSB, &data_LSB, true);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_B2_MSB, &data_MSB, true);
	B2 = (data_MSB << 8) | data_LSB;
	ESP_LOGI(TAG_BMP180, "B2= %d", B2);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_MB_LSB, &data_LSB, true);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_MB_MSB, &data_MSB, true);
	MB = (data_MSB << 8) | data_LSB;
	ESP_LOGI(TAG_BMP180, "MB= %d", MB);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_MC_LSB, &data_LSB, true);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_MC_MSB, &data_MSB, true);
	MC = (data_MSB << 8) | data_LSB;
	ESP_LOGI(TAG_BMP180, "MC= %d", MC);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_MD_LSB, &data_LSB, true);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_MD_MSB, &data_MSB, true);
	MD = (data_MSB << 8) | data_LSB;
	ESP_LOGI(TAG_BMP180, "MD= %d", MD);

}
void bmp180::get_temp(void) {
	// write 0x2E (addr temp) into 0x74 (press oss3 addr);
	i2c_->write(BMP180_ADDR, BMP180_ADDR_PRESS_OSS_3, BMP180_ADDR_TEMP, true); 
	
	// wait 4.5 ms
	delay_us(4500);

	uint8_t data_MSB, data_LSB;
	int temperature;
	i2c_->read(BMP180_ADDR, 0xF6, &data_MSB, true);
	i2c_->read(BMP180_ADDR, 0xF7, &data_LSB, true);
	temperature = (data_MSB << 8) | data_LSB;

	ESP_LOGI(TAG_BMP180, "Temperature: %d", temperature);

}
void bmp180::get_pres(void) {

}
