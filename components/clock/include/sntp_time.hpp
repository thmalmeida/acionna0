#ifndef SNTP_TIME_HPP__
#define SNTP_TIME_HPP__

// date and time operations
#include "date_time.hpp"

// uC ESP32 specifics
#include "esp_sntp.h"

/* Simple Network Time Protocol (SNTP) library is desined for ESP32 chip only
 *
 */

#define NTP_SERVER_1	"pool.ntp.org"
#define NTP_SERVER_2	"a.st1.ntp.br"
#define NTP_SERVER_3	"gps.ntp.br"

class SNTP_Time{
public:
	SNTP_Time() {
		init();
	}

	~SNTP_Time() {
		esp_netif_sntp_deinit();
	}

	void init() {
		esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG(NTP_SERVER_2);
		// setup only
		esp_netif_sntp_init(&config);

		// start the client service
		esp_netif_sntp_start();
	}

	void sync() {
		if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK) {
			printf("Failed to update system time within 10s timeout");
		}
	}

private:

};

#endif /* SNTP_TIME_HPP__ */
