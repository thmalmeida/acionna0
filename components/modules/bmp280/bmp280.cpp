#include "bmp280.hpp"

const char* TAG_BMP280 = "BMP280";

BMP280::BMP280(I2C_Driver *i2c) : i2c_(i2c) {
	#ifdef BMP280_DEBUG
	printf("BMP280 status: %u\n", static_cast<uint8_t>(probe()));
	#endif
	delay_ms(BMP280_DELAY_INIT_MS);
}

// user app functions
bool BMP280::probe(void) {
	// Return true if it is alive
	bool ret = i2c_->probe(BMP280_ADDR);
	delay_ms(BMP280_DELAY_READ_MS);
	return ret;
}
void BMP280::init(void) {

	// Read eeprom calibration values;
	read_burst_calib();
	// read_calib();

	// Configuration and control registers setup
	// ctrl_meas register
	osrs_t_(2);		// 010 - 2x oversampling, 17 bit (page 26/49);
	osrs_p_(5);		// 101 - 16x oversampling, 20 bit;
	mode_(3);		// 11 - normal mode (table 10, page 15/49);
	// config register set
	t_sb_(2);		// 001 - t_standby = 62.5 ms (table 11, pag. 17/49);
	filter_(2);		// 2   - /4 (table 6, pag 14/49); 

	// Debug purposes
	#ifdef BMP280_DEBUG
	printf("dig_P9_ = %d\n", dig_P9_);

	printf("osrs_t:%u, osrs_p:%u, mode_:%u, t_sb_:%u, filter_:%u, chip id: 0x%02x, \n",
				osrs_t_(),
				osrs_p_(),
				mode_(),
				t_sb_(),
				filter_(),
				chip_id_());
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
	read_burst_adc();
	temperature_ = calc_true_temperature_(u_temperature_);
	pressure_ = calc_true_pressure_(u_pressure_);

	// u_pressure_read_();
	// u_temperature_read_();

	printf("osrs_t:%u, osrs_p:%u, mode_:%u, t_sb_:%u, filter_:%u, chip id: 0x%02x, \n",
			osrs_t_(),
			osrs_p_(),
			mode_(),
			t_sb_(),
			filter_(),
			chip_id_());
}
void BMP280::reset(void) {
	i2c_->write(BMP280_REG_RESET, BMP280_CMD_RESET);
	delay_ms(BMP280_DELAY_WRITE_MS);
}

