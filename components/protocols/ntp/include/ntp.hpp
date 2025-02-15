#ifndef NTP_HPP__
#define NTP_HPP__

// 20240924 - by thmalmeida

/* Network Time Protocol (NTP)
 * Ref.: https://datatracker.ietf.org/doc/html/rfc958
 *
 */


/* NTPv4 RFC 5905 replaces RFC 1305 (NTPv3)
* Ref.: https://www.rfc-editor.org/info/rfc5905
* 
* 
*/
#include "socket_udp.hpp"
#include "net_utils.hpp"

#include "date_time.hpp"

//***C++11 Style:***
#include <chrono>

// define to debug print
#define NTP_DEBUG		1

#define NTP_SERVER_00	"thmalmeida.us.to"
#define NTP_SERVER_01	"pool.ntp.org"
#define NTP_SERVER_02	"a.st1.ntp.br"			// 2001:12ff:0:7::186, 200.160.7.186	200.160.7.186	 and NTS
#define NTP_SERVER_03	"b.st1.ntp.br"			// ____ 	NTS
#define NTP_SERVER_04	"c.st1.ntp.br"			// ____ 	NTS
#define NTP_SERVER_05	"d.st1.ntp.br"			// ____ 	NTS
#define NTP_SERVER_06	"a.ntp.br"				// ____ 	NTS
#define NTP_SERVER_07	"b.ntp.br"				// ____ 	NTS
#define NTP_SERVER_08	"c.ntp.br"				// ____ 	NTS
#define NTP_SERVER_09	"gps.ntp.br"			// 2001:12ff:0:7::193 , 200.160.7.193 NTS
#define NTP_SERVER_10	"oceania.pool.ntp.org"
#define NTP_SERVER_11	"au.pool.ntp.org"
#define NTP_SERVER_12	"cn.pool.ntp.org"
#define NTP_SERVER_13 	"jp.pool.ntp.org"
#define NTP_SERVER_14 	"nz.pool.ntp.org"

// NTP server selected
#define NTP_SERVER NTP_SERVER_02

// Difference between Jan 1, 1900 and Jan 1, 1970
#define UNIX_OFFSET 2208988800L

struct ntp_packet_t {
	uint8_t li_status;			// Leap indicator is a two-bit code. Status is a six-bit code	
									// li.   Two bits.   Leap indicator. 0b 00 000 000
									// vn.   Three bits. Version number of the protocol.
									// mode. Three bits. Client will pick mode 3 for client.
	uint8_t ref_clock_type;		// Reference clock type. Stratum level of the local clock
	uint8_t poll;				// maximum interval between successive messages
	uint8_t precision;			// precision of local clock

	uint32_t root_delay;		// estimate error. Fixed point between bit 15 and 16
	uint32_t root_dispersion;	// estimated drift rate 
	uint32_t ref_clock_id;		// Reference clock identifier
	// uint32_t est_error;				// Estimate error. Fixed point between bit 15 and 16
	// int32_t est_drift_rate;			// Estimated Drift Rate
	// uint32_t ref_clock_id;			// Reference Clock Identifier

	// Reference timestamp.
	uint32_t ref_ts_secs;		// second
	uint32_t ref_ts_frac;		// fractional part of second

	// Originate time-stamp seconds.
	uint32_t origin_ts_secs;	// second
	uint32_t origin_ts_frac;	// fraction of a second

	// Receive timestamp is a 64 bit data. This is what we need mostly to get current time.
	uint32_t recv_ts_secs;		// seconds part
	uint32_t recv_ts_fracs;		// fractional of second part

	// Transmit timestamp is a 64 bit info
	uint32_t transmit_ts_secs;	// Transmit timestamp in second.
	uint32_t transmit_ts_frac;	// Transmit timestamp fractional of a second
};	// Total of 48 bytes
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

	uint64_t transmit_timestamp;	// Transmit timestamp.
} ntp_packet_1;
typedef struct {
	uint8_t li_vn_mode;      // Eight bits. li, vn, and mode. 00|100|011, li = 0, vn = 4 and mode = 3
							// li.   Two bits.   Leap indicator.
							// vn.   Three bits. Version number of the protocol.
							// mode. Three bits. Client will pick mode 3 for client.

	uint8_t stratum;         // Eight bits. Stratum level of the local clock.
	uint8_t poll;            // Eight bits. Maximum interval between successive messages.
	uint8_t precision;       // Eight bits. Precision of the local clock.

	uint32_t rootDelay;      // 32 bits. Total round trip delay time.
	uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
	uint32_t refId;          // 32 bits. Reference clock identifier.

	uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
	uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

	uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
	uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

	uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
	uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

	uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
	uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.
} ntp_packet_2;              // Total: 384 bits or 48 bytes.

enum class ntp_mode {
	client = 0,
	server = 1
};

class NTP{
public:
	NTP(void) {
		// client mode
		init();
	}
	NTP(const char *addr, uint16_t port) : client_(addr, port), port_(port) {
		// client mode
		init();
	}
	NTP(const char *addr) : client_(addr, port_) {
		// client mode
		init();
	}
		NTP(int port) :  port_(port) {
		// server mode: TODO
		init();
	}
	~NTP(void) {}

