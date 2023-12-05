# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.25)

set(PROJECT_VER "v1.0.14-20231203-2243")

set(EXTRA_COMPONENT_DIRS
						components/agro_mesh/modules/clock
						components/basic_defines
						components/connections
						components/crc
						components/eeprom
						components/helper
						components/math
						components/modules
						components/ota
						components/peripherals
						components/protocols
						components_extra/esp_websocket_client
						)


#get_filename_component(ProjectId ${CMAKE_CURRENT_LIST_DIR} NAME)
#string(REPLACE " " "_" ProjectId ${ProjectId})
#project(${ProjectId})
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(acionna0)

#idf_build_set_property(COMPILE_OPTIONS "-Wno-error=class-memaccess" APPEND)
#idf_build_set_property(COMPILE_OPTIONS "-fdiagnostics-color=always" APPEND)