i2c_ans BMP280::read_burst_reg(void) {
	uint8_t data[3];

	i2c_ans ret = i2c_->read(BMP280_ADDR, BMP280_REG_STATUS, &data[0], 3);
	delay_ms(BMP280_DELAY_READ_MS);

	for(int i=0; i<3; i++) {
		printf("data[%d]: 0x%02x\n", i, data[i]);
	}

	if(ret == i2c_ans::ok) {
		status_reg_ = data[0];
		ctrl_meas_reg_ = data[1];
		config_reg_ = data[2];
	} else {
		printf("error read\n");
	}

	return ret;
}
i2c_ans BMP280::read_burst_adc(void) {
	uint8_t data[6];

	i2c_ans ret = i2c_->read(BMP280_ADDR, BMP280_REG_PRESS_MSB, &data[0], 6);
	delay_ms(BMP280_DELAY_READ_MS);

	for(int i=0; i<6; i++) {
		printf("data[%d]: 0x%02x\n", i, data[i]);
	}

	if(ret == i2c_ans::ok) {
		u_pressure_ = 		(data[0] << 12) | (data[1] << 4) | (data[2] >> 4);
		u_temperature_ =	(data[3] << 12) | (data[4] << 4) | (data[5] >> 4);
	} else {
		printf("error read\n");
	}

	return ret;
}
i2c_ans BMP280::read_burst_calib(void) {
	uint8_t data[26];

	i2c_ans ret = i2c_->read(BMP280_ADDR, BMP280_REG_DIG_T1_LSB, &data[0], 26);
	delay_ms(BMP280_DELAY_READ_MS);

	dig_T1_ = (data[1] << 8) | data[0];
	dig_T2_ = (data[3] << 8) | data[2];
	dig_T3_ = (data[5] << 8) | data[4];
	dig_P1_ = (data[7] << 8) | data[6];
	dig_P2_ = (data[9] << 8) | data[8];
	dig_P3_ = (data[11] << 8) | data[10];
	dig_P4_ = (data[13] << 8) | data[12];
	dig_P5_ = (data[15] << 8) | data[14];
	dig_P6_ = (data[17] << 8) | data[16];
	dig_P7_ = (data[19] << 8) | data[18];
	dig_P8_ = (data[21] << 8) | data[20];
	dig_P9_ = (data[23] << 8) | data[22];

	#ifdef BMP280_DEBUG
	for(int i=0; i<24; i+=2) {
		printf("%d: 0x%02x 0x%02x = %d\n", i, data[i+1], data[i], static_cast<int16_t>((data[i+1] << 8) | data[i]));
	}	
	// printf("dig_T1_ = %d\n", dig_T1_);
	// printf("dig_T2_ = %d\n", dig_T2_);
	// printf("dig_T3_ = %d\n", dig_T3_);
	// printf("dig_P1_ = %d\n", dig_P1_);
	#endif
	return ret;
}
void BMP280::read_calib(void) {
	uint8_t MSB, LSB;
	// Read calibration data from BMP280 internals EEPROM;
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_T1_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_T1_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	dig_T1_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_T1_ = %d\n", dig_T1_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_T2_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_T2_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	dig_T2_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_T2_ = %d\n", dig_T2_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_T3_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_T3_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	dig_T3_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_T3_ = %d\n", dig_T3_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P1_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P1_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	dig_P1_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P1_ = %d\n", dig_P1_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P2_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P2_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	dig_P2_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P2_ = %d\n", dig_P2_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P3_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P3_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	dig_P3_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P3_ = %d\n", dig_P3_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P4_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P4_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	dig_P4_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P4_ = %d\n", dig_P4_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P5_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P5_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	dig_P5_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P5_ = %d\n", dig_P5_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P6_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P6_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	dig_P6_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P6_ = %d\n", dig_P6_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P7_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P7_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	dig_P7_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P7_ = %d\n", dig_P7_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P8_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P8_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	dig_P8_ = (MSB << 8) | LSB;
	#ifdef BMP280_DEBUG
	printf("dig_P8_ = %d\n", dig_P8_);
	#endif

	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P9_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_DIG_P9_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	dig_P9_ = (MSB << 8) | LSB;	
}

