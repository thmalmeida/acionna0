#ifndef SSD1306_H__
#define SSD1306_H__

#include "i2c_driver.hpp"
#include "esp_log.h"

#include <string.h>

/* Command address */
#define SSD1306_ADDR_default				0x3C	// device default address
#define SSD1304_SA0_bit						0		// depends of D/C pin. By hardware.

#define SSD1306_ADDR						(SSD1306_ADDR_default 	| SSD1304_SA0_bit) //  - 0b 0011 110[SA0=0] or 0b 0011 110[SA0=1]

/* list of I2C cmd addresses */
/* 1. Fundamental command table */
#define SSD1306_REG_SHIFT_VERTICAL			0xD3	// vertical shift
#define SSD1306_RESET						0x7F	// Reset (or contrast control register?)
#define SSD1306_REG_CONSTRAST_CONTROL		0x81	// constrast control register. 256 values - D/C = 0, set contrast mode followed by A[7:0] level byte;
#define SSD1306_REG_NORMAL_DISPLAY_MODE		0xA4	// Entire display on - 8.5 page 23;
#define SSD1306_CMD_NORMAL_DISPLAY			0xA6	// set normal/inverse display
#define SSD1306_INVERSE						0xA7
#define SSD1306_CMD_DISPLAY_POWER			0xAE	// set display ON/OFF

// Hardware configuration registers
#define SSD1306_CMD_MUX_RATIO_RESET			0x3F	// A[5:0] - 0b0011 1111 reset command
#define SSD1306_DIV_CLOCK					0xD5	// A[3:0] from 1 to 16 division display fosc (verify it)
#define SSD1306_REG_DISPLAY_START_LINE		0x40	// display start line
#define SSD1306_REG_SCAN_DIRECTION			0xC0	// set COM output scan direction
#define SSD1306_REG_HARDWARE_CONFIG			0xDA	// Set COM Pins Hardware Configuration

// Hardware Configuration command
#define SSD1306_REG_MUX_RATIO				0xA8	// addr for set multiplex ratio
#define SSD1306_REG_SET_DISPLAY_OFFSET		0xD3	// set display offset on vertical shift

#define SSD1306_CMD_DISPLAY_OFFSET_RST		0x00	// display offset reset command (from 0 to 64)
#define SSD1306_CMD_SEGMENT_REMAP			0xA0	// x0 = 0 col. addr 0 to SEG0 (Reset); x0 = 1 col. addr 127 mapped to SEG0
#define SSD1306_CMD_H_SCROLL_DISABLE		0x2E	// horizontal scroll disable

// 3. Addressing Setting Command Table
#define SSD1306_CMD_LOWER_COLUMN			0x00	// Set the lower nibble of the column start address register for Page Addressing Mode using X[3:0] as data bits. The initial display line register is reset to 0000b after RESET.
#define SSD1306_CMD_HIGHER_COLUMN			0x10	// Set the higher nibble of the column start address register for Page Addressing Mode using X[3:0] as data bits. The initial display line register is reset to 0000b after RESET.
#define SSD1306_CMD_MEM_ADDR_MODE_HOR		0x00
#define SSD1306_CMD_MEM_ADDR_MODE_VER		0x01
#define SSD1306_CMD_MEM_ADDR_MODE_PAGE		0x02
#define SSD1306_CMD_MEM_ADDR_MODE			0x20
// horizontal/vertical addressing mode
#define SSD1306_CMD_COLUMN_ADDR				0x21	// Setup column start and end address


// 5. Timing & Driving Scheme Setting Command Table
#define SSD1306_CMD_CLK_DIV					0xD5	// display clock divide
#define SSD1306_CMD_NOP						0xE3	// no operation command. Section 10.1.20 on page 43

// charge pump
#define SSD1306_CMD_CHARGE_PUMP				0x8D	// charge pump cmd	

#define SSD1306_CO_BIT						0
#define SSD1306_DC_BIT						0

#define SSD1306_DEBUG						1


/* 2. Scrolling command table */

/* 4. Hardware Configuration (Panel resolution & layout related) Command Table */
#define SSD1306_NOP							0xE3	// command for no operation (NOP)

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

enum class ssd1306_addr_mode {
	horizontal = 0,
	vertical,
	page
};

class SSD1306 {
	public:
	
	SSD1306(I2C_Driver *i2c);

	void init(void);
	bool probe(void);
	void soft_reset(void);

	// void clear(void);

	void list_addr(void);

	void draw(void);

	private:

	// 1. Fundamental commands
	void set_contrast_(uint8_t value);			// Constrast value range is 0-255
	void entire_display_on_(uint8_t x0);		// entire display on - x0 = 0 follow RAM, x0 = 1 ignore RAM;
	void normal_inverse_display_(uint8_t x0);	// Normal or inverse display
	void power(uint8_t state);					// display on/off (normal <---> sleep)

	// 2. Scrolling Command Table

	// 3. Addressing Setting Command Table
	// type of memory addressing (horizontal, vertical or page)
	void memory_addr_mode_(ssd1306_addr_mode value);
	// Start Address for Page Addressing Mode
	void set_lower_column_(uint8_t col);
	// Set Higher Column Start Address for Page Addressing Mode
	void set_higher_column_(uint8_t col);
	// set start/stop column addr (hor/ver mode only)
	void set_column_addr_(uint8_t col_start, uint8_t col_stop);
	
	void page_start_(uint8_t page);

	// 4. Hardware Configuration (Panel resolution & layout related) Command Table
	// hardware config - mux ratio command 
	void set_display_start_line_(uint8_t line);

	void set_segment_remap_(uint8_t x0);

	void mux_ratio_(uint8_t cmd);

	void scan_direction_(uint8_t x3);

	void set_display_offset_(uint8_t cmd);

	void hardware_config_(uint8_t a4, uint8_t a5);

	// 5. Timing & Driving Scheme Setting Command Table
	void set_osc_frequency_(uint8_t div, uint8_t freq_type);

	// void nop_(void);							// no operation

	// 1. Charge Pump Command Table
	void charge_pump_en_(uint8_t a2);

	// 4. Hardware Configuration (Panel resolution & layout related) Command Table
	

	// control byte constructor;
	uint8_t ctrl_byte_(uint8_t co, uint8_t dc);

	ssd1306_addr_mode addr_mode_;

	I2C_Driver *i2c_;
	uint8_t status_byte_, data_raw_[6], first_init_ = 1;
};

#endif