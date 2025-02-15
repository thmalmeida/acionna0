#include "socket_udp.hpp"
#include "socket_tcp.hpp"
#include "ntp.hpp"

#include <iostream>

void client_tcp(void) {
	SOCKET_TCP client("127.0.0.1", 5003);

	client.connect_to_server();

}
void client_udp(void) {
	SOCKET_UDP client("127.0.0.1", 5003);

	char buffer[40];
	int len = 0;

	int count = 0;
	while (1) {

		sprintf(buffer, "%d", count++);
		client.send(&buffer[0], sizeof(buffer));
		client.receive(buffer, &len);
		
		sleep(1);
	}
}


void client_ntp(void) {
	// SOCKET_UDP ntp("")
	
}

// DNS resolvers
void resolv1(void) {
	hostent* myHostent = gethostbyname("google.com");
	if (!myHostent) {
		std::cout << "gethostbyname() failed" << "\n";
	} else {
		std::cout << myHostent->h_name << "\n";
		char ip[INET6_ADDRSTRLEN];

		for (unsigned int i = 0; myHostent->h_addr_list[i] != NULL; ++i) {
			std::cout << inet_ntop(myHostent->h_addrtype, myHostent->h_addr_list[i], ip, sizeof(ip)) << "\n";
		}
	}
}
// void* getSinAddr(addrinfo *addr)
// {
//     switch (addr->ai_family)
//     {
//         case AF_INET:
//             return &(reinterpret_cast<sockaddr_in*>(addr->ai_addr)->sin_addr);

//         case AF_INET6:
//             return &(reinterpret_cast<sockaddr_in6*>(addr->ai_addr)->sin6_addr);
//     }

//     return NULL;
// }

void nslookup2(const char *addr) {
	addrinfo hints = {};
	hints.ai_flags = AI_CANONNAME;		// input flags
	hints.ai_family = AF_UNSPEC;		// protocol family for socket
	hints.ai_socktype = SOCK_STREAM;	// socket type
	hints.ai_protocol = IPPROTO_TCP;	// protocol for socket

	addrinfo *res, *addr_;

	int ret = getaddrinfo(addr, NULL, &hints, &res);
	if (ret != 0) {
		std::cout << "getaddrinfo() failed: " << gai_strerror(ret) << "\n";
	} else {
		std::cout << res->ai_canonname << "\n";

		addr_ = res;
		char ip[INET6_ADDRSTRLEN];

		do {
			switch (addr_->ai_family) {
				case AF_INET:
					std::cout << inet_ntop(addr_->ai_family, &(reinterpret_cast<sockaddr_in*>(addr_->ai_addr)->sin_addr), ip, sizeof(ip)) << std::endl;
					break;
				case AF_INET6:
					std::cout << inet_ntop(addr_->ai_family, &(reinterpret_cast<sockaddr_in6*>(addr_->ai_addr)->sin6_addr), ip, sizeof(ip)) << std::endl;
					break;
			}	

			// std::cout << inet_ntop(addr_->ai_family, getSinAddr(addr_), ip, sizeof(ip)) << std::endl;
			addr_ = addr_->ai_next;
		} while (addr_);

		freeaddrinfo(res);
	}	
}

int main(void) {

	char str[] = "thmalmeida.us.to";
	nslookup2(str);
	// client_udp();
	// client_tcp();

	return 0;
}
