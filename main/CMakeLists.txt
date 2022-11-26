idf_component_register(SRCS main.cpp
							acionna/acionna.cpp
							# acionna/bt_stuffs.cpp
							acionna/helper.cpp
							acionna/pipepvc.cpp
							acionna/pump.cpp
							acionna/pwm_ledc.cpp
							acionna/sensors/sensor_power.cpp
							acionna/sensors/sensor_pressure.cpp
							acionna/valves.cpp
							acionna/well.cpp
							acionna/ws_setup.cpp
							acionna/ws_keep_alive.c
							acionna/https_ota.cpp
                    		INCLUDE_DIRS .
							EMBED_TXTFILES ../server_certs/ca_cert.pem)


# Ref.: https://github.com/espressif/esp-idf/blob/master/docs/en/api-guides/build-system.rst
# idf_component_register(EMBED_TXTFILES server_certs/ca_cert.pem)
