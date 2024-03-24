#include "bmp180.hpp"

const char* TAG_BMP180 = "BMP180";

BMP180::BMP180(I2C_Driver *i2c) : i2c_(i2c) {
	printf("BMP180 status: %u\n", static_cast<uint8_t>(probe()));

	init();
}
bool BMP180::probe(void) {
	// Return true if it is alive
	return i2c_->probe(BMP180_ADDR);
}
void BMP180::init(void) {
	uint8_t data_MSB, data_LSB;

	// Read calibration data from BMP180 internals EEPROM;
	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC1_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC1_MSB, &data_MSB);
	AC1_ = (data_MSB << 8) | data_LSB;
	printf("AC1= %d\n", AC1_);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC2_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC2_MSB, &data_MSB);
	AC2_ = (data_MSB << 8) | data_LSB;
	printf("AC2= %d\n", AC2_);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC3_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC3_MSB, &data_MSB);
	AC3_ = (data_MSB << 8) | data_LSB;
	printf("AC3= %d\n", AC3_);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC4_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC4_MSB, &data_MSB);
	AC4_ = (data_MSB << 8) | data_LSB;
	printf("AC4= %d\n", AC4_);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC5_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC5_MSB, &data_MSB);
	AC5_ = (data_MSB << 8) | data_LSB;
	printf("AC5= %d\n", AC5_);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC6_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_AC6_MSB, &data_MSB);
	AC6_ = (data_MSB << 8) | data_LSB;
	printf("AC6= %d\n", AC6_);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_B1_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_B1_MSB, &data_MSB);
	B1_ = (data_MSB << 8) | data_LSB;
	printf("B1= %d\n", B1_);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_B2_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_B2_MSB, &data_MSB);
	B2_ = (data_MSB << 8) | data_LSB;
	printf("B2= %d\n", B2_);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_MB_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_MB_MSB, &data_MSB);
	MB_ = (data_MSB << 8) | data_LSB;
	printf("MB= %d\n", MB_);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_MC_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_MC_MSB, &data_MSB);
	MC_ = (data_MSB << 8) | data_LSB;
	printf("MC= %d\n", MC_);

	i2c_->read(BMP180_ADDR, BMP180_ADDR_MD_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_ADDR_MD_MSB, &data_MSB);
	MD_ = (data_MSB << 8) | data_LSB;
	printf("MD= %d\n", MD_);

}
int BMP180::altitude(void) {
	return 0;
}
double BMP180::temperature(void) {
	return temperature_;
}
double BMP180::pressure(void) {
	return pressure_;
}
int BMP180::pressure_sea_level(void) {
	return 0;
}
void BMP180::fetch(void) {
	temperature_ = calc_true_temperature_(u_temperature_());
	pressure_ = calc_true_pressure_(u_pressure_());
}
void BMP180::soft_reset(void) {
	i2c_->write(BMP180_ADDR, BMP180_ADDR_SOFT_RST);
}

uint16_t BMP180::u_temperature_(void) {
	// write 0x2E (addr temp) into 0x74 (press oss3 addr);
	i2c_->write(BMP180_ADDR, BMP180_ADDR_PRESS_OSS_3, BMP180_ADDR_TEMP); 
	
	// wait 4.5 ms
	delay_us(4500);
	// delay_ms(5);	// cause error

	uint8_t MSB, LSB;
	i2c_->read(BMP180_ADDR, 0xF6, &MSB);
	i2c_->read(BMP180_ADDR, 0xF7, &LSB);

	return ((MSB << 8) | LSB);
}
uint32_t BMP180::u_pressure_(void) {
	i2c_->write(BMP180_ADDR, BMP180_ADDR_CR, 0x34+(oss_()<<6));

	// Wait
	delay_us(5);

	uint8_t MSB, LSB, XLSB;
	int pressure;
	i2c_->read(BMP180_ADDR, 0xF6, &MSB);
	i2c_->read(BMP180_ADDR, 0xF7, &LSB);
	i2c_->read(BMP180_ADDR, 0xF8, &XLSB);

	pressure = ((MSB << 16) + (LSB << 8) + (XLSB)) >> (8 - oss_());

	return pressure;
}
double BMP180::calc_true_temperature_(uint16_t temp) {
	double X1 = (temp-AC6_)*AC5_/32768.0;
	double X2 = (MC_*2048.0)/(X1+MD_);
	B5_ = X1 + X2;
	double T = (B5_+8.0)/16.0;

	return T;
}
double BMP180::calc_true_pressure_(uint32_t press) {
	double B6 = B5_ - 4000.0;
	double X1 = (B2_*(B6*B6/4096.0))/65536.0;
	double X2 = AC2_*B6/2048.0;
	double X3 = X1 + X2;
	double B3 = ((static_cast<uint32_t>(AC1_*4.0 + X3) << oss_()) + 2.0)/4.0;
	X1 = AC3_*B6/8192.0;
	X2 = (B1_*(B6*B6/4096.0))/65536.0;
	X3 = ((X1+X2)+2.0)/4.0;
	double B4 = AC4_*(static_cast<uint32_t>(X3 + 32768.0))/32768.0;
	double B7 = static_cast<uint32_t>(press-B3)*(50000 >> oss_());
	if(static_cast<uint32_t>(B7) < 0x80000000) {
		p_ = (B7*2.0)/B4;
	} else {
		p_ = (B7/B4)*2.0;
	}

	X1 = (p_/256.0)*(p_/256.0);
	X1 = (X1*3038.0)/65536.0;
	X2 = (-7357.0*p_)/65536.0;
	p_ = p_ + (X1+X2+3791.0)/16.0;

	return p_;
}
void BMP180::oss_(uint8_t value) {
	uint8_t ctrl_meas;
	i2c_->read(BMP180_ADDR, BMP180_ADDR_CR, &ctrl_meas);

	ctrl_meas &= 0x3F;
	ctrl_meas |= (value << 6);

	i2c_->write(BMP180_ADDR, BMP180_ADDR_CR, ctrl_meas);
}
uint8_t BMP180::oss_(void) {
	return ctrl_meas_() >> 6;
}
uint8_t BMP180::sco_(void) {
	return (ctrl_meas_() >> 5) & 0xFE;
}
uint8_t BMP180::ctrl_meas_(void) {
	uint8_t ctrl_meas_r;
	i2c_->read(BMP180_ADDR, BMP180_ADDR_CR, &ctrl_meas_r);
	return ctrl_meas_r;
}
uint8_t BMP180::chip_id_(void) {
	uint8_t data;
	i2c_->read(BMP180_ADDR, BMP180_ADDR_ID, &data);

	return data;
}