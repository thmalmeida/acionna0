#ifndef BPM180_HPP__
#define BPM180_HPP__

/*
* Finished writed on 20240322;
* by thmalmeida
*/

#include "i2c_driver.hpp"
#include "esp_log.h"

#define BMP180_ADDR             0x77
#define BMP180_ADDR_TEMP        0x2E	// Conversion time 4.5 ms
#define BMP180_ADDR_PRESS_OSS_0 0x34	// Max conv. time: 4.5 ms
#define BMP180_ADDR_PRESS_OSS_1 0x74	// 7.5 ms.  0x34 + (OSS << 6)
#define BMP180_ADDR_PRESS_OSS_2 0xB4	// 13.5 ms. 
#define BMP180_ADDR_PRESS_OSS_3 0xF4	// 25.5 ms
#define BMP180_ADDR_ID          0xD0	// id reset state value (for ok response)	     
#define BMP180_ADDR_SOFT_RST	0xE0	// register to soft reset
#define BMP180_ADDR_CR			0xF4	// Control Register

// Address register for calibration coefficients
#define BMP180_ADDR_AC1_MSB		0xAA
#define BMP180_ADDR_AC1_LSB		0xAB
#define BMP180_ADDR_AC2_MSB		0xAC
#define BMP180_ADDR_AC2_LSB		0xAD
#define BMP180_ADDR_AC3_MSB		0xAE
#define BMP180_ADDR_AC3_LSB		0xAF
#define BMP180_ADDR_AC4_MSB		0xB0
#define BMP180_ADDR_AC4_LSB		0xB1
#define BMP180_ADDR_AC5_MSB		0xB2
#define BMP180_ADDR_AC5_LSB		0xB3
#define BMP180_ADDR_AC6_MSB		0xB4
#define BMP180_ADDR_AC6_LSB		0xB5
#define BMP180_ADDR_B1_MSB		0xB6
#define BMP180_ADDR_B1_LSB		0xB7
#define BMP180_ADDR_B2_MSB		0xB8
#define BMP180_ADDR_B2_LSB		0xB9
#define BMP180_ADDR_MB_MSB		0xBA
#define BMP180_ADDR_MB_LSB		0xBB
#define BMP180_ADDR_MC_MSB		0xBC
#define BMP180_ADDR_MC_LSB		0xBD
#define BMP180_ADDR_MD_MSB		0xBE
#define BMP180_ADDR_MD_LSB		0xBF

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
*/

class BMP180 {
public:
	BMP180(I2C_Driver *i2c);

	bool probe(void);
	void init(void);

	// get uncompensated temperature (UT)
	uint16_t u_temperature(void);

	// get uncompensated pressure (UP)
	uint32_t u_pressure(void);

	double calc_true_temperature(void);
	double calc_true_pressure(void);

	// calculate the altitude based on pressure;
	int altitude(void);
	int pressure_sea_level(void);

	void soft_reset(void);

private:

	// set oss value on ctrl_meas register
	void oss_(uint8_t value);

	// get oss value on ctrl_meas register
	uint8_t oss_(void);

	int UT = 0; 				// Uncompesated temperature;
	int UP = 0;					// Uncompensated pressure;

	// Calibration coefficients
	int16_t AC1_, AC2_, AC3_, B1_, B2_, MB_, MC_, MD_;
	uint16_t AC4_, AC5_, AC6_;

	// Calculated coefficients;
	double B5_, p_;

	// i2c driver pointer;
	I2C_Driver *i2c_;
};

#endif