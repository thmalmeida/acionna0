#ifndef __BPM180_HPP__
#define __BPM180_HPP__

#include "i2c_master.hpp"
#include "esp_log.h"

#define BMP180_ADDR             0x77
#define BMP180_ADDR_TEMP        0x2E	// Conversion time 4.5 ms
#define BMP180_ADDR_PRESS_OSS_0 0x34	// 4.5 ms
#define BMP180_ADDR_PRESS_OSS_1 0x74	// 7.5 ms
#define BMP180_ADDR_PRESS_OSS_2 0xB4	// 13.5 ms
#define BMP180_ADDR_PRESS_OSS_3 0xF4	// 25.5 ms
#define BMP180_ADDR_ID          0xD0	// id reset state value (for ok response)	     
#define BMP180_ADDR_SOFT_RST	0xE0	// register to soft reset

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

class bmp180 {

    public:
    bmp180(I2C_Master *i2c);
    void init(void);
    void probe(void);
    void get_temp(void);
    void get_pres(void);

    int16_t AC1, AC2, AC3, B1, B2, MB, MC, MD;
    uint16_t AC4, AC5, AC6;

    private:
    I2C_Master *i2c_;

};

#endif