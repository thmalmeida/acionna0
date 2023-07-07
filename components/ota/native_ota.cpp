#include "native_ota.hpp"

static const char *TAG_OTA = "OTA";
/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = { 0 };
extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

#define OTA_URL_SIZE 256

OTA_struct OTA_update;

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
	switch (evt->event_id) {
		case HTTP_EVENT_ERROR:
			ESP_LOGI(TAG_OTA, "HTTP_EVENT_ERROR");
			break;
	
		case HTTP_EVENT_ON_CONNECTED:
			ESP_LOGI(TAG_OTA, "HTTP_EVENT_ON_CONNECTED");
			OTA_update.OTA_http_client_state = OTA_http_client_states::connected;
			break;

		case HTTP_EVENT_HEADER_SENT:
			ESP_LOGI(TAG_OTA, "HTTP_EVENT_HEADER_SENT");
			break;
		
		case HTTP_EVENT_ON_HEADER:
			ESP_LOGI(TAG_OTA, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
			if(strcmp(evt->header_key, "Content-Length") == 0)
			{
				OTA_update.image_size = static_cast<uint32_t>(atoi(evt->header_value));
				// ESP_LOGI(TAG_OTA, "Image size: %d", OTA_update.image_size);
			}
			break;

		case HTTP_EVENT_ON_DATA:
			// ESP_LOGI(TAG_OTA, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
			break;

		case HTTP_EVENT_ON_FINISH:
			ESP_LOGI(TAG_OTA, "HTTP_EVENT_ON_FINISH");
			break;

		case HTTP_EVENT_DISCONNECTED:
			ESP_LOGI(TAG_OTA, "HTTP_EVENT_DISCONNECTED");
			OTA_update.OTA_http_client_state = OTA_http_client_states::disconnected;
			break;

		case HTTP_EVENT_REDIRECT:
			ESP_LOGI(TAG_OTA, "HTTP_EVENT_REDIRECT");
			break;

		default:
			break;
    }
    return ESP_OK;
}
static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}
static void __attribute__((noreturn)) task_fatal_error(void)
{
    ESP_LOGE(TAG_OTA, "Exiting task due to fatal error...");
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}
static void print_sha256(const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    ESP_LOGI(TAG_OTA, "%s: %s", label, hash_print);
}
static void ota_task(void *pvParameter)
{
	ota_info();

	esp_http_client_config_t config = {};
	config.url = FIRMWARE_UPGRADE_URL;
	config.cert_pem = (char *)server_cert_pem_start;
	config.timeout_ms = OTA_RECV_TIMEOUT;
	config.event_handler = _http_event_handler;
	config.keep_alive_enable = true;

    // esp_http_client_config_t config = {
    //     .url = FIRMWARE_UPGRADE_URL,
    //     .cert_pem = (char *)server_cert_pem_start,
	// 	.timeout_ms = OTA_RECV_TIMEOUT,
	// 	.event_handler = _http_event_handler,
    //     .keep_alive_enable = true,
    // };

#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL_FROM_STDIN
    char url_buf[OTA_URL_SIZE];
    if (strcmp(config.url, "FROM_STDIN") == 0) {
        example_configure_stdin_stdout();
        fgets(url_buf, OTA_URL_SIZE, stdin);
        int len = strlen(url_buf);
        url_buf[len - 1] = '\0';
        config.url = url_buf;
    } else {
        ESP_LOGE(TAG_OTA, "Configuration mismatch: wrong firmware upgrade image url");
        abort();
    }
#endif

#ifdef CONFIG_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
#endif

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG_OTA, "Failed to initialise HTTP connection");
        task_fatal_error();
    }

	esp_err_t err;
    err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_OTA, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        task_fatal_error();
    }

    esp_http_client_fetch_headers(client);

     OTA_update.update_partition = esp_ota_get_next_update_partition(NULL);
    assert(OTA_update.update_partition != NULL);
    ESP_LOGI(TAG_OTA, "Writing to partition subtype %u at offset 0x%08lx", OTA_update.update_partition->subtype, OTA_update.update_partition->address);

    OTA_update.binary_file_length_write = 0;
    /*deal with all receive packet*/
    bool image_header_was_checked = false;

	/* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
	esp_ota_handle_t update_handle = 0 ;

    while (1) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        if (data_read < 0) {
            ESP_LOGE(TAG_OTA, "Error: SSL data read error");
            http_cleanup(client);
            task_fatal_error();
        } else if (data_read > 0) {
            if (image_header_was_checked == false) {
                // esp_app_desc_t new_app_info;
                esp_image_segment_header_t new_app_size;
                // esp_image_header_t new_app_header;

                if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                    // check current version with downloading
                    memcpy(&OTA_update.update_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                    ESP_LOGI(TAG_OTA, "New firmware version: %s", OTA_update.update_app_info.version);
                    ESP_LOGI(TAG_OTA, "New firmware project name: %s", OTA_update.update_app_info.project_name);

                    memcpy(&new_app_size, &ota_write_data[sizeof(esp_image_header_t)], sizeof(esp_image_segment_header_t));
                    ESP_LOGI(TAG_OTA, "New firmware load addr: 0x08%lx", new_app_size.load_addr);
                    ESP_LOGI(TAG_OTA, "New firmware length: %lu", new_app_size.data_len);

                    // memcpy(&new_app_header, &ota_write_data[0], sizeof(esp_image_header_t));
                    // ESP_LOGI(TAG_OTA, "New firmware min chip rev: %d", new_app_header.min_chip_rev);

                    // esp_app_desc_t running_app_info;
					ESP_LOGI(TAG_OTA, "Running firmware version: %s", OTA_update.running_app_info.version);

                    const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
                    esp_app_desc_t invalid_app_info;
                    if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
                        ESP_LOGI(TAG_OTA, "Last invalid firmware version: %s", invalid_app_info.version);
                    }

                    // check current version with last invalid partition
                    if (last_invalid_app != NULL) {
                        if (memcmp(invalid_app_info.version, OTA_update.update_app_info.version, sizeof(OTA_update.update_app_info.version)) == 0) {
                            ESP_LOGW(TAG_OTA, "New version is the same as invalid version.");
                            ESP_LOGW(TAG_OTA, "Previously, there was an attempt to launch the firmware with %s version, but it failed.", invalid_app_info.version);
                            ESP_LOGW(TAG_OTA, "The firmware has been rolled back to the previous version.");
                            http_cleanup(client);
                            // infinite_loop();
                        }
                    }
#ifndef CONFIG_SKIP_VERSION_CHECK
                    if (memcmp(OTA_update.update_app_info.version, OTA_update.running_app_info.version, sizeof(OTA_update.update_app_info.version)) == 0) {
                        ESP_LOGW(TAG_OTA, "Current running version is the same as a new. We will not continue the update.");
                        http_cleanup(client);
                        esp_ota_abort(update_handle);
                        (void)vTaskDelete(NULL);
                        return;
                        // infinite_loop();
                    } 
#endif
                    image_header_was_checked = true;

                    err = esp_ota_begin(OTA_update.update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
                    if (err != ESP_OK) {
                        ESP_LOGE(TAG_OTA, "esp_ota_begin failed (%s)", esp_err_to_name(err));
                        http_cleanup(client);
                        esp_ota_abort(update_handle);
                        task_fatal_error();
                    }
					OTA_update.state = OTA_process_states::updating;
                    ESP_LOGI(TAG_OTA, "esp_ota_begin succeeded");
                } else {
                    ESP_LOGE(TAG_OTA, "received package is not fit len");
                    http_cleanup(client);
                    esp_ota_abort(update_handle);
                    task_fatal_error();
                }
            }
            
            err = esp_ota_write( update_handle, (const void *)ota_write_data, data_read);
            if (err != ESP_OK) {
                http_cleanup(client);
                esp_ota_abort(update_handle);
                task_fatal_error();
            }
            OTA_update.binary_file_length_write += data_read;
			
            // ESP_LOGI(TAG_OTA, "Written image length %d", OTA_update.binary_file_length_write);
			ESP_LOGI(TAG_OTA, "%.1f %%", static_cast<float>(OTA_update.binary_file_length_write/(float)OTA_update.image_size*100.0));
        } else if (data_read == 0) {
           /*
            * As esp_http_client_read never returns negative error code, we rely on
            * `errno` to check for underlying transport connectivity closure if any
            */
            if (errno == ECONNRESET || errno == ENOTCONN) {
                ESP_LOGE(TAG_OTA, "Connection closed, errno = %d", errno);
                break;
            }
            if (esp_http_client_is_complete_data_received(client) == true) {
                ESP_LOGI(TAG_OTA, "Connection closed");
                break;
            }
        }
    }
    ESP_LOGI(TAG_OTA, "Total Write binary data length: %d", OTA_update.binary_file_length_write);
    if (esp_http_client_is_complete_data_received(client) != true) {
        ESP_LOGE(TAG_OTA, "Error in receiving complete file");
        http_cleanup(client);
        esp_ota_abort(update_handle);
        task_fatal_error();
    }

    err = esp_ota_end(update_handle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG_OTA, "Image validation failed, image is corrupted");
        } else {
            ESP_LOGE(TAG_OTA, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        }
        http_cleanup(client);
        task_fatal_error();
    }

    err = esp_ota_set_boot_partition(OTA_update.update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_OTA, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        http_cleanup(client);
        task_fatal_error();
    }

    // Change state to finish update successfully
	OTA_update.state = OTA_process_states::finish_update;

    // Refresh update_app_info for reading purposes
    esp_ota_get_partition_description(OTA_update.update_partition, &OTA_update.update_app_info);

    // esp_restart();
    // return;

	ESP_LOGI(TAG_OTA, "http client cleanup function\n");	
    http_cleanup(client);

    ESP_LOGI(TAG_OTA, "Prepare to restart system!");
    (void)vTaskDelete(NULL);

    while(1)
    {
        ESP_LOGI(TAG_OTA, "Couldn't delete ota task!\n");
    }
}

