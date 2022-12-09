#ifndef HTTPD_SETUP_HPP__
#define HTTPD_SETUP_HPP__

#include <sys/param.h>
#include <string>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #inclu   de "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"

#include "esp_http_server.h"    // Header for http_server websocket

#include "basic_defines_conn.hpp"

#define HTTPD_WS_SERVER_PORT 9000
#define WS_DATA_LEN 128

struct async_resp_arg {
	httpd_handle_t hd;
	int fd;
};

// websocket server handle parameters
extern async_resp_arg ws_server_sock0;		// pointer to connection socket
extern uint8_t ws_server_data[WS_DATA_LEN];	// buffer received
extern uint8_t ws_server_data_len;			// buffer length received
extern uint8_t ws_server_data_flag;			// flag to advise new buffer
extern conn_states ws_server_client_state;	// ws server client connection state
extern httpd_handle_t ws_server;			// Server instance global declaration

esp_err_t ws_event_handler(httpd_req_t* req);
void httpd_server_start(void); // suppose to be httpd_handle_t instead of void
void httpd_server_stop(void);
void ws_server_send(std::string data);
// void esp_restart_async(void*);
// void esp_shutdown_h_now(void);


#endif
