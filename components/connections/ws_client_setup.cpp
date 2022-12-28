// -- Websocket client part ----------------------
#include "ws_client_setup.hpp"

esp_websocket_client_handle_t client;

uint8_t ws_client_data[BUFFER_LEN];
uint16_t ws_client_data_len = 0;
uint8_t ws_client_data_flag = 0;
conn_states ws_client_state = conn_states::disconnected;
// int i_ws;

void ws_client_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
	esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;

	switch (event_id)
	{
		case WEBSOCKET_EVENT_CONNECTED:
			ESP_LOGI(TAG_WS_CLIENT, "WEBSOCKET_EVENT_CONNECTED");
			ws_client_state = conn_states::connected;
			break;
		
		case WEBSOCKET_EVENT_DISCONNECTED:
			ESP_LOGI(TAG_WS_CLIENT, "WEBSOCKET_EVENT_DISCONNECTED");
			ws_client_state = conn_states::disconnected;
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
void ws_client_start(void)
{
	esp_websocket_client_config_t websocket_cfg = {};
	websocket_cfg.uri = CONFIG_WS_CLIENT_URI;
	websocket_cfg.port = CONFIG_WS_CLIENT_PORT;
	websocket_cfg.ping_interval_sec = 8;
	
	// websocket handle point create;
	client = esp_websocket_client_init(&websocket_cfg);

	// Resgister event function to loop
	esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, ws_client_event_handler, (void *)client);

	if(esp_websocket_client_start(client) != ESP_OK)
	{
		ESP_LOGI(TAG_WS_CLIENT, "Error starting websocket client");
	}
}
void ws_client_stop(void)
{
	if (esp_websocket_client_is_connected(client)) {
		esp_websocket_client_close(client, portMAX_DELAY);
		
		if(esp_websocket_client_destroy(client) == ESP_OK)
			ESP_LOGI(TAG_WS_CLIENT, "Websocket Stopped");
	}
}
void ws_client_send(std::string data)
{
	if (esp_websocket_client_is_connected(client)) {
		esp_websocket_client_send_text(client, reinterpret_cast<char*>(&data), data.length(), portMAX_DELAY);
	}
}
