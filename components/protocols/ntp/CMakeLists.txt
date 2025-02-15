idf_component_register(SRCS ntp.cpp
                            net_utils.cpp
                       INCLUDE_DIRS include
                       REQUIRES socket_udp clock)