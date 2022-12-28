#ifndef __AHT10_HPP__
#define __AHT10_HPP__

#include "i2c_master.hpp"
#include "esp_log.h"

#include <string.h>

/*
*	Command address
*/

/* list of I2C addresses */
#define AHT10_ADDR						0x38	// device address

/* list of command registers */
#define AHT10_REG_INIT					0xE1 // Is this a calibration command for measuring
#define AHT10_REG_TRIG_MEAS				0xAC // start measurement register
#define AHT10_REG_SOFT_RST				0xBA // soft reset register
#define AHT10_REG_READ_STATUS			0x71 //read status byte register 0b 0111 0001

/* calibration register controls */
#define AHT10_INIT_CTRL_NORMAL_MODE		0x00 //normal mode on/off       bit[6:5], for AHT1x only	0b 0000 0000
#define AHT10_INIT_CTRL_CYCLE_MODE		0x20 //cycle mode on/off        bit[6:5], for AHT1x only	0b 0010 0000
#define AHT10_INIT_CTRL_CMD_MODE		0x40 //command mode  on/off     bit[6:5], for AHT1x only	0b 0100 0000 or 0x60 with 0b 0110 0000
#define AHT10_INIT_CTRL_CAL_ON			0x08 //calibration coeff on/off bit[3]						0b 0000 1000
#define AHT10_INIT_CTRL_NOP				0x00 //NOP control, send after any "AHT1X_INIT_CTRL..."

#define AHT10_MODE_NORMAL				0xA8 // set normal mode										0b 1010 1000

/* measurement register controls */
#define AHT10_START_MEAS_CTRL			0x33 // measurement controls, suspect this is temperature and humidity DAC resolution
#define AHT10_NOP_CTRL					0x00 // NOP control, send after start measurement control

/* sensor delays */
#define AHT10_DELAY_CMD					10 //delay between commands, in milliseconds
#define AHT10_DELAY_MEASUREMENT			75 //wait for measurement to complete, in milliseconds
#define AHT10_DELAY_POWER_ON			40 //wait for AHT1x to initialize after power-on, in milliseconds
#define AHT10_DELAY_SOFT_RESET			20 //less than 20 milliseconds

#define AHT10_ERROR						0xFF				// returns 255, if communication error is occurred

/* Status bit description
	 _________________________________________________________
*	|7		|6		5		|4		|3		|2		1		0 |
*	|status,|status_mode,___|CRC,	|calibr,|FIFO[en full empt|
*
*	Status mode:
		 	 0		0	NOR
			 0		1	CYC
			 1		x	CMD
*/
#define AHT10_STATUS_BIT_BUSY		7	// Busy indication. 1: busy, measurement state, 0: device idle
#define AHT10_STATUS_BIT_MODE6		6	// current working mode. 00: NOR, 01: CYC, 1x CMD
#define AHT10_STATUS_BIT_MODE5		5	// current working mode. 00: NOR, 01: CYC, 1x CMD
#define AHT10_STATUS_BIT_CRC		4	// CRC (Reserved?)
#define AHT10_STATUS_BIT_CAL		3	// Calibration enable. 1: calibrated, 0: not calibrated
#define AHT10_STATUS_BIT_FIFO_EN	2	// (Reserved?)
#define AHT10_STATUS_BIT_FIFO_FULL	1
#define AHT10_STATUS_BIT_FIFO_EMPTY	0

class aht10 {
	public:
	
	aht10(I2C_Master *i2c);

	void init(uint8_t mode);
	bool probe(void);
	void soft_reset(void);
		
	uint8_t read_status_register(void);
	void trig_meas(void);
	bool get_status_bit(uint8_t bit_value, bool new_read);
	float get_humidity(void);
	float get_temperature(void);
	
	void print_status_bits(void);
	void print_raw_data(void);
	
	private:

	I2C_Master *i2c_;
	uint8_t status_byte_, data_raw_[6], first_init_ = 1;
};

#endif