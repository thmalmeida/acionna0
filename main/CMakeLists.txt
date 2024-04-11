idf_component_register(SRCS main.cpp
							setup.cpp
							acionna/acionna.cpp
							acionna/pipepvc.cpp
							acionna/pump.cpp
							acionna/sensors/sensor_power.cpp
							acionna/sensors/sensor_pressure.cpp
							acionna/valves.cpp
							acionna/well.cpp
                    		INCLUDE_DIRS "."
							# REQUIRES connections)
							REQUIRES adc aht10 ahtx0 bmp180 bmp280 clock connections ds3231 eeprom helper i2c_driver math ota pcy8575 pwm ssd1306 timer)


# Ref.: https://github.com/espressif/esp-idf/blob/master/docs/en/api-guides/build-system.rst
# idf_component_register(EMBED_TXTFILES server_certs/ca_cert.pem)
