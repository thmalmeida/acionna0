#include "nvs_flash.h"
#include "acionna/ws_setup.hpp"

// #include "dht.hpp"
// #include "gpio.hpp"
// #include "dallas_temperature.h"
// #define TAG_SENSORS "Sensors"
// GPIO_Basic gpio_humidity_sensor(HUMIDITY_SENSOR, true);
// DHT dht0{&gpio_humidity_sensor};

// GPIO_Basic gpio_sensor(DS18B20_DATA);
// OneWire onewire(&gpio_sensor);
// Dallas_Temperature temp_sensor(&onewire);

// void run_dht(void*)
// {
// 	dht0.begin();
// 	temp_sensor.begin();
	
// 	std::uint8_t temp_sensor_count = temp_sensor.getDeviceCount();

// 	ESP_LOGI(TAG_SENSORS, "Temp sensors count: %u", temp_sensor_count);

// 	int time_to_read = 2;

// 	while(1)
// 	{
// 		temp_sensor.requestTemperatures();

// 		if(dht0.read2())
// 		{
// 			ESP_LOGI(TAG_SENSORS, "Temp outside: %.2f, Temp inside: %.2f, Humidity: %.2f%%", temp_sensor.getTempCByIndex(0), (float)dht0.getTempCelsius(0)*0.1, (float)dht0.getHumidity(0)*0.1);
// 			// time_to_read = 10*60;
// 		}
// 		else
// 		{
// 			// for(int i=0; i<40; i++)
// 			// {
// 			// 	ESP_LOGI(TAG_DHT, "Cicle[%d] low: %d high: %d\n", i+1, dht0.cycles[i].expectLow, dht0.cycles[i].expectHigh);
// 			// }

// 			ESP_LOGI(TAG_DHT, "error reading");
// 			// time_to_read = 2;
// 		}
// 		vTaskDelay((time_to_read)*1000 / portTICK_PERIOD_MS);
// 	}
// }

extern "C" void app_main(void)
{
	nvs_flash_init();

	// xTaskCreate(run_dht, "dht_test", 4096, NULL, 5, NULL);
	xTaskCreate(machine_run, "run_acionna0", 4096, NULL, 5, NULL);
}