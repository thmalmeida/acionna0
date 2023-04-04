# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/thmalmeida/esp/esp-idf/components/bootloader/subproject"
  "/home/thmalmeida/workspace_esp32/acionna0/build/bootloader"
  "/home/thmalmeida/workspace_esp32/acionna0/build/bootloader-prefix"
  "/home/thmalmeida/workspace_esp32/acionna0/build/bootloader-prefix/tmp"
  "/home/thmalmeida/workspace_esp32/acionna0/build/bootloader-prefix/src/bootloader-stamp"
  "/home/thmalmeida/workspace_esp32/acionna0/build/bootloader-prefix/src"
  "/home/thmalmeida/workspace_esp32/acionna0/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/thmalmeida/workspace_esp32/acionna0/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/thmalmeida/workspace_esp32/acionna0/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
