idf_component_register(SRCS helper.cpp
                            rtc_time.cpp
							INCLUDE_DIRS include
							REQUIRES nvs_flash ds3231 esp_timer)
