#ifndef WS_CLIENT_SETUP_HPP__
#define WS_CLIENT_SETUP_HPP__

#include <sys/param.h>
#include <string>

#include "esp_system.h"
#include "esp_event.h"
#include "esp_err.h"
#include "esp_log.h"

#include "esp_websocket_client.h"

#define CONFIG_WEBSOCKET_URI "ws://192.168.1.10"
#define CONFIG_WEBSOCKET_PORT 8999
#define TAG_WS_CLIENT "WS_CLIENT"

enum class ws_client_states {
	disconnected = 0,
	connected
};

#endif
