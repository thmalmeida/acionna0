#ifndef __SSD1306_H__
#define __SSD1306_H__

#include "i2c_master.hpp"
#include "esp_log.h"

#include <string.h>

/* Command address */
#define SSD1306_ADDR_default			0x2C	// device default address
#define SSD1304_SA0_bit					1		// depends of D/C pin. By hardware. 

#define SSD1306_ADDR					(SSD1306_ADDR_default | SSD1304_SA0_bit) //  - 0b 0011 110[SA0=0] or 0b 0011 110[SA0=1]

/* list of I2C cmd addresses */
/* 1. Fundamental command table */
#define SSD1306_VER_SHIFT				0xD3	// vertical shift
#define SSD1306_RESET					0x7F	// Reset
#define SSD1306_CONSTRAST				0x81	// D/C = 0, set contrast mode followed by A[7:0] level byte;
#define SSD1306_NORMAL					0xA6	
#define SSD1306_INVERSE					0xA7
#define SSD1306_MUX_RATIO				0xA8	// addr for set multiplex ratio
#define SSD1306_DISPLAY_ON				0xAF	// turn on (normal mode)
#define SSD1306_DISPLAY_OFF				0xAE	// Power off or sleep mode

/* 2. Scrolling command table */


/* 3. Addressing Setting Command Table */


/* 4. Hardware Configuration (Panel resolution & layout related) Command Table */
#define SSD1306_NOP						0xE3	// command for no operation (NOP)

/* 5. Timing & Driving Scheme Setting Command Table */

/* Software initialization sequence
	01- Set MUX Ratio: 							A8h, 3Fh
	02- Set Display Offset: 					D3h, 00h
	03- Set Display Start Line: 				40h
	04- Set Segment re-map:						A0h/A1h
	05- Set COM Output Scan Direction:			C0h/C8h
	06- Set COM Pins hardware configuration:	DAh, 02
	07- Set Contrast Control:					81h, 7Fh
	08- Disable Entire Display On:				A4h
	09- Set Normal Display:						A6h
	10-Set Osc Frequency:						D5h, 80h
	11- Enable charge pump regulator:			8Dh, 14h
	12- Display On:								AFh
*/

/*
	Control byte occurs on first byte after slave address.
		
	Control byte:
		0b[Co][D/C]00 0000 (Co D/C follow by six zeros)

	where:
		Continuation bit (Co): 0- data bytes only; 
		Data/Command Selection bit (D/C): 0- following data byte as command (cmd Reg); 1- following data byte as data (GDDRAM).
	
	When the next byte is data (D/C = 1), the data byte is stored on Graphic Display Data RAM (GDDRAM) column
addr and increase automatically.

	Second byte is D[7:0].
*/

class ssd1306 {
	public:
	
	ssd1306(I2C_Master *i2c);

	void init(void);
	bool probe(void);
	void soft_reset(void);

	void clear(void);
	void power(bool state);

	private:

	I2C_Master *i2c_;
	uint8_t status_byte_, data_raw_[6], first_init_ = 1;
};

#endif