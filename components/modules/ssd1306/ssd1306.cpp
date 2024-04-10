#include "ssd1306.hpp"

const char* TAG_SSD1306 = "SSD1306";

SSD1306::SSD1306(I2C_Driver *i2c) : i2c_(i2c) {
	// init();

	// init1();
	// init2();
	// init3();

	// hardware_config_(0, 1);
	// set_contrast_(0x9F);

	// init4();
	// power(1);
}
void SSD1306::init(void) {

	// 00- set display off - 0xAE
	power(0);

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
	set_contrast_(0x0F);

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

	// Set memory mode to horizontal
	memory_addr_mode_(ssd1306_addr_mode::horizontal);

	// Other configurations
	set_vcomh_deselect_level_(ssd1306_vcomh_level::vcc_077);

	// Enable display on to follow RAM;
	entire_display_on_(0);
	// scroll_(0);
	// segment_remap_(0);
	// scan_direction_(1);
}
void SSD1306::init1(void) {
	uint8_t data[5];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_DISPLAY_POWER;
	data[2] = SSD1306_DIV_CLOCK;
	data[3] = 0x80;		// suggested ratio
	data[4] = SSD1306_REG_MUX_RATIO;

	i2c_->write(SSD1306_ADDR, &data[0], 5);
	#ifdef SSD1306_DEBUG
	printf("init1: ");
	for(int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif	
}
void SSD1306::init2(void) {
	uint8_t data[5];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_SHIFT_VERTICAL;
	data[2] = 0x00;		// no offset
	data[3] = SSD1306_REG_DISPLAY_START_LINE | 0x00; // line 0
	data[4] = SSD1306_CMD_CHARGE_PUMP;

	i2c_->write(SSD1306_ADDR, &data[0], 5);
	#ifdef SSD1306_DEBUG
	printf("init2: ");
	for(int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif	
}
void SSD1306::init3(void) {
	uint8_t data[5];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_MEM_ADDR_MODE;
	data[2] = 0x00;		// act like ks0108
	data[3] = SSD1306_CMD_SEGMENT_REMAP | 0x01;
	data[4] = SSD1306_REG_SCAN_DIRECTION | (1 << 3);

	i2c_->write(SSD1306_ADDR, &data[0], 5);
	#ifdef SSD1306_DEBUG
	printf("init2: ");
	for(int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif	
}
void SSD1306::init4(void) {
	uint8_t data[5];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_MEM_ADDR_MODE;
	data[2] = 0x00;		// act like ks0108
	data[3] = SSD1306_CMD_SEGMENT_REMAP | 0x01;
	data[4] = SSD1306_REG_SCAN_DIRECTION | (1 << 3);

	i2c_->write(SSD1306_ADDR, &data[0], 5);
	#ifdef SSD1306_DEBUG
	printf("init2: ");
	for(int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif	
}
bool SSD1306::probe(void) {
	bool alive = i2c_->probe(SSD1306_ADDR);
	return alive;
}
void SSD1306::soft_reset(void) {
	// printf("cmd soft reset\n");
	i2c_->write(SSD1306_ADDR, SSD1306_RESET);
	// delay_ms(SSD1306_DELAY_SOFT_RESET);
}
void SSD1306::list_addr(void) {
	// i2c_->probe_find();
	i2c_->probe_list();
}
void SSD1306::position(uint8_t x, uint8_t y) {
	// memory_addr_mode_(ssd1306_addr_mode::horizontal);
	
	page_addr_(0, 7);
	column_addr_(0, 127);
}
void SSD1306::draw(void) {
	uint8_t data[1025];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::data_array);
	for(int i=1; i<sizeof(data); i++) {
		data[i] = 0x01;
	}

	i2c_->write(SSD1306_ADDR, &data[0], sizeof(data));
	#ifdef SSD1306_DEBUG
	printf("draw: ");
	for(int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}

void SSD1306::config(void) {

}

void SSD1306::refresh(void) {

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
	for(int i=0; i<sizeof(data); i++) {
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
	for(int i=0; i<sizeof(data); i++) {
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
	for(int i=0; i<sizeof(data); i++) {
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
	for(int i=0; i<sizeof(data); i++) {
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
	for(int i=0; i<sizeof(data); i++) {
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

	i2c_->write(SSD1306_ADDR, &data[0], 3);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("memory_addressing_mode_: ");
	for(int i=0; i<sizeof(data); i++) {
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

	i2c_->write(SSD1306_ADDR, &data[0], 4);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("page_addr_: ");
	for(int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::column_addr_(uint8_t col_start, uint8_t col_end) {
	uint8_t data[4];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_MEM_ADDR_MODE;
	data[2] = col_start;
	data[3] = col_end;

	i2c_->write(SSD1306_ADDR, &data[0], 3);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_column_addr_: ");
	for(int i=0; i<sizeof(data); i++) {
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

	i2c_->write(SSD1306_ADDR, &data[0], 3);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("page_addr_: ");
	for(int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::set_lower_column_(uint8_t col) {
	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_LOWER_COLUMN | col;

	i2c_->write(SSD1306_ADDR, &data[0], 2);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_lower_column: ");
	for(int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::set_higher_column_(uint8_t col) {
	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_HIGHER_COLUMN | col;

	i2c_->write(SSD1306_ADDR, &data[0], 2);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_higher_column: ");
	for(int i=0; i<sizeof(data); i++) {
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

	i2c_->write(SSD1306_ADDR, &data[0], 3);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("mux_ratio_: ");
	for(int i=0; i<sizeof(data); i++) {
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

	i2c_->write(SSD1306_ADDR, &data[0], 3);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_display_offset_: ");
	for(int i=0; i<sizeof(data); i++) {
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

	i2c_->write(SSD1306_ADDR, &data[0], 3);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_display_start_line_: ");
	for(int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::scan_direction_(uint8_t x3) {
	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_REG_SCAN_DIRECTION | (x3 << 3);

	i2c_->write(SSD1306_ADDR, &data[0], 2);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("scan_direction_: ");
	for(int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
void SSD1306::segment_remap_(uint8_t x0) {
	uint8_t data[2];
	data[0] = static_cast<uint8_t>(ssd1306_ctrl_byte::cmd_array);
	data[1] = SSD1306_CMD_SEGMENT_REMAP | x0;

	i2c_->write(SSD1306_ADDR, &data[0], 2);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("segment_remap_: ");
	for(int i=0; i<sizeof(data); i++) {
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

	i2c_->write(SSD1306_ADDR, &data[0], 3);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("hardware_config_: ");
	for(int i=0; i<sizeof(data); i++) {
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

	i2c_->write(SSD1306_ADDR, &data[0], 3);

	#ifdef SSD1306_DELAY_AFTER_CMD
	delay_ms(SSD1306_CMD_DELAY);
	#endif

	#ifdef SSD1306_DEBUG
	printf("set_osc_frequency_: ");
	for(int i=0; i<sizeof(data); i++) {
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
	for(int i=0; i<sizeof(data); i++) {
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
	for(int i=0; i<sizeof(data); i++) {
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
	for(int i=0; i<sizeof(data); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");
	#endif
}
