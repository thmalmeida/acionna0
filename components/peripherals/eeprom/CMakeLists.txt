idf_component_register(SRCS esp_eeprom.cpp
                       INCLUDE_DIRS include                       # Edit following two lines to set component requirements (see docs)
                       REQUIRES "nvs_flash")

