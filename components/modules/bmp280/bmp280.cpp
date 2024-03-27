#include "bmp280.hpp"

const char* TAG_BMP280 = "BMP280";

BMP280::BMP280(I2C_Driver *i2c) : i2c_(i2c) {
	#ifdef BMP280_DEBUG
	printf("BMP280 status: %u\n", static_cast<uint8_t>(probe()));
	#endif
}

// user app functions
bool BMP280::probe(void) {
	// Return true if it is alive
	return i2c_->probe(BMP280_ADDR);
}
void BMP280::init(void) {
	uint8_t MSB, LSB;

	// Read calibration data from BMP280 internals EEPROM;
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_T1_LSB, &LSB);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_T1_MSB, &MSB);
	dig_T1_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_T1_ = %d\n", dig_T1_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_T2_LSB, &LSB);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_T2_MSB, &MSB);
	dig_T2_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_T2_ = %d\n", dig_T2_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_T3_LSB, &LSB);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_T3_MSB, &MSB);
	dig_T3_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_T3_ = %d\n", dig_T3_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P1_LSB, &LSB);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P1_MSB, &MSB);
	dig_P1_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P1_ = %d\n", dig_P1_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P2_LSB, &LSB);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P2_MSB, &MSB);
	dig_P2_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P2_ = %d\n", dig_P2_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P3_LSB, &LSB);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P3_MSB, &MSB);
	dig_P3_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P3_ = %d\n", dig_P3_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P4_LSB, &LSB);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P4_MSB, &MSB);
	dig_P4_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P4_ = %d\n", dig_P4_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P5_LSB, &LSB);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P5_MSB, &MSB);
	dig_P5_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P5_ = %d\n", dig_P5_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P6_LSB, &LSB);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P6_MSB, &MSB);
	dig_P6_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P6_ = %d\n", dig_P6_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P7_LSB, &LSB);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P7_MSB, &MSB);
	dig_P7_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P7_ = %d\n", dig_P7_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P8_LSB, &LSB);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P8_MSB, &MSB);
	dig_P8_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P8_ = %d\n", dig_P8_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P9_LSB, &LSB);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P9_MSB, &MSB);
	dig_P9_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P9_ = %d\n", dig_P9_);

	printf("osrs_t: %u, osrs_p: %u, chip id: 0x%02x, \n", osrs_t_(), osrs_p_(), chip_id_());
	#endif
}
int32_t BMP280::altitude(void) {
	// 44330*(1-(p/p0)^(1/5.255))
	return static_cast<int32_t>(44330.0*(1.0-pow(static_cast<double>(pressure_)/static_cast<double>(pressure_sea_level_), 1.0/5.255)));
}
int32_t BMP280::temperature(void) {
	return temperature_;
}
int32_t BMP280::pressure(void) {
	return pressure_;
}
void BMP280::pressure_sea_level(uint32_t pressure, int32_t altitude) {
	// Not tested yet!
	pressure_sea_level_ = static_cast<double>(pressure)/(pow(1.0-altitude/44330, 5.225));
}
void BMP280::fetch(void) {
	temperature_ = calc_true_temperature_(u_temperature_());
	pressure_ = calc_true_pressure_(u_pressure_());
}
void BMP280::reset(void) {
	i2c_->write(BMP280_REG_RESET, BMP280_CMD_RESET);
}

