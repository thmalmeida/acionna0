idf_component_register(SRCS
							https_ota.cpp
							advanced_https_ota.cpp
							native_ota.cpp
							INCLUDE_DIRS .
							REQUIRES app_update esp_https_ota esp_http_client esp_event nvs_flash
							EMBED_TXTFILES server_certs/ca_cert.pem)


# nvs_flash is for native ota only