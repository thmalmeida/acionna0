#ifndef __PCY8575_HPP__
#define __PCY8575_HPP__

#include "i2c_master.hpp"
#include "esp_log.h"

#include <string.h>
#include <stdio.h>

/*
*	Command address
*/

/* list of I2C addresses */
#define PCY8575_ADDR					0x53	// device address

/* list of command registers */
#define PCY8575_REG_PROBE				0x00
#define PCY8575_REG_SOFT_RESET			0x01
#define PCY8575_REG_CONFIG				0x02
#define PCY8575_REG_PUT					0x03
#define PCY8575_REG_GET					0x04
#define PCY8575_REG_TEMPERATURE			0x05
#define PCY8575_REG_UPTIME				0x06
#define PCY8575_REG_IRMS				0x07

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

protocol example

PROBE:		write													read
Start | ADDR - R/W = 0 | PROBE | Stop | ... delay ... | Start | ADDR - R/W = 1 | read byte 1 |

SOFT RESET:	write
Start | ADDR - R/W = 0 | RESET | Stop |

CONFIG:		write				  P07-P00   P15-P00
Start | ADDR - R/W = 0 | CONFIG	| byte 0  | byte 1  | Stop |

PUT:		write
Start | ADDR - R/W = 0 | CONFIG | Stop | ... delay ... | Start | ADDR - R/W = 1 | read byte 1 |

GET:		write												 Read			  P07-P00   P15-P00
Start | ADDR - R/W = 0 | GET    | Stop | ... delay ... | Start | ADDR - R/W = 1 | byte 0  | byte 1 |

GET TEMP:	write												 Read					16 bits
Start | ADDR - R/W = 0 | TEMP   | Stop | ... delay ... | Start | ADDR - R/W = 1 | byte L  | byte H |

UPTIME:		write												 Read			     	32 bits
Start | ADDR - R/W = 0 | UPTIME | Stop | ... delay ... | Start | ADDR - R/W = 1 | byte L  | byte L | byte H | byte H |

IRMS:		write																		16 bits
Start | ADDR - R/W = 0 | UPTIME | Stop | ... delay ... | Start | ADDR - R/W = 1 | byte L  | byte H |
*/

class pcy8575 {

	public:

	pcy8575(I2C_Master *i2c);

	void init(uint8_t mode);
	bool probe(void);
	void soft_reset(void);
	void write(int pin, int level);
	int read(int pin);
	void put(uint16_t word);
	uint16_t get(void);
	uint16_t temperature(void);
	uint32_t uptime(void);
	uint16_t irms(void);
	
	private:

	I2C_Master *i2c_;
	uint16_t output_ = 0;
};

#endif