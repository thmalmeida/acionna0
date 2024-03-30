#ifndef SSD1306_H__
#define SSD1306_H__

#include "i2c_driver.hpp"
#include "esp_log.h"

#include <string.h>

/* Command address */
#define SSD1306_ADDR_default			0x3C	// device default address
#define SSD1304_SA0_bit					0		// depends of D/C pin. By hardware.

#define SSD1306_ADDR					(SSD1306_ADDR_default | SSD1304_SA0_bit) //  - 0b 0011 110[SA0=0] or 0b 0011 110[SA0=1]

/* list of I2C cmd addresses */
/* 1. Fundamental command table */
#define SSD1306_SHIFT_VERTICAL			0xD3	// vertical shift
#define SSD1306_RESET					0x7F	// Reset (or contrast control register?)
#define SSD1306_CONSTRAST				0x81	// D/C = 0, set contrast mode followed by A[7:0] level byte;
#define SSD1306_NORMAL_DISPLAY_MODE		0xA4	// 8.5 page 23;
#define SSD1306_NORMAL					0xA6	
#define SSD1306_INVERSE					0xA7

// Hardware configuration registers
#define SSD1306_CMD_MUX_RATIO_RESET		0x3F	// A[5:0] - 0b0011 1111 reset command
#define SSD1306_DIV_CLOCK				0xD5	// A[3:0] from 1 to 16 division display fosc (verify it)
#define SSD1306_REG_DISPLAY_START_LINE	0x40	// display start line
#define SSD1306_REG_SCAN_DIRECTION		0xC0	// set COM output scan direction
#define SSD1306_REG_HARDWARE_CONFIG		0xDA	// Set COM Pins Hardware Configuration

// Fundamental command table
#define SSD1306_REG_CONTRAST_CONTROL	0x81	// constrast control register. 256 values
#define SSD1306_REG_ENTIRE_DISPLAY_ON	0xA4	// Entire display on
#define SSD1306_CMD_NORMAL_DISPLAY		0xA6	// set normal/inverse display
#define SSD1306_CMD_DISPLAY_POWER		0xAE	// set display ON/OFF

// Hardware Configuration command
#define SSD1306_REG_MUX_RATIO			0xA8	// addr for set multiplex ratio
#define SSD1306_REG_SET_DISPLAY_OFFSET	0xD3	// set display offset on vertical shift

#define SSD1306_CMD_DISPLAY_OFFSET_RST	0x00	// display offset reset command (from 0 to 64)
#define SSD1306_CMD_SEGMENT_REMAP		0xA0	// x0 = 0 col. addr 0 to SEG0 (Reset); x0 = 1 col. addr 127 mapped to SEG0
#define SSD1306_CMD_H_SCROLL_DISABLE	0x2E	// horizontal scroll disable

// 5. Timing & Driving Scheme Setting Command Table
#define SSD1306_CMD_CLK_DIV				0xD5	// display clock divide
#define SSD1306_CMD_NOP					0xE3	// no operation command. Section 10.1.20 on page 43

// charge pump
#define SSD1306_CMD_CHARGE_PUMP			0x8D	// charge pump cmd	


/* 2. Scrolling command table */


/* 3. Addressing Setting Command Table */


/* 4. Hardware Configuration (Panel resolution & layout related) Command Table */
#define SSD1306_NOP						0xE3	// command for no operation (NOP)

/* 5. Timing & Driving Scheme Setting Command Table */

/* Software initialization sequence
	01- Reset MUX Ratio: 						A8h, 3Fh
	02- Set Display Offset: 					D3h, 00h
	03- Set Display Start Line: 				40h
	04- Set Segment re-map:						A0h/A1h
	05- Set COM Output Scan Direction:			C0h/C8h
	06- Set COM Pins hardware configuration:	DAh, 02
	07- Set Contrast Control:					81h, 7Fh
	08- Disable Entire Display On:				A4h
	09- Set Normal Display:						A6h
	10- Set Osc Frequency:						D5h, 80h
	11- Enable charge pump regulator:			8Dh, 14h
	12- Display On:								AFh
*/

/*
	Control byte occurs on first byte after slave address.
		
	Control byte:
		0b[Co][D/C]00 0000 (Co D/C follow by six zeros)

	where:
		Continuation bit (Co): 0- data bytes only; 1- 
		Data/Command Selection bit (D/C): 0- following data byte as command (cmd Reg); 1- following byte as display data to GDDRAM.
	
	When the next byte (after slave addr byte) has the bit D/C = 1, the data byte is stored on Graphic Display Data RAM (GDDRAM) column
addr and increase automatically.

	Second byte is D[7:0].
*/

class SSD1306 {
	public:
	
	SSD1306(I2C_Driver *i2c);

	void init(void);
	bool probe(void);
	void soft_reset(void);

	// void clear(void);
	void power(uint8_t state);

	void list_addr(void);

	private:

	// hardware config - mux ratio command 
	void mux_ratio_(uint8_t cmd);
	void hardware_config_(uint8_t a4, uint8_t a5);


	/* Fundamental commands */
	void set_contrast_(uint8_t value);			// Constrast value range is 0-255
	void entire_display_on_(uint8_t x0);		// entire display on - x0 = 0 follow RAM, x0 = 1 ignore RAM;
	void set_normal_display_(uint8_t a);		// Normal or inverse display

	// Timing & Driving Scheme Setting Command Table
	void set_osc_frequency_(uint8_t div, uint8_t freq_type);

	// void nop_(void);							// no operation


	void scan_direction_(uint8_t x3);

	void set_display_offset_(uint8_t cmd);

	void set_display_start_line_(uint8_t line);
	
	void set_segment_remap_(uint8_t x0);

	// 1. Charge Pump Command Table
	void charge_pump_en_(uint8_t a2);

	

	// control byte constructor;
	uint8_t ctrl_byte_(uint8_t co, uint8_t dc);


	I2C_Driver *i2c_;
	uint8_t status_byte_, data_raw_[6], first_init_ = 1;
};

#endif