#ifndef DS1307_HPP__
#define DS1307_HPP__

#include <string.h>
#include "esp_log.h"
#include "i2c_driver.hpp"

#define DS1307_ADDR         0x68        // I2C device address

/* Register address list */
#define DS1307_REG_SECONDS	0x00	// Second - range from 0 to 59
#define DS1307_REG_MINUTES	0x01	// Minutes - range from 0 to 59
#define DS1307_REG_HOURS	0x02	// Hours
#define DS1307_REG_DAY		0x03	// day of week 1 to 7
#define DS1307_REG_DATE		0x04	// day of month 1 to 31
#define DS1307_REG_MONTH	0x05	// month
#define DS1307_REG_YEAR		0x06	//

#define DS1307_RAM_START	0x08	// RAM start address
#define DS1307_RAM_END		0x3F	// RAM end address
#define DS1307_RAM_SIZE		56		// RAM size in bytes

enum class ds1307_ctrl {
	sqw_1Hz = 0,
	sqw_4096Hz
};

class DS1307 {
public:

	DS1307(I2C_Driver *i2c);

	// initialize the module
	void init(void);
	// get date time raw data by one i2c read
	void fetch(void);
	// enable/disable RTC 
	void enable(uint8_t status);

	// update RTC
	void date_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
	void date(uint16_t year, uint8_t month, uint8_t day);
	void time(uint8_t hour, uint8_t min, uint8_t sec);

	// get functions. Should run fetch before get.
	uint8_t second(void);
	uint8_t minute(void);
	uint8_t hour(void);
	uint8_t week(void);
	uint8_t day(void);
	uint8_t month(void);
	uint16_t year(void);

	// void unix_time(uint32_t time);
	// uint32_t unix_time(void);
	// void ctrl_(ds1307_ctrl ctrl_mode);

private:

	uint8_t decode_second_(uint8_t data_raw);
	uint8_t decode_minute_(uint8_t data_raw);
	uint8_t decode_hour_(uint8_t data_raw);
	uint8_t decode_week_(uint8_t data_raw);
	uint8_t decode_day_(uint8_t data_raw);
	uint8_t decode_month_(uint8_t data_raw);
	uint16_t decode_year_(uint8_t data_raw);

	uint8_t encode_second_(uint8_t sec);
	uint8_t encode_minute_(uint8_t sec);
	uint8_t encode_hour_(uint8_t hour);
	uint8_t encode_week_(uint8_t week);
	uint8_t encode_day_(uint8_t day);
	uint8_t encode_month_(uint8_t month);
	uint8_t encode_year_(uint16_t year);

	uint8_t sec_;
	uint8_t min_;
	uint8_t hour_;
	uint8_t week_;			// day of week 0 - 6
	uint8_t day_;
	uint8_t month_;
	uint16_t year_;

	uint8_t ctrl_;			// control byte

	// Data raw
	uint8_t data_raw_[8];

	// struct data_raw {
	// 	uint8_t sec_;
	// 	uint8_t min_;
	// 	uint8_t hour_;
	// 	uint8_t day_;
	// 	uint8_t month_;
	// 	uint8_t year_;
	// 	uint8_t ctrl_;
	// } raw_;

	I2C_Driver *i2c_;		// pointer to I2C peripheral driver
};

#endif
