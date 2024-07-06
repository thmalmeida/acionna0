#ifndef NTP_HPP__
#define NTP_HPP__

// date and time operations
#include "date_time.hpp"

// uC ESP32 specifics
#include "esp_sntp.h"

/* Network Time Protocol (NTP)
 * Ref.: https://datatracker.ietf.org/doc/html/rfc958
 *
 */

#define NTP_SERVER_1	"pool.ntp.org"
#define NTP_SERVER_2	"a.st1.ntp.br"
#define NTP_SERVER_3	"gps.ntp.br"

// Total of 48 bytes
typedef struct {

	uint8_t li_status;				// Leap indicator is a two-bit code. Status is a six-bit code

	uint8_t ref_clock_type;			// Reference clock type. Stratum level
	int16_t precision;				// Precision

	uint32_t est_error;				// Estimate error. Fixed point between bit 15 and 16
	int32_t est_drift_rate;			// Estimated Drift Rate
	uint32_t ref_clock_id;			// Reference Clock Identifier

	// MSB 32-bit integer and LSBits 32-bit fractional part
	uint64_t ref_timestamp;			// Reference Timestamp

	uint64_t orig_timestamp;		// Originate Timestamp

	uint64_t receive_timestamp;		// Receive Timestamp

	uint64_t transmit_timestamp;	// Transmit timestamp

} ntp_packet;


class NTP_Time{
public:
	NTP_Time() {
		init();
	}

	~NTP_Time() {
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
