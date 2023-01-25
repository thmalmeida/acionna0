#ifndef NATIVE_OTA_HPP__
#define NATIVE_OTA_HPP__

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

#include "esp_app_format.h"

#include "errno.h"

#define FIRMWARE_UPGRADE_URL CONFIG_FIRMWARE_UPGRADE_URL    // "http://192.168.1.8/acionna/acionna0.bin"
#define OTA_RECV_TIMEOUT CONFIG_OTA_RECV_TIMEOUT            // 3000

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
	uint8_t running_partition_sha_256[HASH_LEN];
	uint8_t update_partition_sha_256[HASH_LEN];
};

extern OTA_struct OTA_update;

void ota_conv_img_sha256(const uint8_t *image_hash, char *image_hash_str);
void ota_change_boot_partition(void);
void ota_partitions_sha256sum(void);
void ota_mark_valid(void);
void ota_mark_invalid(void);
void ota_set_partition(void);
void ota_start(void);
void ota_info(void);
#endif