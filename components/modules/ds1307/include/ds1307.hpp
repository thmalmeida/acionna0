#ifndef DS1307_HPP__
#define DS1307_HPP__

#include "i2c_driver.hpp"

#define DS1307_ADDR			0x68	// I2C device address

/* Register address list */
#define DS1307_REG_SECONDS	0x00	// Second - range from 0 to 59
#define DS1307_REG_MINUTES	0x01	// Minutes - range from 0 to 59
#define DS1307_REG_HOURS	0x02	// Hours - 0 - 23 if bit 6 = 0. Else, if bit6=1 range from 1-12 and bit5 = 1 to PM and 0 for AM;
#define DS1307_REG_WEEK_DAY	0x03	// day of week 1 to 7
#define DS1307_REG_DAY		0x04	// day of month 1 to 31
#define DS1307_REG_MONTH	0x05	// month range 1 - 12
#define DS1307_REG_YEAR		0x06	// year range from 00 to 99 after year 2000
#define DS1307_REG_CTRL		0x07	// Register control for outputs SQW and OUT. Rate Select (RS) bits

#define DS1307_RAM_START	0x08	// RAM start address
#define DS1307_RAM_END		0x3F	// RAM end address
#define DS1307_RAM_SIZE		56		// RAM size in bytes

#define DS1307_MILLENNIUM	2000	// Current millenium - suppose to be changed when achieve the year 2100

enum class ds1307_ctrl {
	sqw_1_Hz = 0,					// RS[1:0] = 00, bit4 = 1, SQW has 1 Hz, OUT to X
	sqw_4096_Hz,					// RS[1:0] = 01, bit4 = 1, SQW has 4092 Hz, OUT to X
	sqw_8192_Hz,					// RS[1:0] = 10, bit4 = 1, SQW has 8192 Hz, OUT to X
	sqw_32768_Hz,					// RS[1:0] = 11, bit4 = 1, SQW has 32768 Hz, OUT to X
	sqw_low,						// RS[1:0] = xx, bit4 = 0, bit6 = 0, SQW/OUTPUT is low
	sqw_high						// RS[1:0] = xx, bit4 = 0, bit6 = 1, SQW/OUTPUT is high
};

enum class ds1307_format {
	H24 = 0,
	H12
};

enum class ds1307_period {
	AM = 0,
	PM = 1
};

class DS1307 {
public:

	DS1307(I2C_Driver *i2c);

	// ping module answer
	bool probe(void) noexcept;
	// initialize the module
	void init(void);
	// get raw date and time data by one i2c read in sequence and decode it
	void fetch(void);
	// enable/disable RTC 
	void enable(uint8_t status);
	// configure de SQW and OUT outputs
	void ctrl(ds1307_ctrl mode);
	// set time format 12/24 hrs
	void format(ds1307_format format);
	// get AM/PM time for 12 hrs only
	ds1307_period period(void);

	// update RTC DS1307 registers
	void date_time(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
	void date(uint16_t year, uint8_t month, uint8_t day);
	void time(uint8_t hour, uint8_t min, uint8_t sec);

	// Get functions. Should run fetch before any get.
	uint16_t year(void);
	uint8_t month(void);
	uint8_t day(void);
	uint8_t week_day(void);
	uint8_t hour(void);
	uint8_t minute(void);
	uint8_t second(void);

	// void unix_time(uint32_t time);
	// uint32_t unix_time(void);

private:
	// Time/register values
	uint8_t sec_;				// sec range from 0-59;
	uint8_t min_;				// min range from 0-59;
	uint8_t hour_;				// hour 1-12 for 12hrs format; 0-23 for 24 hrs format;
	uint8_t week_day_;			// day of week 1-7;
	uint8_t day_;				// day of month 1-31;
	uint8_t month_;				// month 1-12
	uint8_t year_;				// year 00-99;

	uint8_t ctrl_;				// control byte

	ds1307_format format_;		// 1- 12 hours (1-12); 0- 24 hours format range is 0-23.
	ds1307_period period_;		// 0- AM; 1- PM;

	uint8_t data_raw_[8];		// Data raw readed from DS1307 registers
	// uint8_t data_RAM_[56];		// data RAM readed from DS1307 RAM - not used here

	I2C_Driver *i2c_;			// pointer to I2C peripheral driver

	// Decode functions - convert register data raw readed from DS1307 to decimal readable value
	uint8_t decode_second_(uint8_t data_raw);
	uint8_t decode_minute_(uint8_t data_raw);
	uint8_t decode_hour_(uint8_t data_raw);
	uint8_t decode_week_day_(uint8_t data_raw);
	uint8_t decode_day_(uint8_t data_raw);
	uint8_t decode_month_(uint8_t data_raw);
	uint8_t decode_year_(uint8_t data_raw);

	// Encode functions - convert decimal value to byte register DS1307 format
	uint8_t encode_second_(uint8_t sec);
	uint8_t encode_minute_(uint8_t sec);
	uint8_t encode_hour_(uint8_t hour);
	uint8_t encode_week_day_(uint8_t week);
	uint8_t encode_day_(uint8_t day);
	uint8_t encode_month_(uint8_t month);
	uint8_t encode_year_(uint16_t year);
};

#endif