// pressure/temperature uncompensated values
uint32_t BMP280::u_temperature_(void) {
	// write 0x2E (addr temp) into 0x74 (press oss3 addr);
	// i2c_->write(BMP280_ADDR, BMP280_ADDR_PRESS_OSS_3, BMP280_ADDR_TEMP); 
	
	// wait 4.5 ms
	delay_us(4500);
	// delay_ms(5);

	uint8_t XLSB, LSB, MSB;
	i2c_->read(BMP280_ADDR, 0xFC, &XLSB);
	i2c_->read(BMP280_ADDR, 0xF7, &LSB);
	i2c_->read(BMP280_ADDR, 0xF6, &MSB);
	


	return ((MSB << 8) | LSB);
}
uint32_t BMP280::u_pressure_(void) {
	// i2c_->write(BMP280_ADDR, BMP280_REG_CTRL_MEAS, 0x34+(oss_()<<6));

	// Wait 4.5 ms
	delay_us(4500);
	// delay_ms(5);

	uint8_t MSB, LSB, XLSB;
	int pressure = 0;
	i2c_->read(BMP280_ADDR, 0xF6, &MSB);
	i2c_->read(BMP280_ADDR, 0xF7, &LSB);
	i2c_->read(BMP280_ADDR, 0xF8, &XLSB);

	// pressure = ((MSB << 16) + (LSB << 8) + (XLSB)) >> (8 - oss_());

	return pressure;
}
int32_t BMP280::calc_true_temperature_(uint32_t adc_T) {
	// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - (static_cast<int32_t>(dig_T1_)<<1))) * (static_cast<int32_t>(dig_T2_))) >> 11;
	var2 = (((((adc_T>>4) - (static_cast<int32_t>(dig_T1_))) * ((adc_T>>4) - (static_cast<int32_t>(dig_T1_)))) >> 12) * (static_cast<int32_t>(dig_T3_))) >> 14;
	t_fine_ = var1 + var2;
	T = (t_fine_ * 5 + 128) >> 8;
	return T;	
}
int32_t BMP280::calc_true_pressure_(uint32_t adc_P) {
	// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
	// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
	int64_t var1, var2, p;
	var1 = (static_cast<int64_t>(t_fine_)) - 128000;
	var2 = var1 * var1 * static_cast<int64_t>(dig_P6_);
	var2 = var2 + ((var1*static_cast<int64_t>(dig_P5_))<<17);
	var2 = var2 + ((static_cast<int64_t>(dig_P4_))<<35);
	var1 = ((var1 * var1 * static_cast<int64_t>(dig_P3_))>>8) + ((var1 * static_cast<int64_t>(dig_P2_))<<12);
	var1 = ((((static_cast<int64_t>(1))<<47)+var1))*(static_cast<int64_t>(dig_P1_))>>33;

	if (var1 == 0) {
		return 0; // avoid exception caused by division by zero
	}
	
	p = 1048576-adc_P;
	p = (((p<<31)-var2)*3125)/var1;
	var1 = ((static_cast<int64_t>(dig_P9_)) * (p>>13) * (p>>13)) >> 25;
	var2 = ((static_cast<int64_t>(dig_P8_)) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + ((static_cast<int64_t>(dig_P7_))<<4);
	return static_cast<uint32_t>(p);

	// #ifdef BMP280_DEBUG
	// printf("B6: %ld, X1: %ld, X2: %ld, X3: %ld, B3: %ld\n", B6, X1, X2, X3, B3);
	// #endif
}

// bit read/write operations for config register
uint8_t BMP280::t_sb_(void) {
	return (config_() >> 5) & 0x13;
}
void BMP280::t_sb_(uint8_t value) {
	uint8_t config = config_();

	config &= 0x1F;
	config |= (value << 5);

	config_(config);
}
uint8_t BMP280::filter_(void) {
	return (config_() >> 2) & 0x13;
}
void BMP280::filter_(uint8_t value) {
	uint8_t config = config_();

	config &= 0xE3;
	config |= (value << 2);

	config_(config);	
}
uint8_t BMP280::spi3w_en_(void) {
	return (config_() >> 0) & 0x01;
}
void BMP280::spi3w_en_(uint8_t value) {
	uint8_t config = config_();

	config &= 0xFE;
	config |= (value << 0);

	config_(config);	
}

// bit operation for ctrl_meas register
uint8_t BMP280::osrs_t_(void) {
	return ((ctrl_meas_() >> 5) & 0x13);
}
void BMP280::osrs_t_(uint8_t value) {
	uint8_t ctrl_meas = ctrl_meas_();

	ctrl_meas &= 0x1F;	// clear osrs_t bits
	ctrl_meas |= (value << 5);
	
	ctrl_meas_(ctrl_meas);
}
uint8_t BMP280::osrs_p_(void) {
	return ((ctrl_meas_() >> 2) & 0x13);
}
void BMP280::osrs_p_(uint8_t value) {
	uint8_t ctrl_meas = ctrl_meas_();

	ctrl_meas &= 0xE3;	// clear osrs_p bits
	ctrl_meas |= (value << 2);

	ctrl_meas_(ctrl_meas);
}
uint8_t BMP280::mode_(void) {
	return ((ctrl_meas_() >> 0) & 0x03);
}
void BMP280::mode_(uint8_t value) {
	uint8_t ctrl_meas = ctrl_meas_();

	ctrl_meas &= 0xFC;	// clear mode bits
	ctrl_meas |= (value << 0);

	ctrl_meas_(ctrl_meas);
}

// reg read/write operations
uint8_t BMP280::config_(void) {
	uint8_t config;
	i2c_->read(BMP280_ADDR, BMP280_REG_CONFIG, &config);

	return config;	
}
void BMP280::config_(uint8_t value) {
	i2c_->write(BMP280_ADDR, BMP280_REG_CONFIG, value);
}
uint8_t BMP280::ctrl_meas_(void) {
	uint8_t ctrl_meas;
	i2c_->read(BMP280_ADDR, BMP280_REG_CTRL_MEAS, &ctrl_meas);

	return ctrl_meas;
}
void BMP280::ctrl_meas_(uint8_t value) {
	i2c_->write(BMP280_ADDR, BMP280_REG_CTRL_MEAS, value);
}
uint8_t BMP280::status_(void) {
	uint8_t status_reg;
	i2c_->read(BMP280_ADDR, BMP280_REG_STATUS, &status_reg);

	return status_reg;
}

uint8_t BMP280::chip_id_(void) {
	uint8_t data;
	i2c_->read(BMP280_ADDR, BMP280_REG_ID, &data);

	return data;
}