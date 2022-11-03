#include "helper.hpp"

uint16_t timesec_to_day(uint32_t seconds)
{
	return (seconds/(60*60*24));
}
uint8_t timesec_to_hour(uint32_t seconds)
{
	return 	((seconds%(60*60*24))/3600);
}
uint8_t timesec_to_min(uint32_t seconds)
{
	return ((seconds%3600)/60);
}
uint8_t timesec_to_sec(uint32_t seconds)
{
	return ((seconds%3600)%60);
}
uint32_t daytime_to_timesec(uint8_t hour, uint8_t min, uint8_t sec)
{
	return (hour*60*60 + min*60 + sec);
}
void _delay_ms(uint32_t milliseconds)
{
	sys_delay_ms(milliseconds);
}
void _delay_us(uint32_t microseconds)
{
	ets_delay_us(microseconds);
}
void json_test(void)
{
	DynamicJsonDocument doc(1024);

	doc["sensor"] = "gps";
	doc["time"]   = 1351824120;
	doc["data"][0] = 48.756080;
	doc["data"][1] = 2.302038;

	char buffer[100];
	serializeJson(doc, buffer);
	printf(buffer);
}
// typedef struct __attribute__ ((__packed__)) Pkt {
//     uint32_t timestamp;
//     uint8_t code;
//     uint8_t type; // read, write, execute (PUT, POST, GET, DELETE)
//     uint32_t value;
// }
// void pkt_create(uint8_t *pkt_to_send, uint8_t *pkt_struct, int length)
// {

// }