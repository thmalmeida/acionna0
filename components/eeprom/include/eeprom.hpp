#ifndef EEPROM_HPP__
#define EEPROM_HPP__

#include <iostream>

#include <string.h>

#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

//#define CONFIG_EEPROM_DEBUG 1
#define CONFIG_EEPROM_NAMESPACE "eeprom0"

esp_err_t eeprom_init(const char* partition_name);
char* eeprom_get_partition_name(void);
void eeprom_read_str(const char* key_word);
esp_err_t eeprom_read_int32(const char* key_word, int32_t *value);
esp_err_t eeprom_write_int32(const char* key_word, int32_t value);

#ifdef CONFIG_EEPROM_DEBUG
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
void eeprom_example(void);
#endif

#endif