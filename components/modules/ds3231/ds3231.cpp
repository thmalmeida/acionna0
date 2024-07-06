#include "ds3231.hpp"

DS3231::DS3231(I2C_Driver *i2c) : i2c_(i2c) {
	init();
}

bool DS3231::probe(void) noexcept {
	return i2c_->probe(DS3231_ADDR);
}

void DS3231::init(void) {
	// 1- data raw fetch read all registers
	fetch();

	// 2- setup control register for SQW/OUTPUT signal
	// ctrl(DS3231_ctrl::sqw_low);

	// 3- Set hour format
	format(ds3231_format::H24);

	// 4- enable RTC;
	enable(1);
}
void DS3231::fetch(void) {
	i2c_->read(DS3231_ADDR, DS3231_REG_SECONDS, &data_raw_[0], sizeof(data_raw_));

	sec_ = decode_second_(data_raw_[0]);
	min_ = decode_minute_(data_raw_[1]);
	hour_ = decode_hour_(data_raw_[2]);
	week_day_ = decode_week_day_(data_raw_[3]);
	day_ = decode_day_(data_raw_[4]);
	month_ = decode_month_(data_raw_[5]);
	year_ = decode_year_(data_raw_[6]);
	ctrl_ = data_raw_[14];

	// Read the bit 6 of hour register from 02h addr (DS3231/DS3231)
	if((data_raw_[2] >> 6) & 0x01) {
		format_ = ds3231_format::H12;

		if((data_raw_[2] >> 5) & 0x01)
			period_ = ds3231_period::PM;
		else
			period_ = ds3231_period::AM;
	}
	else
		format_ = ds3231_format::H24;
}
void DS3231::enable(uint8_t status) {
	uint8_t data, EOSC_bit;
	i2c_->read(DS3231_ADDR, DS3231_REG_CTRL, &data);

	EOSC_bit = (data >> 7) & 0x01;

	if(status) {
		if(EOSC_bit) {
			data &= 0x7F;
			i2c_->write(DS3231_ADDR, DS3231_REG_CTRL, data);
		}
	} else {
		if(!EOSC_bit) {
			data |= 0x80;
			i2c_->write(DS3231_ADDR, DS3231_REG_CTRL, data);
		}
	}
}
void DS3231::ctrl(ds3231_ctrl mode) {
	ctrl_ = 0x00;	// clear ctrl buffer

}
void DS3231::format(ds3231_format format) {
	if(format != format_) {
		switch (format) {
			case ds3231_format::H12:
				data_raw_[2] |= (1<<6);
				break;

			case ds3231_format::H24:
				data_raw_[2] &= ~(1 << 6);
				break;
		}
		i2c_->write(DS3231_ADDR, DS3231_REG_HOURS, data_raw_[2]);
	}
}
ds3231_period DS3231::period(void) {
	return period_;
}

// Set functions - update DS3231 registers
void DS3231::date_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
	data_raw_[0] = encode_second_(sec);
	data_raw_[1] = encode_minute_(min);
	data_raw_[2] = encode_hour_(hour);
	data_raw_[3] = encode_week_day_(1);
	data_raw_[4] = encode_day_(day);
	data_raw_[5] = encode_month_(month);
	data_raw_[6] = encode_year_(year);

	i2c_->write(DS3231_ADDR, DS3231_REG_SECONDS, &data_raw_[0], 7);
}
void DS3231::date(uint16_t year, uint8_t month, uint8_t day) {
	data_raw_[4] = encode_day_(day);
	data_raw_[5] = encode_month_(month);
	data_raw_[6] = encode_year_(year);

	i2c_->write(DS3231_ADDR, DS3231_REG_DAY, &data_raw_[4], 3);
}
void DS3231::time(uint8_t hour, uint8_t min, uint8_t sec) {
	data_raw_[0] = encode_second_(sec);
	data_raw_[1] = encode_minute_(min);
	data_raw_[2] = encode_hour_(hour);

	i2c_->write(DS3231_ADDR, DS3231_REG_SECONDS, &data_raw_[0], 3);
}

