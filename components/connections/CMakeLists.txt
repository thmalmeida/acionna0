idf_component_register(SRCS
							bt_setup.cpp
							httpd_setup.cpp
							wifi_setup.cpp
                            ws_client_setup.cpp
                            ws_keep_alive.c
							INCLUDE_DIRS .
							REQUIRES pwm esp_http_client esp_http_server bt esp_websocket_client gpio)