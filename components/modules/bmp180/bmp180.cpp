#include "bmp180.hpp"

const char* TAG_BMP180 = "BMP180";

BMP180::BMP180(I2C_Driver *i2c) : i2c_(i2c) {
	#ifdef BMP180_DEBUG
	printf("BMP180 status: %u\n", static_cast<uint8_t>(probe()));
	#endif
}
bool BMP180::probe(void) {
	// Return true if it is alive
	return i2c_->probe(BMP180_ADDR);
}
void BMP180::init(void) {
	uint8_t data_MSB, data_LSB;

	// Read calibration data from BMP180 internals EEPROM;
	i2c_->read(BMP180_ADDR, BMP180_REG_AC1_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_AC1_MSB, &data_MSB);
	AC1_ = (data_MSB << 8) | data_LSB;
	#ifdef BMP180_DEBUG
	printf("AC1 = %d\n", AC1_);
	#endif

	i2c_->read(BMP180_ADDR, BMP180_REG_AC2_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_AC2_MSB, &data_MSB);
	AC2_ = (data_MSB << 8) | data_LSB;
	#ifdef BMP180_DEBUG
	printf("AC2 = %d\n", AC2_);
	#endif

	i2c_->read(BMP180_ADDR, BMP180_REG_AC3_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_AC3_MSB, &data_MSB);
	AC3_ = (data_MSB << 8) | data_LSB;
	#ifdef BMP180_DEBUG
	printf("AC3 = %d\n", AC3_);
	#endif

	i2c_->read(BMP180_ADDR, BMP180_REG_AC4_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_AC4_MSB, &data_MSB);
	AC4_ = (data_MSB << 8) | data_LSB;
	#ifdef BMP180_DEBUG
	printf("AC4 = %d\n", AC4_);
	#endif

	i2c_->read(BMP180_ADDR, BMP180_REG_AC5_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_AC5_MSB, &data_MSB);
	AC5_ = (data_MSB << 8) | data_LSB;
	#ifdef BMP180_DEBUG
	printf("AC5 = %d\n", AC5_);
	#endif

	i2c_->read(BMP180_ADDR, BMP180_REG_AC6_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_AC6_MSB, &data_MSB);
	AC6_ = (data_MSB << 8) | data_LSB;
	#ifdef BMP180_DEBUG
	printf("AC6 = %d\n", AC6_);
	#endif

	i2c_->read(BMP180_ADDR, BMP180_REG_B1_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_B1_MSB, &data_MSB);
	B1_ = (data_MSB << 8) | data_LSB;
	#ifdef BMP180_DEBUG
	printf("B1 = %d\n", B1_);
	#endif

	i2c_->read(BMP180_ADDR, BMP180_REG_B2_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_B2_MSB, &data_MSB);
	B2_ = (data_MSB << 8) | data_LSB;
	#ifdef BMP180_DEBUG
	printf("B2 = %d\n", B2_);
	#endif

	i2c_->read(BMP180_ADDR, BMP180_REG_MB_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_MB_MSB, &data_MSB);
	MB_ = (data_MSB << 8) | data_LSB;
	#ifdef BMP180_DEBUG
	printf("MB = %d\n", MB_);
	#endif

	i2c_->read(BMP180_ADDR, BMP180_REG_MC_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_MC_MSB, &data_MSB);
	MC_ = (data_MSB << 8) | data_LSB;
	#ifdef BMP180_DEBUG
	printf("MC = %d\n", MC_);
	#endif

	i2c_->read(BMP180_ADDR, BMP180_REG_MD_LSB, &data_LSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_MD_MSB, &data_MSB);
	MD_ = (data_MSB << 8) | data_LSB;
	#ifdef BMP180_DEBUG
	printf("MD = %d\n", MD_);

	printf("oss: %u, sco: %u, chip id: 0x%02x, \n", oss_(), sco_(), chip_id_());
	#endif
}
int32_t BMP180::altitude(void) {
	// 44330*(1-(p/p0)^(1/5.255))
	return static_cast<int32_t>(44330.0*(1.0-pow(static_cast<double>(pressure_)/static_cast<double>(pressure_sea_level_), 1.0/5.255)));
}
int32_t BMP180::temperature(void) {
	return temperature_;
}
int32_t BMP180::pressure(void) {
	return pressure_;
}
void BMP180::pressure_sea_level(uint32_t pressure, int32_t altitude) {
	// Not tested yet!
	pressure_sea_level_ = static_cast<double>(pressure)/(pow(1.0-altitude/44330, 5.225));
}
void BMP180::fetch(void) {
	temperature_ = calc_true_temperature_(u_temperature_());
	pressure_ = calc_true_pressure_(u_pressure_());
}
void BMP180::soft_reset(void) {
	i2c_->write(BMP180_ADDR, BMP180_REG_RESET);
}

