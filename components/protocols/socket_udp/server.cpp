/* 20240605 */ 

#include "socket_udp.hpp"
#include "socket_tcp.hpp"

void server_tcp(void) {
	SOCKET_TCP server(5003);

	while(1) {
		server.listen_port();

		server.wait_connection();
	}

	server.close_port();

	printf("Server closed!\n");
}
void server_udp(void) {
	SOCKET_UDP server(5003);
}

int main(void) {
	// server_udp();
	server_tcp();

	return 0;
}