#include "setup.hpp"
// #include "esp_task_wdt.h"

#include "ssd1306.hpp"               // For OLED display
#include "bmp180.hpp"
#include "bmp280.hpp"
#include "ahtx0.hpp"
#include "aht10.hpp"
#include "ds1307.hpp"

const char* TAG_SETUP = "SETUP";

void test_bmp180(void) {

	I2C_Driver i2c(1, I2C_SDA, I2C_SCL);
	BMP180 sensor0(&i2c);

	if(sensor0.probe()) {
		sensor0.init();
		printf("Init BMP180\n");
	}

	while(1) {
		sensor0.fetch();
		printf("Pressure: %d Pa, Temp: %.1f C, Altitude: %ld\n", static_cast<int>(sensor0.pressure()), static_cast<double>(sensor0.temperature())/10.0, sensor0.altitude());

		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}
void test_bmp280(void) {
	// inverted pins
	I2C_Driver i2c(1, I2C_SDA, I2C_SCL);
	BMP280 sensor0(&i2c);

	int count = 0;

	if(sensor0.probe()) {
		sensor0.init();
	}

	while(1) {
		sensor0.fetch();
		printf("%02d - Pressure: %lu hPa, %.2f Pa, Temp: %.2f C, Altitude: %.1f\n", count++, sensor0.pressure_hPa(), sensor0.pressure(), static_cast<double>(sensor0.temperature())/100.0, sensor0.altitude());

		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}
void test_aht10(void) {
	I2C_Driver i2c(I2C_NUM_1, I2C_SCL, I2C_SDA);
	aht10 sensor0(&i2c);

	int count = 0;

	i2c.probe_find(0x00);

	while(1) {
		// ESP_LOGI(sensor0.get_status_bit(7, true);
		sensor0.trig_meas();
		ESP_LOGI(TAG_SETUP, "Count: %d, Humidity: %.2f %%, Temperature: %.2f C", count++, sensor0.humidity(), sensor0.temperature());
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
void test_ahtx0(void) {
	I2C_Driver i2c(1, I2C_SDA, I2C_SCL);

	// i2c.probe_list();

	AHTX0 sensor0(&i2c);
	// BMP280 sensor1(&i2c);

	sensor0.init();
	// sensor1.init();

	int count = 0;
	while(1) {
		sensor0.fetch();
		// sensor1.fetch();
		printf("%02d - Humidity: %.2f %%, Temperature: %.2f C\n", count++, sensor0.humidity(), sensor0.temperature());
		// printf("Pressure: %lu hPa, %.2f Pa, Temp: %.2f C, Altitude: %.1f\n", sensor1.pressure_hPa(), sensor1.pressure(), static_cast<double>(sensor1.temperature())/100.0, sensor1.altitude());
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}
void test_ssd1306(void) {
	I2C_Driver i2c(1, I2C_SDA, I2C_SCL);
	SSD1306 d0(&i2c);

	// i2c.probe_list();

	d0.init();
	d0.clear();

	int count = 0;
	uint8_t i = 0;
	while(1) {

		if(i < 64) {
			d0.draw_pixel(i, i*2);
			i++;
		}
		else {
			i=0;
		}
		printf("%2d\n", count++);
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}
void test_ssd1306_sensors(void) {
	
	I2C_Driver i2c(1, I2C_SDA, I2C_SCL);
	// i2c.probe_list();

	SSD1306 d0(&i2c);
	// BMP280 s0(&i2c);
	AHTX0 s1(&i2c);

	d0.init();
	d0.clear();

	// s0.init();
	s1.init();

	char str[60];

	// double temp_f;
	// int temp_int;
	// int temp_dec;
	// int count = 0;

	while(1) {

		// s0.fetch();
		s1.fetch();

		// Get temperature
		// temp_f = s1.temperature();

		// separate integer from decimal part
		// temp_int = static_cast<int>(temp_f);
		// temp_dec = static_cast<int>((temp_f-static_cast<double>(temp_int))*10.0);

		// print temperature value
		// sprintf(str, "%d", temp_int);
		// d0.print_Arial24x32(0,0, str);
		// sprintf(str, "%d", temp_dec);
		// d0.print_Arial24x32(48,0, str);
		
		// print temp value 2
		sprintf(str, "%.1f", s1.temperature());
		d0.print_Arial24x32(str, 0, 0);

		// print degree unit
		sprintf(str, "%c", 127);
		d0.print_Arial24x32(str, 96, 0);
		d0.print_Arial24x32('C', 105, 0);

		// Get humidity
		sprintf(str, "%.1f", s1.humidity());
		d0.print_Arial16x24(str, 0, 40);
		d0.print_Arial16x24("%", 4*16+10, 40);

		// Debug console print
		// printf("%d:  %.2f°C,  %.2f%%, %.2f°C, %.2fPa\n", count++, s1.temperature(), s1.humidity(), s0.temperature(), s0.pressure());

		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}
}
void test_ds1307(void) {
	I2C_Driver i2c(1, I2C_SCL, I2C_SDA);
	i2c.probe_list();

	DS1307 rtc0(&i2c);
	rtc0.init();
	rtc0.date_time(2024, 04, 17, 10, 28, 10);

	int count = 0;
	while(1) {
		rtc0.fetch();
		printf("%03d - %02d/%02d/%d %02d:%02d:%02d\n", count++, rtc0.day(), rtc0.month(), rtc0.year(), rtc0.hour(), rtc0.minute(), rtc0.second());
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

void test_chip_info(void) {
	/* Print chip information */
	esp_chip_info_t chip_info;
	uint32_t flash_size;
	esp_chip_info(&chip_info);
	printf("Model %d chip with %d CPU core(s), WiFi%s%s%s, ",
		static_cast<int>(chip_info.model),
		chip_info.cores,
		(chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
		(chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
		(chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

	unsigned major_rev = chip_info.revision / 100;
	unsigned minor_rev = chip_info.revision % 100;
	printf("silicon revision v%d.%d, ", major_rev, minor_rev);

	if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
		printf("Get flash size failed");
		return;
	}

	printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
	(chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

	printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
}
void test_i2c_to_gpio(void *pvParameter) {

	I2C_Driver i2c0(I2C_NUM_1, I2C_SCL, I2C_SDA);
	// pcy8575 i2c_to_gpio(&i2c);

	// uint8_t data[4];

	// uint8_t slave_addr = 0x23 >> 1;
	uint8_t slave_addr = 0x53;


	// uint8_t slave_reg = 0x01;

	const int n_bytes = 8;
	uint8_t value0 = 0x80;
	int i;
	uint8_t data_tx[n_bytes];
	for(i=0; i< n_bytes; i++) {
		data_tx[i] = value0++;
	}

	uint8_t data_rx[n_bytes] = {0};

	uint8_t T0 = 0;

	i2c_ans ret;

	while(1) {
		if(T0) {
			i2c0.read(slave_addr, 0xAF, &data_rx[0], n_bytes);
			// i2c0.read_only(slave_addr, &data_rx[0], n_bytes, true);
			for(int i=0; i<n_bytes; i++) {
				ESP_LOGI(TAG_SETUP, "Read byte[%d]: 0x%02x",i, data_rx[i]);
			}
			ESP_LOGI(TAG_SETUP, "");

		}
		else{
			ret = i2c0.write(slave_addr, data_tx[0], &data_tx[1], n_bytes);
			
			if(ret == i2c_ans::ok) {
				for(i=0; i<n_bytes; i++) {
							ESP_LOGI(TAG_SETUP, "writing byte data_tx[%d]: 0x%02x", i, data_tx[i]);
							data_tx[i]++;
				}
				ESP_LOGI(TAG_SETUP, "");
			}
			else {
				switch (ret) 
				{
					case i2c_ans::error_read:
						ESP_LOGI(TAG_SETUP, "Error read");
						break;
					case i2c_ans::error_write:
						ESP_LOGI(TAG_SETUP, "Error write");
						break;
					case i2c_ans::error_ack:
						ESP_LOGI(TAG_SETUP, "Error ACK");
						break;
					default:
						ESP_LOGI(TAG_SETUP, "Error: %d", static_cast<int>(ret));
						break;
				}
			}
		}

		T0 = !T0;

		// ESP_LOGI(TAG_SETUP, "Read data: 0x%02x\n", data);
		// i2c0.probe_find(0x00);
		// ESP_LOGI(TAG_SETUP, "Again!");

		// if(i2c.read(0x10, 0x00, &data[0], 4, true))
		// // if(i2c.read_only(0x10, &data[0], 4, true))
		// {
		// 	for(int i=0; i<4; i++) {
		// 		ESP_LOGI(TAG_SETUP, "Data: 0x%02x\n", data[i]);
		// 	}
		// }
		// // if(!i2c_to_gpio.probe()) {
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}
void test_sensors(void *pvParameter) {
	// test_bmp180();
	// test_bmp280();
	// test_aht10();
	// test_ahtx0();
	// test_ssd1306();
	test_ssd1306_sensors();
	// test_ds1307();
}
void test_adc_dma(void *pvParameter) {
	
	ADC_Driver adc0(adc_mode::stream);
	// adc0.stream_config(0, 3);
	adc0.channel_config(0, 3);

	// Number of samples depends of sample frequency, signal frequency and number of cycles;
	const int n_samples = POINTS_PER_CYCLE*N_CYCLES;

	// adc raw data array;
	uint16_t adc_buffer[n_samples];						// são 700 pontos

	// time domain load current;
	double iL_t[n_samples];

	// rms load current;
	double iL_rms;

	// some dsp process;
	DSP s0;
	
	while(1) {

		// Read stream array from ADC using DMA;
		// adc0.stream_read(0, &adc_buffer[0], n_samples);
		adc0.read(0, &adc_buffer[0], n_samples);

		// Convert digital ADC raw array to iL(t) signal;
		s0.calc_iL_t(&adc_buffer[0], &iL_t[0], n_samples);

		// Find the RMS value from iL(t) signal
		iL_rms = s0.calc_rms(&iL_t[0], n_samples);

		// print adc raw values for debug purposes
		printf("adc_buffer: ");
		for(int i=0; i<n_samples; i++) {
			printf("%u, ", adc_buffer[i]);
		}
		printf("\n");

		// print rms load current in Amperes
		ESP_LOGI(TAG_SETUP, "iL_rms:%.2lf A\n", iL_rms);

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
void test_adc(void *pvParameter) {
	
	ADC_Driver adc0(adc_mode::oneshot);
	// adc0.oneshot_channel_config(0, 3, 12);
	adc0.channel_config(0, 3, 12);

	uint16_t adc_data;
	uint16_t adc_data_ss;

	while(1) {
		// adc0.stream_read(0, &adc_buffer[0], n_samples);
		adc_data_ss = adc0.read(0, 100);
		adc_data = adc0.read(0);
		ESP_LOGI(TAG_SETUP, "adc_data: %u, adc_data_ss: %u\n", adc_data, adc_data_ss);
		// printf("adc_buffer: ");
		// for(int i=0; i<n_samples; i++) {
		// 	printf("%u ", adc_buffer[i]);
		// }
		// printf("\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
void test_timer(void *pvParameter) {
	TIMER_driver tim0(1, 0, 1000000);
	ADC_Driver adc0(adc_mode::oneshot);

	// adc0.oneshot_channel_config(0, 3, 12);
	adc0.channel_config(0, 3);

	tim0.enable();
	tim0.start();

	uint64_t timestamp = 0;
	double Fs = 0;
	const int n_points = 233;
	uint16_t adc_buffer[n_points];

	test_chip_info();
	
	while(1) {

		tim0.set_count(0);
		for(int i=0; i<n_points; i++) {

			adc_buffer[i] = adc0.read(0);	// this takes aproximatelly 41 us;
			delay_us(246);
		}
		timestamp = tim0.get_count();

		Fs = 1.0/(static_cast<double>(timestamp)/static_cast<double>(n_points))*1000000;
		ESP_LOGI(TAG_SETUP, "adc_value: %d, timestamp: %llu us, Fs: %.1lf Samples/s", adc_buffer[0], timestamp, Fs);

		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
void test_wifi(void *pvParameter)
{
	wifi_sta_init(30);

	int count = 0;
	while(1) {
		ESP_LOGI(TAG_SETUP, "Count: %d", count++);
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}

void isr_1sec(void *pvParameter) {
	while(1) {
		flag_1sec = 1;
		// vTaskDelay(1000 / portTICK_PERIOD_MS);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
void isr_100ms(void *pvParameter) {
	while(1) {
		flag_100ms = 1;
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void machine_run(void *pvParameter) {

	ADC_Driver adc(adc_mode::oneshot);
	I2C_Driver i2c(I2C_NUM_1, I2C_SCL, I2C_SDA);

	Acionna acionna0(&adc, &i2c);

	xTaskCreate(&isr_1sec, "isr_1sec_", 1024, NULL, 5, NULL);

	while(1) {
		acionna0.run();

		if(flag_1sec) {
			flag_1sec = 0;
			acionna0.run_every_second();
			// esp_task_wdt_reset();
			// vTaskDelay(1 / portTICK_PERIOD_MS);
			vTaskDelay(pdMS_TO_TICKS(1));

		}
		
		// if(flag_100ms) {
			// flag_100ms = 0;
			// acionna0.run_every_100ms();
		// }
		// vTaskDelay(1 / portTICK_PERIOD_MS);
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
