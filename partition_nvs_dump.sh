#!/bin/bash

# Ref.: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/partition-tables.html
#parttool.py --port "/dev/ttyUSB0" read_partition --partition-type=data --partition-subtype=nvs --partition-name=storage0 --output "storage0.bin"
parttool.py --port "/dev/ttyUSB0" read_partition --partition-name=nvs --output "storage0.bin"

# Erase partition with name 'storage'
parttool.py --port "/dev/ttyUSB0" erase_partition --partition-name=storage0

# Write to partition 'factory' the contents of a file named 'factory.bin'
parttool.py --port "/dev/ttyUSB1" write_partition --partition-name=factory --input "factory.bin"

# Print the size of default boot partition
parttool.py --port "/dev/ttyUSB1" get_partition_info --partition-boot-default --info size