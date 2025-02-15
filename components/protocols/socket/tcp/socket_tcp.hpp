#ifndef SOCKET_TCP_HPP__
#define SOCKET_TCP_HPP__

/* C++14 */
// Header of TCP client-server model 
// #include <bits/stdc++.h>	// not used 
#include <arpa/inet.h>
#include <netdb.h> 			// For TCP only ?
#include <netinet/in.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <unistd.h> // read(), write(), close()

#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 


#define DESTINATION_HOST "127.0.0.1"
// #define PORT 5000
#define MAX 80 
#define MAXLINE 1024

enum class socket_tcp_mode {
	server = 0,
	client
};

#define SA struct sockaddr 

class SOCKET_TCP {
public:

	// server mode
	SOCKET_TCP(uint16_t port) : port_(port) {
		mode_ = socket_tcp_mode::server;
		init();
		bind_server();
	}

	// client mode
	SOCKET_TCP(const char* server_addr, uint16_t port) : port_(port)  {
		mode_ = socket_tcp_mode::client;
		init();
	}

	~SOCKET_TCP(void) {
	}

	int init(void) {
		// Creating socket file descriptor and verification
		sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
		if( sockfd_ < 0 ) {
			printf("socket creation failed"); 
			return 1; 
		} else {
			printf("socket created!\n");
		}
		bzero(&server_addr_, sizeof(server_addr_));

		// clear server and client addr structure
		memset(&server_addr_, 0, sizeof(server_addr_)); 
		memset(&client_addr_, 0, sizeof(client_addr_));


		// Assig server data (IP and PORT)
		server_addr_.sin_family = AF_INET;				// IPv4 
		server_addr_.sin_port = htons(port_);
		if(mode_ == socket_tcp_mode::server) {
			server_addr_.sin_addr.s_addr = htonl(INADDR_ANY);	//in_addr_t or uint32_t		
			printf("Server mode\n");
		} else if (mode_ == socket_tcp_mode::client) {
			// assign IP addr
			// server_addr_.sin_addr.s_addr = inet_addr(DESTINATION_HOST);

			// IPv4 demo of inet_ntop() and inet_pton()
			char server_addr_str_[INET_ADDRSTRLEN];

			// store this IP address in sa:
			inet_pton(AF_INET, DESTINATION_HOST, &(server_addr_.sin_addr));

			// now get it back and print it
			inet_ntop(AF_INET, &(server_addr_.sin_addr), server_addr_str_, INET_ADDRSTRLEN);
			// char *inet_ntoa(struct in_addr in);
			// int inet_aton(const char *cp, struct in_addr *inp);
			// in_addr_t inet_addr(const char *cp);
			printf("Client mode\n destination addr: %s:%d\n", server_addr_str_, port_);
		}

		return 0;
	}
	int bind_server(void) {
		// Bind the socket with the server address 
		if (bind(sockfd_, (struct sockaddr *)&server_addr_, sizeof(server_addr_)) < 0 ) {
				printf("bind failed\n");
				return 1;
		} else {
			printf("Bind socket with server successfully!\n");
			return 0;
		}
	}
	int listen_port(void) {

		if(listen(sockfd_, 5)) {
			printf("Listen failed...\n");
			return 1;
		} else {
			printf("listening on port: %d\n", port_);
			return 0;
		}
	}

	// TCP server - wait accept
	int wait_connection(void) {
		len_ = sizeof(client_addr_);  //len is value/result

		// Accept the data packet from client and verification
		connfd_ = accept(sockfd_, (struct sockaddr*)&client_addr_, &len_);

		if(connfd_ < 0) {
			printf("TCP server accept failed...\n");
			return 1;
		} else {
			printf("TCP server accept connection!\n");
			return 0;
		}
	}

	// TCP client - connect to a server
	int connect_to_server(void) {
		// connect the client socket to server socket
		if (connect(sockfd_, (SA*)&server_addr_, sizeof(server_addr_)) != 0) {
			printf("connection with the server failed...\n");
			return 1;
		} else {
			printf("connected to the server..\n");
			return 0;
		}
	}

	void close_port(void) {
		close(sockfd_);
	}

	// Chat function for TCP protocol
	void f1(void) {
		char buffer[MAX];
		int n;

		while(1) {
			bzero(buffer, MAX); 

			printf("wait receive msg\n");
			// read the message from client and copy it in buffer 
			read(connfd_, buffer, sizeof(buffer));

			printf("%s:%d  %s\n",inet_ntoa(client_addr_.sin_addr), ntohs(client_addr_.sin_port), buffer_);

			

		}
	}

private:

	// server parameters
	int sockfd_;									// socket descriptor
	int connfd_;									// connection descriptor for TCP

	struct sockaddr_in server_addr_, client_addr_;	// server and client struct addr
	socklen_t len_;									// packet size
	int n_bytes_;									// answer for reception

	uint16_t port_;									// destination port/server bind port

	char buffer_[MAXLINE];

	socket_tcp_mode mode_;							// behavior client/server
};

#endif