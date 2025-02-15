#ifndef NET_UTILS_HPP__
#define NET_UTILS_HPP__

// created by thmalmeida on 20240922

/* C++14 */
// Header of UDP client-server model 
// #include <bits/stdc++.h>

// system
#include <stdint.h>
#include <stdio.h>
#include <string.h> // for memset()

// Network includes
#ifdef ESP32_UC
// #include "esp_netif.h"

// #include "lwip/err.h"
// #include "lwip/sockets.h"
// #include "lwip/sys.h"
// #include <lwip/netdb.h>

#else
// Unix includes
// #include <sys/types.h> 
// #include <sys/socket.h>
#include <arpa/inet.h> // inet_ntop() and inet_pton()
// #include <netinet/in.h>
#include <netdb.h>		// getaddrinfo() defined here
// #include <unistd.h> // read(), write(), close()

#endif

namespace netutils {

inline void endianess_show(uint32_t x) {
	// uint32_t x = 0x12345678;
	const uint8_t* p = reinterpret_cast<const uint8_t*>(&x);
	printf("0x%02x%02x%02x%02x\n", p[0], p[1], p[2], p[3]);
}

// DNS resolvers
inline int nslookup(const char* domain_name, char* addr_ip) {
	struct addrinfo hints, *results, *p;
	memset(&hints, '0', sizeof(hints));	// make sure the struct is empty
	hints.ai_family = AF_UNSPEC;		// don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;	// TCP stream socket type
	hints.ai_flags = AI_CANONNAME;	// input flags - Request for canonical name

	hints.ai_protocol = IPPROTO_TCP;	// protocol for socket

	int status = getaddrinfo(domain_name, NULL, &hints, &results);
	if (status != 0) {
		// printf("getaddrinfo() failed with error: %s [%d]\n", gai_strerror(status), status);
		printf("getaddrinfo() failed with error: [%d]\n", status);
		return 1;
	}

	// struct sockaddr_in *ip_access;

	for(p = results; p != NULL; p = p->ai_next) {
		void *addr;
		// char *ipver;
		int ipver = 0;

		if (p->ai_family == AF_INET) {	// IPv4
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			ipver = 4;
		} else {						// IPv6
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			ipver = 6;
		}
		
		// convert the IP to a string and print it:
		inet_ntop(p->ai_family, addr, addr_ip, INET6_ADDRSTRLEN);
		printf("IPv%d: %s\n", ipver, addr_ip);
	}

	freeaddrinfo(results);	// free the linked list
	return 0;
}

// void* getSinAddr(addrinfo *addr) {
// 	switch (addr->ai_family)
// 	{
// 		case AF_INET:
// 		return &(reinterpret_cast<sockaddr_in*>(addr->ai_addr)->sin_addr);

// 		case AF_INET6:
// 		return &(reinterpret_cast<sockaddr_in6*>(addr->ai_addr)->sin6_addr);
// 	}

// 	return NULL;
// }
// int nslookup_0(const char *domain_name, char *addr_ip, uint16_t *port) {

// 	addrinfo hints, *result, *addr_0;
// 	memset(&hints, 0, sizeof(hints));
// 	hints.ai_flags = AI_CANONNAME;		// input flags
// 	hints.ai_family = AF_UNSPEC;		// protocol family for socket
// 	hints.ai_socktype = SOCK_STREAM;	// socket type
// 	hints.ai_protocol = IPPROTO_TCP;	// protocol for socket

// 	int error = getaddrinfo(domain_name, NULL, &hints, &addr_0);
// 	if (error != 0) {
// 		std::cout << "getaddrinfo() failed: " << gai_strerror(error) << "\n";
// 		return -1;
// 	} else {
// 		// print the received url to resolve
// 		// std::cout << res->ai_canonname << "\n";
// 		// addr_0 = result;
// 		char ip[INET6_ADDRSTRLEN];
// 		// char addr_ip[100];

// 		do {
// 			printf("PORT_A: %u\n", *port);
// 			strcpy(addr_ip, inet_ntop(addr_0->ai_family, getSinAddr(addr_0), ip, sizeof(ip)));
// 			// strcpy(addr_ip, inet_ntoa(addr_0->ai_addr));
// 			printf("PORT_B: %u\n", *port);
// 			// std::cout << addr_ip << std::endl;
// 			addr_0 = addr_0->ai_next;
// 			printf("PORT_C: %u\n", *port);
// 		} while (addr_0);
// 	}
// 	// freeaddrinfo(result);
// 	freeaddrinfo(addr_0);
// 	printf("%s:%u\n", addr_ip, *port);
// 	return 0;
// }
// void nslookup_1(void) {
// 	hostent* myHostent = gethostbyname("google.com");
// 	if (!myHostent) {
// 		std::cout << "gethostbyname() failed" << "\n";
// 	} else {
// 		std::cout << myHostent->h_name << "\n";
// 		char ip[INET6_ADDRSTRLEN];

// 		for (unsigned int i = 0; myHostent->h_addr_list[i] != NULL; ++i) {
// 			std::cout << inet_ntop(myHostent->h_addrtype, myHostent->h_addr_list[i], ip, sizeof(ip)) << "\n";
// 		}
// 	}
// }
// void nslookup_2(void) {

// }
// int nslookup_3(const char *domain_name, char *addr_ip, uint16_t *port) {
// 	struct addrinfo *result;
// 	struct addrinfo *res;
// 	int error;

// 	error = getaddrinfo(domain_name, NULL, NULL, &result);

// 	if(error !=0) {
//        fprintf(stderr, "error in getaddrinfo: %s\n", gai_strerror(error));
//         return EXIT_FAILURE;
// 	}
	
// 	/* loop over all returned results and do inverse lookup */
// 	for (res = result; res != NULL; res = res->ai_next) {
// 		char hostname[NI_MAXHOST] = "";

// 		error = getnameinfo(res->ai_addr, res->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, 0); 
// 		if (error != 0)
// 		{
// 			fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(error));
// 			continue;
// 		}
// 		if (*hostname != '\0')
// 			printf("hostname: %s\n", hostname);
// 	}   

// 	freeaddrinfo(result);
// 	return EXIT_SUCCESS;
// }
}
#endif