// Get functions
uint16_t DS3231::year(void) {
	return year_ + DS3231_MILLENNIUM;
}
uint8_t DS3231::month(void) {
	return month_;
}
uint8_t DS3231::day(void) {
	return day_;
}
uint8_t DS3231::week_day(void) {
	return week_day_;
}
uint8_t DS3231::hour(void) {
	return hour_;
}
uint8_t DS3231::minute(void) {
	return min_;
}
uint8_t DS3231::second(void) {
	return sec_;
}

// Decode - convert DS3231/DS3231 byte format to decimal value
uint8_t DS3231::decode_second_(uint8_t data_raw) {
	return ((data_raw >> 4) & 0x07)*10 + (data_raw & 0x0F);
}
uint8_t DS3231::decode_minute_(uint8_t data_raw) {
	return decode_second_(data_raw);
}
uint8_t DS3231::decode_hour_(uint8_t data_raw) {
	// check 12 or 24 hour format by the bit 6
	if(!((data_raw >> 6) & 0x01))
		return (data_raw >> 4)*10 + (data_raw & 0x0F);
	else 
		return ((data_raw >> 4) & 0x01)*10 + (data_raw & 0x0F);

}
uint8_t DS3231::decode_week_day_(uint8_t data_raw) {
	return data_raw;
}
uint8_t DS3231::decode_day_(uint8_t data_raw) {
	return (data_raw >> 4)*10 + (data_raw & 0x0F);
}
uint8_t DS3231::decode_month_(uint8_t data_raw) {
	return (data_raw >> 4)*10 + (data_raw & 0x0F);
}
uint8_t DS3231::decode_year_(uint8_t data_raw) {
	return (data_raw >> 4)*10 + (data_raw & 0x0F);
}

// Encode - prepare decimal value to register format to DS3231/DS3231
uint8_t DS3231::encode_second_(uint8_t sec) {
	return (sec/10 << 4) | (sec%10);
}
uint8_t DS3231::encode_minute_(uint8_t min) {
	return (min/10 << 4) | (min%10);
}
uint8_t DS3231::encode_hour_(uint8_t hour) {
	return (hour/10 << 4) | hour%10;
}
uint8_t DS3231::encode_week_day_(uint8_t week) {
	return week%10;
}
uint8_t DS3231::encode_day_(uint8_t day) {
	return (day/10 << 4) | (day%10);
}
uint8_t DS3231::encode_month_(uint8_t month) {
	return (month/10 << 4) | (month%10);
}
uint8_t DS3231::encode_year_(uint16_t year) {
	return ((year-2000)/10 << 4) | ((year-2000)%10);
}


// void DS3231::init(void) {
// 	_set_bit_reg(REG_HOUR, BIT_1224, format_);
// 	enableAlarm1Int(false);
// 	enableAlarm2Int(false);
// 	enableInterrupt(false);
// }

// void DS3231::setDateTime(DateTime *dateTime)
// {
// 	uint8_t date[] = {_encode(dateTime->getSecond()),
// 					_encode(dateTime->getMinute()),
// 					_encode(dateTime->getHour()),
// 					//_encode(dateTime->dayOfWeek()),
// 					1,
// 					_encode(dateTime->getDay()),
// 					_encode(dateTime->getMonth()),
// 					_encode((uint8_t)(dateTime->getYear()-2000))
// 					};

// 	_send(REG_SEC,date,7);
// }
// void DS3231::getDateTime(DateTime* dateTime)
// {
// 	uint8_t date[7];

// 	_receive(REG_SEC, date, 7);

// 	dateTime->setDateTime((uint16_t)_decodeY(date[6])+2000,
// 							_decode(date[5]),
// 							_decode(date[4]),
// 							_decodeH(date[2]),
// 							_decode(date[1]),
// 							_decode(date[0]));
// }
// void DS3231::setTime(DateTime *dateTime)
// {
// 	uint8_t date[] = {_encode(dateTime->getSecond()),
// 					_encode(dateTime->getMinute()),
// 					_encode(dateTime->getHour())
// 					};

// 	_send(REG_SEC,date,3);
// }

