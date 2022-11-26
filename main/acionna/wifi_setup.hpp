/*
 * wifi_stuffs.h
 *
 *  Created on: 2 de nov. de 2021
 *      Author: thmalmeida
 */
#ifndef WIFI_SETUP_HPP
#define WIFI_SETUP_HPP
;

//#include "driver/gpio.h"
//#include "led_strip.h"


//#include "addr_from_stdin.h"

// #include "../build/config/sdkconfig.h"

;#define PORT_UDP_ORIG 9000
#define IP_ADDR_DEST "192.168.1.9"
#define PORT_UDP_DEST 9001


#define PORT_TCP_ORIG				9002
#define KEEPALIVE_IDLE				1
#define KEEPALIVE_INTERVAL			1
#define KEEPALIVE_COUNT				2

#define WIFI_SSID_STA	"0yd017"			// CONFIG_ESP_WIFI_SSID
#define WIFI_SSID_AP    "0xd018"
#define IP_END          30                  // local ip end: 192.168.1.IP_END
#define WIFI_PASS		"praticamente"	    //CONFIG_ESP_WIFI_PASSWORD
#define WIFI_MAX_RETRY	600				    // CONFIG_ESP_MAXIMUM_RETRY

#define DEFAULT_SCAN_LIST_SIZE      10
/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

enum class wifi_states {
	disconnected = 0,
	connected
}

extern wifi_states wifi_state;
extern uint8_t esp_got_ip;

//static const char *payload = "Message from ESP32 ";
//int flag_payload_received = 0;

// void do_transmit_payload(void);
// void do_receive_payload(void);
// void tcp_server_task(void *pvParameters);
//static void udp_client_task(void *pvParameters);
//static void udp_server_task(void *pvParameters);

#endif