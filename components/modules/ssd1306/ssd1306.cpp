#include "ssd1306.hpp"

/* created by thmalmeida on 20240401
*
* lasts modified: 20240507
*
*/

const char* TAG_SSD1306 = "SSD1306";

SSD1306::SSD1306(I2C_Driver *i2c) : i2c_(i2c) {
	init();

	// init1();
	// init2();
	// init3();

	// hardware_config_(0, 1);
	// set_contrast_(0x9F);

	// init4();
	// power(1);
}
bool SSD1306::probe(void) {
	bool alive = i2c_->probe(SSD1306_ADDR);
	return alive;
}
void SSD1306::init(void) {

	// 00- set display off - 0xAE
	power(0);
	delay_ms(100);

	// 01- Set mux ratio for - 0xA8, 0x3F
	mux_ratio_(SSD1306_CMD_MUX_RATIO_RESET);
	
	// 02- Set display offset - 
	set_display_offset_(SSD1306_CMD_DISPLAY_OFFSET_RST);

	// 03- Set display start line
	set_display_start_line_(0);

	// 04- Set segment re-map
	segment_remap_(0);

	// 05- set COM Output scan direction
	scan_direction_(0);

	// 06- Set COM Pins hardware configuration
	hardware_config_(0, 1);

	// 07- set contrast control
	set_contrast_(0x1F);

	// 08- disable entire display on
	entire_display_on_(1);

	// 09- set normal/inverse display
	normal_inverse_display_(0);

	// 10- set osc frequency
	set_osc_frequency_(0, 8);

	// 11- charge pump set
	charge_pump_en_(1);

	// 12- display on
	power(1);

	// Other configurations
	set_vcomh_deselect_level_(ssd1306_vcomh_level::vcc_077);

	// Enable display on to follow RAM;
	entire_display_on_(0);

	memory_addr_mode_(ssd1306_addr_mode::horizontal);
	position(0, 0);


	// scroll_(0);
	// segment_remap_(0);
	// scan_direction_(1);
}
void SSD1306::clear(void) {

	position(0, 0);

	uint8_t data[1025];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::data_array);
	
	for(int k = 1; k<1025; k++)
		data[k] = 0x00;

	// for(int j = 0; j<8; j++) {
		// Page mode
		// page_start_(j);
		// set_lower_column_(0);
		// set_higher_column_(0);

		// HOR mode
		// page_addr_(j, 7);
		// column_addr_(0, 127);

		i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

		#ifdef SSD1306_DEBUG
		printf("clear: ");
		for(int i=0; i<sizeof(data); i++) {
			printf("0x%02x ", data[i]);
		}
		printf("\n");
		#endif
	// }
}
void SSD1306::position(uint8_t x, uint8_t y) {
	// memory_addr_mode_(ssd1306_addr_mode::horizontal);
	page_addr_(y/8, 7);
	column_addr_(x, 127);
}
void SSD1306::draw_point(uint8_t x, uint8_t y, uint8_t size) {

}
void SSD1306::draw_pixel(uint8_t x, uint8_t y) {
	
	uint8_t line, line_pixel;
	line = x/8;
	line_pixel = x%8;

	page_addr_(line, 7);
	column_addr_(y, 127);

	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::data_array);
	data[1] = (1 << line_pixel);

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("draw_pixel: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("x:%d, y:%d\n", x, y);
	printf("line:%d, y:%d\n", line, y);
	printf("\n");
	#endif	
}
void SSD1306::config(void) {
	// Set memory mode to _______
	memory_addr_mode_(ssd1306_addr_mode::horizontal);

	// Page Mode
	// Set the page start address of the target display location by command B0h to B7h.
	// page_start_(0);

	// Set the lower start column address of pointer by command 00h~0Fh.
	// set_lower_column_(0);

	// Set the upper start column address of pointer by command 10h~1Fh.
	// set_higher_column_(0);


	// Horizontal Mode
	column_addr_(0, 127);
	page_addr_(0, 7);
}
void SSD1306::print(char c) {
	int offset = 5*(static_cast<int>(c)-32);
	uint8_t data[7];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::data_array);
	for(int i=0; i<5; i++) {
		data[i+1] = FontNew5x8[offset+i];
	}

	data[6] = 0x00;

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("print: %c, %d, o:%d  - ", c, static_cast<int>(c), offset);
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::print(const char *s) {
	while(*s) {
		print(*s);
		s++;
	}
}
void SSD1306::print(const char *s, uint8_t x, uint8_t y) {
	position(x, y);
	print(s);
}
void SSD1306::print_Arial16x24(const char* s, uint8_t x, uint8_t y) {
	// Arial16x24 font
	int k = 0;
	while(*s) {
		print_Arial16x24(*s, x + 16*k, y);
		s++;
		k++;
	}
}
void SSD1306::print_Arial16x24(char c, uint8_t x, uint8_t y) {
	// Arial16x24 font
	// Each character has 49 bytes (exclude the first one)
	int n_bytes = 49;
	int n_cols = 16;
	int n_rows = 3;

	int offset = n_bytes*(static_cast<int>(c)-32);

	uint8_t data[n_cols+1];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::data_array); 

	for(int j=0; j<n_rows; j++) {
		for(int i=1; i<n_cols+1; i++) {
			data[i] = Arial16x24[offset + ((i-1)*n_rows + (j+1))];
		}

		// Bytes write sequence (16 bytes length at column * 3 bytes row (3 * 8 bits = 24 bits) = 48 bytes).
		// 1 4 7 10 13 ... 43 46
		// 2 5 8 11 14 ... 44 47
		// 3 6 9 12 15 ... 45 48

		position(x, 8*j+y);
		i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));
	}

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("print: %c, %d, o:%d  - ", c, static_cast<int>(c), offset);
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::print_Arial24x32(char c, uint8_t x, uint8_t y) {
	// Arial24x32 font
	// Each character has 121 bytes (exclude the first one)
	int n_bytes = 97;
	int n_cols = 24;
	int n_rows = 4;

	int offset = n_bytes*(static_cast<int>(c)-32);

	uint8_t data[n_cols+1];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::data_array); 

	for(int j=0; j<n_rows; j++) {
		for(int i=1; i<n_cols+1; i++) {
			data[i] = Arial24x32[offset + ((i-1)*n_rows + (j+1))];
		}

		// 01 05 09 13 17 ... (16 bytes length)
		// 02 06 10 14 18 ...
		// 03 07 11 15 19 ...
		// 04 08 12 16 20

		position(x, 8*j+y);
		i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));
	}
}
void SSD1306::print_Arial24x32(const char *s, uint8_t x, uint8_t y) {
	// Arial24x32 font
	int k = 0;
	while(*s) {
		print_Arial24x32(*s, x + 24*k, y);
		s++;
		k++;
	}
}
void SSD1306::print_Arial24x32_Numbers(char c, uint8_t x, uint8_t y) {
	// Arial24x32 font
	// Each character has 121 bytes (exclude the first one)
	int n_bytes = 97;
	int n_cols = 24;
	int n_rows = 4;

	int offset = n_bytes*(static_cast<int>(c)-32-16);

	uint8_t data[n_cols+1];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::data_array); 

	for(int j=0; j<n_rows; j++) {
		for(int i=1; i<n_cols+1; i++) {
			if(c == ' ') {
				data[i] = 0x00;
			} else {
				data[i] = Arial24x32_Numbers[offset + ((i-1)*n_rows + (j+1))];
			}
		}

		// 01 05 09 13 17 ... (16 bytes length)
		// 02 06 10 14 18 ...
		// 03 07 11 15 19 ...
		// 04 08 12 16 20

		position(x, 8*j+y);
		i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));
	}
}
void SSD1306::print_Arial24x32_Numbers(const char *s, uint8_t x, uint8_t y) {
	// Arial24x32 font
	int k = 0;
	while(*s) {
		print_Arial24x32_Numbers(*s, x + 24*k, y);
		s++;
		k++;
	}
}


