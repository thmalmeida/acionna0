/*
 * ws_setup.hpp
 *
 *  Created on: 17 de jul. de 2022
 *      Author: thmalmeida
 */
#ifndef WS_SETUP_HPP
#define WS_SETUP_HPP

#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_err.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#include "esp_http_server.h"    // Header for http_server websocket
// #include "esp_http_client.h"
// #include "esp_websocket_client.h"

#include "acionna.hpp"
#include "https_ota.hpp"

esp_err_t ws_event_handler(httpd_req_t* req);
void httpd_server_start(void); // suppose to be httpd_handle_t instead of void
void httpd_server_stop(httpd_handle_t server);
void connection_event_handler(void* handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wifi_sta_init(void);
void print_auth_mode(int authmode);
void print_cipher_type(int pairwise_cipher, int group_cipher);
void wifi_scan(void);
// static void wifi_conn_led_indicator_pwm(void);
void machine_run(void *pvParameter);

#endif

// httpd_handle_t start_webserver(void);
// void stop_webserver(httpd_handle_t server);
// void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
// void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);