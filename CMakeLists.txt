# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.25)

set(PROJECT_VER "v1.29-20250215-0846")
# set(PROJECT_VER "v1.28-20240830-0722")
#set(PROJECT_VER "v1.28-20240711-0949")
# set(PROJECT_VER "v1.27-20240709-1943")
# set(PROJECT_VER "v1.0.27-20240709-1230")
# set(PROJECT_VER "v1.0.26-20240705-2114")
# set(PROJECT_VER "v1.0.25-20240629-2109")

set(EXTRA_COMPONENT_DIRS
						# components/agro_mesh/modules/clock
						components/basic_defines
						components/clock
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