// void DS3231::getTime(DateTime* dateTime)
// {
// 	uint8_t date[3];

// 	_receive(REG_SEC,date,3);

// 	dateTime->setTime(_decodeH(date[2]),
// 				_decode(date[1]),
// 				_decode(date[0]));
// }

// void DS3231::setDate(DateTime *dateTime)
// {
// 	uint8_t date[] = {_encode(dateTime->getDay()),
// 					_encode(dateTime->getMonth()),
// 					_encode((uint8_t)(dateTime->getYear()-2000))
// 					};

// 	_send(REG_DATE,date,3);
// }

// void DS3231::getDate(DateTime *dateTime)
// {
// 	uint8_t date[3];

// 	_receive(REG_DATE,date,3);

// 	dateTime->setDate((uint16_t)(_decodeY(date[2])+2000),
// 					_decode(date[1]),
// 					_decode(date[0])
// 					);
// }

// void DS3231::setTimeMode(uint8_t mode)
// {
// 	_set_bit_reg(REG_HOUR,6,mode);
// }

// uint8_t DS3231::getTimeMode()
// {
// 	return _get_bit_reg(REG_HOUR,6);
// }

// void DS3231::setDOW(uint8_t dow)
// {
// 	_send(REG_DOW,dow);
// }

// uint8_t DS3231::getDOW()
// {
// 	return _receive(REG_DOW);
// }

// void DS3231::enable32kHz(bool enable)
// {
// 	_set_bit_reg(REG_STATUS, BIT_EN32kHz, enable);
// }

// void DS3231::enableInterrupt(uint8_t enable /*= true*/)
// {
// 	_set_bit_reg(REG_CON, BIT_INTCN, enable);
// }

// void DS3231::enableSQWRate(DS3231SQWrate rate, uint8_t enable /*= true*/)
// {
// 	if(enable)
// 		enableInterrupt(false);

// 	_set_bit_reg(REG_CON, BIT_BBSQW, enable);

// 	switch(rate){
// 		case SQW_RATE_1K:
// 			_set_bit_reg(REG_CON,BIT_RS2,false);
// 			_set_bit_reg(REG_CON,BIT_RS1,true);
// 		break;
// 		case SQW_RATE_4K:
// 			_set_bit_reg(REG_CON,BIT_RS2,true);
// 			_set_bit_reg(REG_CON,BIT_RS1,false);
// 		break;
// 		case SQW_RATE_8K:
// 			_set_bit_reg(REG_CON,BIT_RS2,true);
// 			_set_bit_reg(REG_CON,BIT_RS1,true);
// 		break;
// 		default: /* SQW_RATE_1 */
// 			_set_bit_reg(REG_CON,BIT_RS2,false);
// 			_set_bit_reg(REG_CON,BIT_RS1,false);
// 		break;
// 	}
// }

// void DS3231::enableAlarm2Int(bool enable /*= true*/)
// {
// 	_set_bit_reg(REG_CON,BIT_A2IE,enable);
// }

// void DS3231::enableAlarm1Int(bool enable /*= true*/)
// {
// 	_set_bit_reg(REG_CON,BIT_A1IE,enable);
// }

// uint8_t DS3231::clearAlarmFlags()
// {
// 	uint8_t alarms = _receive(REG_STATUS) & ((1<<BIT_A2F)|(1<<BIT_A1F));

// 	_set_bit_reg(REG_STATUS, BIT_A2F, false);
// 	_set_bit_reg(REG_STATUS, BIT_A1F, false);

// 	return alarms;
// }

