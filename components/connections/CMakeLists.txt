idf_component_register(SRCS
							bt_setup.cpp
							httpd_ssl_setup.cpp
							wifi_setup.cpp
                            ws_client_setup.cpp
                            ws_keep_alive.c
							INCLUDE_DIRS "."
							# REQUIRES esp_http_client esp_http_server bt gpio esp_hw_support pwm)
							REQUIRES esp_websocket_client esp_http_client esp_http_server esp_https_server bt  gpio esp_hw_support pwm
							EMBED_TXTFILES "certs/servercert.pem" "certs/prvtkey.pem")
							# REQUIRES esp_wifi)
