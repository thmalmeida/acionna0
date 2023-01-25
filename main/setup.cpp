#include "setup.hpp"

const char* TAG_SETUP = "SETUP";

static I2C_Master i2c(I2C_NUM_1, I2C_SCL, I2C_SDA, I2C_NORMAL_SPEED_HZ, 1);
aht10 sensor0(&i2c);
// bmp180 sensor1(&i2c);
void test_sensors(void *pvParameter) {
	// Hello answer test
	// if(sensor0.probe()) {
	// 	ESP_LOGI(TAG_SETUP, "AHT10 answer OK!");
	// }
	// else {
	// 	while(!sensor0.probe())
	// 	{
	// 		ESP_LOGI(TAG_SETUP, "Trying to probe");
	// 		vTaskDelay(2000 / portTICK_PERIOD_MS);
	// 	}
	// }
	// sensor0.init(2);
	int count = 0;
	while(1) {
		// ESP_LOGI(sensor0.get_status_bit(7, true);
		sensor0.trig_meas();
		ESP_LOGI(TAG_SETUP, "Count: %d, Humidity: %.2f %%, Temperature: %.2f C", count++, sensor0.get_humidity(), sensor0.get_temperature());
		// sensor0.print_raw_data();
		// sensor0.get_status_bit(3, true);
		// sensor0.print_status_bits();
		// if(((count % 3) == 0) && (!sensor0.get_status_bit(6, false)))  {
		// 	ESP_LOGI(TAG_SETUP, "AHT init again!!!");
		// 	sensor0.init(2);
		// }
		vTaskDelay(10000 / portTICK_PERIOD_MS);
	}
}

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