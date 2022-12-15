#ifndef https_ota_hpp__
#define https_ota_hpp__

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"

#define FIRMWARE_UPGRADE_URL CONFIG_FIRMWARE_UPGRADE_URL    // "http://192.168.1.8/acionna/acionna0.bin"
#define OTA_RECV_TIMEOUT CONFIG_OTA_RECV_TIMEOUT            // 3000

esp_err_t _http_event_handler(esp_http_client_event_t *evt);
// esp_err_t validate_image_header(esp_app_desc_t *new_app_info);
// esp_err_t _http_client_init_cb(esp_http_client_handle_t http_client);
void advanced_ota_example_task(void *pvParameter);
void advanced_ota_start(void);
void ota_start(void);
void ota_task(void*);
void ota_get_info(void);

#endif