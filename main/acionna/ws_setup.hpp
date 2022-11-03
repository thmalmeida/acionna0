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
#include "esp_http_client.h"
#include "esp_websocket_client.h"

#include "acionna.hpp"

#define WIFI_SSID_STA	"0yd020"			// CONFIG_ESP_WIFI_SSID
#define WIFI_SSID_AP    "0xd018"
#define IP_END          33                  // local ip end: 192.168.1.IP_END
#define WIFI_PASS		"praticamente"	    //CONFIG_ESP_WIFI_PASSWORD
#define WIFI_MAX_RETRY	600				    // CONFIG_ESP_MAXIMUM_RETRY

#define DEFAULT_SCAN_LIST_SIZE      10
/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


// void led_indicator();

esp_err_t ws_event_handler(httpd_req_t* req);
void httpd_server_start(void); // suppose to be httpd_handle_t instead of void
void httpd_server_stop(httpd_handle_t server);
void connection_event_handler(void* handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wifi_sta_init(void);
void print_auth_mode(int authmode);
void print_cipher_type(int pairwise_cipher, int group_cipher);
void wifi_scan(void);
// static void wifi_conn_led_indicator_pwm(void);
void machine_run(void*);

#endif

// httpd_handle_t start_webserver(void);
// void stop_webserver(httpd_handle_t server);
// void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
// void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);