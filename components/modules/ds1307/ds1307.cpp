#include "ds1307.hpp"

DS1307::DS1307(I2C_Driver *i2c) : i2c_(i2c) {
	init();
}

bool DS1307::probe(void) {
	return i2c_->probe(DS1307_ADDR);
}

void DS1307::init(void) {

	// 1- data raw fetch read all registers;
	fetch();

	// 2- setup control register for SQW/OUTPUT signal
	ctrl(ds1307_ctrl::sqw_low);

	// 3- Set hour format
	format(ds1307_format::H24);

	// 4- enable RTC;
	enable(1);

	// 4- Set date and time
	// date(2024, 4, 17);
	// time(06, 05, 20);
}
void DS1307::fetch(void) {

	i2c_->read(DS1307_ADDR, DS1307_REG_SECONDS, &data_raw_[0], sizeof(data_raw_));

	sec_ = decode_second_(data_raw_[0]);
	min_ = decode_minute_(data_raw_[1]);
	hour_ = decode_hour_(data_raw_[2]);
	week_day_ = decode_week_day_(data_raw_[3]);
	day_ = decode_day_(data_raw_[4]);
	month_ = decode_month_(data_raw_[5]);
	year_ = decode_year_(data_raw_[6]);
	ctrl_ = data_raw_[7];

	// Read the bit 6 of hour register from 02h addr (DS1307/DS3231)
	if((data_raw_[2] >> 6) & 0x01) {
		format_ = ds1307_format::H12;

		if((data_raw_[2] >> 5) & 0x01)
			period_ = ds1307_period::PM;
		else
			period_ = ds1307_period::AM;
	}
	else
		format_ = ds1307_format::H24;
}
void DS1307::enable(uint8_t status) {
	uint8_t data, CH_bit;
	i2c_->read(DS1307_ADDR, DS1307_REG_SECONDS, &data);
	
	CH_bit = (data >> 7) & 0x01;

	if(status) {
		if (CH_bit) {
			data &= 0x7F;
			i2c_->write(DS1307_ADDR, DS1307_REG_SECONDS, data);
		}
	} else {
		if(!CH_bit) {
			data |= 0x80;
			i2c_->write(DS1307_ADDR, DS1307_REG_SECONDS, data);
		}
	}
}
void DS1307::ctrl(ds1307_ctrl mode) {
	ctrl_ = 0x00;	// clear ctrl buffer
	switch (mode) {
		case ds1307_ctrl::sqw_1_Hz:
			ctrl_ = (1 << 4) | (0 << 0);
			break;

		case ds1307_ctrl::sqw_4096_Hz:
			ctrl_ = (1 << 4) | (1 << 0);
			break;
		
		case ds1307_ctrl::sqw_8192_Hz:
			ctrl_ = (1 << 4) | (2 << 0);
			break;
		
		case ds1307_ctrl::sqw_32768_Hz:
			ctrl_ = (1 << 4) | (3 << 0);
			break;

		case ds1307_ctrl::sqw_low:
			ctrl_ = (0 << 7) | (0 << 4);
			break;

		case ds1307_ctrl::sqw_high:
			ctrl_ = (1 << 7) | (0 << 4);
			break;

		default:
			ctrl_ = (0 << 7) | (0 << 4);	// sqw 0 and out 0
			break;
	}
	i2c_->write(DS1307_ADDR, DS1307_REG_CTRL, ctrl_);
}
void DS1307::format(ds1307_format format) {
	if(format != format_) {
		if(format == ds1307_format::H12)
			data_raw_[2] |= (1<<6);
		else if(format == ds1307_format::H24)
			data_raw_[2] &= ~(1 << 6);

		i2c_->write(DS1307_ADDR, DS1307_REG_HOURS, data_raw_[2]);
	}
}
ds1307_period DS1307::period(void) {
	return period_;
}

// Set functions - update DS1307 registers
void DS1307::date_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
	data_raw_[0] = encode_second_(sec);
	data_raw_[1] = encode_minute_(min);
	data_raw_[2] = encode_hour_(hour);
	data_raw_[3] = encode_week_day_(1);
	data_raw_[4] = encode_day_(day);
	data_raw_[5] = encode_month_(month);
	data_raw_[6] = encode_year_(year);

	i2c_->write(DS1307_ADDR, DS1307_REG_SECONDS, &data_raw_[0], 7);
}
void DS1307::date(uint16_t year, uint8_t month, uint8_t day) {
	data_raw_[4] = encode_day_(day);
	data_raw_[5] = encode_month_(month);
	data_raw_[6] = encode_year_(year);

	i2c_->write(DS1307_ADDR, DS1307_REG_DAY, &data_raw_[4], 3);
}
void DS1307::time(uint8_t hour, uint8_t min, uint8_t sec) {
	data_raw_[0] = encode_second_(sec);
	data_raw_[1] = encode_minute_(min);
	data_raw_[2] = encode_hour_(hour);

	i2c_->write(DS1307_ADDR, DS1307_REG_SECONDS, &data_raw_[0], 3);
}

// Get functions
uint16_t DS1307::year(void) {
	return year_ + DS1307_MILLENNIUM;
}
uint8_t DS1307::month(void) {
	return month_;
}
uint8_t DS1307::day(void) {
	return day_;
}
uint8_t DS1307::week_day(void) {
	return week_day_;
}
uint8_t DS1307::hour(void) {
	return hour_;
}
uint8_t DS1307::minute(void) {
	return min_;
}
uint8_t DS1307::second(void) {
	return sec_;
}

// Decode - convert DS1307 byte format to decimal value
uint8_t DS1307::decode_second_(uint8_t data_raw) {
	return ((data_raw >> 4) & 0x07)*10 + (data_raw & 0x0F);
}
uint8_t DS1307::decode_minute_(uint8_t data_raw) {
	return decode_second_(data_raw);
}
uint8_t DS1307::decode_hour_(uint8_t data_raw) {
	// check 12 or 24 hour format by the bit 6
	if(!((data_raw >> 6) & 0x01))
		return (data_raw >> 4)*10 + (data_raw & 0x0F);
	else 
		return ((data_raw >> 4) & 0x01)*10 + (data_raw & 0x0F);

}
uint8_t DS1307::decode_week_day_(uint8_t data_raw) {
	return data_raw;
}
uint8_t DS1307::decode_day_(uint8_t data_raw) {
	return (data_raw >> 4)*10 + (data_raw & 0x0F);
}
uint8_t DS1307::decode_month_(uint8_t data_raw) {
	return (data_raw >> 4)*10 + (data_raw & 0x0F);
}
uint8_t DS1307::decode_year_(uint8_t data_raw) {
	return (data_raw >> 4)*10 + (data_raw & 0x0F);
}

// Encode - prepare decimal value to register format to DS1307
uint8_t DS1307::encode_second_(uint8_t sec) {
	return (sec/10 << 4) | (sec%10);
}
uint8_t DS1307::encode_minute_(uint8_t min) {
	return (min/10 << 4) | (min%10);
}
uint8_t DS1307::encode_hour_(uint8_t hour) {
	return (hour/10 << 4) | hour%10;
}
uint8_t DS1307::encode_week_day_(uint8_t week) {
	return week%10;
}
uint8_t DS1307::encode_day_(uint8_t day) {
	return (day/10 << 4) | (day%10);
}
uint8_t DS1307::encode_month_(uint8_t month) {
	return (month/10 << 4) | (month%10);
}
uint8_t DS1307::encode_year_(uint16_t year) {
	return ((year-2000)/10 << 4) | ((year-2000)%10);
}
