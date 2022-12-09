/*
 * wifi_stuffs.h
 *
 *  Created on: 2 de nov. de 2021
 *      Author: thmalmeida
 */
#ifndef WIFI_SETUP_HPP
#define WIFI_SETUP_HPP

#include "esp_system.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"

#include "esp_wifi.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"

#include "httpd_setup.hpp"

#include "basic_defines_conn.hpp"

#include "pwm_ledc.hpp"						// flash led to indicate wifi connection state

#define WIFI_SSID_STA	"0yd017"			// CONFIG_ESP_WIFI_SSID
#define WIFI_PASS		"praticamente"	    //CONFIG_ESP_WIFI_PASSWORD
#define IP_END          30                  // local ip end: 192.168.1.IP_END

// #define WIFI_MAX_RETRY	600				    // CONFIG_ESP_MAXIMUM_RETRY

// #define WIFI_SSID_AP    "0yd018"

#define DEFAULT_SCAN_LIST_SIZE      7
/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

enum class ip_get_types {
	static_ip = 0,
	dhcp
};

enum class ip_states {
	ip_not_defined = 0,
	ip_defined
};

extern ip_states ip_state;
extern conn_states wifi_state;

void wifi_sta_init(void);
void wifi_sta_stop(void);
void wifi_get_info(void);
void print_auth_mode(int authmode);
void print_cipher_type(int pairwise_cipher, int group_cipher);
void wifi_scan();
void wifi_scan2(uint16_t &number, wifi_ap_record_t* ap_info, uint16_t &ap_count);
void wifi_connection_event_handler(void* handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

#endif