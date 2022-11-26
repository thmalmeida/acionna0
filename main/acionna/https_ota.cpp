/* Advanced HTTPS OTA example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "https_ota.hpp"

// #if CONFIG_BOOTLOADER_APP_ANTI_ROLLBACK
// #include "esp_efuse.h"
// #endif

#define CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL "http://192.168.1.8/acionna/acionna0.bin"

static const char *TAG = "https_ota";
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

// #define OTA_URL_SIZE 256
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
	switch (evt->event_id) {
	case HTTP_EVENT_ERROR:
		ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
		break;
	case HTTP_EVENT_ON_CONNECTED:
		ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
		break;
	case HTTP_EVENT_HEADER_SENT:
		ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
		break;
	case HTTP_EVENT_ON_HEADER:
		ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
		break;
	case HTTP_EVENT_ON_DATA:
		ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
		break;
	case HTTP_EVENT_ON_FINISH:
		ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
		break;
	case HTTP_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
		break;
	}
	return ESP_OK;
}
esp_err_t validate_image_header(esp_app_desc_t *new_app_info)
{
	if (new_app_info == NULL) {
		return ESP_ERR_INVALID_ARG;
	}

	const esp_partition_t *running = esp_ota_get_running_partition();
	esp_app_desc_t running_app_info;
	if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
		ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
	}

#ifndef CONFIG_EXAMPLE_SKIP_VERSION_CHECK
	if (memcmp(new_app_info->version, running_app_info.version, sizeof(new_app_info->version)) == 0) {
		ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
		return ESP_FAIL;
	}
#endif

#ifdef CONFIG_BOOTLOADER_APP_ANTI_ROLLBACK
	/**
	 * Secure version check from firmware image header prevents subsequent download and flash write of
	 * entire firmware image. However this is optional because it is also taken care in API
	 * esp_https_ota_finish at the end of OTA update procedure.
	 */
	const uint32_t hw_sec_version = esp_efuse_read_secure_version();
	if (new_app_info->secure_version < hw_sec_version) {
		ESP_LOGW(TAG, "New firmware security version is less than eFuse programmed, %d < %d", new_app_info->secure_version, hw_sec_version);
		return ESP_FAIL;
	}
#endif

	return ESP_OK;
}
esp_err_t _http_client_init_cb(esp_http_client_handle_t http_client)
{
	esp_err_t err = ESP_OK;
	/* Uncomment to add custom headers to HTTP request */
	// err = esp_http_client_set_header(http_client, "Custom-Header", "Value");
	return err;
}
void ota_task(void*)
{
// #if defined(CONFIG_BOOTLOADER_APP_ROLLBACK_ENABLE)
	/**
	 * We are treating successful WiFi connection as a checkpoint to cancel rollback
	 * process and mark newly updated firmware image as active. For production cases,
	 * please tune the checkpoint behavior per end application requirement.
	 */
	// const esp_partition_t *running = esp_ota_get_running_partition();
	// esp_ota_img_states_t ota_state;
	// if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
	// 	if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
	// 		if (esp_ota_mark_app_valid_cancel_rollback() == ESP_OK) {
	// 			ESP_LOGI(TAG, "App is valid, rollback cancelled successfully");
	// 		} else {
	// 			ESP_LOGE(TAG, "Failed to cancel rollback");
	// 		}
	// 	}
	// }
// #endif
	/* Ensure to disable any WiFi power save mode, this allows best throughput
	 * and hence timings for overall OTA operation.
	 */
	// esp_wifi_set_ps(WIFI_PS_NONE);
// or
	/* WIFI_PS_MIN_MODEM is the default mode for WiFi Power saving. When both
	 * WiFi and Bluetooth are running, WiFI modem has to go down, hence we
	 * need WIFI_PS_MIN_MODEM. And as WiFi modem goes down, OTA download time
	 * increases.
	 */
	// esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

	ESP_LOGI(TAG, "Starting Advanced OTA example");

	esp_err_t ota_finish_err = ESP_OK;
	esp_http_client_config_t config;

	config.url = CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL;
	config.cert_pem = (char *)server_cert_pem_start;
	// config.event_handler = _http_event_handler;
	config.timeout_ms = 5000;
	config.keep_alive_enable = true;
	config.skip_cert_common_name_check = false;				// cert CN check

	esp_https_ota_config_t ota_config;
	ota_config.http_config = &config;
	ota_config.http_client_init_cb = _http_client_init_cb;	// Register a callback to be invoked after esp_http_client is initialized
	ota_config.partial_http_download = false;
	// ota_config.max_http_request_size = CONFIG_EXAMPLE_HTTP_REQUEST_SIZE;

	esp_https_ota_handle_t https_ota_handle = NULL;
	esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed");
		vTaskDelete(NULL);
	}

	esp_app_desc_t app_desc;
	err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "esp_https_ota_read_img_desc failed");
		goto ota_end;
	}

	err = validate_image_header(&app_desc);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "image header verification failed");
		goto ota_end;
	}

	ESP_LOGI(TAG, "esp_https_ota_perform start...");
	while (1) {
		err = esp_https_ota_perform(https_ota_handle);
		if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
			break;
		}
		// esp_https_ota_perform returns after every read operation which gives user the ability to
		// monitor the status of OTA upgrade by calling esp_https_ota_get_image_len_read, which gives length of image
		// data read so far.
		ESP_LOGI(TAG, "Image bytes read: %d, err: %d", esp_https_ota_get_image_len_read(https_ota_handle), err);
	}

	if (esp_https_ota_is_complete_data_received(https_ota_handle) == true)
	{
		ESP_LOGI(TAG, "Complete data received!!!");
		
		ota_finish_err = esp_https_ota_finish(https_ota_handle);
		if ((err == ESP_OK) && (ota_finish_err == ESP_OK))
		{
			ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			esp_restart();
		} else {
			if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
					ESP_LOGE(TAG, "Image validation failed, image is corrupted");
			}
			ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed 0x%x", ota_finish_err);
			vTaskDelete(NULL);
		}
	}

	// if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
	//     // the OTA image was not completely received and user can customise the response to this situation.
	//     ESP_LOGE(TAG, "Complete data was not received.");
	// } else {
	//     ota_finish_err = esp_https_ota_finish(https_ota_handle);
	//     if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) {
	//         ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");
	//         vTaskDelay(1000 / portTICK_PERIOD_MS);
	//         esp_restart();
	//     } else {
	//         if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
	//             ESP_LOGE(TAG, "Image validation failed, image is corrupted");
	//         }
	//         ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed 0x%x", ota_finish_err);
	//         vTaskDelete(NULL);
	//     }
	// }