// void DS3231::configAlarm2(DS3231Alarm2Config type_alarm, DateTime *dateTime /* = NULL */)
// {
// 	switch(type_alarm){
// 		case PER_MINUTE:
// 			_set_bit_reg(REG_ALARM2_DATE, 7);
// 			_set_bit_reg(REG_ALARM2_HOUR, 7);
// 			_set_bit_reg(REG_ALARM2_MIN, 7);
// 		break;
// 		case MINUTES_MATCH:
// 			_send(REG_ALARM2_MIN, _encode(dateTime->getMinute()));
// 			_set_bit_reg(REG_ALARM2_DATE, 7);
// 			_set_bit_reg(REG_ALARM2_HOUR, 7);
// 			_set_bit_reg(REG_ALARM2_MIN, 7, false);
// 		break;
// 		case HOUR_MIN_MATCH:
// 		{
// 			uint8_t data[] = {_encode(dateTime->getMinute()),
// 					_encode(dateTime->getHour())};
// 			_send(REG_ALARM2_MIN, data, 2);
// 			_set_bit_reg(REG_ALARM2_DATE, 7);
// 			_set_bit_reg(REG_ALARM2_HOUR, 7, false);
// 			_set_bit_reg(REG_ALARM2_MIN, 7, false);
// 		}
// 		break;
// 		case DATE_HOUR_MIN_MATCH:
// 		{
// 			uint8_t data[] = {_encode(dateTime->getMinute()),
// 					_encode(dateTime->getHour()),
// 					_encode(dateTime->getDay())};
// 			_send(REG_ALARM2_MIN, data, 3);
// 			_set_bit_reg(REG_ALARM2_DATE, 7, false);
// 			_set_bit_reg(REG_ALARM2_HOUR, 7, false);
// 			_set_bit_reg(REG_ALARM2_MIN, 7, false);
// 			_set_bit_reg(REG_ALARM2_DATE, 6, false);
// 		}
// 		break;
// 		case DAY_HOUR_MIN_MATCH:
// 		{
// 			uint8_t data[] = {_encode(dateTime->getMinute()),
// 					_encode(dateTime->getHour()),
// 					_encode(dateTime->dayOfWeek())};
// 			_send(REG_ALARM2_MIN, data, 3);
// 			_set_bit_reg(REG_ALARM2_DATE, 7, false);
// 			_set_bit_reg(REG_ALARM2_HOUR, 7, false);
// 			_set_bit_reg(REG_ALARM2_MIN, 7, false);
// 			_set_bit_reg(REG_ALARM2_DATE, 6, true);
// 		}
// 		break;
// 		default:
// 		break;
// 	}
// }

// void DS3231::configAlarm1(DS3231Alarm1Config type_alarm, DateTime *dateTime /* = NULL */)
// {
// 	switch(type_alarm){
// 		case PER_SECOND:
// 			_set_bit_reg(REG_ALARM1_DATE, 7);
// 			_set_bit_reg(REG_ALARM1_HOUR, 7);
// 			_set_bit_reg(REG_ALARM1_MIN, 7);
// 			_set_bit_reg(REG_ALARM1_SEC, 7);
// 		break;
// 		case SECONDS_MATCH:
// 			_send(REG_ALARM1_SEC, _encode(dateTime->getSecond()));
// 			_set_bit_reg(REG_ALARM1_DATE, 7);
// 			_set_bit_reg(REG_ALARM1_HOUR, 7);
// 			_set_bit_reg(REG_ALARM1_MIN, 7);
// 			_set_bit_reg(REG_ALARM1_SEC, 7, false);
// 		break;
// 		case MIN_SEC_MATCH:
// 		{
// 			uint8_t data[] = {_encode(dateTime->getSecond()),
// 					_encode(dateTime->getMinute())};
// 			_send(REG_ALARM1_SEC, data, 2);
// 			_set_bit_reg(REG_ALARM1_DATE, 7);
// 			_set_bit_reg(REG_ALARM1_HOUR, 7);
// 			_set_bit_reg(REG_ALARM1_MIN, 7, false);
// 			_set_bit_reg(REG_ALARM1_SEC, 7, false);
// 		}
// 		break;
// 		case HOUR_MIN_SEC_MATCH:
// 		{
// 			uint8_t data[] = {_encode(dateTime->getSecond()),
// 					_encode(dateTime->getMinute()),
// 					_encode(dateTime->getHour())};
// 			_send(REG_ALARM1_SEC, data, 3);
// 			_set_bit_reg(REG_ALARM1_DATE, 7);
// 			_set_bit_reg(REG_ALARM1_HOUR, 7, false);
// 			_set_bit_reg(REG_ALARM1_MIN, 7, false);
// 			_set_bit_reg(REG_ALARM1_SEC, 7, false);
// 		}
// 		break;
// 		case DATE_HOUR_MIN_SEC_MATCH:
// 		{
// 			uint8_t data[] = {_encode(dateTime->getSecond()),
// 					_encode(dateTime->getMinute()),
// 					_encode(dateTime->getHour()),
// 					_encode(dateTime->getDay())};
// 			_send(REG_ALARM1_SEC,data, 4);
// 			_set_bit_reg(REG_ALARM1_DATE, 7, false);
// 			_set_bit_reg(REG_ALARM1_HOUR, 7, false);
// 			_set_bit_reg(REG_ALARM1_MIN, 7, false);
// 			_set_bit_reg(REG_ALARM1_SEC, 7, false);
// 			_set_bit_reg(REG_ALARM1_DATE, 6, false);
// 		}
// 		break;
// 		case DAY_HOUR_MIN_SEC_MATCH:
// 		{
// 			uint8_t data[] = {_encode(dateTime->getSecond()),
// 					_encode(dateTime->getMinute()),
// 					_encode(dateTime->getHour()),
// 					_encode(dateTime->dayOfWeek())};
// 			_send(REG_ALARM1_SEC,data, 4);
// 			_set_bit_reg(REG_ALARM1_DATE, 7, false);
// 			_set_bit_reg(REG_ALARM1_HOUR, 7, false);
// 			_set_bit_reg(REG_ALARM1_MIN, 7, false);
// 			_set_bit_reg(REG_ALARM1_SEC, 7, false);
// 			_set_bit_reg(REG_ALARM1_DATE, 6);
// 		}
// 		break;
// 		default:
// 		break;
// 	}
// }