	void init() {
		#ifdef NTP_DEBUG
		printf("NTP client start\n");
		// std::cout << "NTP client" << std::endl;
		#endif

		// Set fuse time in hours;
		dt0.fuse(-3);
	}
	void server_name(const char* server_name_str) {
		// copy server name
		strcpy(server_name_, server_name_str);
		
		// get ip address of url time server
		netutils::nslookup(server_name_, server_ip_);

		// set server ip addr;
		server_addr(server_ip_);
	}
	void server_name(const char* server_name_str, uint16_t port) {
		// set server port;
		server_port(port);

		// set server name
		server_name(server_name_str);
	}
	void server_addr(const char* server_ip_str) {
		client_.addr(server_ip_str);
	}
	void server_addr(const char* server_ip_str, uint16_t port) {
		server_port(port);
		server_addr(server_ip_str);
	}
	void server_port(uint16_t port) {
		port_ = port;
		client_.port(port);
	}
	int fetch(void) {
		// clear ntp_packet before fill
		memset(&ntp_packet_, 0, sizeof(ntp_packet_));

		// build ntp header pkt: set li = 0, vn = 4 and mode = 3;
		li_vn_mode_(0, 4, 3);

		// send ntp pkt
		client_.send(&ntp_packet_, sizeof(ntp_packet_));
	
		// get instant time on sent pkg
		begin_ = std::chrono::steady_clock::now();

		// wait receive return ntp packet until timeout
		if(client_.receive(&ntp_packet_, &rx_size_) == 0) {
			end_ = std::chrono::steady_clock::now();
			unix_time_ =  ntohl(ntp_packet_.transmit_ts_secs) - UNIX_OFFSET;
			// time_t tx_ts_sec = (time_t) (unix_time_);

			dt0.unix_time(unix_time());

			delay_ = std::chrono::duration_cast<std::chrono::microseconds>(end_ - begin_).count();

			// Summary
			#ifdef NTP_DEBUG
			print_ntp_pkt_();
			#endif

			return 0;
		} else {
			#ifdef NTP_DEBUG
			printf("Timeout\n");
			// std::cerr << "Timeout" << std::endl;
			#endif
			return 1;
		}
	}
	int timeout(int t_sec) {
		return client_.timeout(t_sec);
	}
	
	// server mode
	void server_run(void) {
		// client_.bind_server()
	}
	
	uint32_t unix_time(void) {		
		return unix_time_;
	}

private:
	ntp_packet_t ntp_packet_;			// ntp packet
	SOCKET_UDP client_;					// udp socket to send and receive msgs
	size_t rx_size_;

	Date_Time dt0;

	std::chrono::steady_clock::time_point begin_, end_;
	uint32_t delay_;					// delay in us

	uint32_t unix_time_;
	char server_name_[30];				// the server name
	char server_ip_[INET6_ADDRSTRLEN];	// ip address of server name - 16 ipv4 or 46 for ipv6 size
	uint16_t port_ = 0;					// port of ntp server (default is 123)
		
	ntp_mode mode_;						// behavior mode client or server;

	void li_vn_mode_(uint8_t li, uint8_t vn, uint8_t mode) {
		ntp_packet_.li_status = (li << 6) | (vn << 3) | (mode << 0);
	}
	#ifdef NTP_DEBUG
	void print_ntp_pkt_(void) {

		printf("\e[35mServer\t\t: \e[0m%s:%d (%s)\n", server_ip_, port_, server_name_);
		printf("\e[35mVersion\t\t: \e[0m%d\n", ((ntp_packet_.li_status) >> 3) & 0x07);
		printf("\e[35mStratum\t\t: \e[0m%u\n", ntp_packet_.ref_clock_type);
		printf("\e[35mPrecision\t: \e[0m%u\n", ntp_packet_.precision);

		printf("\e[35mpool\t\t: \e[0m%u\n", ntp_packet_.poll);

		printf("\e[35mdelay [ms]\t: \e[0m%.3f\n", static_cast<double>(delay_)/1000.0);

		printf("\e[34mroot delay\t: \e[0m%lu\n", static_cast<long unsigned int>(ntp_packet_.root_delay));
		printf("\e[34mroot dispersion\t: \e[0m%lu\n", static_cast<long unsigned int>(ntp_packet_.root_dispersion));
		printf("\e[34mref clock id\t: \e[0m%lu\n", static_cast<long unsigned int>(ntp_packet_.ref_clock_id));

		printf("\e[34mref ts sec\t: \e[0m%lu\n", static_cast<long unsigned int>(ntp_packet_.ref_ts_secs));
		printf("\e[34mref ts frac\t: \e[0m%lu\n", static_cast<long unsigned int>(ntp_packet_.ref_ts_frac));

		printf("\e[34morg ts sec\t: \e[0m%lu\n", static_cast<long unsigned int>(ntp_packet_.origin_ts_secs));
		printf("\e[34morg ts frac\t: \e[0m%lu\n", static_cast<long unsigned int>(ntp_packet_.origin_ts_frac));

		printf("\e[34mrx ts sec\t: \e[0m%lu\n", static_cast<long unsigned int>(ntp_packet_.recv_ts_secs));
		printf("\e[34mrx ts frac\t: \e[0m%lu\n", static_cast<long unsigned int>(ntp_packet_.recv_ts_fracs));

		printf("\e[34mtx ts sec\t: \e[0m%lu\n", static_cast<long unsigned int>(ntp_packet_.transmit_ts_secs));
		printf("\e[34mtx ts frac\t: \e[0m%lu\n", static_cast<long unsigned int>(ntp_packet_.transmit_ts_frac));

		printf("\e[34munix time\t: \e[0m%lu\n", static_cast<long unsigned int>(unix_time()));

		printf("\e[33mDate time\t: \e[0m%.2d/%.2d/%.4d, %.2d:%.2d:%.2d\n", dt0.day(), dt0.month(), dt0.year(), dt0.hour(), dt0.minute(), dt0.second());
	}
	#endif
};

#endif /* NTP_HPP__ */
