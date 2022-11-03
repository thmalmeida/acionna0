#include <iostream>

#include "nvs_flash.h"

int flash_read(const char* key);
void flash_write(int data, const char* key);