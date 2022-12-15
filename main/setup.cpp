#include "setup.hpp"
#include <stdio.h>
// #include "protocol_examples_common.h"

const char* TAG_SETUP = "SETUP";

void machine_run(void *pvParameter)
{
	// wifi and ws server init - ws server will ask to init into connection_event_handler when got IP.
	wifi_sta_init();

	// Bluetooth init
	#ifdef CONFIG_BT_ENABLE
	bt_init();
	#endif
	// Acionna init
	Acionna acionna0;

	// ws client initialize
	// ws_client_start();

	while(1)
	{
		acionna0.run();
			// if(wifi_state == state_conn::connected)
			// {
			// 	if(httpd_ws_get_fd_info(sock0.hd, sock0.fd) == HTTPD_WS_CLIENT_WEBSOCKET)
			// 	{
			// 		httpd_ws_frame_t ws_pkt;
			// 		memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
			// 		ws_pkt.payload = reinterpret_cast<uint8_t*>(&buffer[0]);
			// 		// ws_pkt.payload = (uint8_t*)buffer;
			// 		ws_pkt.len = strlen(buffer);
			// 		ws_pkt.type = HTTPD_WS_TYPE_TEXT;

			// 		httpd_ws_send_frame_async(sock0.hd, sock0.fd, &ws_pkt);
			// 	}
			// };

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}















// Sensors type
/*
water pressure
air pressure
humidity
water level
voltage
current
temperature
*/

// typedef struct ws_Pkt {
// 	uint8_t id;
// 	uint8_t code;
// 	uint8_t code_type:2;      // get, post, put or delete
// 	uint8_t sensor_type:4;
// 	uint32_t value;
// };

// void pkt_create(void)
// {

// }


// // Start the httpd server
// ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
// if (httpd_start(&server, &config) == ESP_OK) {
//     // Registering the ws handler
//     ESP_LOGI(TAG, "Registering URI handlers");
//     httpd_register_uri_handler(server, &ws);
//     return server;
// }
// ESP_LOGI(TAG, "Error starting server!");
// return NULL;
// }
// void stop_webserver(httpd_handle_t server)
// {
//     // Stop the httpd server
//     httpd_stop(server);
// }
// void disconnect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
// {
//     httpd_handle_t* server = (httpd_handle_t*) arg;
//     if (*server) {
//         ESP_LOGI(TAG, "Stopping webserver");
//         stop_webserver(*server);
//         *server = NULL;
//     }
// }
// void connect_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
// {
//     httpd_handle_t* server = (httpd_handle_t*) arg;
//     if (*server == NULL) {
//         ESP_LOGI(TAG, "Starting webserver");
//         *server = start_webserver();
//     }
// }