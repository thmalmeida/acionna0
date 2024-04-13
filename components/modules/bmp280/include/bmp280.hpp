#ifndef BMP280_HPP__
#define BMP280_HPP__

/*
* Finished writed on 20240324;
* by thmalmeida
*/	

#include "i2c_driver.hpp"
#include "esp_log.h"

#include <cmath>

#define BMP280_ADDR             0x77	// Device address

#define BMP280_REG_STATUS		0xF3	// status register. bit 3 - measuring; bit 0 - im_update[0]
#define BMP280_REG_CTRL_MEAS	0xF4	// Control measure - osrs_t[2:0] osrs_p[2:0] mode[1:0]
#define BMP280_REG_CONFIG		0xF5	// Config - t_sb[2:0] filter[2:0] bit 1 (void) spi3w_en[0]

#define BMP280_REG_PRESS_MSB	0xF7
#define BMP280_REG_PRESS_LSB	0xF8
#define BMP280_REG_PRESS_XLSB	0xF9	// press_xlsb[7:4] bits only
#define BMP280_REG_TEMP_MSB		0xFA
#define BMP280_REG_TEMP_LSB		0xFB
#define BMP280_REG_TEMP_XLSB	0xFC	// temp_xlsb[7:4]

#define BMP280_REG_RESET		0xE0	// Register addr to reset
#define BMP280_REG_ID			0xD0	// chip id number - 0x58

#define BMP280_CMD_RESET		0xB6	// write this value into reg reset to complete power-on-reset procedure.

// Address register for calibration coefficients
#define BMP280_REG_DIG_T1_LSB	0x88
#define BMP280_REG_DIG_T1_MSB	0x89
#define BMP280_REG_DIG_T2_LSB	0x8A
#define BMP280_REG_DIG_T2_MSB	0x8B
#define BMP280_REG_DIG_T3_LSB	0x8C
#define BMP280_REG_DIG_T3_MSB	0x8D
#define BMP280_REG_DIG_P1_LSB	0x8E
#define BMP280_REG_DIG_P1_MSB	0x8F
#define BMP280_REG_DIG_P2_LSB	0x90
#define BMP280_REG_DIG_P2_MSB	0x91
#define BMP280_REG_DIG_P3_LSB	0x92
#define BMP280_REG_DIG_P3_MSB	0x93
#define BMP280_REG_DIG_P4_LSB	0x94
#define BMP280_REG_DIG_P4_MSB	0x95
#define BMP280_REG_DIG_P5_LSB	0x96
#define BMP280_REG_DIG_P5_MSB	0x97
#define BMP280_REG_DIG_P6_LSB	0x98
#define BMP280_REG_DIG_P6_MSB	0x99
#define BMP280_REG_DIG_P7_LSB	0x9A
#define BMP280_REG_DIG_P7_MSB	0x9B
#define BMP280_REG_DIG_P8_LSB	0x9C
#define BMP280_REG_DIG_P8_MSB	0x9D
#define BMP280_REG_DIG_P9_LSB	0x9E
#define BMP280_REG_DIG_P9_MSB	0x9F

#define BMP280_REG_RESERVED_LSB	0xA0
#define BMP280_REG_RESERVED_MSB	0xA1

#define BMP280_DELAY_WRITE_MS	10		// some time delay after write
#define BMP280_DELAY_READ_MS	10
#define BMP280_DELAY_INIT_MS	50		// wait stablish after power (not in manual);

// #define BMP280_DEBUG			1

/* 
* Pressure reads with maximum of ___ S/s and temperature of __ S/s;
* UT - Temperature data (20-bit)
* UP - pressure data 	(20-bit)
*
* Three power modes:
* 	- sleep mode;
* 	- normal mode;
* 	- forced mode.
*
* Table 10: mode settings (page 15)
* mode[1:0]		Mode
*	00			sleep mode
* 01 or 10		Forced mode
* 	11			Normal mode
*
* Table 11: t_sb settings
* t_sb[1:0]		t_standby [ms]
* 000			0.5
* 001			62.5
* 010			125
* 011			250
* 100			500
* 101			1000
* 110			2000
* 111			4000
*
* Table 21: register settings osrs_p
* osrs_p		Pressure oversampling
* 000			Skipped (output set to 0x80000)
* 001			x1
* 010			x2
* 011			x4
* 100			x8
* 100, others	x16 
*
* Table 22: register settings osrs_t
* osrs_t		Temperature oversampling
* 000			skipped (output set to 0x80000)
* 001			x1
* 010			x2
* 011			x4
* 100			x8
* 101 110 111	x16
*
* Filter options
* filter[2:0]	Filter coeff	Samples to reach >= 75% of step response	
* 000			Filter off		1
* 001			2				2	
* 010			4				5
* 011			8				11
* 100			16				22
*
* 176 bit EEPROM is partitioned in 11 words of 16 bits each
*
* Accuracy:
* Pressure: 	0.16 Pa (=0.0016 hPa = 0.0001 mbar)
* Temperature:	0.01 C
*
* Range:
* Pressure:		300 to 1100 hPa
* Temperature:	0 to +65 C
*/

