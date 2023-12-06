idf_component_register(SRCS 
							convert_char_to_hex.c
							convert_char_type.c
							datetime.cpp
							delay.cpp
							my_string.cpp 
							number.cpp
							restart_async.c
							time_operations.cpp 
                       INCLUDE_DIRS include
                       REQUIRES esp_rom basic_defines esp_timer)
                       