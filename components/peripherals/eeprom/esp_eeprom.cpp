#include "esp_eeprom.hpp"

int flash_read(const char* key)
{
	nvs_handle_t handle;
	int data=0;

	esp_err_t err = nvs_open(key, NVS_READONLY, &handle);
	if (err == ESP_OK)
	{
		int32_t data_stored;
		err = nvs_get_i32(handle, key, &data_stored);
		switch (err)
		{
			case ESP_OK:
				data = data_stored;
				break;
			case ESP_ERR_NVS_NOT_FOUND:
				break;
			default :
				break;
		}
		// Close
		nvs_close(handle);

		return data;
	}
	else
		return -1;
}
void flash_write(int data, const char* key)
{
	nvs_handle_t handle;
	// esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
	esp_err_t err = nvs_open(key, NVS_READWRITE, &handle);
	if (err == ESP_OK)
	{
		err = nvs_set_i32(handle, key, data);
		err = nvs_commit(handle);
		// Close
		nvs_close(handle);
	}
}