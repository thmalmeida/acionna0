#include "eeprom.hpp"

#ifdef CONFIG_EEPROM_DEBUG
static const char *TAG_EEPROM = "EEPROM";
#endif

int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
static char partition_name_[15] = "not specified";
static const char eeprom_namespace_[16] = CONFIG_EEPROM_NAMESPACE;

static esp_err_t eeprom_open(nvs_handle_t *my_handle, const char* partition_name, nvs_open_mode_t nvs_access_mode) {
	#ifdef CONFIG_EEPROM_DEBUG
	ESP_LOGI(TAG_EEPROM, "Opening %s partition of namespace %s...", partition_name, eeprom_namespace_);
	#endif
	esp_err_t err = nvs_open_from_partition(partition_name, eeprom_namespace_, nvs_access_mode, my_handle);
	if (err != ESP_OK) {
		#ifdef CONFIG_EEPROM_DEBUG
		ESP_LOGI(TAG_EEPROM, "Error (%s) opening NVS handle!", esp_err_to_name(err));
		#endif
	}
	return err;
}
static void eeprom_close(nvs_handle_t *my_handle) {
	nvs_close(*my_handle);
}
esp_err_t eeprom_init(const char* partition_name) {
	strcpy(partition_name_, partition_name);
	#ifdef CONFIG_EEPROM_DEBUG
	ESP_LOGI(TAG_EEPROM, "Initializing %s NVS partition...", partition_name_);
	#endif
	return nvs_flash_init_partition(partition_name_);
}
char* eeprom_get_partition_name(void) {
	return partition_name_;
}
void eeprom_read_str(const char* key_word) {

}
esp_err_t eeprom_read_int32(const char* key_word, int32_t *value) {

	nvs_handle_t* my_handle_p = new nvs_handle_t;
	esp_err_t err = eeprom_open(my_handle_p, partition_name_, NVS_READONLY);

	if(err == ESP_OK)
	{
		err = nvs_get_i32(*my_handle_p, key_word, value);
		#ifdef CONFIG_EEPROM_DEBUG
		ESP_LOGI(TAG_EEPROM, "Reading value from NVS...");
		switch (err) {
			case ESP_OK:
				ESP_LOGI(TAG_EEPROM, "Read success value: %d", *value);
				break;
			case ESP_ERR_NVS_NOT_FOUND:
				ESP_LOGI(TAG_EEPROM, "void value");
				break;
			default :
				ESP_LOGI(TAG_EEPROM, "Error (%s) reading!", esp_err_to_name(err));
				break;
		}
		#endif
		eeprom_close(my_handle_p);
		delete my_handle_p;
	}
	else
	{
		#ifdef CONFIG_EEPROM_DEBUG
		ESP_LOGI(TAG_EEPROM, "Error opening nvs");
		#endif
		return err;
		delete my_handle_p;
	}

	return err;
}
esp_err_t eeprom_write_int32(const char* key_word, int32_t value) {
	nvs_handle_t* my_handle_p = new nvs_handle_t;
	esp_err_t err = eeprom_open(my_handle_p, partition_name_, NVS_READWRITE);

	if(err == ESP_OK) {
		esp_err_t err = nvs_set_i32(*my_handle_p, key_word, value);

		if(err == ESP_OK)
		{
			#ifdef CONFIG_EEPROM_DEBUG
			ESP_LOGI(TAG_EEPROM, "write value %d to %s partition with key word %s!", value, partition_name_, key_word);
			#endif
		}
		else
		{
			#ifdef CONFIG_EEPROM_DEBUG
			ESP_LOGI(TAG_EEPROM, "write Failed!");
			#endif
			eeprom_close(my_handle_p);
			delete my_handle_p;
			return err;
		}

		// Commit written value.
		// After setting any values, nvs_commit() must be called to ensure changes are written
		// to flash storage. Implementations may write to storage at other times,
		// but this is not guaranteed.
		err = nvs_commit(*my_handle_p);
		#ifdef CONFIG_EEPROM_DEBUG
		ESP_LOGI(TAG_EEPROM, "Committing updates in NVS ... ");

		if(err == ESP_OK)
			ESP_LOGI(TAG_EEPROM, "commit Done!");
		else
			ESP_LOGI(TAG_EEPROM, "commit Failed!");
		#endif

		eeprom_close(my_handle_p);
	}
	else
	{
		#ifdef CONFIG_EEPROM_DEBUG
		ESP_LOGI(TAG_EEPROM, "Error opening nvs");
		#endif
	}

	delete my_handle_p;

	return err;
}

#ifdef CONFIG_EEPROM_DEBUG
void eeprom_example(void) {
	eeprom_init("storage0");
	ESP_LOGI(TAG_EEPROM, "NVS get partition name: %s", eeprom_get_partition_name());

	int32_t temp0 = 0;
	int32_t temp1 = 0;

	// Reading temp0
	esp_err_t err = eeprom_read_int32("temp0", &temp0);
	if(err == ESP_OK)
		temp0++;
	else
		ESP_LOGI(TAG_EEPROM, "Error (%s) reading", esp_err_to_name(err));

	// Reading temp1
	err = eeprom_read_int32("temp1", &temp1);
	if(err == ESP_OK)
		temp1++;
	else
		ESP_LOGI(TAG_EEPROM, "Error (%s) reading", esp_err_to_name(err));

	// Writing temp 0
	err = eeprom_write_int32("temp0", temp0);

	// Writing temp 1
	err = eeprom_write_int32("temp1", temp1);

	// Restart module
	for (int i = 10; i >= 0; i--) {
		printf("Restarting in %d seconds...\n", i);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	printf("Restarting now.\n");
	fflush(stdout);
	esp_restart();
}
#endif