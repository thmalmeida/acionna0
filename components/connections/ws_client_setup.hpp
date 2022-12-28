#ifndef WS_CLIENT_SETUP_HPP__
#define WS_CLIENT_SETUP_HPP__

#include <sys/param.h>
#include <string>

#include "esp_system.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"

#include "esp_websocket_client.h"

#include "basic_defines_conn.hpp"


#define TAG_WS_CLIENT "WS_CLIENT"


void ws_client_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void ws_client_start(void);
void ws_client_stop(void);
void ws_client_send(std::string data);
#endif
