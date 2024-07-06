#ifndef I2C_Driver_H__
#define I2C_Driver_H__

#include <cstdint>
#include <cstring>

#include "delay.hpp"

#include "esp_log.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"

// #include "pinout.hpp"

// Message returns for read, write errors and ok communication
enum class i2c_ans {
	ok = 0,
	error_read,
	error_write,
	error_ack
};

#define I2C_SPEED_FAST_HZ	400000
#define I2C_SPEED_NORMAL_HZ	100000

#define I2C_COMMAND_WAIT_MS	100		// -1 means wait forever

/* 
	I2C protocol is coordinated by master. Always start by write before read command. It requires a minimum delay time to slave process
the requested data.
	
	The master device initiates the data communication by a start condition. This condition is established by
pulling the SDA from HIGH to LOW while the SCL stays HIGH.
		___________
	SDA            \__________________________
                _______        ______
	SCL _______/       \______/      \______/


	The stop condition is defined by pulling the "SDA input" from LOW to HIGH while the SCL stays HIGH.

     			Stop condition
	                _________________________
	SDA ___________/
                _______        ______
	SCL _______/       \______/      \______/

	Definition of the data transfer condition
				read   |master change
     		   |stable |change|	 
	    __________________ __________________
	SDA __________________X__________________
                _______        ______
	SCL _______/       \______/      \______/

Protocol specification by type of request (R/W)

Write has R/W = 0;
Read has R/W = 1;

Command: X, Type: write.
|_________________________________write_____________________________________|
|                 | 7-bits + 1-bit | 1-byte | ... | 1-byte |                |
| Start condition | ADDR + R/W = 0 | cmd_X  | ... | cmd_N  | Stop condition |

Command: Y, Type: read.
|___________________________write____________________________| little delay |____________________________________Read_________________________________________| 
|                 | 7-bits + 1-bit | 1-byte |                |   between    |                | 7-bits + 1-bit | 1-byte | 1-byte |...| 1-byte |                |
| Start condition | ADDR + R/W = 1 | cmd_X  | Stop condition |   Starts ... | Start condition| ADDR + R/W = 1 | byte 0 | byte 1 |...| byte n | Stop condition |

Command: Z, Type: read only (some modules require read only to answer (process takes time) a write command required)
|____________________________________Read_________________________________________| 
|                | 7-bits + 1-bit | 1-byte | 1-byte |...| 1-byte |                |
| Start condition| ADDR + R/W = 1 | byte 0 | byte 1 |...| byte n | Stop condition |

	The acknowledge bit (ACK) is clocked by SCL but read by master SDA. It's a 1-bit only.
It's optional and can be enable or disable. And remember, the I2C hardware takes care about this bit. 
The following Figure shows the ACK bit on a write command. It was not showed before for simplicity purpose.

Command: X, Type: write.
|________________________________write_____________________________________|
|                 | 7-bits + 1-bit | 1-bit | 1-byte | 1-bit | ... | 1-byte | 1-bit |                |
| Start condition | ADDR + R/W = 0 |  ACK  | cmd_X  |  ACK  | ... | cmd_N  | ACK   | Stop condition |

*/

class I2C_Driver{
	public:

		I2C_Driver(int port, int scl, int sda, uint32_t freq = I2C_SPEED_NORMAL_HZ);
		~I2C_Driver();

		// i2c driver initialize
		void init(void);

		// i2c driver deinitialize
		void deinit(void);

		i2c_ans write(uint8_t slave_addr, uint8_t reg);
		i2c_ans write(uint8_t slave_addr, uint8_t reg, uint8_t data);
		i2c_ans write(uint8_t slave_addr, uint8_t reg, uint8_t *data, size_t len);
		i2c_ans write(uint8_t slave_addr, uint8_t *data, size_t len);

		// The following reads is precedded by write before read;
		i2c_ans read(uint8_t slave_addr, uint8_t *data);
		i2c_ans read(uint8_t slave_addr, uint8_t *data, size_t len);	// read only
		i2c_ans read(uint8_t slave_addr, uint8_t reg, uint8_t *data);
		i2c_ans read(uint8_t slave_addr, uint8_t reg, uint8_t *data, size_t len);
		i2c_ans read(uint8_t slave_address, const uint8_t *write_buffer, size_t write_buffer_len, uint8_t *read_buffer, size_t read_buffer_len);		
		// i2c_ans read(uint8_t slave_address, const uint8_t* write_buffer, size_t write_buffer_len, uint8_t* read_buffer, size_t read_buffer_len);

		// Check if that addr is alive
		bool probe(uint8_t addr) noexcept;

		// Advanced
		uint8_t probe_find(uint8_t addr_init = 0);
		void probe_list(void);

	protected:
		int	i2c_master_port_;
		int pin_scl_;
		int pin_sda_;
		uint32_t freq_;

		// ESP32 specifics
		i2c_master_dev_handle_t dev_handle_;
		i2c_master_bus_handle_t bus_handle_;
		i2c_device_config_t dev_cfg_;
};

#endif /* I2C_MASTER_H__ */
