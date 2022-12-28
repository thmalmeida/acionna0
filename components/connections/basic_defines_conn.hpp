#ifndef BASIC_DEFINES_CONN_HPP__
#define BASIC_DEFINES_CONN_HPP__

#include "sdkconfig.h"

#define BUFFER_LEN 300

enum class conn_states {
	disconnected = 0,
	connected
};

#endif