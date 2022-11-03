#ifndef BT_STUFFS_HPP__
#define BT_STUFFS_HPP__
/*
* bt_stuffs.c
*
*  Created on: 30 de out. de 2021
*      Author: thmalmeida
*/

#include <iostream>
#include <sstream>
#include <string>

// #include <stdint.h>
// #include <string.h>
// #include <stdbool.h>
// #include <stdio.h>
//#include "nvs.h"
// #include "nvs_flash.h"
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

// #include "time.h"
// #include "sys/time.h"

#include "acionna.hpp"

// #include "../build/config/sdkconfig.h"

#define TAG_BT "BT_SPP"
#define SPP_SERVER_NAME "SPP_SERVER"
#define DEVICE_NAME "Cacimba_2cv"
//#define SPP_SHOW_DATA 0
//#define SPP_SHOW_SPEED 1
//#define SPP_SHOW_MODE SPP_SHOW_DATA    /*Choose show mode: show data or speed*/

#define SPP_DATA_LEN 100

// state_conn bt_state; // status connection
// uint32_t bt_sock0;                          // sock handle connection
// uint8_t bt_sock0_len;                       // pkt sock length

// void print_speed(void);
void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
void bt_init(void);
void send_echo_task(void *pvParameters);

#endif
