#ifndef __PCY8575_HPP__
#define __PCY8575_HPP__

#include "i2c_driver.hpp"
#include "esp_log.h"

#include <string.h>
#include <stdio.h>

/*
*	Command address
*/

/* list of I2C addresses */
#define PCY8575_ADDR					0x53	// device address

/* list of command registers - opcodes */
#define PCY8575_REG_PROBE		0x00
#define PCY8575_REG_SOFT_RESET	0x01
#define PCY8575_REG_CONFIG		0x02
#define PCY8575_REG_PUT			0x03
#define PCY8575_REG_GET			0x04
#define PCY8575_REG_TEMPERATURE	0x05
#define PCY8575_REG_UPTIME		0x06
#define PCY8575_REG_RST_REASON	0x07
#define PCY8575_REG_I_PROCESS	0x08
#define PCY8575_REG_IRMS		0x09
#define PCY8575_REG_I_DATA		0x0A
#define PCY8575_REG_TEST		0x0B
#define PCY8575_REG_I_SET_NP	0x0C
#define PCY8575_REG_I_GET_NP	0x0D

#define PCY8575_DELAY_CMD		10		// time delay between i2c write/read cmd in [ms]


/* write mode

There are 16 controlled pins
____________.________.________.
slave_addr|0| opcode | byte 1

opcode:
	- PROBE ok:		0x00
	- Soft RESET:	0x01
	- CONFIG ports:	0x02
	- PUT:	 		0x03
	- GET:			0x04
	- TEMP:			0x05
	- UPTIME:		0x06
	- IRMS:			0x07
	- RST_REASON:	0x08

protocol example

PROBE:		write													read
Start | ADDR - R/W = 0 | PROBE	| Stop | ... delay ... | Start | ADDR - R/W = 1 | byte 0 | Stop |

SOFT RESET:	write
Start | ADDR - R/W = 0 | RESET	| Stop |

CONFIG:		write				  P07-P00   P15-P00
Start | ADDR - R/W = 0 | CONFIG	| byte 0  | byte 1  | Stop |

PUT:		write
Start | ADDR - R/W = 0 | PUT    | byte 0  | byte 1  | Stop |

GET:		write												 Read			  P07-P00   P15-P00
Start | ADDR - R/W = 0 | GET    | Stop | ... delay ... | Start | ADDR - R/W = 1 | byte 0  | byte 1 | Stop |

GET TEMP:	write												 Read					16 bits
Start | ADDR - R/W = 0 | TEMP   | Stop | ... delay ... | Start | ADDR - R/W = 1 | byte L  | byte H | Stop |

UPTIME:		write												 Read			     	32 bits
Start | ADDR - R/W = 0 | UPTIME | Stop | ... delay ... | Start | ADDR - R/W = 1 | byte L  | byte L | byte H | byte H | Stop |

IRMS:		write																		16 bits
Start | ADDR - R/W = 0 | UPTIME | Stop | ... delay ... | Start | ADDR - R/W = 1 | byte L  | byte H | Stop |

RST REASON:	write												 Read				8 bits
Start | ADDR - R/W = 0 | REASON | Stop | ... delay ... | Start | ADDR - R/W = 1 | byte L  | Stop |
*/

#define N_SAMPLES 397

class pcy8575 {

	public:

	pcy8575(I2C_Driver *i2c);

	void init(uint8_t mode);
	bool probe(void);
	void soft_reset(void);
	void write(int pin, int level);
	int read(int pin);
	void put(uint16_t word);
	uint16_t get(void);
	void get1(void);
	uint16_t get2(void);
	uint16_t temperature(void);
	uint32_t uptime(void);
	void i_process(uint8_t mode);
	uint16_t irms(void);
	void i_data(void);
	void i_n_points(int length);
	uint16_t i_n_points(void);
	void data_test(void);
	uint8_t reset_reason(void);

	uint16_t n_samples = N_SAMPLES;

	uint16_t stream_array_raw[N_SAMPLES];
	
	private:

	I2C_Driver *i2c_;
	uint16_t output_ = 0;
};

#endif