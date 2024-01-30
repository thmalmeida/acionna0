#include "dht.hpp"

// TODO: check checksum, interruptions

// Code ready to use with AM2302 (DHT22). To use with DHT11 change return methods into get_temp and get humidity
DHT::DHT(GPIO_DRIVER* gpio){
	this->_gpio = gpio;
	_gpio->mode(GPIO_MODE_INPUT);
	_gpio->pull(GPIO_PULLUP_ONLY);
}
void DHT::begin() {
	_gpio->mode(GPIO_MODE_INPUT_OUTPUT);
	_gpio->write(1);
	delay_ms(50);	// To fit in DHT11 and AM2302.
	_gpio->write(0);
	delay_ms(20);	// To fit in DHT11 and AM2302.

	_gpio->write(1);
	delay_ms(1);	// To fit in DHT11 and AM2302.

	// Bus master has released time. Tgo: 30 us. (return to up and wait for DHT pulse response)
	_gpio->mode(GPIO_MODE_INPUT);
	_gpio->pull(GPIO_PULLUP_ONLY);
}
bool DHT::requestRead(){
	return read();
}
int16_t DHT::getTempCelsius(int sensor_DHT11){

	uint8_t signal = 0x80 & _data[2];

	if(sensor_DHT11)
	{
		// For DHT11 the answer is 8 bit for integral part. The decimal part is always zero.
		return  ((_data[2] & 0x7F) + _data[3] * 0.01) * (signal ? -1 : 1);
	}
	else
		// For DHT22 AM2302 the answer is a 16 bit wide.
		return ((((_data[2] & 0x7F) << 8) | (_data[3] << 0)) * (signal ? -1 : 1));
}
uint16_t DHT::getHumidity(int sensor_DHT11){
	
	if(sensor_DHT11)
		// For DHT11
		return (_data[0] + (_data[1]*0.01));
	else
		// For DHT22 AM2302
		return ((_data[0] << 8) | (_data[1] << 0));
}
bool DHT::checkSum(){
	return ((_data[0] + _data[1] + _data[2] + _data[3]) == _data[4]);
}
bool DHT::read2() {
	// Clear data buffer
	_data[0] = _data[1] = _data[2] = _data[3] = _data[4] = 0;

	// Suppose to pin in high level because it is an input pull up mode set on initialization;
	// DHT11 and AM2302 has similar answer curve times. The Tbe time has some differences.
	// DHT11 requires at least 18 ms. AM2302 requires from 0.8 to 20 ms (max). Typical 1 ms.

	// Host the start signal down time. Tbe: 1-20 ms.
	_gpio->mode(GPIO_MODE_INPUT_OUTPUT);
	_gpio->write(0);
	delay_ms(20);	// To fit in DHT11 and AM2302.

	// Bus master has released time. Tgo: 30 us. (return to up and wait for DHT pulse response)
	_gpio->write(1);
	_gpio->mode(GPIO_MODE_INPUT);
	_gpio->pull(GPIO_PULLUP_ONLY);
	
	// Response to low time. Trel: 80 us
	if(count_pulse_us(0, 85, 1) < 1)
	{
		ESP_LOGI(TAG_DHT, "error wait low pulse\n");
		return false;
	}

	// In response to high time. Treh: 80 us
	if(count_pulse_us(1, 85, 0) < 1)
	{
		ESP_LOGI(TAG_DHT, "error wait high pulse\n");
		return false;
	}

	struct Cycles{
		int pulse_time_low;
		int pulse_time_high;
	};

	struct Cycles cycles[40];
	
	// Starting receiving 40 bits data;
	for(int i = 0; i < 40; i++)
	{
		// Signal "0", "1" low time
		cycles[i].pulse_time_low = count_pulse_us(0, 55, 0);
		
		// Signal "0" high time. Th0 = 26 us  OR  Signal "1" high time. Th1 = 70 us.
		cycles[i].pulse_time_high = count_pulse_us(1, 75, 0);
	}

	// Sensor to release the bus time. Ten: 45-55 us
	if(count_pulse_us(0, 60, 0) < 1)
	{
		ESP_LOGI(TAG_DHT, "couldn't find end signal");
		// return false;
	}
	
	for(int i = 0; i < 40; i++)
	{
		// check error cycle read
		if(cycles[i].pulse_time_low == 0 || cycles[i].pulse_time_high == 0)
		{
			ESP_LOGI(TAG_DHT, "error on pulse i[%d]", i);
			return false;
		}

		_data[i/8] = (_data[i/8] << 1);

		if(cycles[i].pulse_time_high > cycles[i].pulse_time_low)
		{
			_data[i/8] = _data[i/8] | 1;
		}
	}

	// for(uint8_t i = 0; i < 40; i++){
	// 	if(cycles[i].pulse_time_low == 0 || cycles[i].pulse_time_high == 0)
	// 	{
	// 		ESP_LOGI(TAG_DHT, "error on pulse i[%d]", i);
	// 		return false;
	// 	}

	// 	_data[i/8] <<= 1;

	// 	if(cycles[i].pulse_time_high > cycles[i].pulse_time_low)
	// 		_data[i/8] |= 1;
	// }
	// return checkSum();
	return true;
}
bool DHT::read(){
	_data[0] = _data[1] = _data[2] = _data[3] = _data[4] = 0;

	// gpio->setOutput();
	_gpio->mode(GPIO_MODE_OUTPUT);

	_gpio->write(1);
	delay_ms(250);

	_gpio->write(0);
	delay_ms(20);

	struct Cycles{
		int pulse_time_low;
		int pulse_time_high;
	};

	struct Cycles cycles[40];

	{
		// InterruptLock lock;

		_gpio->write(1);
		_gpio->pull(GPIO_PULLUP_ONLY);
		_gpio->mode(GPIO_MODE_INPUT);
		
		delay_us(40);


		if(waitPulse(0) == 0)
			return false;

		if(waitPulse(1) == 0)
			return false;

		//Starting sending data
		for(uint8_t i = 0; i < 40; i++){
			cycles[i].pulse_time_low = waitPulse(0);
			cycles[i].pulse_time_high = waitPulse(1);
		}
	}

	for(uint8_t i = 0; i < 40; i++){
		if(cycles[i].pulse_time_low == 0 || cycles[i].pulse_time_high == 0)
			return false;

		_data[i/8] <<= 1;

		if(cycles[i].pulse_time_high > cycles[i].pulse_time_low)
			_data[i/8] |= 1;
	}


	return checkSum();
}
int DHT::count_pulse_us(int pulse, int timeout, int transition_edge)
{
	int i = 0;
	// int timeout = 250;
	// wait_pulse

	if(transition_edge)
	{
		while(_gpio->read() == !pulse)
		{
			if(i>timeout)
			{
				return -1;
			}
			delay_us(1);
			i++;
		}
	}

	// count pulse time
	i = 0;
	while(_gpio->read() == pulse)
	{
		if(i > timeout)
			return 0;
		delay_us(1);
		i++;
	}
	return i;
}
uint8_t DHT::waitPulse(uint8_t pulse, uint8_t us /* = 80 */)
{
	int i = 0;
	while(_gpio->read() == pulse)
	{
		if(i > us)
			return 0;
		delay_us(1);
		i++;
	}
	return i;
}
void DHT::set_high()
{
	_gpio->mode(GPIO_MODE_OUTPUT);
	_gpio->write(1);
}
void DHT::set_low() {
	_gpio->mode(GPIO_MODE_OUTPUT);
	_gpio->write(0);
}

// class InterruptLock {
// 	public:
// 		InterruptLock(){
// 			disableInterrupts();
// 		}
// 		~InterruptLock() {
// 			enableInterrupts();
// 		}
// };
