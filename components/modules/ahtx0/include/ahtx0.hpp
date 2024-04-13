#ifndef __AHTX0_HPP__
#define __AHTX0_HPP__

#include "i2c_driver.hpp"
#include "esp_log.h"

#include <string.h>

/*
*	Command address
*/

/* list of I2C addresses */
#define AHTX0_ADDR						0x38 // device address

/* list of command registers */
#define AHT10_REG_INIT					0xE1
#define AHT20_REG_INIT					0xBE // Is this a calibration command for measuring (0xE1 for AHT10)
#define AHTX0_REG_TRIG_MEAS				0xAC // start measurement register
#define AHTX0_REG_SOFT_RST				0xBA // soft reset register
#define AHTX0_REG_STATUS				0x71 //read status byte register 0b 0111 0001

/* calibration register controls */
#define AHT10_INIT_CTRL_NORMAL_MODE		0x00 //normal mode on/off       bit[6:5], for AHT1x only	0b 0000 0000
#define AHT10_INIT_CTRL_CYCLE_MODE		0x20 //cycle mode on/off        bit[6:5], for AHT1x only	0b 0010 0000
#define AHT10_INIT_CTRL_CMD_MODE		0x40 //command mode  on/off     bit[6:5], for AHT1x only	0b 0100 0000 or 0x60 with 0b 0110 0000
#define AHTX0_INIT_CTRL_CAL_ON			0x08 //calibration coeff on/off bit[3]						0b 0000 1000
#define AHTX0_INIT_CTRL_NOP				0x00 //NOP control, send after any "AHT1X_INIT_CTRL..."

// #define AHT10_MODE_NORMAL				0xA8 // set normal mode										0b 1010 1000

/* measurement register controls */
#define AHTX0_START_MEAS_CTRL			0x33 // measurement controls, suspect this is temperature and humidity DAC resolution
#define AHTX0_NOP_CTRL					0x00 // NOP control, send after start measurement control

/* sensor delays */
#define AHTX0_DELAY_CMD					50	 //delay between commands, in milliseconds. It was 10 ms only
#define AHTX0_DELAY_MEASUREMENT			80	 //wait for measurement to complete, in milliseconds (75 ms for AHT10)
#define AHTX0_DELAY_POWER_ON			40	 //wait for AHT1x to initialize after power-on, in milliseconds
#define AHTX0_DELAY_SOFT_RESET			20	 //less than 20 milliseconds

#define AHTX0_ERROR						0xFF // returns 255, if communication error is occurred

// #define AHTX0_DEBUG						1

/* AHTX0 - Status register byte with its bits description
	 _______________________________________________________________
*	|7		|6		|5		|4		|3		|2		|1		| 0		|
*	|status,|status_mode,___|CRC,	|calibr,|FIFO[en full empt		|
*
*	Status mode:
*		 	 0		0	NOR
*			 0		1	CYC
*			 1		x	CMD*
*
*
*	AHTX0 status reg byte
*	________________________________________________________________
*	|7		|6		|5		|4		|3		|2		|1		|0		|
*	|Busy	|	retain		|retain	|Cal EN	|		Retain			|
*/
#define AHTX0_STATUS_BIT_BUSY		7	// Busy indication. 1: busy, measurement state, 0: device idle
// #define AHT20_STATUS_BIT_MODE6		6	// current working mode. 00: NOR, 01: CYC, 1x CMD
// #define AHT20_STATUS_BIT_MODE5		5	// current working mode. 00: NOR, 01: CYC, 1x CMD
// #define AHT20_STATUS_BIT_CRC		4	// CRC (Reserved?)
#define AHTX0_STATUS_BIT_CAL		3	// Calibration enable. 1: calibrated, 0: not calibrated
// #define AHT20_STATUS_BIT_FIFO_EN	2	// (Reserved?)
// #define AHT20_STATUS_BIT_FIFO_FULL	1
// #define AHT20_STATUS_BIT_FIFO_EMPTY	0

enum class ahtx0_mode {
	NORMAL_MODE = 0,
	CYCLE_MODE,
	COMMAND_MODE,
	CALIBRATE_ONLY_MODE,
	A8_MODE
};

enum class ahtx0_model {
	aht10 = 0,
	aht20 = 1
};

class AHTX0 {
public:
		
	AHTX0(I2C_Driver *i2c, ahtx0_model model = ahtx0_model::aht10);
	/* @brief init function must called after power on/reset
	*  @param mode operation mode, leave it blank to normal mode select */
	void init(ahtx0_mode mode = ahtx0_mode::NORMAL_MODE);	

	bool probe(void);
	/* @brief Soft reset by command */
	void reset(void);

	/* @brief Fetch function trigger measures */
	void fetch(void);
	/* @brief Get humidity
	*  @return humidity in float */
	float humidity(void);

	/* @brief Get the temperature
	*  @return temperature in degrees using float point */
	float temperature(void);
	
	/* Debug functions */
	void print_status_bits(void);
	void print_raw_data(void);

private:
	// status reg byte - busy flag (bit 7); cal enable (bit 3);
	uint8_t status_(void);			// status register
	uint8_t busy_(void);			// busy flag is the bit 7 into status register
	uint8_t calibrated_(void);		// cal en flag is the bit 3 into status register

	void crc_check_(void);			// CRC error check (not implemented yet)

	uint8_t ahtx_reg_init_addr_;	// status register addr selected with AHT model;
	uint8_t data_len_;
	uint8_t status_byte_, data_raw_[7], first_init_ = 1;

	I2C_Driver *i2c_;
};

#endif