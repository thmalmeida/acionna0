/*
 * ws_setup.hpp
 *
 *  Created on: 17 de jul. de 2022
 *      Author: thmalmeida
 */
#ifndef WS_SETUP_HPP
#define WS_SETUP_HPP

#include "acionna/acionna.hpp"

// #include "pwm_ledc.hpp"

void machine_run(void *pvParameter);

#endif











// void json_test(void)
// {
// 	DynamicJsonDocument doc(1024);

// 	doc["sensor"] = "gps";
// 	doc["time"]   = 1351824120;
// 	doc["data"][0] = 48.756080;
// 	doc["data"][1] = 2.302038;

// 	char buffer[100];
// 	serializeJson(doc, buffer);
// 	printf(buffer);
// }
// typedef struct __attribute__ ((__packed__)) Pkt {
//     uint32_t timestamp;
//     uint8_t code;
//     uint8_t type; // read, write, execute (PUT, POST, GET, DELETE)
//     uint32_t value;
// }
// void pkt_create(uint8_t *pkt_to_send, uint8_t *pkt_struct, int length)
// {

// }