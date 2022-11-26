/* WebSocket Echo Server Example

This example code is in the Public Domain (or CC0 licensed, at your option.)

Unless required by applicable law or agreed to in writing, this
software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.
*/

/*
TODO list
- funções de habilitação do sistema de verificação;
- flags de verificação 
*/

#include "ws_setup.hpp"
// #include "bt_stuffs.hpp"
#include "pwm_ledc.hpp"

#include <gpio.hpp>
#include <stdio.h>

/* A simple example that demonstrates using websocket echo server
*/
static const char *TAG_WS = "ws_echo_server";
static const char *TAG_WIFI = "wifi station";
static const char *TAG_IP = "IP stuffs";

static int s_retry_num = 0;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
/*
* Structure holding server handle
* and internal socket fd in order
* to use out of request send
*/

// -- Websocket client part ----------------------
#include "esp_websocket_client.h"

#define CONFIG_WEBSOCKET_URI "ws://192.168.1.10"
#define CONFIG_WEBSOCKET_PORT 8999
#define TAG_WS_CLIENT "WS_CLIENT"

enum class ws_client_states {
	disconnected = 0,
	connected
};

ws_client_states ws_client_state = ws_client_states::disconnected;
static esp_websocket_client_config_t websocket_cfg = {};
static esp_websocket_client_handle_t client;
static int i_ws;
static void ws_client_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;

	switch (event_id)
	{
		case WEBSOCKET_EVENT_CONNECTED:
			ESP_LOGI(TAG_WS_CLIENT, "WEBSOCKET_EVENT_CONNECTED");
			ws_client_state = ws_client_states::connected;
			break;
		
		case WEBSOCKET_EVENT_DISCONNECTED:
			ESP_LOGI(TAG_WS_CLIENT, "WEBSOCKET_EVENT_DISCONNECTED");
			ws_client_state = ws_client_states::disconnected;
			break;
		
		case WEBSOCKET_EVENT_DATA:
			ESP_LOGI(TAG_WS_CLIENT, "WEBSOCKET_EVENT_DATA");
			ESP_LOGI(TAG_WS_CLIENT, "Received opcode=%d", data->op_code);
			if (data->op_code == 0x08 && data->data_len == 2) {
				ESP_LOGW(TAG_WS_CLIENT, "Received closed message with code=%d", 256*data->data_ptr[0] + data->data_ptr[1]);
			} else {
				ESP_LOGW(TAG_WS_CLIENT, "Received=%.*s", data->data_len, (char *)data->data_ptr);
			}
			ESP_LOGW(TAG_WS_CLIENT, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

			// xTimerReset(shutdown_signal_timer, portMAX_DELAY);
			break;

		case WEBSOCKET_EVENT_ERROR:
			ESP_LOGI(TAG_WS_CLIENT, "WEBSOCKET_EVENT_ERROR");
			break;
	}
}
static void ws_client_start(void)
{
	websocket_cfg.uri = CONFIG_WEBSOCKET_URI;
	websocket_cfg.port = CONFIG_WEBSOCKET_PORT;

	client = esp_websocket_client_init(&websocket_cfg);
	esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, ws_client_event_handler, (void *)client);

	esp_websocket_client_start(client);
}
static void ws_client_stop(void)
{
	if (esp_websocket_client_is_connected(client)) {
		esp_websocket_client_close(client, portMAX_DELAY);
		ESP_LOGI(TAG_WS_CLIENT, "Websocket Stopped");
		esp_websocket_client_destroy(client);
	}
}
static void ws_client_run(char* data, int len)
{
	// char data[32];
	if (esp_websocket_client_is_connected(client)) {
		// int len = sprintf(data, "hello %04d", i_ws++);
		// ESP_LOGI(TAG_WS_CLIENT, "Sending %s", data);
		esp_websocket_client_send_text(client, data, len, portMAX_DELAY);
	}
}
// ------------------------

struct async_resp_arg
{
	httpd_handle_t hd;
	int fd;
};
const httpd_uri_t ws = {
	"/ws",                 // uri
	HTTP_GET,               // method
	ws_event_handler,       // handler: esp_err_t (*handler)(httpd_req_t *r)
	NULL,                   // user_ctx
	true,                   // is_websocket
	true,                   // handle_ws_control_frames
	NULL                    // supported_subprotocol
};
httpd_handle_t server = NULL;   // Server instance global declaration
httpd_config_t server_config = {
	tskIDLE_PRIORITY+5,     // task_priority
	4096,                   // stack_size
	tskNO_AFFINITY,         // core_id
	9000,                   // server_port
	32768,                  // ctrl_port
	7,                      // max_open_sockets
	8,                      // max_uri_handlers
	8,                      // max_resp_headers
	5,                      //backlog_conn
	true,                  // lru_purge_enable
	5,                      // recv_wait_timeout
	5,                      // send_wait_timeout
	NULL,                   // global_user_ctx
	NULL,                   // global_user_ctx_free_fn
	NULL,                   // global_transport_ctx
	NULL,                   // global_transport_ctx_free_fn
	false,					// enable/disable linger
	10,						// linger timeout in seconds
	NULL,                   // open_fn
	NULL,                   // close_fn
	NULL                    // uri_match_fn
};