// 1. Fundamental Command Table
void SSD1306::set_contrast_(uint8_t value) {
	uint8_t data[3];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_REG_CONSTRAST_CONTROL;
	data[2] = value;

	i2c_->write(SSD1306_ADDR, &data[0], 3);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_contrast_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::entire_display_on_(uint8_t x0) {
	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_REG_NORMAL_DISPLAY_MODE | x0;

	i2c_->write(SSD1306_ADDR, &data[0], 2);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("entire_display_on_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::normal_inverse_display_(uint8_t x0) {
	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_NORMAL_DISPLAY | x0;

	i2c_->write(SSD1306_ADDR, &data[0], 2);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_normal_display_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::power(uint8_t x0) {
	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_DISPLAY_POWER | (x0 << 0);

	i2c_->write(SSD1306_ADDR, &data[0], 2);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("power: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}

// 2. Scrolling Command Table
void SSD1306::scroll_(bool status) {
	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_H_SCROLL_DISABLE | static_cast<uint8_t>(status);

	i2c_->write(SSD1306_ADDR, &data[0], 2);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("scroll_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif	
}

// 3. Addressing Setting Command Table
void SSD1306::memory_addr_mode_(ssd1306_addr_mode value) {
	
	uint8_t data[3];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_MEM_ADDR_MODE;
	data[2] = static_cast<uint8_t>(value) & 0x03;

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("memory_addressing_mode_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::page_addr_(uint8_t page_start, uint8_t page_end) {
	uint8_t data[4];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_PAGE_ADDR;
	data[2] = page_start;
	data[3] = page_end;

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("page_addr_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::column_addr_(uint8_t col_start, uint8_t col_end) {
	uint8_t data[4];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_COLUMN_ADDR;
	data[2] = col_start;
	data[3] = col_end;

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_column_addr_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::page_start_(uint8_t page) {
	uint8_t data[3];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_PAGE_START;
	data[2] = page;

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("page_addr_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::set_lower_column_(uint8_t col) {
	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_LOWER_COLUMN | col;

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_lower_column: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::set_higher_column_(uint8_t col) {
	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_HIGHER_COLUMN | col;

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_higher_column: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}

// 4. Hardware Configuration commands
void SSD1306::mux_ratio_(uint8_t cmd) {
	uint8_t data[3];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_REG_MUX_RATIO;
	data[2] = cmd;

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("mux_ratio_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::set_display_offset_(uint8_t cmd) {
	uint8_t data[3];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_REG_SET_DISPLAY_OFFSET;
	data[2] = cmd;

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_display_offset_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::set_display_start_line_(uint8_t line) {
	uint8_t data[3];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_REG_DISPLAY_START_LINE;
	data[2] = line;

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_display_start_line_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::scan_direction_(uint8_t x3) {
	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_REG_SCAN_DIRECTION | (x3 << 3);

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("scan_direction_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::segment_remap_(uint8_t x0) {
	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_SEGMENT_REMAP | x0;

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("segment_remap_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::hardware_config_(uint8_t a5, uint8_t a4) {
	uint8_t data[3];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_REG_HARDWARE_CONFIG;
	data[2] = 0x02 | (a5 << 1) | (a4 << 4);

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("hardware_config_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}

// 5. Timing & Driving Scheme Setting Command Table
void SSD1306::set_osc_frequency_(uint8_t div, uint8_t freq_type) {
	uint8_t data[3];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_CLK_DIV;
	data[2] = (freq_type << 4) | (div << 0);

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_osc_frequency_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::set_vcomh_deselect_level_(ssd1306_vcomh_level level) {
	uint8_t data[3];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_VCOM_DESELECT_LEVEL;
	data[2] = static_cast<uint8_t>(level);

	i2c_->write(SSD1306_ADDR, &data[0], 3);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_vcomh_deselect_level_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif	
}
void SSD1306::nop_(void) {
	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_NOP;

	i2c_->write(SSD1306_ADDR, &data[0], 2);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("nop_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif	
}

// 1. Charge Pump Command Table
void SSD1306::charge_pump_en_(uint8_t a2) {
	uint8_t data[3];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_CHARGE_PUMP;
	data[2] = 0x10 | (a2 << 2);

	i2c_->write(SSD1306_ADDR, &data[0], 3);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("charge_pump_en_: ");
	for(unsigned int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}








// void SSD1306::soft_reset(void) {
// 	// printf("cmd soft reset\n");
// 	i2c_->write(SSD1306_ADDR, SSD1306_RESET);
// 	// delay_ms(SSD1306_DELAY_SOFT_RESET);
// }

// void SSD1306::init1(void) {
// 	uint8_t data[5];
// 	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
// 	data[1] = SSD1306_CMD_DISPLAY_POWER;
// 	data[2] = SSD1306_DIV_CLOCK;
// 	data[3] = 0x80;		// suggested ratio
// 	data[4] = SSD1306_REG_MUX_RATIO;

// 	i2c_->write(SSD1306_ADDR, &data[0], 5);
// 	#ifdef SSD1306_DEBUG
// 	printf("init1: ");
// 	for(unsigned int i=0; i<sizeof(data); i++) {
// 		printf("0x%02x ", data[i]);
// 	}
// 	printf("\n");
// 	#endif	
// }
// void SSD1306::init2(void) {
// 	uint8_t data[5];
// 	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
// 	data[1] = SSD1306_CMD_SHIFT_VERTICAL;
// 	data[2] = 0x00;		// no offset
// 	data[3] = SSD1306_REG_DISPLAY_START_LINE | 0x00; // line 0
// 	data[4] = SSD1306_CMD_CHARGE_PUMP;

// 	i2c_->write(SSD1306_ADDR, &data[0], 5);
// 	#ifdef SSD1306_DEBUG
// 	printf("init2: ");
// 	for(unsigned int i=0; i<sizeof(data); i++) {
// 		printf("0x%02x ", data[i]);
// 	}
// 	printf("\n");
// 	#endif	
// }
// void SSD1306::init3(void) {
// 	uint8_t data[5];
// 	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
// 	data[1] = SSD1306_CMD_MEM_ADDR_MODE;
// 	data[2] = 0x00;		// act like ks0108
// 	data[3] = SSD1306_CMD_SEGMENT_REMAP | 0x01;
// 	data[4] = SSD1306_REG_SCAN_DIRECTION | (1 << 3);

// 	i2c_->write(SSD1306_ADDR, &data[0], 5);
// 	#ifdef SSD1306_DEBUG
// 	printf("init2: ");
// 	for(unsigned int i=0; i<sizeof(data); i++) {
// 		printf("0x%02x ", data[i]);
// 	}
// 	printf("\n");
// 	#endif	
// }
// void SSD1306::init4(void) {
// 	uint8_t data[5];
// 	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
// 	data[1] = SSD1306_CMD_MEM_ADDR_MODE;
// 	data[2] = 0x00;		// act like ks0108
// 	data[3] = SSD1306_CMD_SEGMENT_REMAP | 0x01;
// 	data[4] = SSD1306_REG_SCAN_DIRECTION | (1 << 3);

// 	i2c_->write(SSD1306_ADDR, &data[0], 5);
// 	#ifdef SSD1306_DEBUG
// 	printf("init2: ");
// 	for(unsigned int i=0; i<sizeof(data); i++) {
// 		printf("0x%02x ", data[i]);
// 	}
// 	printf("\n");
// 	#endif	
// }