void ota_conv_img_sha256(const uint8_t *image_hash, char *image_hash_str) {
	// char hash_print[HASH_LEN*2+1];
	// char hash_print_temp[3];

	image_hash_str[HASH_LEN*2] = 0;
	for (int i = 0; i < HASH_LEN; ++i) {
		sprintf(&image_hash_str[i * 2], "%02x", image_hash[i]);
    }

	// ESP_LOGI(TAG_OTA, "sha256 start\n");
	// // sprintf(hash_print, "%02x", image_hash[0]);
	// for(int i=1; i<HASH_LEN; i++) {
	// 	sprintf(hash_print_temp, "%02x", image_hash[i]);
	// 	strcat(hash_print, hash_print_temp);
	// 	ESP_LOGI(TAG_OTA, "%02x", image_hash[i]);
	// }
	// ESP_LOGI(TAG_OTA, "sha256 memcpy\n");
    // memcpy(image_hash_str, hash_print, sizeof(hash_print));
	// strcpy(image_hash_str, hash_print);
    // strcat(image_hash_str, "\n");
}
void ota_partitions_sha256sum(void) {

   	uint8_t sha_256[HASH_LEN] = { 0 };
	esp_partition_t partition;

	// get sha256 digest for the partition table
	partition.address   = ESP_PARTITION_TABLE_OFFSET;
	partition.size      = ESP_PARTITION_TABLE_MAX_LEN;
	partition.type      = ESP_PARTITION_TYPE_DATA;
	esp_partition_get_sha256(&partition, sha_256);
	print_sha256(sha_256, "SHA-256 for the partition table: ");

	// get sha256 digest for bootloader
	partition.address   = ESP_BOOTLOADER_OFFSET;
	partition.size      = ESP_PARTITION_TABLE_OFFSET;
	partition.type      = ESP_PARTITION_TYPE_APP;
	esp_partition_get_sha256(&partition, sha_256);
	print_sha256(sha_256, "SHA-256 for bootloader: ");

	// get sha256 digest for running partition
	esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
	print_sha256(sha_256, "SHA-256 for current firmware: ");

	// esp_ota_get_app_elf_sha256(&sha_256[0], sizeof(sha_256));
	// print_sha256(sha_256, "SHA-256 get elf firmware: ");
}
void ota_mark_valid(void) {
    esp_ota_mark_app_valid_cancel_rollback();
}
void ota_mark_invalid(void) {
    esp_ota_mark_app_invalid_rollback_and_reboot();
}
void ota_info(void) {

	if((OTA_update.configured_partition == NULL) || (OTA_update.running_partition == NULL)) { 
		OTA_update.configured_partition = esp_ota_get_boot_partition();
		OTA_update.running_partition = esp_ota_get_running_partition();

		if(esp_ota_get_partition_description(OTA_update.running_partition, &OTA_update.running_app_info) != ESP_OK)
		{
			ESP_LOGI(TAG_OTA, "Error getting running app info\n");
		}

		OTA_update.num_ota_partitions = esp_ota_get_app_partition_count();

		esp_partition_get_sha256(OTA_update.running_partition, OTA_update.running_partition_sha_256);
	}

	esp_err_t err;
	err = esp_ota_get_state_partition(OTA_update.running_partition, &OTA_update.running_state);
	if(err != ESP_OK)
	{
		ESP_LOGI(TAG_OTA, "error get ota state\n");
	}
    // if (OTA_update.configured_partition != OTA_update.running_partition) {
    //     ESP_LOGI(TAG_OTA, "Configured OTA boot partition at offset 0x%08x, but running from offset 0x%08x",
    //              OTA_update.configured_partition->address, OTA_update.running_partition->address);
    //     ESP_LOGI(TAG_OTA, "(This can happen if either the OTA boot data or preferred boot image become corrupted somehow.)");
    // }
    // ESP_LOGI(TAG_OTA, "Running partition type %d subtype %d (offset 0x%08x)", OTA_update.running_partition->type, OTA_update.running_partition->subtype, OTA_update.running_partition->address);


		// if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
			// // run diagnostic function ...
			// bool diagnostic_is_ok = diagnostic();
			// if (diagnostic_is_ok) {
			//     ESP_LOGI(TAG_OTA, "Diagnostics completed successfully! Continuing execution ...");
			//     esp_ota_mark_app_valid_cancel_rollback();
			// } else {
			//     ESP_LOGE(TAG_OTA, "Diagnostics failed! Start rollback to the previous version ...");
			//     esp_ota_mark_app_invalid_rollback_and_reboot();
			// }
		// }
	// }
}
void ota_change_boot_partition(void) {
	
	esp_partition_t* next_partition = NULL;
	if(esp_ota_get_next_update_partition(next_partition) == ESP_OK)
	{
		ESP_LOGI(TAG_OTA, "next partition: %s\n", next_partition->label);
		esp_ota_set_boot_partition(next_partition);
	}
}
void ota_start(void) {
    xTaskCreate(&ota_task, "ota_task", 8192, NULL, 5, NULL);
}

// static void infinite_loop(void)
// {
//     int i = 0;
//     ESP_LOGI(TAG_OTA, "When a new firmware is available on the server, press the reset button to download it");
//     while(1) {
//         ESP_LOGI(TAG_OTA, "Waiting for a new firmware ... %d", ++i);
//         vTaskDelay(2000 / portTICK_PERIOD_MS);
//     }
// }
// static bool diagnostic(void)
// {
//     gpio_config_t io_conf;
//     io_conf.intr_type    = GPIO_INTR_DISABLE;
//     io_conf.mode         = GPIO_MODE_INPUT;
//     io_conf.pin_bit_mask = (1ULL << CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);
//     io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
//     io_conf.pull_up_en   = GPIO_PULLUP_ENABLE;
//     gpio_config(&io_conf);

//     ESP_LOGI(TAG_OTA, "Diagnostics (5 sec)...");
//     vTaskDelay(5000 / portTICK_PERIOD_MS);

//     bool diagnostic_is_ok = gpio_get_level(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);

//     gpio_reset_pin(CONFIG_EXAMPLE_GPIO_DIAGNOSTIC);
//     return diagnostic_is_ok;
// }