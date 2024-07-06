#ifndef ST7735_HPP__
#define ST7735_HPP__

#include "esp_log.h"

#include "spi_driver.hpp"
// #include "fonts.h"

// 1000 8
// 1001 9
// 1010 A
// 1011 B
// 1100 C
// 1101 D
// 1110 E
// 1111 F

/* 132Hx162V x 18 bits color (Full color) ST7735S TFT 1.8 inch display drive */

/*  Ref.: page 30 of ST7735S datasheet v1.4
 *  D/CX pin - 0 (low), command byte; 1 (high) to display data RAM.
 * 
 *  Any instruction can be sent in any order to the driver;
 *  The SPI peripheral is initialized when CSX (Chip Select) is high;
 *  SPI transmission the MSB is transmitted first and;
*/

class ST7735 {
public:
	ST7735(void) {}

	void init(void);
	
private:

};



#endif