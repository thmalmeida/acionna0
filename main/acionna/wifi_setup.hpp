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



//static const char *payload = "Message from ESP32 ";
//int flag_payload_received = 0;

void do_transmit_payload(void);
void do_receive_payload(void);
void tcp_server_task(void *pvParameters);
//static void udp_client_task(void *pvParameters);
//static void udp_server_task(void *pvParameters);

#endif