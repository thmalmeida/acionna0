#ifndef SSD1306_H__
#define SSD1306_H__

#include "i2c_driver.hpp"
#include "esp_log.h"

#include <string.h>

#include "fonts.h"

// 1000 8
// 1001 9
// 1010 A
// 1011 B
// 1100 C
// 1101 D
// 1110 E
// 1111 F

/* 128x64 pixels SSD1306 OLED display drive */

/* Command address */
#define SSD1306_ADDR_default				0x3C	// device default address - 0b 0011 1100 - 0x3C
#define SSD1304_SA0_bit						0		// depends of D/C pin. By hardware.

#define SSD1306_ADDR						(SSD1306_ADDR_default 	| SSD1304_SA0_bit) //  - 0b 0011 110[SA0=0] or 0b 0011 110[SA0=1]

/* list of I2C cmd addresses */
/* 1. Fundamental command table */
#define SSD1306_CMD_SHIFT_VERTICAL			0xD3	// vertical shift
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
#define SSD1306_REG_SCAN_DIRECTION			0xC0	// set COM output scan direction/ 0xC8
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
#define SSD1306_CMD_PAGE_ADDR				0x22	// setup page start and end address
#define SSD1306_CMD_PAGE_START				0xB0	// setup page start range from 0xB0 (PAGE0) to 0xB7 (PAGE7)

// 5. Timing & Driving Scheme Setting Command Table
#define SSD1306_CMD_CLK_DIV					0xD5	// display clock divide
#define SSD1306_CMD_NOP						0xE3	// no operation command. Section 10.1.20 on page 43
#define SSD1306_CMD_VCOM_DESELECT_LEVEL		0xDB	// Set VcomH deselect level

// charge pump
#define SSD1306_CMD_CHARGE_PUMP				0x8D	// charge pump cmd	


// #define SSD1306_DEBUG						1
// #define SSD1306_DELAY_AFTER_CMD				1
#define SSD1306_CMD_DELAY					10		// delay after command [ms]


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

// Addressing mode
enum class ssd1306_addr_mode {
	horizontal = 0,
	vertical,
	page
};

// Control byte types
enum class ssd1306_ctrl_byte {
	cmd_array = 0x00,		// Command Stream		Co = 0; D/C = 0, 0b0000 0000
	data_array = 0x40		// data stream			Co = 0; D/C = 1, 0b0100 0000
	// cmd_byte = 0x80,		// Single Command byte  Co = 1; D/C = 0, 0b1000 0000
	// data_byte = 0xC0		// Single Data Byte		Co = 1; D/C = 1, 0b1100 0000
};

// VcomH deselect levels
enum class ssd1306_vcomh_level {
	vcc_065 = 0x00,
	vcc_077 = 0x20,			// ~0.77 x Vcc (Reset)
	vcc_083 = 0x30
};

class SSD1306 {
public:
	SSD1306(I2C_Driver *i2c);
	
	bool probe(void);
	void init(void);
	void config(void);
	void clear(void);
	void power(uint8_t state);					// display on/off (normal <---> sleep)
	/* @brief Set pointer to (x, y) coordinate 128x64 pixels
	*  @param x 0 to 127
	*  @param y 0 to 63
	*/
	void position(uint8_t x, uint8_t y);

	// Character print functions
	void print(char c);
	void print(uint8_t x, uint8_t y, char c);
	
	void print(const char *s);
	void print(uint8_t x, uint8_t y, const char *s);

	void print_Arial16x24(uint8_t x, uint8_t line, char c);
	void print_Arial16x24(uint8_t x, uint8_t line, const char* s);

	void print_Arial24x32(uint8_t x, uint8_t y, char c);
	void print_Arial24x32(uint8_t x, uint8_t y, const char *s);

