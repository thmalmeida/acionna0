#include "setup.hpp"
#include <stdio.h>
// #include "protocol_examples_common.h"

const char* TAG_SETUP = "SETUP";

void machine_run(void *pvParameter)
{
	// wifi and ws server init;
	wifi_sta_init();

    // ESP_ERROR_CHECK(esp_netif_init());
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    // ESP_ERROR_CHECK(example_connect());

	// ws server will ask to init into connection_event_handler when got IP.

	Acionna acionna0;

	// ws client initialize
	// ws_client_start();

	// acionna initialize
	

	// dht0.begin();
	// temp_sensor.begin();
	// temp_sensor_count = temp_sensor.getDeviceCount();
	// ESP_LOGI(TAG_SENSORS, "Temp sensors count: %u", temp_sensor_count);

	// pwm_ledc led0(2, 1, 50, 0);

	// int count = 0;
	// int pwm_state = 0;

	while(1)
	{
		// count++;
		// if(count == 3)
		// {
		// 	// ota_start();
		// 	advanced_ota_start();
		// }
		acionna0.run();

		// if(count > 3)
		// {
		// 	if(pwm_state)
		// 	{
		// 		pwm_state = 0;
		// 	}
		// 	else
		// 	{
		// 		pwm_state = 1;
		// 	}

		// 	count = 0;
		// }
		// else
		// 	count++;

		// if(!count)
		// {
		// 	if(pwm_state)
		// 		led0.pwm_ledc_set_duty(80);
		// 	else
		// 		led0.pwm_ledc_set_duty(10);
		// }		

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





// void send_json_data_back() {
// 	if(wifi_state == state_conn::connected)
// 	{
// 		if(httpd_ws_get_fd_info(sock0.hd, sock0.fd) == HTTPD_WS_CLIENT_WEBSOCKET)
// 		{
// 			DynamicJsonDocument doc(1024);
// 			doc["id"] = IP_END;
// 			doc["p1"] = acionna0.pipe1_.pressure_mca();
// 			doc["p2"] = acionna0.pipe1_.pressure_mca();
// 			doc["ton"] = acionna0.pump1_.time_on();
// 			doc["toff"] = acionna0.pump1_.time_off();
// 			doc["k1"] = static_cast<int>(acionna0.pump1_.state_k1());
// 			doc["k2"] = static_cast<int>(acionna0.pump1_.state_k2());
// 			doc["k3"] = static_cast<int>(acionna0.pump1_.state_k3());
// 			doc["rth"] = static_cast<int>(acionna0.pump1_.state_Rth());

// 			serializeJson(doc, buffer);
// 			// preparing json data
// 			// sprintf(buffer, "{\"id\":%d, \"p\":%d, \"ton\":%u, \"toff\":%u, \"k1\":%d, \"k2\":%d, \"k3\":%d, \"rth\":%d}", 
// 			// 								IP_END,
// 			// 								acionna0.pipe1_.pressure_mca(),
// 			// 								acionna0.pump1_.time_on(),
// 			// 								acionna0.pump1_.time_off(),
// 			// 								static_cast<int>(acionna0.pump1_.state_k1()),
// 			// 								static_cast<int>(acionna0.pump1_.state_k2()),
// 			// 								static_cast<int>(acionna0.pump1_.state_k3()),
// 			// 								static_cast<int>(acionna0.pump1_.state_Rth()));




// 			// if(ws_client_state == ws_client_states::connected)
// 			// {
// 			// 	ws_client_run(&buffer[0], strlen(buffer));
// 			// }
// 		}
// 		else
// 			acionna0.signal_json_data_back = 0;
// 	}
// }
















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