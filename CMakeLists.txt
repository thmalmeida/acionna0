# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

set(PROJECT_VER "v1.0.2")

set(EXTRA_COMPONENT_DIRS
						components/agro_mesh/modules/clock
						components/basic_defines
						components/connections
						components/crc
						components/dsp
						components/eeprom
						components/helper
						components/modules
						components/ota
						components/peripherals
						components/protocols
						)


#get_filename_component(ProjectId ${CMAKE_CURRENT_LIST_DIR} NAME)
#string(REPLACE " " "_" ProjectId ${ProjectId})
#project(${ProjectId})
project(acionna0)

#idf_build_set_property(COMPILE_OPTIONS "-Wno-error=class-memaccess" APPEND)
#idf_build_set_property(COMPILE_OPTIONS "-fdiagnostics-color=always" APPEND)