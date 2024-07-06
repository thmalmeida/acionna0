#ifndef DS3231_HPP__
#define DS3231_HPP__

/**
 * \file
 * \brief Biblioteca para utilização do módulo RTC DS3231
 *
 * DS3231 é um RTC de alta precisão. Ele se comunica através de uma porta I2C,
 * em modo \i standard ou \i fast. É possível a configuração de 2 alarmes internos,
 * em diferentes modalidades, gerando estes alarmes interrupções externas.
 *
 * Também há pinos externos para verificar a frequência de 32kHz, e possibilidade de ondas
 * quadradas nas frequências de 1Hz, 1kHz, 4kHz e 8kHz.
 *
 * O dispositivo há um sensor de temperatura interno para compensação de diferenças de
 * temperatura.
 *
 * \note Interrupções devem pegar o <i>FALLING EDGE</i>
 *
 * Manual: <a href="https://datasheets.maximintegrated.com/en/ds/DS3231.pdf">DS3231</a>
 *
 * TODO Testar implementação para modo de 12h (AM/PM)
 *
 * \author Rafael Cunha <rnascunha@gmail.com>
 * \date 07/2016
 */

/**
 * \defgroup ds3231 DS3231
 *
 * \brief Definição da classe, tipos e macros referente a utilização de LCDs.
 * @{
 */

/*
 * TODO Criar helper class DateTime, Convert, BitWise
 */

#include "i2c_driver.hpp"
#include "datetime.h"

// Device address
#define DS3231_ADDR				0x68		// (device address << 1) | R/W bit
// #define DS3231_ADDR				0b01101000	// 0x68 >> 1

/* Register address list - size = 19 address positions */
// Date time registers
#define DS3231_REG_SECONDS		0x00
#define DS3231_REG_MINUTES		0x01
#define DS3231_REG_HOURS		0x02
#define DS3231_REG_WEEK_DAY		0x03
#define DS3231_REG_DAY			0x04
#define DS3231_REG_MONTH		0x05
#define DS3231_REG_YEAR			0x06

// Alarm 1 registers
#define DS3231_REG_ALARM1_SEC	0x07
#define DS3231_REG_ALARM1_MIN	0x08
#define DS3231_REG_ALARM1_HOUR	0x09
#define DS3231_REG_ALARM1_DATE	0x0A

// Alarm 2 regiters
#define DS3231_REG_ALARM2_MIN	0x0B
#define DS3231_REG_ALARM2_HOUR	0x0C
#define DS3231_REG_ALARM2_DATE	0x0D

// Control and status registers
#define DS3231_REG_CTRL			0x0E
#define DS3231_REG_STATUS		0x0F
#define DS3231_REG_AGING		0x10

// Temperature register
#define DS3231_REG_TEMPM		0x11
#define DS3231_REG_TEMPL		0x12

#define DS3231_MILLENNIUM	2000	// Current millenium - suppose to be changed when achieve the year 2100

// Output type of NINT/SQW pin
#define OUTPUT_SQW				0
#define OUTPUT_INT				1

// Bits
#define BIT_AMPM				5	//1: PM | 0: AM
#define BIT_1224				6	//1: 12 | 0: 24
#define BIT_CENTURY				7

//Control register bits
#define BIT_EOSC				7
#define BIT_BBSQW				6
#define BIT_CONV				5
#define BIT_RS2					4
#define BIT_RS1					3
#define BIT_INTCN				2
#define BIT_A2IE				1
#define BIT_A1IE				0

//Status register bits
#define BIT_OSF					7
#define BIT_EN32kHz				3	//1: enable /0: disable
#define BIT_BSY					2
#define BIT_A2F					1	//1: Alarm 2 match
#define BIT_A1F					0	//1: Alarm 1 match

// Not done yet! 20240627
enum class ds3231_ctrl {
	mode1 = 0,
	mode2
};

enum class ds3231_format {
	H24 = 0,
	H12
};

enum class ds3231_period {
	AM = 0,
	PM = 1
};

