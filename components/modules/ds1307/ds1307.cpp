#include "ds1307.hpp"

DS1307::DS1307(I2C_Driver *i2c) : i2c_(i2c) {

}

void DS1307::init(void) {

	// 1- data raw fetch get read all registers;
	fetch();

	// 2- enable RTC;
	enable(1);


}
void DS1307::fetch(void) {

	i2c_->read(DS1307_ADDR, DS1307_REG_SECONDS, &data_raw_[0], sizeof(data_raw_));

	sec_ = decode_second_(data_raw_[0]);
	min_ = decode_minute_(data_raw_[1]);
	hour_ = decode_hour_(data_raw_[2]);
	week_ = decode_week_(data_raw_[3]);
	day_ = decode_day_(data_raw_[4]);
	month_ = decode_month_(data_raw_[5]);
	year_ = decode_year_(data_raw_[6]);
	ctrl_ = data_raw_[7];
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

uint8_t DS1307::decode_second_(uint8_t data_raw) {
	return ((data_raw >> 4) & 0x07)*10 + (data_raw & 0x0F);
}
uint8_t DS1307::decode_minute_(uint8_t data_raw) {
	return decode_second_(data_raw);
}
uint8_t DS1307::decode_hour_(uint8_t data_raw) {
	// check 12 or 24 hour format by the bit 6
	// if((data_raw >> 6) & 0x01) {
		// data_
	// }
	return (data_raw >> 4)*10 + (data_raw & 0x0F);
}
uint8_t DS1307::decode_week_(uint8_t data_raw) {
	return data_raw;
}
uint8_t DS1307::decode_day_(uint8_t data_raw) {
	return (data_raw >> 4)*10 + (data_raw & 0x0F);
}
uint8_t DS1307::decode_month_(uint8_t data_raw) {
	return (data_raw >> 4)*10 + (data_raw & 0x0F);
}
uint16_t DS1307::decode_year_(uint8_t data_raw) {
	return (data_raw >> 4)*10 + (data_raw & 0x0F) + 2000;
}

uint8_t DS1307::encode_second_(uint8_t sec) {
	return (sec/10 << 4) | (sec%10);
}
uint8_t DS1307::encode_minute_(uint8_t min) {
	return (min/10 << 4) | (min%10);
}
uint8_t DS1307::encode_hour_(uint8_t hour) {
	return (hour/10 << 4) | hour%10;
}
uint8_t DS1307::encode_week_(uint8_t week) {
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

void DS1307::date_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
	data_raw_[0] = encode_second_(sec);
	data_raw_[1] = encode_minute_(min);
	data_raw_[2] = encode_hour_(hour);
	data_raw_[3] = encode_week_(1);
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
uint8_t DS1307::second(void) {
	return sec_;
}
uint8_t DS1307::minute(void) {
	return min_;
}
uint8_t DS1307::hour(void) {
	return hour_;
}
uint8_t DS1307::week(void) {
	return week_;
}
uint8_t DS1307::day(void) {
	return day_;
}
uint8_t DS1307::month(void) {
	return month_;
}
uint16_t DS1307::year(void) {
	return year_;
}