idf_component_register(SRCS main.cpp
							setup.cpp
							acionna/acionna.cpp
							acionna/pipepvc.cpp
							acionna/pump.cpp
							acionna/sensors/sensor_power.cpp
							acionna/sensors/sensor_pressure.cpp
							acionna/valves.cpp
							acionna/well.cpp
                    		INCLUDE_DIRS .
							REQUIRES eeprom helper pwm i2c_master ds3231 clock connections adc protocol_examples_common ota bmp180 aht10)


# Ref.: https://github.com/espressif/esp-idf/blob/master/docs/en/api-guides/build-system.rst
# idf_component_register(EMBED_TXTFILES server_certs/ca_cert.pem)
