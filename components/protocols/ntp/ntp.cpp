// #ifndef NTP_HPP__
// #define NTP_HPP__

// // 20240924 - by thmalmeida

// /* Network Time Protocol (NTP)
//  * Ref.: https://datatracker.ietf.org/doc/html/rfc958
//  *
//  */


// /* NTPv4 RFC 5905 replaces RFC 1305 (NTPv3)
// * Ref.: https://www.rfc-editor.org/info/rfc5905
// * 
// * 
// */
// #include "socket_udp.hpp"
// #include "net_utils.hpp"
// // #include "inttypes.h"
// // #include <cstdint>

// //***C++11 Style:***
// #include <chrono>

// #define NTP_SERVER_0	"thmalmeida.us.to"
// #define NTP_SERVER_1	"pool.ntp.org"
// #define NTP_SERVER_2	"a.st1.ntp.br"
// #define NTP_SERVER_3	"gps.ntp.br"

// #define UNIX_OFFSET 2208988800L

// #define NTP_SERVER NTP_SERVER_1


// // typedef struct {

// // 	uint8_t li_status;				// Leap indicator is a two-bit code. Status is a six-bit code

// // 	uint8_t ref_clock_type;			// Reference clock type. Stratum level
// // 	int16_t precision;				// Precision

// // 	uint32_t est_error;				// Estimate error. Fixed point between bit 15 and 16
// // 	int32_t est_drift_rate;			// Estimated Drift Rate
// // 	uint32_t ref_clock_id;			// Reference Clock Identifier

// // 	// MSB 32-bit integer and LSBits 32-bit fractional part
// // 	uint64_t ref_timestamp;			// Reference Timestamp

// // 	uint64_t orig_timestamp;		// Originate Timestamp

// // 	uint64_t receive_timestamp;		// Receive Timestamp

// // 	uint64_t transmit_timestamp;	// Transmit timestamp.

// // } ntp_packet_1;
// // typedef struct {
// // 	uint8_t li_vn_mode;      // Eight bits. li, vn, and mode. 00|100|011, li = 0, vn = 4 and mode = 3
// // 							// li.   Two bits.   Leap indicator.
// // 							// vn.   Three bits. Version number of the protocol.
// // 							// mode. Three bits. Client will pick mode 3 for client.

// // 	uint8_t stratum;         // Eight bits. Stratum level of the local clock.
// // 	uint8_t poll;            // Eight bits. Maximum interval between successive messages.
// // 	uint8_t precision;       // Eight bits. Precision of the local clock.

// // 	uint32_t rootDelay;      // 32 bits. Total round trip delay time.
// // 	uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
// // 	uint32_t refId;          // 32 bits. Reference clock identifier.

// // 	uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
// // 	uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

// // 	uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
// // 	uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

// // 	uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
// // 	uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

// // 	uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
// // 	uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.
// // } ntp_packet_2;              // Total: 384 bits or 48 bytes.

// NTP::NTP(void);
// NTP::NTP(const char *addr, int port) : client_(addr, port), port_(port);
// NTP::NTP(const char *addr) : client_(addr, port_);
// NTP::NTP(int port);
// NTP::~NTP();

// void NTP::init() {
// 	std::cout << "NTP server: " << NTP_SERVER << ", pkt_size: " << sizeof(ntp_packet_) << std::endl;
// }
// void NTP::server_name(const char* server_name, int port) {
//     strcpy(server_name_, server_name);
//     port_ = port;


//     // get ip address of url time server
//     nslookup(server_name, ip_str_);
//     // printf("ip: %s\n", ip_str_);
//     // set ip addr to udp socket
//     client_.addr_port(ip_str_, port);
// }
// void NTP::server_addr(const char* server_ip) {
//     if(port_ == 0)
//         port_ = 123;

//     server_addr(server_ip, port_);
// }
// void NTP::server_addr(const char* server_ip, int port) {
//     client_.addr_port(server_ip, port);
// }
// void NTP::fetch() {

//     memset(&ntp_packet_, 0, sizeof(ntp_packet_));

//     // build ntp header pkt: set li = 0, vn = 4 and mode = 3;
//     li_vn_mode_(0, 4, 3);

//     // send ntp pkt
//     client_.send(&ntp_packet_, sizeof(ntp_packet_));

//     // get instant time on sent pkg
//     std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

//     // size of rx packet
//     size_t size_rx;
//     // printf("Received 1\n");
//     if(client_.receive(&ntp_packet_, &size_rx) == 0) {
//         std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

//         // std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
//         std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()/1000.0 << "[ms]" << std::endl;
//         // std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;

//         ntp_packet_.transmit_ts_secs = ntohl(ntp_packet_.transmit_ts_secs);
//         time_t tx_ts_sec = (time_t) (ntp_packet_.transmit_ts_secs - UNIX_OFFSET);

//         printf("Time: %s\n", ctime(&tx_ts_sec));

//         print_ntp_pkt_();

//     } else {
//         std::cerr << "Timeout" << std::endl;
//     }
// }
// int NTP::timeout(int t_sec) {
//     return client_.timeout(t_sec);
// }
// void NTP::print_date(void) {

// }
// uint32_t NTP::unix_time(void) {
//     uint32_t t0 = 0;
    
//     return t0;
// }
// void NTP::li_vn_mode_(uint8_t li, uint8_t vn, uint8_t mode) {
//     ntp_packet_.li_status = (li << 6) | (vn << 3) | (mode << 0);
// }
// void NTP::print_ntp_pkt_(void) {
//     printf("li_status: 0x%02x\n", ntp_packet_.li_status);
    
//     printf("ref clock type: %u\n", ntp_packet_.ref_clock_type);
//     printf("pool: %u\n", ntp_packet_.poll);
//     printf("precision: %u\n", ntp_packet_.precision);

//     printf("root delay: %lu\n", static_cast<long unsigned int>(ntp_packet_.root_delay));
//     printf("root dispersion: %lu\n", static_cast<long unsigned int>(ntp_packet_.root_dispersion));
//     printf("ref clock id: %lu\n", static_cast<long unsigned int>(ntp_packet_.ref_clock_id));

//     printf("ref ts sec : %lu\n", static_cast<long unsigned int>(ntp_packet_.ref_ts_secs));
//     printf("ref ts frac: %lu\n", static_cast<long unsigned int>(ntp_packet_.ref_ts_frac));

//     printf("org ts sec : %lu\n", static_cast<long unsigned int>(ntp_packet_.origin_ts_secs));
//     printf("org ts frac: %lu\n", static_cast<long unsigned int>(ntp_packet_.origin_ts_frac));

//     printf("rx ts sec : %lu\n", static_cast<long unsigned int>(ntp_packet_.recv_ts_secs));
//     printf("rx ts frac: %lu\n", static_cast<long unsigned int>(ntp_packet_.recv_ts_fracs));

//     printf("transmitted ts sec : %lu\n", static_cast<long unsigned int>(ntp_packet_.transmit_ts_secs));
//     printf("transmitted ts frac: %lu\n", static_cast<long unsigned int>(ntp_packet_.transmit_ts_frac));
// }
