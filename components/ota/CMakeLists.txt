idf_component_register(SRCS https_ota.cpp
                            advanced_https_ota.cpp
                            INCLUDE_DIRS .
                            REQUIRES app_update esp_https_ota esp_http_client
                            EMBED_TXTFILES server_certs/ca_cert.pem)