class BMP280 {
public:
	BMP280(I2C_Driver *i2c);
	~BMP280(void) {}

	// test alive
	bool probe(void);

	// chip initialize. Load eeprom calibration values;
	void init(void);

	// return temp in scale of 0.01 C;
	double temperature(void);
	
	// return the pressure in Pa
	double pressure(void);

	// return pressure in scale of 0.16 [Pa] (depends of oversampling parameters)
	int32_t pressure_hPa(void);

	// return altitude in [m]
	double altitude(void);

	// not tested yet. Suppose to calibrate the sea level pressure
	void pressure_sea_level(uint32_t pressure, int32_t altitude);

	// must call before ask for pressure and temperature;
	void fetch(void);

	// command chip reset
	void reset(void);


private:

	// burst reads

	// registers sequence read
	i2c_ans read_burst_reg_(void);

	// adc sequence read
	i2c_ans read_burst_adc_(void);

	// calibration coefficients sequence read
	i2c_ans read_burst_calib_(void);

	// read calibration parameters one by one (not used)
	void read_calib_(void);


	// bit operations for config register
	uint8_t t_sb_(void);
	void t_sb_(uint8_t value);
	uint8_t filter_(void);
	void filter_(uint8_t value);
	uint8_t spi3w_en_(void);
	void spi3w_en_(uint8_t value);

	// set oversampling pressure register osrs_p[2:0] bits
	void osrs_p_(uint8_t value);

	// get oversampling pressure register osrs_p[2:0] bits
	uint8_t osrs_p_(void);

	// set oversampling temperature register osrs_t[2:0] bits at ctrl_meas reg
	void osrs_t_(uint8_t value);

	// get oversampling temperature register osrs_t[2:0] bits at ctrl_meas reg
	uint8_t osrs_t_(void);

	// set mode bits at ctrl_meas reg;
	void mode_(uint8_t value);

	// get mode bits at ctrl_meas reg;
	uint8_t mode_(void);

	// uncompensated temperature (UT) - 20 bits
	int32_t u_temperature_read_(void);

	// uncompensated pressure (UP) - 20 bits
	int32_t u_pressure_read_(void);

	int32_t calc_true_temperature_(int32_t adc_T);

	uint32_t calc_true_pressure_(int32_t adc_P);
	double calc_true_pressure_2_(int32_t adc_P);

	// config register - s_sb[2:0], filter[2:0], spi3w_en[0]
	uint8_t config_(void);

	void config_(uint8_t value);

	/// @brief set ctrl_means reg
	/// @param value value
	void ctrl_meas_(uint8_t value);

	// ctrl_meas register
	uint8_t ctrl_meas_(void);

	/// @return status register
	uint8_t status_(void);

	// get chip id - 0x55
	uint8_t chip_id_(void);

	// t_fine carries fine temperature as global value
	int32_t t_fine_ = 0;

	// Pressure at sea level [Pa]
	int32_t pressure_sea_level_ = 101325;

	// 8-bit registers
	uint8_t status_reg_, ctrl_meas_reg_, config_reg_;

	// Calibration coefficients
	uint16_t dig_T1_, dig_P1_;
	int16_t dig_T2_, dig_T3_, dig_P2_, dig_P3_, dig_P4_, dig_P5_, dig_P6_, dig_P7_, dig_P8_, dig_P9_;

	int32_t temperature_;					// Temperature in Celcius degree;
	double pressure_;						// pressure in [Pa]
	uint32_t pressure_hPa_;					// Pressure in [hPa]


	// 20 bit positive integers stored on 32 bit signed integer
	int32_t u_temperature_;					// uncompensated temperature - raw value
	int32_t u_pressure_;					// uncompensated pressure - raw value

	// i2c driver pointer;
	I2C_Driver *i2c_;
};

#endif