// void DS3231::startConvTemp()
// {
// 	if(_get_bit_reg(REG_CON, BIT_CONV))
// 		return;

// 	_set_bit_reg(REG_CON, BIT_CONV);
// }

// float DS3231::getTemp()
// {
// 	uint8_t data[2];

// 	_receive(REG_TEMPM, data, 2);

// 	return (float)data[0] + ((data[1] >> 6) * 0.25f);
// }

// /* Private */
// void DS3231::_send(uint8_t reg, uint8_t* data, size_t length)
// {
// 	i2c_->write(DS3231_ADDR, reg, data, length);
// }

// void DS3231::_send(uint8_t reg,uint8_t data)
// {
// 	i2c_->write(DS3231_ADDR, reg, data);
// }

// void DS3231::_receive(uint8_t reg,uint8_t* data, size_t length)
// {
// 	i2c_->read(DS3231_ADDR, reg, data, length);
// }

// uint8_t DS3231::_receive(uint8_t reg)
// {
// 	uint8_t data;
// 	i2c_->read(DS3231_ADDR, reg, &data);

// 	return data;
// }

// void DS3231::_set_bit_reg(uint8_t reg, uint8_t bit, bool value /* = true*/)
// {
// 	uint8_t r = _receive(reg);

// 	r &= ~(1 << bit);
// 	r |= (value << bit);
// 	_send(reg, r);
// }

// uint8_t DS3231::_get_bit_reg(uint8_t reg, uint8_t bit)
// {
// 	return (_receive(reg) >> bit) & 1;
// }

// uint8_t	DS3231::_decode(uint8_t value)
// {
// 	uint8_t decoded = value & 127;
// 	decoded = (decoded & 15) + 10 * ((decoded & (15 << 4)) >> 4);
// 	return decoded;
// }

// uint8_t DS3231::_decodeH(uint8_t value)
// {
//   if (value & 128)
//     value = (value & 15) + (12 * ((value & 32) >> 5));
//   else
//     value = (value & 15) + (10 * ((value & 48) >> 4));
//   return value;
// }

// uint8_t	DS3231::_decodeY(uint8_t value)
// {
// 	return (value & 15) + 10 * ((value & (15 << 4)) >> 4);
// }

// uint8_t DS3231::_encode(uint8_t value)
// {
// 	return ((value / 10) << 4) + (value % 10);
// }