enum class ip_get_type {
	static_ip = 0,
	dhcp_ip
};

static async_resp_arg sock0;
static ip_get_type ip_get_mode = ip_get_type::static_ip;
static state_conn wifi_state = state_conn::disconnected;
static state_conn wifi_state_old = state_conn::disconnected;
extern state_conn bt_state;

ACIONNA acionna0;
GPIO_Basic led0(LED_0);
pwm_ledc led_wifi_indicator(2, 1, 0, 1);

// Bluetooth handle parameters 
extern uint32_t bt_sock0;
extern uint8_t bt_sock0_len;
extern state_conn bt_state; // status connection

// variables to main run
static states_flag signal_send = states_flag::disable;

// --------- DHT and DS18B20 sensors ---------
#include "dht.hpp"
#include "gpio.hpp"
#include "dallas_temperature.h"
#define TAG_SENSORS "Sensors"
GPIO_Basic gpio_humidity_sensor(HUMIDITY_SENSOR, true);
DHT dht0{&gpio_humidity_sensor};

GPIO_Basic gpio_sensor(DS18B20_DATA);
OneWire onewire(&gpio_sensor);
Dallas_Temperature temp_sensor(&onewire);
std::uint8_t temp_sensor_count;
// -------------------------------------------

char buffer[100] = "not ocurred\n";

uint8_t flag_ip_got = 0;

static void esp_restart_async(void*)
{
	vTaskDelay(2000 / portTICK_PERIOD_MS);
	esp_restart();
}
static void esp_shutdown_h_now(void)
{
	if(server)
		httpd_server_stop(server);
	
	if(wifi_state == state_conn::connected)
	{
		ESP_ERROR_CHECK(esp_wifi_disconnect());
		ESP_ERROR_CHECK(esp_wifi_stop());
	}

	ESP_LOGI(TAG_WS, "Shutdown peripherals successfully. Restarting...");
	xTaskCreate(&esp_restart_async, "esp_restart_task", 1024, NULL, 5, NULL);
}
esp_err_t ws_event_handler(httpd_req_t *req)
{
	if (req->method == HTTP_GET) {
		ESP_LOGI(TAG_WS, "Handshake done, the new connection was opened");
		ESP_LOGI(TAG_WS, "req->method %u", req->method);

		sock0.fd = httpd_req_to_sockfd(req);
		sock0.hd = req->handle;

		return ESP_OK;
	}

	ESP_LOGI(TAG_WS, "req->method %u", req->method);

	// httpd_close_func_t
	// if(req->free_ctx)

	// printf("req->method= %d", req->method);

	httpd_ws_frame_t ws_pkt;
	uint8_t *buf = NULL;
	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	// ws_pkt.type = HTTPD_WS_TYPE_TEXT;
	/* Set max_len = 0 to get the frame len */
	esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG_WS, "httpd_ws_recv_frame failed to get frame len with %d", ret);
		return ret;
	}

	ESP_LOGI(TAG_WS, "frame len is %d", ws_pkt.len);
	if (ws_pkt.len) {
		/* Alocate and clear buffer. ws_pkt.len + 1 is for NULL termination as we are expecting a string */
		buf = (uint8_t*)calloc(1, ws_pkt.len + 1);  // alocate memory for an array and initialize with zeros.
		if (buf == NULL)                            // Test if it is okay.
		{
			ESP_LOGE(TAG_WS, "Failed to calloc memory for buf");
			return ESP_ERR_NO_MEM;
		}
		ws_pkt.payload = buf;                       // Clear payload buffer

		/* Set max_len = ws_pkt.len to get the frame payload */
		// ESP_LOGI(TAG_WS, "Got packet with message: %s", ws_pkt.payload);
		ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG_WS, "httpd_ws_recv_frame failed with %d", ret);
			free(buf);
			return ret;
		}
		ESP_LOGI(TAG_WS, "Got packet with message: %s", ws_pkt.payload);
	}
	ESP_LOGI(TAG_WS, "Packet type: %d", ws_pkt.type);