// Frequency of Square Wave output
typedef enum DS3231SQWrate{
	SQW_RATE_1 = 0,
	SQW_RATE_1K,
	SQW_RATE_4K,
	SQW_RATE_8K
} DS3231SQWrate;

// Alarm 1 config
typedef enum DS3231Alarm1Config {
	PER_SECOND = 0,
	SECONDS_MATCH,
	MIN_SEC_MATCH,
	HOUR_MIN_SEC_MATCH,
	DATE_HOUR_MIN_SEC_MATCH,
	DAY_HOUR_MIN_SEC_MATCH
} DS3231Alarm1Config;

//Alarm 2 config
typedef enum DS3231Alarm2Config{
	PER_MINUTE = 0,
	MINUTES_MATCH,
	HOUR_MIN_MATCH,
	DATE_HOUR_MIN_MATCH,
	DAY_HOUR_MIN_MATCH
} DS3231Alarm2Config;

class DS3231 {
public:
	// static constexpr const uint8_t reg = 0b01101000;

	DS3231(I2C_Driver *i2c);

	// ping module answer
	bool probe() noexcept;
	// initialize the module;
	void init(void);
	// get raw date and time data by one i2c read in sequence and decode it;
	void fetch(void);
	// Enable/disable RTC
	void enable(uint8_t status);
	// configure output pins
	void ctrl(ds3231_ctrl mode);
	// set time format 12/24 hrs
	void format(ds3231_format format);
	// get AM/PM time for 12 hrs only
	ds3231_period period(void);

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

	// void setDateTime(DateTime *dateTime);
	// void getDateTime(DateTime *dateTime);

	// void setTime(DateTime *dateTime);
	// void getTime(DateTime *dateTime);

	// void setDate(DateTime *dateTime);
	// void getDate(DateTime *dateTime);

	// void setTimeMode(uint8_t mode);
	// uint8_t getTimeMode();

	// void setDOW(uint8_t dow);
	// uint8_t getDOW();

	// void enable32kHz(bool enable);

	// void enableInterrupt(uint8_t enable = true);
	// void enableSQWRate(DS3231SQWrate rate, uint8_t enable = true);

	// void enableAlarm2Int(bool enable = true);
	// void enableAlarm1Int(bool enable = true);

	// uint8_t clearAlarmFlags();

	// void configAlarm2(DS3231Alarm2Config type_alarm, DateTime *dateTime = NULL);
	// void configAlarm1(DS3231Alarm1Config type_alarm, DateTime *dateTime = NULL);

	// void startConvTemp();
	// float getTemp();

protected:
	// Time/register values
	uint8_t sec_;				// sec range from 0-59;
	uint8_t min_;				// min range from 0-59;
	uint8_t hour_;				// hour 1-12 for 12hrs format; 0-23 for 24 hrs format;
	uint8_t week_day_;			// day of week 1-7;
	uint8_t day_;				// day of month 1-31;
	uint8_t month_;				// month 1-12
	uint8_t year_;				// year 00-99;

	uint8_t ctrl_;				// control byte

	ds3231_format format_;		// 1- 12 hours (1-12); 0- 24 hours format range is 0-23.
	ds3231_period period_;

	uint8_t data_raw_[19];		// Data raw readed from DS1307 registers

	I2C_Driver *i2c_;

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

	// void _send(uint8_t reg, uint8_t* data, size_t length);
	// void _send(uint8_t reg, uint8_t data);

	// void _receive(uint8_t reg, uint8_t* data, size_t length);
	// uint8_t _receive(uint8_t reg);

	// void _set_bit_reg(uint8_t reg, uint8_t bit_mask, bool value = true);
	// uint8_t _get_bit_reg(uint8_t reg, uint8_t bit_mask);

	// static uint8_t _decode(uint8_t value);
	// static uint8_t _decodeH(uint8_t value);
	// static uint8_t _decodeY(uint8_t value);
	// static uint8_t _encode(uint8_t value);
};

/**@}*/

#endif /* __DS3231_H__ */
