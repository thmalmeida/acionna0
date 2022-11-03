/*
* wifi_stuffs.h
*
*  Created on: 2 de nov. de 2021
*      Author: thmalmeida
*/
#include "wifi_setup.hpp"


//static void transmitter(void)
//{
//	// send() can return less bytes than supplied length.
//	// Walk-around for robust implementation.
//	int to_write = len;
//	rx_buffer[0]=rx_buffer[0]+1;
//	while (to_write > 0)
//	{
//		int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
//		if (written < 0)
//		{
//			ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//		}
//		to_write -= written;
//	}
//}
//static void receiver(const int sock)
//{
////	int len;
////	char rx_buffer[128];
//
//	do
//	{
//		len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
//
//		if (len < 0)
//		{
//			ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
//		}
//		else if (len == 0)
//		{
//			ESP_LOGW(TAG, "Connection closed");
//		}
//		else
//		{
//			rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
//			ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);
//
//			flag_payload_received = 1;
//		}
//
//		sInstr
//
//
//
//	} while (len > 0);
//}


int sock0;
int len0 = 0;
static char rx_buffer[128] = "";

void do_transmit_payload(void)
{
	// send() can return less bytes than supplied length.
	// Walk-around for robust implementation.
	int to_write = len0;
	rx_buffer[0]=rx_buffer[0]+1;
	while (to_write > 0)
	{
		int written = send(sock0, rx_buffer + (len0 - to_write), to_write, 0);
		if (written < 0)
		{
			ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
		}
		to_write -= written;
	}
}
void do_receive_payload(void)
{
//	int len0;
//	char rx_buffer[128];
	do
	{
		len0 = recv(sock0, rx_buffer, sizeof(rx_buffer) - 1, 0);
		if (len0 < 0)
		{
			ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
		}
		else if (len0 == 0)
		{
			ESP_LOGW(TAG, "Connection closed");
		}
		else
		{
			rx_buffer[len0] = 0; // Null-terminate whatever is received and treat it like a string
			ESP_LOGI(TAG, "Received %d bytes: %s", len0, rx_buffer);

			do_transmit_payload();
		}
	} while (len0 > 0);
}
void tcp_server_task(void *pvParameters)
{
	char addr_str[128];
	int addr_family = (int)pvParameters;
	int ip_protocol = 0;
	int keepAlive = 1;
	int keepIdle = KEEPALIVE_IDLE;
	int keepInterval = KEEPALIVE_INTERVAL;
	int keepCount = KEEPALIVE_COUNT;
	struct sockaddr_storage dest_addr;

	if (addr_family == AF_INET)
	{
		struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
		dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
		dest_addr_ip4->sin_family = AF_INET;
		dest_addr_ip4->sin_port = htons(PORT_TCP_ORIG);
		ip_protocol = IPPROTO_IP;
	}

	int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
	if (listen_sock < 0)
	{
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		vTaskDelete(NULL);
		return;
	}

	int opt = 1;
	setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	ESP_LOGI(TAG, "Socket created");

	int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

	if (err != 0)
	{
		ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
		ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
		goto CLEAN_UP;
	}
	ESP_LOGI(TAG, "Socket bound, port %d", PORT_TCP_ORIG);

	err = listen(listen_sock, 1);
	if (err != 0)
	{
		ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
		goto CLEAN_UP;
	}

	while (1)
	{
		ESP_LOGI(TAG, "Socket listening on port %d", PORT_TCP_ORIG);

		struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
		socklen_t addr_len = sizeof(source_addr);
		sock0 = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
		if (sock0 < 0)
		{
			ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
			break;
		}
		// Set tcp keepalive option
		setsockopt(sock0, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(int));
		setsockopt(sock0, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(int));
		setsockopt(sock0, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(int));
		setsockopt(sock0, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(int));
		// Convert ip address to string
		if (source_addr.ss_family == PF_INET)
		{
			inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
		}
		ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);
		do_receive_payload();
		shutdown(sock0, 0);
		close(sock0);
	}

	CLEAN_UP:
	close(listen_sock);
	vTaskDelete(NULL);
}

