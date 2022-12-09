#ifndef NATIVE_OTA_HPP__
#define NATIVE_OTA_HPP__

#include <iostream>
#include <sstream>

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "nvs.h"
#include "nvs_flash.h"
// #include "driver/gpio.h"
// #include "protocol_examples_common.h"
#include "errno.h"

#if CONFIG_EXAMPLE_CONNECT_WIFI
// #include "esp_wifi.h"
#endif

#define BUFFSIZE 1024
#define HASH_LEN 32 /* SHA-256 digest length */

enum class OTA_http_client_states {
	disconnected = 0,
	connected
};

enum class OTA_process_states {
	idle = 0,
	updating,
	finish_update
};

struct OTA_struct {
	int32_t image_size;
	OTA_http_client_states OTA_http_client_state = OTA_http_client_states::disconnected;
	int binary_file_length_write = 0;
	OTA_process_states state = OTA_process_states::idle;
	const esp_partition_t *update_partition = NULL;
	const esp_partition_t *configured_partition = NULL;
	const esp_partition_t *running_partition = NULL;
	esp_ota_img_states_t running_state;
	esp_app_desc_t update_app_info;
	esp_app_desc_t running_app_info;
	uint8_t num_ota_partitions;
};

extern OTA_struct OTA_update;

void native_ota_start(void);
void native_ota_info(void);
#endif