idf_component_register(SRCS	clockin.cpp
							date_time.cpp
							sys_time.cpp
							INCLUDE_DIRS include
							REQUIRES esp_timer)
							# REQUIRES lwip nvs_flash esp_timer ds3231 ds1307)