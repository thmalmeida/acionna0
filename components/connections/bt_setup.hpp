#ifndef BT_STUFFS_HPP__
#define BT_STUFFS_HPP__
/*
* bt_stuffs.c
*
*  Created on: 30 de out. de 2021
*      Author: thmalmeida
*/
#include <iostream>
#include <cstring>      // for memset
#include <sstream>
#include <string>

#include "esp_log.h"

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"

#include "basic_defines_conn.hpp"

#define TAG_BT "BT_SPP"
#define SPP_SERVER_NAME "SPP_SERVER"
#define DEVICE_NAME CONFIG_BT_NAME

#define SPP_BT_DATA_LEN 128

// Bluetooth handle parameters
extern uint32_t bt_sock0;					// pointer to connection socket
extern uint8_t bt_data[SPP_BT_DATA_LEN];	// buffer received
extern uint8_t bt_data_len;					// buffer length received
extern uint8_t bt_data_flag;				// flag to advise new buffer
extern conn_states bt_state;					// status connection

void bt_init(void);
void bt_event_handler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
void bt_send(std::string msg);
// void send_echo_task(void *pvParameters);

#endif