uint16_t BMP180::u_temperature_(void) {
	// write 0x2E (addr temp) into 0x74 (press oss3 addr);
	i2c_->write(BMP180_ADDR, BMP180_REG_PRESS_OSS_3, BMP180_REG_TEMP); 
	
	// wait 4.5 ms
	delay_us(4500);
	// delay_ms(5);

	uint8_t MSB, LSB;
	i2c_->read(BMP180_ADDR, BMP180_REG_OUT_MSB, &MSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_OUT_LSB, &LSB);

	return ((MSB << 8) | LSB);
}
uint32_t BMP180::u_pressure_(void) {
	i2c_->write(BMP180_ADDR, BMP180_REG_CTRL_MEAS, 0x34+(oss_()<<6));

	// Wait 4.5 ms
	delay_us(4500);
	// delay_ms(5);

	uint8_t MSB, LSB, XLSB;
	int pressure;
	i2c_->read(BMP180_ADDR, BMP180_REG_OUT_MSB, &MSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_OUT_LSB, &LSB);
	i2c_->read(BMP180_ADDR, BMP180_REG_OUT_XLSB, &XLSB);

	pressure = ((MSB << 16) + (LSB << 8) + (XLSB)) >> (8 - oss_());

	return pressure;
}
int32_t BMP180::calc_true_temperature_(uint16_t temp) {
	int32_t X1 = (temp-AC6_)*AC5_/32768;
	int32_t X2 = (MC_*2048)/(X1+MD_);
	B5_ = X1 + X2;
	int32_t T = (B5_+8)/16;
	
	#ifdef BMP180_DEBUG
	printf("UT:  X1: %ld, X2: %ld, B5_: %ld\n", X1, X2, B5_);
	#endif

	return T;
}
int32_t BMP180::calc_true_pressure_(uint32_t press) {
	int32_t B6 = B5_ - 4000;
	int32_t X1 = (B2_*(B6*B6/4096))/65536;
	int32_t X2 = AC2_*B6/2048;
	int32_t X3 = X1 + X2;
	int32_t B3 = (((AC1_*4 + X3) << oss_()) + 2)/4;

	#ifdef BMP180_DEBUG
	printf("B6: %ld, X1: %ld, X2: %ld, X3: %ld, B3: %ld\n", B6, X1, X2, X3, B3);
	#endif
	X1 = AC3_*B6/8192;
	X2 = (B1_*(B6*B6/4096))/65536;
	X3 = ((X1+X2)+2)/4;
	uint32_t B4 = AC4_*(X3 + 32768)/32768;
	uint32_t B7 = (press-B3)*(50000 >> oss_());
	if(B7 < 0x80000000) {
		p_ = (B7*2)/B4;
	} else {
		p_ = (B7/B4)*2;
	}

	#ifdef BMP180_DEBUG
	printf("X1: %ld, X2: %ld, X3: %ld, B4: %lu, B7: %lu, p_: %ld\n", X1, X2, X3, B4, B7, p_);
	#endif
	X1 = (p_/256)*(p_/256);
	#ifdef BMP180_DEBUG
	printf("X1: %ld\n", X1);
	#endif
	X1 = (X1*3038)/65536;
	X2 = (-7357*p_)/65536;
	p_ = p_ + (X1+X2+3791)/16;
	#ifdef BMP180_DEBUG
	printf("X1: %ld, X2: %ld, p_: %ld\n", X1, X2, p_);
	#endif

	return p_;
}

void BMP180::oss_(uint8_t value) {
	uint8_t ctrl_meas;
	i2c_->read(BMP180_ADDR, BMP180_REG_CTRL_MEAS, &ctrl_meas);

	ctrl_meas &= 0x3F;	// apply mask to get sco and measurement control bits and clear oss[1:0] to put a new one value;
	ctrl_meas |= (value << 6);

	i2c_->write(BMP180_ADDR, BMP180_REG_CTRL_MEAS, ctrl_meas);
}
uint8_t BMP180::oss_(void) {
	return (ctrl_meas_() >> 6) & 0x03;
}
uint8_t BMP180::sco_(void) {
	return (ctrl_meas_() >> 5) & 0x01;
}

uint8_t BMP180::ctrl_meas_(void) {
	uint8_t ctrl_meas_r;
	i2c_->read(BMP180_ADDR, BMP180_REG_CTRL_MEAS, &ctrl_meas_r);
	return ctrl_meas_r;
}
uint8_t BMP180::chip_id_(void) {
	uint8_t data;
	i2c_->read(BMP180_ADDR, BMP180_REG_ID, &data);

	return data;
}