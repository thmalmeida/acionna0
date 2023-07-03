idf_component_register(SRCS
							bt_setup.cpp
							httpd_setup.cpp
							wifi_setup.cpp
							# wifi_setup0.c
                            ws_client_setup.cpp
                            ws_keep_alive.c
							INCLUDE_DIRS "."
							REQUIRES esp_http_client esp_http_server bt esp_websocket_client gpio esp_hw_support pwm)
							# REQUIRES esp_wifi)
