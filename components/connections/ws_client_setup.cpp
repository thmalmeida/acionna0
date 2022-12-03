// -- Websocket client part ----------------------
#include "ws_client_setup.hpp"

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