	// draw tools
	void draw_pixel(uint8_t x, uint8_t y);
	void draw_point(uint8_t x, uint8_t y, uint8_t size);

private:
	// 1. Fundamental commands
	void set_contrast_(uint8_t value);			// Constrast value range is 0-255
	void entire_display_on_(uint8_t x0);		// entire display on - x0 = 0 follow RAM, x0 = 1 ignore RAM;
	void normal_inverse_display_(uint8_t x0);	// Normal or inverse display

	// 2. Scrolling Command Table
	/* @brief Scroll enable/disable
	* @param status true: enable; false: disable
	*/
	void scroll_(bool status);

	// 3. Addressing Setting Command Table

	/* @brief type of memory addressing (horizontal, vertical or page)
	* @param value ssd1306_addr_mode choose type
	*/
	void memory_addr_mode_(ssd1306_addr_mode value);

	// --- HORIZONTAL/VERTICAL MODES only
	/* @brief HOR/VER MODE - Setup page start and end address
	*  @param page_start A[2:0] : Page start Address, range : 0-7d, (RESET = 0d)
	*  @param page_end Set GDDRAM Page Start Address (PAGE0~PAGE7) for Page Addressing Mode using X[2:0].
	*/
	void page_addr_(uint8_t page_start, uint8_t page_end);
	/* @brief HOR/VER MODE - set start/stop column addr (hor/ver mode only)
	* @param col_start A[6:0] : Column start address, range : 0-127d, A0 (RESET=0d) 
	* @param col_stop B[6:0]: Column end address, range : 0-127d, (RESET =127d)
	*/
	void column_addr_(uint8_t col_start, uint8_t col_end);

	// --- PAGE MODE only
	/* @brief PAGE MODE - Set GDDRAM Page Start Address
	*  @param page 0 (PAGE0) to 7 (PAGE7) for Page Addressing Mode using X[2:0].
	*/
	void page_start_(uint8_t page);
	/* @brief PAGE MODE - Start Address for Page Addressing Mode
	*  @param col 
	*/
	void set_lower_column_(uint8_t col);
	/* @brief PAGE MODE - Set Higher Column Start Address for Page Addressing Mode
	*  @param col 
	*/
	void set_higher_column_(uint8_t col);

	// 4. Hardware Configuration (Panel resolution & layout related) Command Table
	/* @brief Set Multiplex Ratio to N+1 MUX
	* Range from 16MUX to 64MUX
	* 111111b of 0x3F is 63d or 64MUX (reset)
	* 0 to 14 are invalid entry
	* @param cmd range from 0x0F (16MUX) to 0x3F (64MUX)
	*/
	void mux_ratio_(uint8_t cmd);

	void set_display_offset_(uint8_t cmd);
	
	void set_display_start_line_(uint8_t line);
	
	void scan_direction_(uint8_t x3);

	/* @brief Set Segment Re-map 
	* @param x0 0, column addr 0 set to SEG0(RESET); 1, column addr 127 set to SEG0.
	*/
	void segment_remap_(uint8_t x0);

	void hardware_config_(uint8_t a5, uint8_t a4);

	// 5. Timing & Driving Scheme Setting Command Table
	/* @brief Set Display Clock Divide Ratio/ Oscillator Frequency (D5h)
	* @param div range from 1 to 16
	* @param freq has 16 values with default = 8
	*/
	void set_osc_frequency_(uint8_t div, uint8_t freq_type);

	void set_vcomh_deselect_level_(ssd1306_vcomh_level level);

	void nop_(void);						// no operation

	// 1. Charge Pump Command Table
	/* @brief Charge pump settings 
	* @param a2 0 = disable charge pump (RESET); 1 = en charge pump during display on
	*/
	void charge_pump_en_(uint8_t a2);

	uint8_t buffer[1024];			// 128*64 / 8 bytes for display RAM;

	ssd1306_addr_mode addr_mode_;

	I2C_Driver *i2c_;
	uint8_t status_byte_, data_raw_[6], first_init_ = 1;
};

#endif