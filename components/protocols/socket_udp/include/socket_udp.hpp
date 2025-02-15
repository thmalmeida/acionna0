#ifndef SOCKET_UDP_HPP__
#define SOCKET_UDP_HPP__

/* C++14 */
// Header of UDP client-server model 
// #include <bits/stdc++.h>

#define ESP32_UC

#ifdef ESP32_UC
// #include <sys/param.h>

// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"

// #include "esp_system.h"
// #include "esp_event.h"
// #include "esp_log.h"

// #include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#else
// Unix includes
#include <arpa/inet.h> 
// #include <netinet/in.h>
// #include <sys/types.h> 
// #include <sys/socket.h> 
#include <unistd.h> // read(), write(), close()
#endif

// system
// #include <stdio.h>
#include <string.h>
// #include <stdlib.h>
// #include <iostream>

// #define SOCKET_UDP_DEBUG	1
#define DESTINATION_HOST "127.0.0.1"
#define MAXLINE_UDP 128

enum class socket_udp_mode {
	server = 0,
	client
};

class SOCKET_UDP {
public:

	// server mode - port and bind
	SOCKET_UDP(uint16_t port) : port_(port) {
		mode_ = socket_udp_mode::server;
		init();
		addr("127.0.0.1", port_);
		bind_server();
	}

	// client mode - addr + port
	SOCKET_UDP(const char* server_addr, uint16_t port) : port_(port)  {
		mode_ = socket_udp_mode::client;

		init();
		addr(server_addr, port_);
	}
	SOCKET_UDP(void) {
		mode_ = socket_udp_mode::client;
		init();
	}

	~SOCKET_UDP(void) {
		close_port();
	}