ota_end:
	esp_https_ota_abort(https_ota_handle);
	ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed");
	vTaskDelete(NULL);
}
void ota_get_info(void)
{
	ESP_LOGI(TAG, "There is %d ota partitions", esp_ota_get_app_partition_count());

	const esp_partition_t *configured = esp_ota_get_boot_partition();
	const esp_partition_t *running = esp_ota_get_running_partition();
	
	// esp_ota_img_states_t ota_state;
	esp_app_desc_t running_app_info;
	esp_app_desc_t configured_app_info;

	// if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK)
	// {
	// 	switch (ota_state)
	// 		{
	// 			/*!< Monitor the first boot. In bootloader this state is changed to ESP_OTA_IMG_PENDING_VERIFY. */
	// 			case ESP_OTA_IMG_NEW:
	// 			ESP_LOGI(TAG, "ESP_OTA_IMG_NEW");
	// 			break;
				
	// 			/*!< First boot for this app was. If while the second boot this state is then it will be changed to ABORTED. */
	// 			case ESP_OTA_IMG_PENDING_VERIFY:
	// 			ESP_LOGI(TAG, "ESP_OTA_IMG_PENDING_VERIFY");
	// 			break;

	// 			/*!< App was confirmed as workable. App can boot and work without limits. */
	// 			case ESP_OTA_IMG_VALID:
	// 			ESP_LOGI(TAG, "ESP_OTA_IMG_VALID");
	// 			break;

	// 			/*!< App was confirmed as non-workable. This app will not selected to boot at all. */
	// 			case ESP_OTA_IMG_INVALID:
	// 			ESP_LOGI(TAG, "ESP_OTA_IMG_INVALID");
	// 			break;

	// 			/*!< App could not confirm the workable or non-workable. In bootloader IMG_PENDING_VERIFY state will be changed to IMG_ABORTED. This app will not selected to boot at all. */
	// 			case ESP_OTA_IMG_ABORTED:
	// 			ESP_LOGI(TAG, "ESP_OTA_IMG_ABORTED");
	// 			break;

	// 			/*!< Undefined. App can boot and work without limits. */
	// 			case ESP_OTA_IMG_UNDEFINED:
	// 			ESP_LOGI(TAG, "ESP_OTA_IMG_UNDEFINED");
	// 			break;

	// 			default:
	// 			ESP_LOGI(TAG, "ota state: default");
	// 			break;
	// 		}
	// }

	if(esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
	{
		ESP_LOGI(TAG, "Running fw ver: %s", running_app_info.version);
		ESP_LOGI(TAG, "Date: %s", running_app_info.date);
	}

	if(esp_ota_get_partition_description(configured, &configured_app_info) == ESP_OK)
	{
		ESP_LOGI(TAG, "Boot fw ver: %s", configured_app_info.version);
		ESP_LOGI(TAG, "Date: %s", configured_app_info.date);
	}
}