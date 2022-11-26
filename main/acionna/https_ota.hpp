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
#include "nvs.h"
#include "nvs_flash.h"

esp_err_t _http_event_handler(esp_http_client_event_t *evt);
esp_err_t validate_image_header(esp_app_desc_t *new_app_info);
esp_err_t _http_client_init_cb(esp_http_client_handle_t http_client);
void ota_task(void*);
void ota_get_info(void);

#endif