//static void udp_client_task(void *pvParameters)
//{
//	char rx_buffer[128];
//	char host_ip[] = IP_ADDR_DEST;
//	int addr_family = 0;
//	int ip_protocol = 0;
//
//	while (1)
//	{
//		struct sockaddr_in dest_addr;
//		dest_addr.sin_addr.s_addr = inet_addr(IP_ADDR_DEST);
//		dest_addr.sin_family = AF_INET;
//		dest_addr.sin_port = htons(PORT_UDP_DEST);
//		addr_family = AF_INET;
//		ip_protocol = IPPROTO_IP;
//
//		sock0 = socket(addr_family, SOCK_DGRAM, ip_protocol);
//		if (sock0 < 0)
//		{
//			ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
//			break;
//		}
//		ESP_LOGI(TAG, "Socket created, sending to %s:%d", IP_ADDR_DEST, PORT_UDP_DEST);
//
//		while (1)
//		{
//			int err = sendto(sock0, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
//			if (err < 0)
//			{
//				ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//				break;
//			}
//			ESP_LOGI(TAG, "Message sent");
//
//			struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
//			socklen_t socklen = sizeof(source_addr);
//			len0 = recvfrom(sock0, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
//
//			// Error occurred during receiving
//			if (len0 < 0)
//			{
//				ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
//				break;
//			}
//			// Data received
//			else
//			{
//				rx_buffer[len0] = 0; // Null-terminate whatever we received and treat like a string
//				ESP_LOGI(TAG, "Received %d bytes from %s:", len0, host_ip);
//				ESP_LOGI(TAG, "%s", rx_buffer);
//
//				if (strncmp(rx_buffer, "OK: ", 4) == 0)
//				{
//					ESP_LOGI(TAG, "Received expected message, reconnecting");
//					break;
//				}
//			}
//			vTaskDelay(2000 / portTICK_PERIOD_MS);
//		}
//
//		if (sock0 != -1)
//		{
//			ESP_LOGE(TAG, "Shutting down socket and restarting...");
//			shutdown(sock0, 0);
//			close(sock0);
//		}
//	}
//	vTaskDelete(NULL);
//}
//static void udp_server_task(void *pvParameters)
//{
//	char rx_buffer[128];
//	char addr_str[128];
//	int addr_family = (int)pvParameters;
//	int ip_protocol = 0;
//	struct sockaddr_in6 dest_addr;
//
//	while (1)
//	{
//		if (addr_family == AF_INET)
//		{
//			struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
//			dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
//			dest_addr_ip4->sin_family = AF_INET;
//			dest_addr_ip4->sin_port = htons(PORT_UDP_ORIG);
//			ip_protocol = IPPROTO_IP;
//		}
//		else if (addr_family == AF_INET6)
//		{
//			bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
//			dest_addr.sin6_family = AF_INET6;
//			dest_addr.sin6_port = htons(PORT_UDP_ORIG);
//			ip_protocol = IPPROTO_IPV6;
//		}
//
//		sock0 = socket(addr_family, SOCK_DGRAM, ip_protocol);
//		if (sock0 < 0)
//		{
//			ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
//			break;
//		}
//		ESP_LOGI(TAG, "Socket created");
//
//		int err = bind(sock0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
//		if (err < 0)
//		{
//			ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
//		}
//		ESP_LOGI(TAG, "Socket bound, port %d", PORT_UDP_ORIG);
//
//		while (1)
//		{
//			ESP_LOGI(TAG, "Waiting for data");
//			struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
//			socklen_t socklen = sizeof(source_addr);
//			len0 = recvfrom(sock0, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
//
//			// Error occurred during receiving
//			if (len0 < 0)
//			{
//				ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
//				break;
//			}
//			// Data received
//			else
//			{
//				// Get the sender's ip address as string
//				if (source_addr.ss_family == PF_INET)
//				{
//					inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
//				}
//				else if (source_addr.ss_family == PF_INET6)
//				{
//					inet6_ntoa_r(((struct sockaddr_in6 *)&source_addr)->sin6_addr, addr_str, sizeof(addr_str) - 1);
//				}
//
//				rx_buffer[len0] = 0; // Null-terminate whatever we received and treat like a string...
//				ESP_LOGI(TAG, "Received %d bytes from %s:", len0, addr_str);
//				ESP_LOGI(TAG, "%s", rx_buffer);
//
//				int err = sendto(sock0, rx_buffer, len0, 0, (struct sockaddr *)&source_addr, sizeof(source_addr));
//				if (err < 0)
//				{
//					ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
//					break;
//				}
//			}
//		}
//
//		if(sock0 != -1)
//		{
//			ESP_LOGE(TAG, "Shutting down socket and restarting...");
//			shutdown(sock0, 0);
//			close(sock0);
//		}
//	}
//	vTaskDelete(NULL);
//}