int32_t BMP280::u_pressure_read_(void) {
	// i2c_->write(BMP280_ADDR, BMP280_REG_CTRL_MEAS, 0x34+(oss_()<<6));
	// Wait 4.5 ms
	// delay_us(4500);

	uint8_t MSB, LSB, XLSB;
	i2c_->read(BMP280_ADDR, BMP280_REG_PRESS_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_PRESS_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_PRESS_XLSB, &XLSB);
	delay_ms(BMP280_DELAY_READ_MS);

	int32_t u_press = (MSB << 12) | (LSB << 4) | (XLSB >> 4);

	#ifdef BMP280_DEBUG
	printf("simple read: 0x%02x 0x%02x 0x%02x\n", MSB, LSB, XLSB);
	printf("up: %ld, 0x%08lx\n", u_press, u_press);
	#endif
	return u_press;
}
// pressure/temperature uncompensated values
int32_t BMP280::u_temperature_read_(void) {
	// write 0x2E (addr temp) into 0x74 (press oss3 addr);
	// i2c_->write(BMP280_ADDR, BMP280_ADDR_PRESS_OSS_3, BMP280_ADDR_TEMP); 
		// wait 4.5 ms
	// delay_us(4500);
	// delay_ms(5);

	uint8_t XLSB, LSB, MSB;
	i2c_->read(BMP280_ADDR, BMP280_REG_TEMP_MSB, &MSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_TEMP_LSB, &LSB);
	delay_ms(BMP280_DELAY_READ_MS);
	i2c_->read(BMP280_ADDR, BMP280_REG_TEMP_XLSB, &XLSB);
	delay_ms(BMP280_DELAY_READ_MS);

	int32_t u_temp = (MSB << 12) | (LSB << 4) | (XLSB >> 4);

	#ifdef BMP280_DEBUG
	printf("simple read: 0x%02x 0x%02x 0x%02x\n", MSB, LSB, XLSB);
	printf("ut: %ld, 0x%08lx\n", u_temp, u_temp);
	#endif

	return (u_temp);
}
int32_t BMP280::calc_true_temperature_(int32_t adc_T) {
	// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
	int32_t var1, var2, T;
	var1 = ((((adc_T>>3) - (static_cast<int32_t>(dig_T1_)<<1))) * (static_cast<int32_t>(dig_T2_))) >> 11;
	var2 = (((((adc_T>>4) - (static_cast<int32_t>(dig_T1_))) * ((adc_T>>4) - (static_cast<int32_t>(dig_T1_)))) >> 12) * (static_cast<int32_t>(dig_T3_))) >> 14;
	t_fine_ = var1 + var2;
	T = (t_fine_ * 5 + 128) >> 8;
	return T;	
}
int32_t BMP280::calc_true_pressure_(int32_t adc_P) {
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
	return (config_() >> 5) & 0x07;
}
void BMP280::t_sb_(uint8_t value) {
	uint8_t config = config_();

	config &= 0x1F;
	config |= (value << 5);

	config_(config);
}
uint8_t BMP280::filter_(void) {
	return (config_() >> 2) & 0x07;
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
	return (0x07 & (ctrl_meas_() >> 5));
}
void BMP280::osrs_t_(uint8_t value) {
	uint8_t ctrl_meas = ctrl_meas_();

	printf("osrs_t_ - ctrl_meas: 0x%02x\n", ctrl_meas);
	ctrl_meas &= 0x1F;	// 0b 0001 1111 - clear osrs_t bits
	printf("osrs_t_ - ctrl_meas: 0x%02x\n", ctrl_meas);
	// ctrl_meas &= ~0xE0;
	// ctrl_meas = ctrl_meas | (value << 5);
	ctrl_meas |= (value << 5);
	printf("osrs_t_ - ctrl_meas: 0x%02x\n", ctrl_meas);
	
	ctrl_meas_(ctrl_meas);
}
uint8_t BMP280::osrs_p_(void) {
	return ((ctrl_meas_() >> 2) & 0x07);
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
uint8_t BMP280::status_(void) {
	uint8_t status_reg;
	i2c_->read(BMP280_ADDR, BMP280_REG_STATUS, &status_reg);
	delay_ms(BMP280_DELAY_READ_MS);

	return status_reg;
}
uint8_t BMP280::ctrl_meas_(void) {
	uint8_t ctrl_meas;
	i2c_->read(BMP280_ADDR, BMP280_REG_CTRL_MEAS, &ctrl_meas);
	delay_ms(BMP280_DELAY_READ_MS);
	return ctrl_meas;
}
void BMP280::ctrl_meas_(uint8_t value) {
	i2c_->write(BMP280_ADDR, BMP280_REG_CTRL_MEAS, value);
	delay_ms(BMP280_DELAY_WRITE_MS);
}
uint8_t BMP280::config_(void) {
	uint8_t config;
	i2c_->read(BMP280_ADDR, BMP280_REG_CONFIG, &config);
	delay_ms(BMP280_DELAY_READ_MS);
	return config;	
}
void BMP280::config_(uint8_t value) {
	i2c_->write(BMP280_ADDR, BMP280_REG_CONFIG, value);
	delay_ms(BMP280_DELAY_WRITE_MS);
}

uint8_t BMP280::chip_id_(void) {
	uint8_t data;
	i2c_->read(BMP280_ADDR, BMP280_REG_ID, &data);
	delay_ms(BMP280_DELAY_READ_MS);

	return data;
}