//	// Just for trigger for a specific message.
	// if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp((char*)ws_pkt.payload,"Trigger async") == 0) {
	// 	free(buf);
	// 	ESP_LOGI(TAG_WS, "Trigger async if");
	// 	return trigger_async_send(req->handle, req);
	// }

	// for a copy
	// std::vector<uint8_t> myVector(myString.begin(), myString.end());
	// uint8_t *p = &myVector[0];
	// std::stringstream ss;
	// ss << "Hello, world, " << acionna0.pipe1_.pressure_mca << '\n';
	// std::string myString = ss.str();

	// ws_pkt.payload = reinterpret_cast<uint8_t*>(&myString[0]);
	// ws_pkt.len = myString.size();

	uint8_t* command_str = new uint8_t[16];
	int command_str_len = 0;
	acionna0.parser_1(ws_pkt.payload, ws_pkt.len, command_str, command_str_len);

	// ESP_LOGI(TAG_WS, "%s", command_str);
	// ws_pkt.payload = command_str;
	// ws_pkt.len = command_str_len;

	// char* msg_back = NULL; //new char[40];
	// int msg_back_len = 0;
	std::string msg_back;
	msg_back = acionna0.handle_message(command_str);

	// ESP_LOGI(TAG_WS, "msg_back[%d]: %s", msg_back.length(), msg_back.c_str());

	ws_pkt.payload = reinterpret_cast<uint8_t*>(&msg_back[0]);
	ws_pkt.len = msg_back.length();
	ESP_LOGI(TAG_WS, "msg_back length: %d", msg_back.length());

	if(acionna0.signal_restart)
	{
		ESP_LOGI(TAG_WS, "ws_type_close");
		ret = httpd_ws_send_frame(req, &ws_pkt);

		ws_pkt.type = HTTPD_WS_TYPE_CLOSE;
	}

	ret = httpd_ws_send_frame(req, &ws_pkt);
	if (ret != ESP_OK) {
		ESP_LOGE(TAG_WS, "httpd_ws_send_frame failed with %d", ret);
	}

	delete[] command_str;
	free(buf);
	return ret;
}
void httpd_server_start(void)
{
	// Needs implementation for wss connection
	// Prepare keep-alive engine
	// wss_keep_alive_config_t keep_alive_config = KEEP_ALIVE_CONFIG_DEFAULT();

	// httpd_ssl_config

	// httpd_server_start(arg);
	ESP_LOGI(TAG_WS, "HTTPD server start");

	// httpd_handle_t* server = (httpd_handle_t*) arg;
	// httpd_handle_t server = NULL; // moved to global
	ESP_LOGI(TAG_WS, "server cast to arg");
	if (server == NULL)
	{
		// *server = start_webserver();
		// Start the httpd server
		ESP_LOGI(TAG_WS, "Starting webserver on port: '%d'", server_config.server_port);
		if (httpd_start(&server, &server_config) == ESP_OK) // httpd_start(&server, &config)
		{
			// Registering the ws handler
			ESP_LOGI(TAG_WS, "Registering URI handlers");
			httpd_register_uri_handler(server, &ws);
			// return server;
		}
		else
		{
			ESP_LOGI(TAG_WS, "Error starting server!");
			server = NULL;
		}
	}
	ESP_LOGI(TAG_WS, "leaving httpd_server_start");
}
void httpd_server_stop(httpd_handle_t server)
{
	httpd_stop(server);
}
void connection_event_handler(void* handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT)
	{
		if (event_id == WIFI_EVENT_WIFI_READY)
		{
			ESP_LOGI(TAG_WIFI,"WiFi Ready!");
		}
		else if(event_id == WIFI_EVENT_SCAN_DONE)
		{
			ESP_LOGI(TAG_WIFI, "ESP32 finish scanning AP");
		}
		else if(event_id == WIFI_EVENT_STA_START)
		{
			esp_wifi_connect();
			ESP_LOGI(TAG_WIFI, "ESP32 sta start event");
		}
		else if(event_id == WIFI_EVENT_STA_CONNECTED)
		{
			wifi_state = state_conn::connected;
			ESP_LOGI(TAG_WIFI, "connected to ap SSID:%s password:%s", WIFI_SSID_STA, WIFI_PASS);
			ESP_LOGI(TAG_WIFI, "ESP32 station connected to AP");
		}
		else if(event_id == WIFI_EVENT_STA_DISCONNECTED)
		{
			// if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
			// {
			wifi_state = state_conn::disconnected;
			ESP_LOGI(TAG_WIFI, "ESP32 station disconnected to AP");

		// httpd_handle_t* server = (httpd_handle_t*) handler_arg;
		if (server) {
			ESP_LOGI(TAG_WS, "Stopping webserver");
			httpd_server_stop(server);
			server = NULL;
		}

		esp_wifi_connect();
		s_retry_num++;
		ESP_LOGI(TAG_WIFI, "connect retry: %d", s_retry_num);
		// }else
		// {
		// 	s_retry_num = 0;
		// 	xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		// }
		// ESP_LOGI(TAG_WIFI,"connect to the AP fail");
		}
		else if (event_id == WIFI_EVENT_STA_STOP)
		{
			ESP_LOGI(TAG_WIFI, "ESP32 station stop");
		}
	}
	else if (event_base == IP_EVENT)
	{
		if (event_id == IP_EVENT_STA_GOT_IP)
		{
			ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
			ESP_LOGI(TAG_IP, "got ip0:" IPSTR, IP2STR(&event->ip_info.ip));
			s_retry_num = 0;
			xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

			esp_got_ip = 1;
		}
		else if(event_id == IP_EVENT_STA_LOST_IP)
		{
			ESP_LOGI(TAG_IP,"lost ip!!!!");
			httpd_server_stop(handler_arg);
		}
	}
}
void wifi_sta_init(void)
{
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// Select ip station mode DHCP or STATIC
	switch (ip_get_mode)
	{
		case ip_get_type::dhcp_ip:
		{
			esp_netif_create_default_wifi_sta();
			break;
		}
		case ip_get_type::static_ip:
		{
			esp_netif_t *my_sta = esp_netif_create_default_wifi_sta();
			// assert(my_sta);              // is that for debug?

			esp_netif_dhcpc_stop(my_sta);
			esp_netif_ip_info_t ip_info;
			IP4_ADDR(&ip_info.ip, 192, 168, 1, IP_END);
			IP4_ADDR(&ip_info.gw, 192, 168, 1, 1);
			IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
			esp_netif_set_ip_info(my_sta, &ip_info);
			break;
		}
		default:
			esp_netif_create_default_wifi_sta();
			break;
	}

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &connection_event_handler, NULL, &instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &connection_event_handler, NULL, &instance_got_ip));

// ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

//	wifi_config_t wifi_config = {
//	.sta = {
//	.ssid = EXAMPLE_ESP_WIFI_SSID,
//	.password = EXAMPLE_ESP_WIFI_PASS,
//	/* Setting a password implies station will connect to all security modes including WEP/WPA.
//	* However these modes are deprecated and not advisable to be used. Incase your Access point
//	* doesn't support WPA2, these mode can be enabled by commenting below line */
//	.threshold.authmode = WIFI_AUTH_WPA2_PSK,
//
//	.pmf_cfg = {
//	.capable = true,
//	.required = false
//	},
//	},
//	};

	wifi_config_t wifi_config;
	memset(&wifi_config, 0, sizeof(wifi_config));	// Clear all (RESET)
	// 	sprintf (reinterpret_cast<char*>(wifi_config.sta.ssid), EXAMPLE_ESP_WIFI_SSID );
	//  sprintf (reinterpret_cast<char*>(wifi_config.sta.password), EXAMPLE_ESP_WIFI_PASS);
	memcpy(wifi_config.sta.ssid, WIFI_SSID_STA, strlen(WIFI_SSID_STA));
	memcpy(wifi_config.sta.password, WIFI_PASS, strlen(WIFI_PASS));
	//	strcpy(wifi_config.ap.ssid, (uint8_t const*)(EXAMPLE_ESP_WIFI_SSID));
	//	strcpy(wifi_config.ap.password, (uint8_t const*)(EXAMPLE_ESP_WIFI_PASS));
	wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK; 	// or ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD

// wifi_config.sta.pmf_cfg.capable = true;
// wifi_config.sta.pmf_cfg.required = false;

// wifi ap set
// memcpy(wifi_config.ap.ssid, WIFI_SSID_AP, strlen(WIFI_SSID_AP));
// wifi_config.ap.ssid_len = (uint8_t)strlen(WIFI_SSID_AP);
// wifi_config.ap.channel = 6;
// memcpy(wifi_config.ap.password, WIFI_PASS, strlen(WIFI_PASS));
// wifi_config.ap.max_connection = 4;
// wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;


// Routine test to verify strings
//	int a=strlen(EXAMPLE_ESP_WIFI_SSID);
//
//	printf("SSID: ");
//	for(int i=0; i<a; i++)
//	{
//		printf("%c", wifi_config.sta.ssid[i]);
//	}
//	printf("FIM");
//	printf("\n");
//
//	a=strlen(EXAMPLE_ESP_WIFI_PASS);
//	printf("PASSWD: ");
//	for(int i=0; i<a; i++)
//	{
//		printf("%c", wifi_config.sta.password[i]);
//	}
//	printf("FIM");
//	printf("\n");

// for ap mode
// ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP) );
// ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config) );

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

	ESP_ERROR_CHECK(esp_wifi_start() );

	ESP_LOGI(TAG_WIFI, "wifi_sta_init finished.");

/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
* number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
// EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,	WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,	pdFALSE, pdFALSE, portMAX_DELAY);

// /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
// * happened. */
// if (bits & WIFI_CONNECTED_BIT)
// {
// 	ESP_LOGI(TAG_WIFI, "connected to ap SSID:%s password:%s", WIFI_SSID_STA, WIFI_PASS);
// }
// else if (bits & WIFI_FAIL_BIT)
// {
// 	ESP_LOGI(TAG_WIFI, "Failed to connect to SSID:%s, password:%s", WIFI_SSID_STA, WIFI_PASS);
// }
// else
// {
// 	ESP_LOGE(TAG_WIFI, "UNEXPECTED EVENT");
// }

	ESP_LOGI(TAG_WIFI, "wifi_sta_init end.");
/* wifi_init_sta() finish here */

// ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
// ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

// To delete connection?
/* The event will not be processed after unregister */
// ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
// ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
// vEventGroupDelete(s_wifi_event_group);

// xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);
//	xTaskCreate(udp_server_task, "udp_server", 4096, (void*)AF_INET, 5, NULL);
//    xTaskCreate(udp_client_task, "udp_client", 4096, NULL, 5, NULL);
}
void send_json_data_back() {

	// const char json_overhead_begin[2] = "{";
	// const char json_overhead_comma[3] = ", ";
	// const char json_overhead_end[2] = "}";
	// const unsigned int json_n_itens = 5;
	// std::string json_itens[json_n_itens];

	// strcpy(json_data, json_overhead_begin);
	// for(int i=0; i<json_n_itens; i++)
	// {
		
	// }
	// strcat(json_data, json_overhead_end);

	if(wifi_state == state_conn::connected)
	{
		if(httpd_ws_get_fd_info(sock0.hd, sock0.fd) == HTTPD_WS_CLIENT_WEBSOCKET)
		{
			DynamicJsonDocument doc(1024);
			doc["id"] = IP_END;
			doc["p1"] = acionna0.pipe1_.pressure_mca();
			doc["p2"] = acionna0.pipe1_.pressure_mca();
			doc["ton"] = acionna0.pump1_.time_on();
			doc["toff"] = acionna0.pump1_.time_off();
			doc["k1"] = static_cast<int>(acionna0.pump1_.state_k1());
			doc["k2"] = static_cast<int>(acionna0.pump1_.state_k2());
			doc["k3"] = static_cast<int>(acionna0.pump1_.state_k3());
			doc["rth"] = static_cast<int>(acionna0.pump1_.state_Rth());

			serializeJson(doc, buffer);
			// preparing json data
			// sprintf(buffer, "{\"id\":%d, \"p\":%d, \"ton\":%u, \"toff\":%u, \"k1\":%d, \"k2\":%d, \"k3\":%d, \"rth\":%d}", 
			// 								IP_END,
			// 								acionna0.pipe1_.pressure_mca(),
			// 								acionna0.pump1_.time_on(),
			// 								acionna0.pump1_.time_off(),
			// 								static_cast<int>(acionna0.pump1_.state_k1()),
			// 								static_cast<int>(acionna0.pump1_.state_k2()),
			// 								static_cast<int>(acionna0.pump1_.state_k3()),
			// 								static_cast<int>(acionna0.pump1_.state_Rth()));

			// build websocket frame
			httpd_ws_frame_t ws_pkt;
			memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
			// ws_pkt.payload = (uint8_t*)buffer;
			ws_pkt.payload = reinterpret_cast<uint8_t*>(&buffer[0]);
			ws_pkt.len = strlen(buffer);
			ws_pkt.type = HTTPD_WS_TYPE_TEXT;

			httpd_ws_send_frame_async(sock0.hd, sock0.fd, &ws_pkt);


			// if(ws_client_state == ws_client_states::connected)
			// {
			// 	ws_client_run(&buffer[0], strlen(buffer));
			// }
		}
		else
			acionna0.signal_json_data_back = 0;
	}
}
void send_json_data_server() {

}
void wifi_get_info(void)
{
	wifi_ap_record_t ap;
	esp_wifi_sta_get_ap_info(&ap);
	printf("%d\n", ap.rssi);
}
void print_auth_mode(int authmode)
{
	switch (authmode) {
		case WIFI_AUTH_OPEN:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_OPEN");
			break;
		
		case WIFI_AUTH_WEP:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WEP");
			break;
		
		case WIFI_AUTH_WPA_PSK:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA_PSK");
			break;
		
		case WIFI_AUTH_WPA2_PSK:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA2_PSK");
			break;
		
		case WIFI_AUTH_WPA_WPA2_PSK:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
			break;
		
		case WIFI_AUTH_WPA2_ENTERPRISE:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA2_ENTERPRISE");
			break;

		case WIFI_AUTH_WPA3_PSK:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA3_PSK");
			break;

		case WIFI_AUTH_WPA2_WPA3_PSK:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
			break;
		
		default:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_UNKNOWN");
			break;
	}
}
void print_cipher_type(int pairwise_cipher, int group_cipher)
{
	switch (pairwise_cipher) {
		case WIFI_CIPHER_TYPE_NONE:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
		break;

		case WIFI_CIPHER_TYPE_WEP40:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
		break;
		
		case WIFI_CIPHER_TYPE_WEP104:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
		break;
		
		case WIFI_CIPHER_TYPE_TKIP:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
		break;
		
		case WIFI_CIPHER_TYPE_CCMP:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
		break;

		case WIFI_CIPHER_TYPE_TKIP_CCMP:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
		break;

		default:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
		break;
	}

	switch (group_cipher) {
		case WIFI_CIPHER_TYPE_NONE:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
		break;
		case WIFI_CIPHER_TYPE_WEP40:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
		break;
		case WIFI_CIPHER_TYPE_WEP104:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
		break;
		case WIFI_CIPHER_TYPE_TKIP:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
		break;
		case WIFI_CIPHER_TYPE_CCMP:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
		break;
		case WIFI_CIPHER_TYPE_TKIP_CCMP:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
		break;
		default:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
		break;
	}
}
void wifi_scan() /* Initialize Wi-Fi as sta and set scan method */
{
	uint16_t number = DEFAULT_SCAN_LIST_SIZE;
	wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
	uint16_t ap_count = 0;

	memset(ap_info, 0, sizeof(ap_info));

	esp_wifi_scan_start(NULL, true);

	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
	ESP_LOGI(TAG_WIFI, "Total APs scanned = %u", ap_count);

	for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
		ESP_LOGI(TAG_WIFI, "SSID \t\t%s", ap_info[i].ssid);
		ESP_LOGI(TAG_WIFI, "RSSI \t\t%d", ap_info[i].rssi);
		print_auth_mode(ap_info[i].authmode);
		if (ap_info[i].authmode != WIFI_AUTH_WEP) { 
		print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
		}
		ESP_LOGI(TAG_WIFI, "Channel \t\t%d\n", ap_info[i].primary);
	}
}
void wifi_scan2(uint16_t &number, wifi_ap_record_t* ap_info, uint16_t &ap_count) /* Initialize Wi-Fi as sta and set scan method */
{
	// uint16_t number = DEFAULT_SCAN_LIST_SIZE;
	// wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
	// uint16_t ap_count = 0;

	esp_wifi_scan_start(NULL, true);

	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
	ESP_LOGI(TAG_WIFI, "Total APs scanned = %u", ap_count);
	for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
		ESP_LOGI(TAG_WIFI, "SSID \t\t%s", ap_info[i].ssid);
		ESP_LOGI(TAG_WIFI, "RSSI \t\t%d", ap_info[i].rssi);
		print_auth_mode(ap_info[i].authmode);
		if (ap_info[i].authmode != WIFI_AUTH_WEP) { 
		print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
		}
		ESP_LOGI(TAG_WIFI, "Channel \t\t%d\n", ap_info[i].primary);
	}
}
static void wifi_conn_led_indicator_pwm(void)
{
	if(wifi_state_old != wifi_state)
	{
		wifi_state_old = wifi_state;

		if(wifi_state == state_conn::connected)
		{
			led_wifi_indicator.pwm_ledc_set_duty(3);
		}
		else if(wifi_state == state_conn::disconnected)
		{
			led_wifi_indicator.pwm_ledc_set_duty(50);
		}
		else
		{
			led_wifi_indicator.pwm_ledc_set_duty(90);			
		}
	}
}
// void wifi_conn_led_indicator(void*)
// {
// 	led0.mode(GPIO_MODE_OUTPUT);
// 	led0.write(1);
// 	int estado0 = 0;
// 	while(1)
// 	{
// 		if(wifi_state == state_conn::connected) {
// 			if(estado0)
// 			{
// 				estado0 = 0;
// 				led0.write(0);
// 				vTaskDelay(100 / portTICK_PERIOD_MS);
// 				led0.write(1);
// 			}
// 			else
// 			{
// 				estado0 = 1;
// 				vTaskDelay(2000 / portTICK_PERIOD_MS);
// 			}
// 		}
// 		else if(wifi_state == state_conn::disconnected) {
// 			led0.toggle();
// 			vTaskDelay(100 / portTICK_PERIOD_MS);
// 		}
// 		else
// 			led0.write(0);
// 		}
// }
void machine_run(void *pvParameter)
{
	// wifi and ws server init;
	// bt_init();
	wifi_sta_init();

	// ws server will ask to init into connection_event_handler when got IP.

	// ws client initialize
	// ws_client_start();

	// acionna initialize
	// acionna0.init();

	// dht0.begin();
	// temp_sensor.begin();
	// temp_sensor_count = temp_sensor.getDeviceCount();
	// ESP_LOGI(TAG_SENSORS, "Temp sensors count: %u", temp_sensor_count);


	while(1)
	{
		// acionna0.run();

		// wifi_conn_led_indicator_pwm();

		if(flag_ip_got)
		{
			flag_ip_got = 0;
			// httpd_server_start();
			// ota_get_info();
			// _delay_ms(4000);
			// xTaskCreate(&ota_task, "ota_task0", (1024 * 8), NULL, 5, NULL);
			// ota_task();
			// ws_client_start();
		}



		// if(acionna0.signal_restart) {
		// 	acionna0.signal_restart = 0;
		// 	esp_shutdown_h_now();        
		// } else if(acionna0.signal_wifi_info) {   
		// 	acionna0.signal_wifi_info = 0;
		// 	ESP_LOGI(TAG_WIFI, "signal_wifi_info");

		// 	wifi_ap_record_t wifi_info;

		// 	if (esp_wifi_sta_get_ap_info(&wifi_info)== ESP_OK)
		// 	{
		// 		char *str0 = (char*) wifi_info.ssid;
		// 		sprintf(buffer, "SSID: %s, RSSI: %d\n", str0, static_cast<int>(wifi_info.rssi));
		// 	}
		// 	signal_send = states_flag::enable;
		// } else if(acionna0.signal_wifi_scan) {
		// 	acionna0.signal_wifi_scan = 0;

		// 	uint16_t ap_count = 0;
		// 	uint16_t number = DEFAULT_SCAN_LIST_SIZE;
		// 	wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
		// 	wifi_ap_record_t *ap_info_ptr = &ap_info[0];

		// 	memset(ap_info, 0, sizeof(ap_info));

		// 	wifi_scan2(number, ap_info_ptr, ap_count);
		// 	for(int i=0; (i<DEFAULT_SCAN_LIST_SIZE) && (i<ap_count); i++)
		// 	{
		// 		sprintf(buffer, "Ch: %d, RSSI: %d, SSID: %s\n", ap_info[i].primary, ap_info[i].rssi, ap_info[i].ssid);

		// 		if(bt_state == state_conn::connected)
		// 		{
		// 			uint8_t* spp_pkt_data;
		// 			// *spp_pkt_data = &spp_data[0];
		// 			spp_pkt_data = reinterpret_cast<uint8_t*>(&buffer[0]);

		// 			// for(int i=0; i<strlen(buffer); i++)
		// 			// {
		// 			//     printf("%c", spp_pkt_data[i]);
		// 			// }

		// 			bt_sock0_len = strlen(buffer);
		// 			esp_spp_write(bt_sock0, bt_sock0_len, spp_pkt_data);	// send data;
		// 		}

		// 		if(wifi_state == state_conn::connected)
		// 		{
		// 			if(httpd_ws_get_fd_info(sock0.hd, sock0.fd) == HTTPD_WS_CLIENT_WEBSOCKET)
		// 			{
		// 				httpd_ws_frame_t ws_pkt;
		// 				memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
		// 				ws_pkt.payload = reinterpret_cast<uint8_t*>(&buffer[0]);
		// 				// ws_pkt.payload = (uint8_t*)buffer;
		// 				ws_pkt.len = strlen(buffer);
		// 				ws_pkt.type = HTTPD_WS_TYPE_TEXT;

		// 				httpd_ws_send_frame_async(sock0.hd, sock0.fd, &ws_pkt);
		// 			}
		// 		}
		// 	}
		// } else if (acionna0.signal_request_sensors) {
		// 	ESP_LOGI(TAG_SENSORS, "signal_request_sensors:%d", acionna0.signal_request_sensors);
		// 	acionna0.signal_request_sensors = 0;

		// 	char buffer[100] = "not changed!!\n";
		// 	_delay_ms(2000);
		// 	temp_sensor.requestTemperatures();

		// 	if(dht0.read2())
		// 		sprintf(buffer, "Tout:%.2f, Tin:%.1f, Humidity: %.1f%%\n", temp_sensor.getTempCByIndex(0), (float)dht0.getTempCelsius(0)*0.1, (float)dht0.getHumidity(0)*0.1);
		// 	else
		// 		sprintf(buffer, "Tout:%.2f, Tin:%.1f, Humidity: %.1f%% ER\n", temp_sensor.getTempCByIndex(0), (float)dht0.getTempCelsius(0)*0.1, (float)dht0.getHumidity(0)*0.1);
		// 	_delay_ms(1000);
			
		// 	signal_send = states_flag::enable;
		// 	// if(wifi_state == state_conn::connected)
		// 	// {
		// 	// 	if(httpd_ws_get_fd_info(sock0.hd, sock0.fd) == HTTPD_WS_CLIENT_WEBSOCKET)
		// 	// 	{
		// 	// 		httpd_ws_frame_t ws_pkt;
		// 	// 		memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
		// 	// 		ws_pkt.payload = reinterpret_cast<uint8_t*>(&buffer[0]);
		// 	// 		// ws_pkt.payload = (uint8_t*)buffer;
		// 	// 		ws_pkt.len = strlen(buffer);
		// 	// 		ws_pkt.type = HTTPD_WS_TYPE_TEXT;

		// 	// 		httpd_ws_send_frame_async(sock0.hd, sock0.fd, &ws_pkt);
		// 	// 	}
		// 	// }
		// } else if(acionna0.signal_ram_usage) {
		// 	acionna0.signal_ram_usage = 0;
		// 	sprintf(buffer, "RAM - free:%d, min:%d\n", esp_get_free_internal_heap_size(), esp_get_minimum_free_heap_size());
		// 	signal_send = states_flag::enable;
		// } else if(acionna0.signal_reset_reason) {
		// 	acionna0.signal_reset_reason = 0;
		// 	esp_reset_reason_t last_rst = esp_reset_reason();
		// 	switch (last_rst)
		// 	{
		// 		case ESP_RST_UNKNOWN:	// 0
		// 			sprintf(buffer, "%d: can not be determined\n", static_cast<int>(last_rst));
		// 			break;

		// 		case ESP_RST_POWERON:	// 1
		// 			sprintf(buffer, "%d: power-on\n", static_cast<int>(last_rst));
		// 			break;

		// 		case ESP_RST_EXT:		// 2
		// 			sprintf(buffer, "%d: ext pin\n", static_cast<int>(last_rst));
		// 			break;

		// 		case ESP_RST_SW:		// 3
		// 			sprintf(buffer, "%d: ext pin\n", static_cast<int>(last_rst));
		// 			break;

		// 		case ESP_RST_PANIC:		// 4
		// 			sprintf(buffer, "%d: panic\n", static_cast<int>(last_rst));
		// 			break;

		// 		case ESP_RST_INT_WDT:	// 5
		// 			sprintf(buffer, "%d: int wdt\n", static_cast<int>(last_rst));
		// 			break;

		// 		case ESP_RST_TASK_WDT:	// 6
		// 			sprintf(buffer, "%d: task wdt\n", static_cast<int>(last_rst));
		// 			break;

		// 		case ESP_RST_WDT:		// 7
		// 			sprintf(buffer, "%d: other wdt\n", static_cast<int>(last_rst));
		// 			break;

		// 		case ESP_RST_DEEPSLEEP:	// 8
		// 			sprintf(buffer, "%d: after exit deep sleep\n", static_cast<int>(last_rst));
		// 			break;

		// 		case ESP_RST_BROWNOUT:	// 9
		// 			sprintf(buffer, "%d: brownout rst\n", static_cast<int>(last_rst));
		// 			break;

		// 		case ESP_RST_SDIO:	// 10
		// 			sprintf(buffer, "%d: over SDIO\n", static_cast<int>(last_rst));
		// 			break;

		// 		default:
		// 			break;
		// 	}

		// 	signal_send = states_flag::enable;
		// } else if(acionna0.signal_ota_update) {
		// 	acionna0.signal_ota_update = 0;
		// 	httpd_server_stop(server);
		// 	server = NULL;
		// 	ota_task();
		// 	sprintf(buffer, "ota task inssue\n");
		// 	// signal_send = states_flag::enable;
		// } else if(acionna0.signal_ota_info) {
		// 	acionna0.signal_ota_info = 0;
		// 	ota_get_info();
		// 	sprintf(buffer, "ota info");
		// 	signal_send = states_flag::enable;
		// }

		// if(acionna0.signal_send_async) {
		// 	wifi_ap_record_t wifi_info;

		// 	if (esp_wifi_sta_get_ap_info(&wifi_info)== ESP_OK)
		// 	{
		// 		char *str0 = (char*) wifi_info.ssid;
		// 		sprintf(buffer, "SSID: %s, RSSI: %d", str0, static_cast<int>(wifi_info.rssi));
		// 	}

		// 	httpd_ws_frame_t ws_pkt;
		// 	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
		// 	// ws_pkt.payload = reinterpret_cast<uint8_t*>(&buffer[0]);
		// 	ws_pkt.payload = (uint8_t*)buffer;
		// 	ws_pkt.len = strlen(buffer);
		// 	ws_pkt.type = HTTPD_WS_TYPE_TEXT;

		// 	if(httpd_ws_get_fd_info(sock0.hd, sock0.fd) == HTTPD_WS_CLIENT_WEBSOCKET)
		// 	{
		// 		httpd_ws_send_frame_async(sock0.hd, sock0.fd, &ws_pkt);
		// 	}
		// 	else
		// 	{
		// 		acionna0.signal_send_async = 0;
		// 		ESP_LOGI(TAG_WS, "SOCK0: connection closed");
		// 	}
		// }

		// if(acionna0.signal_json_data_back) {
		// 	send_json_data_back();
		// }

		// if(acionna0.signal_json_data_server) {
		// 	send_json_data_server();
		// }





		// if(acionna0.signal_DHT21)
		// {
		// 	if(dht0.read2())
		// 	{
		// 		ESP_LOGI(TAG_SENSORS, "Temp outside: %.2f, Temp inside: %.2f, Humidity: %.2f%%", temp_sensor.getTempCByIndex(0), (float)dht0.getTempCelsius(0)*0.1, (float)dht0.getHumidity(0)*0.1);
		// 		// count_down = 10*60;
		// 	}
		// 	else
		// 	{
		// 		ESP_LOGI(TAG_DHT, "error reading");
		// 	}
		// }

		// if(acionna0.signal_DS18B20)
		// {
		// 	if(temp_sensor_count)
		// 	{
		// 		temp_sensor.requestTemperatures();
		// 		temp_sensor.getTempCByIndex(0);
		// 	}
		// }

		// if(signal_send == states_flag::enable)
		// {
		// 	signal_send = states_flag::disable;

		// 	if(wifi_state == state_conn::connected)
		// 	{
		// 		if(httpd_ws_get_fd_info(sock0.hd, sock0.fd) == HTTPD_WS_CLIENT_WEBSOCKET)
		// 		{
		// 			httpd_ws_frame_t ws_pkt;
		// 			memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
		// 			ws_pkt.payload = reinterpret_cast<uint8_t*>(&buffer[0]);
		// 			// ws_pkt.payload = (uint8_t*)buffer;
		// 			ws_pkt.len = strlen(buffer);
		// 			ws_pkt.type = HTTPD_WS_TYPE_TEXT;

		// 			httpd_ws_send_frame_async(sock0.hd, sock0.fd, &ws_pkt);
		// 		}
		// 	}

		// 	if(bt_state == state_conn::connected)
		// 	{
		// 		uint8_t* spp_pkt_data;
		// 		// *spp_pkt_data = &spp_data[0];
		// 		spp_pkt_data = reinterpret_cast<uint8_t*>(&buffer[0]);

		// 		// for(int i=0; i<strlen(buffer); i++)
		// 		// {
		// 		//     printf("%c", spp_pkt_data[i]);
		// 		// }

		// 		bt_sock0_len = strlen(buffer);
		// 		esp_spp_write(bt_sock0, bt_sock0_len, spp_pkt_data);	// send data;
		// 	}
		// }

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

// httpd_hanwdle_t start_webserver(void)
// {
//     httpd_handle_t server = NULL;
// httpd_config_t config = {
//     tskIDLE_PRIORITY+5,     // task_priority
//     4096,                   // stack_size
//     tskNO_AFFINITY,         // core_id
//     9000,                   // server_port
//     32768,                  // ctrl_port
//     7,                      // max_open_sockets
//     8,                      // max_uri_handlers
//     8,                      // max_resp_headers
//     5,                      //backlog_conn
//     false,                  // lru_purge_enable
//     5,                      // recv_wait_timeout
//     5,                      // send_wait_timeout
//     NULL,                   // global_user_ctx
//     NULL,                   // global_user_ctx_free_fn
//     NULL,                   // global_transport_ctx
//     NULL,                   // global_transport_ctx_free_fn
//     NULL,                   // open_fn
//     NULL,                   // close_fn
//     NULL                    // uri_match_fn
// };
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