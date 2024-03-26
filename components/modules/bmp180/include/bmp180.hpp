#ifndef BMP180_HPP__
#define BMP180_HPP__

/*
* Finished writed on 20240324;
* by thmalmeida
*/	

#include "i2c_driver.hpp"
#include "esp_log.h"

#include <cmath>						// used for altitude calculation only

#define BMP180_ADDR				0x77	// Device address
#define BMP180_REG_TEMP			0x2E	// Conversion time 4.5 ms
#define BMP180_REG_PRESS_OSS_0	0x34	// Max conv. time: 4.5 ms
#define BMP180_REG_PRESS_OSS_1	0x74	// 7.5 ms.  0x34 + (OSS << 6)
#define BMP180_REG_PRESS_OSS_2	0xB4	// 13.5 ms. 
#define BMP180_REG_PRESS_OSS_3	0xF4	// 25.5 ms
#define BMP180_REG_RESET		0xE0	// register to reset command
#define BMP180_REG_CTRL_MEAS	0xF4	// Control measure register
#define BMP180_REG_OUT_MSB		0xF6	// adc output msb - temperature and pressure
#define BMP180_REG_OUT_LSB		0xF7	// adc output lsb - temperature and pressure
#define BMP180_REG_OUT_XLSB		0xF8	// adc output xlsb - pressure only
#define BMP180_REG_ID			0xD0	// id reset state value (for ok response)	     


// Address register for calibration coefficients
#define BMP180_REG_AC1_MSB		0xAA
#define BMP180_REG_AC1_LSB		0xAB
#define BMP180_REG_AC2_MSB		0xAC
#define BMP180_REG_AC2_LSB		0xAD
#define BMP180_REG_AC3_MSB		0xAE
#define BMP180_REG_AC3_LSB		0xAF
#define BMP180_REG_AC4_MSB		0xB0
#define BMP180_REG_AC4_LSB		0xB1
#define BMP180_REG_AC5_MSB		0xB2
#define BMP180_REG_AC5_LSB		0xB3
#define BMP180_REG_AC6_MSB		0xB4
#define BMP180_REG_AC6_LSB		0xB5
#define BMP180_REG_B1_MSB		0xB6
#define BMP180_REG_B1_LSB		0xB7
#define BMP180_REG_B2_MSB		0xB8
#define BMP180_REG_B2_LSB		0xB9
#define BMP180_REG_MB_MSB		0xBA
#define BMP180_REG_MB_LSB		0xBB
#define BMP180_REG_MC_MSB		0xBC
#define BMP180_REG_MC_LSB		0xBD
#define BMP180_REG_MD_MSB		0xBE
#define BMP180_REG_MD_LSB		0xBF

// #define BMP180_DEBUG			1

/* 
* Pressure reads with maximum of 128 S/s and temperature of 1 S/s;
* UT - Temperature data (16-bit)
* UP - pressure data (16 to 19 bit)
*
*
* Sampling Mode				Parameter 				n samples	conversion time [ms]
							Oversampling_setting
							(OSR)				
* 1- Ultra Low power		0						1			4.5
* 2- Standard				1						2			7.5
* 3- high resolution		2						4			13.5
* 4- ultra high resolution	3						8			25.5
*
* 176 bit EEPROM is partitioned in 11 words of 16 bits each
*
* Accuracy:
* Pressure: 	1 Pa (=0.01 hPa = 0.01 mbar)
* Temperature:	0.1 C
*
* Range:
* Pressure:		300 to 1100 hPa
* Temperature:	0 to +65 C
*/

class BMP180 {
public:
	BMP180(I2C_Driver *i2c);
	~BMP180(void) {}

	bool probe(void);
	void init(void);

	// return temp in scale of 0.1 C;
	int32_t temperature(void);

	// return pressure in [Pa]
	int32_t pressure(void);

	// return altitude in [m]
	int32_t altitude(void);

	// not tested yet. Suppose to calibrate the sea level pressure
	void pressure_sea_level(uint32_t pressure, int32_t altitude);

	// must call before ask for pressure and temperature;
	void fetch(void);

	void soft_reset(void);

private:
	// get uncompensated temperature (UT)
	uint16_t u_temperature_(void);

	// get uncompensated pressure (UP)
	uint32_t u_pressure_(void);

	int32_t calc_true_temperature_(uint16_t temp);

	int32_t calc_true_pressure_(uint32_t press);

	// set oss value on ctrl_meas register
	void oss_(uint8_t value);

	// get oss value on ctrl_meas register
	uint8_t oss_(void);

	// start conversion bit
	uint8_t sco_(void);

	// ctrl_meas register
	uint8_t ctrl_meas_(void);	

	// get chip id - 0x55
	uint8_t chip_id_(void);

	int32_t temperature_;					// Temperature in Celcius degree;
	int32_t pressure_;						// Pressure in [Pa]

	int32_t pressure_sea_level_ = 101325; 	// Pressure at sea level [Pa]

	// Calibration coefficients
	int16_t AC1_, AC2_, AC3_, B1_, B2_, MB_, MC_, MD_;
	uint16_t AC4_, AC5_, AC6_;

	// Calculated coefficients;
	int32_t B5_, p_;

	// i2c driver pointer;
	I2C_Driver *i2c_;
};

#endif