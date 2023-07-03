#ifndef _WIFI_SETUP0_H__
#define _WIFI_SETUP0_H__

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
// #include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

void wifi_init_sta_(void);
// void event_handler_(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

#endif