	// Create a socket only
	int init(void) {
		// Dummy protocol for TCP (= 0)
		ip_protocol_ = IPPROTO_UDP;	// IPPROTO_IP;

		// clear server and client addr structure
		// bzero( (char*) &remote_addr_, sizeof(remote_addr_));
		memset(&remote_addr_, 0, sizeof(remote_addr_));
		memset(&local_addr_, 0, sizeof(local_addr_));

		// Creating socket file descriptor and verification
		if((sockfd_ = socket(AF_INET, SOCK_DGRAM, ip_protocol_)) < 0 ) {
			#ifdef SOCKET_UDP_DEBUG
			printf("socket creation failed\n");
			// std::cerr << "socket creation failed" << std::endl; 
			#endif
			return 1; 
		} else {
			// std::clog << "socket created!" << std::endl;
		}
		return 0;
	}
	// set ip address and port
	void port(uint16_t port) {
		port_ = port;

		if(mode_ == socket_udp_mode::server) {			// Server mode
			local_addr_.sin_port = htons(port_);		// set port to listen
		} else if (mode_ == socket_udp_mode::client) {	// Client mode
			remote_addr_.sin_port = htons(port_);		// set port to listen
		}
	}
	int addr(const char *addr_str) {
		if(mode_ == socket_udp_mode::server) {			// Server mode
			local_addr_.sin_addr.s_addr = INADDR_ANY;	//in_addr_t or uint32_t		
			local_addr_.sin_family = AF_INET;			// set IPv4
		} else if (mode_ == socket_udp_mode::client) {	// Client mode

			// Filling remote/server information 
			remote_addr_.sin_family = AF_INET;			// set IPv4

			// Set ip address - way 1 (deprecated)
			// remote_addr_.sin_addr.s_addr = inet_addr(addr_str);

			// Set ip address - way 2
			inet_pton(remote_addr_.sin_family, addr_str, &(remote_addr_.sin_addr));

			// --- The next following lines is for debug purpose only. Get it back and print it ---
			// IPv4 string of inet_ntop() and inet_pton()
			// char remote_addr_str[INET_ADDRSTRLEN];
			
			// inet_ntop(remote_addr_.sin_family, &(remote_addr_.sin_addr), remote_addr_str, sizeof(remote_addr_str));
			// char *inet_ntoa(struct in_addr in);

			// printf("Client mode. Remote addr: %s:%d\n", remote_addr_str_, port_);

			/*
			use 
			inet_ntop() — Convert Internet address format from binary to text
			or
			inet_pton()
			*/


			/* Refs.: https://www.gta.ufrj.br/ensino/eel878/sockets/inet_ntoaman.html (DEPRECATED)
			struct sockaddr_in antelope;
			char *some_addr;

			inet_aton("10.0.0.1", &antelope.sin_addr); // store IP in antelope

			some_addr = inet_ntoa(antelope.sin_addr); // return the IP
			printf("%s\n", some_addr); // prints "10.0.0.1"

			// and this call is the same as the inet_aton() call, above:
			antelope.sin_addr.s_addr = inet_addr("10.0.0.1");
			*/
		}
		return 0;		
	}
	int addr(const char *addr_str, uint16_t _port) {
		port(_port);

		return addr(addr_str);
	}	
	int timeout(int t_sec) {
		// Set timeout
		struct timeval timeout;
		timeout.tv_sec = t_sec;
		timeout.tv_usec = 0;
		if(setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
			return 1;	// return fail;
		} else {
			return 0;	// return success;
		}
	}
	int bind_server(void) {
		// Bind the socket with the server address 
		if (bind(sockfd_, (struct sockaddr *)&local_addr_, sizeof(local_addr_)) < 0 ) {
				#ifdef SOCKET_UDP_DEBUG
				printf("bind failed\n");
				#endif
				return 1;
		} else {
			#ifdef SOCKET_UDP_DEBUG
			printf("Bind socket with server successfully!\n");
			#endif
			return 0;
		}
	}
	int listen_port(void) {
		#ifdef SOCKET_UDP_DEBUG
		printf("listening on port_: %d\n", port_);
		#endif
		char rx_str[40];
		while(1) {
			receive(rx_str, &n_bytes_);
			// Try the C++ casting operator and you need two of them: one to remove the const and another one to cast to your pointer type: https://stackoverflow.com/questions/43298862/how-to-convert-const-void-to-unsigned-char
			// strcpy(buffer_, reinterpret_cast<char*>(const_cast<void*>(ptr)));
		}

		return 0;
	}
	int send(const void *b_tx, size_t size) {
		// sendto(sockfd_, (const char *)hello_1, strlen(hello_1), MSG_CONFIRM, (const struct sockaddr *) &remote_addr_, sizeof(remote_addr_)); 
		if(sendto(sockfd_, b_tx, size, 0, (const struct sockaddr *) &remote_addr_, sizeof(remote_addr_)) < 0) {
			#ifdef SOCKET_UDP_DEBUG
			printf("udp error sent\n");
			// std::cerr << "udp error sent" << std::endl;
			#endif
			return 1;
		} else {
			// printf("sent %ld bytes to %s on port %d: %s\n", size, inet_ntoa(remote_addr_.sin_addr), ntohs(remote_addr_.sin_port), reinterpret_cast<char*>(const_cast<void*>(b_tx)));
			return 0;
		}

		// send back
		// sendto(sockfd_, (const char *)hello_2, strlen(hello_2), MSG_CONFIRM, (const struct sockaddr *) &local_addr_, len_); 
		// printf("Hello message sent.\n");
	}
	int receive(void *b_tx, size_t *size) {

		// socket: specifies the socket file descriptor;
		// buffer: points to the buffer where the message should be stored;
		// length: specifies the length in bytes of the buffer pointed to by the buffer argument;
		// flags: Specifies the type of message reception. Values of this argument are formed by logically OR'ing zero or more of the following values:
		//		- MSG_WAITALL
		// address: a null pointer, or points to a sockaddr structure in which the sending address is to be stored. The length and format of the address depend on the address family of the socket.
		// address_len: specifies the length of the sockaddr structure pointed to by the address argument.

		// struct sockaddr_storage source_addr;


		len_ = sizeof(local_addr_);  //len is value/result
		// *size = recvfrom(sockfd_, (char *)b_tx, MAXLINE_UDP, 0, (struct sockaddr *) &remote_addr_, &len_);
		int len0 = recvfrom(sockfd_, b_tx, MAXLINE_UDP, 0, (struct sockaddr *) &remote_addr_, &len_);
		// b_tx[*size] = '\0';
		*size = len0;

		if(len0 >= 0) {
			// printf("%ld bytes from %s:%d << %s", *size, inet_ntoa(remote_addr_.sin_addr), ntohs(remote_addr_.sin_port), (char *)b_tx);
			return 0;
		} else {
			#ifdef SOCKET_UDP_DEBUG
			printf("Timeout\n");
			// std::cout << "Timeout" << std::endl;
			#endif
			return 1;
		}
	}
	void close_port(void) {
		close(sockfd_);
	}

private:

	// server parameters
	int sockfd_;									// socket descriptor
	struct sockaddr_in remote_addr_, local_addr_;	// server and client struct addr

	// struct sockaddr_storage remote_addr_, local_addr_;

	socklen_t len_;									// packet size
	size_t n_bytes_;									// answer for reception

	uint16_t port_;									// destination port/server bind port
	int ip_protocol_ = 0;							// Standard well-defined IP protocols on in.h
	char buffer_[MAXLINE_UDP];

	socket_udp_mode mode_;							// behavior client/server
};

#endif