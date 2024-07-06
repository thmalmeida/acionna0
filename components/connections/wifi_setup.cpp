/*
* wifi_stuffs.h
*
*  Created on: 2 de nov. de 2021
*      Author: thmalmeida
*/
#include "wifi_setup.hpp"

ip_get_types ip_get_mode = ip_get_types::dhcp;
ip_states ip_state = ip_states::ip_not_defined;
conn_states wifi_state = conn_states::disconnected;

/* FreeRTOS event group to signal when we are connected*/
// static EventGroupHandle_t s_wifi_event_group;

const char *TAG_WIFI = "wifi setup";
const char *TAG_IP = "IP stuffs";

static int s_retry_num = 0;
uint8_t wifi_ip_end = 29;

// gpio 2, freq 1 Hz, duty 99%, inv 1, channel 0, timer 0, group 0 (low);
pwm_ledc led_wifi(2, 1, 0, 1, 0, 0, 0);

void wifi_sta_init(uint8_t ip_end) {
	// s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// Select ip station mode DHCP or STATIC
	switch (ip_get_mode)
	{
		case ip_get_types::dhcp:
		{
			esp_netif_create_default_wifi_sta();
			ESP_LOGI(TAG_WIFI, "dhcp mode");
			break;
		}
		case ip_get_types::static_ip:
		{
			esp_netif_t *my_sta = esp_netif_create_default_wifi_sta();
			// assert(my_sta);              // is that for debug?

			esp_netif_dhcpc_stop(my_sta);
			esp_netif_ip_info_t ip_info;
			// esp_netif_set_ip4_addr();
			IP4_ADDR(&ip_info.ip, 192, 168, 1, ip_end);
			IP4_ADDR(&ip_info.gw, 192, 168, 1, 1);
			IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
			esp_netif_set_ip_info(my_sta, &ip_info);
			ESP_LOGI(TAG_WIFI, "static ip mode");
			break;
		}
		default:
			esp_netif_create_default_wifi_sta();
			break;
	}

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_connection_event_handler, NULL, &instance_any_id));
	// ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifi_connection_event_handler, NULL, &instance_got_ip));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_connection_event_handler, NULL, &instance_got_ip));

	wifi_config_t wifi_config = {};
	// memset(&wifi_config, 0, sizeof(wifi_config));	// Clear all (RESET)
	// sprintf (reinterpret_cast<char*>(wifi_config.sta.ssid), WIFI_SSID_STA );
	// sprintf (reinterpret_cast<char*>(wifi_config.sta.password), WIFI_PASS);
	// memset(wifi_config.sta.ssid, 0, sizeof(wifi_config.sta.ssid));
	// memset(wifi_config.sta.password, 0, sizeof(wifi_config.sta.password));
	// strcpy(wifi_config.sta.ssid, reinterpret_cast<uint8_t*>WIFI_SSID_STA);
	// strcpy(wifi_config.sta.password, reinterpret_cast<uint8_t*>WIFI_PASS);
	// strcpy(wifi_config.ap.ssid, (uint8_t const*)(EXAMPLE_ESP_WIFI_SSID));
	// strcpy(wifi_config.ap.password, (uint8_t const*)(EXAMPLE_ESP_WIFI_PASS));

	ESP_LOGI(TAG_WIFI, "SSID: %s, pwd: %s,", wifi_config.sta.ssid, wifi_config.sta.password);
	memcpy(&wifi_config.sta.ssid[0], WIFI_SSID_STA, strlen(WIFI_SSID_STA));
	memcpy(&wifi_config.sta.password[0], WIFI_PASS, strlen(WIFI_PASS));
	wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK;
	wifi_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
	memcpy(wifi_config.sta.sae_h2e_identifier, H2E_IDENTIFIER, strlen(H2E_IDENTIFIER));

// Routine test to verify strings
	// int a=strlen(WIFI_SSID_STA);
	// printf("SSID: ");
	// for(int i=0; i<a; i++)
	// {
	// 	printf("%c", wifi_config.sta.ssid[i]);
	// }
	// printf("\n");
	// a=strlen(WIFI_PASS);
	// printf("PASSWD: ");
	// for(int i=0; i<a; i++)
	// {
	// 	printf("%c", wifi_config.sta.password[i]);
	// }
	// printf("\n");

	// ESP_LOGI(TAG_WIFI, "SSID: %s, pwd: %s,", wifi_config.sta.ssid, wifi_config.sta.password);
	// wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA_WPA2_PSK; 	// or ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD
	// wifi_config.sta.sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER;	
	// wifi_config.sta.pmf_cfg.capable = true;
	// wifi_config.sta.pmf_cfg.required = false;

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );
	// ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

	// disable any power save mode;
	#ifndef CONFIG_BT_ENABLE
    // esp_wifi_set_ps(WIFI_PS_NONE);
	#endif

	ESP_LOGI(TAG_WIFI, "wifi_sta_init finished!");

	// WiFi AP setup
	// memcpy(wifi_config.ap.ssid, WIFI_SSID_AP, strlen(WIFI_SSID_AP));
	// wifi_config.ap.ssid_len = (uint8_t)strlen(WIFI_SSID_AP);
	// wifi_config.ap.channel = 6;
	// memcpy(wifi_config.ap.password, WIFI_PASS, strlen(WIFI_PASS));
	// wifi_config.ap.max_connection = 4;
	// wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;
	// ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP) );
	// ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config) );


/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
* number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
// EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,	WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,	pdFALSE, pdFALSE, portMAX_DELAY);

// /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
// * happened. */
// if (bits & WIFI_CONNECTED_BIT)
// {
// 	ESP_LOGI(TAG_WIFI, "connected to ap SSID:%s password:%s", WIFI_SSID_STA, WIFI_PASS);
// }
// else if (bits & WIFI_FAIL_BIT)
// {
// 	ESP_LOGI(TAG_WIFI, "Failed to connect to SSID:%s, password:%s", WIFI_SSID_STA, WIFI_PASS);
// }
// else
// {
// 	ESP_LOGE(TAG_WIFI, "UNEXPECTED EVENT");
// }

// To delete connection?
/* The event will not be processed after unregister */
// ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
// ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
// vEventGroupDelete(s_wifi_event_group);
}
void wifi_sta_stop(void)
{
	if(ws_server)
	{
		ESP_LOGI(TAG_WIFI, "Stopping httpd ws server");
		httpd_server_stop();
	}

	if(wifi_state == conn_states::connected)
	{
		ESP_LOGI(TAG_WIFI, "Disconnecting from AP");
		esp_wifi_disconnect();
	}
	
	ESP_LOGI(TAG_WIFI, "Stopping WiFi block");
	esp_err_t err = esp_wifi_stop();
	if(err != ESP_OK)
	{
		ESP_LOGI(TAG_WIFI, "WiFi stop error");
		return;
	}
}
void wifi_get_info(void)
{
	wifi_ap_record_t ap;
	esp_wifi_sta_get_ap_info(&ap);
	printf("%d\n", ap.rssi);
}
void wifi_get_mac(uint8_t* wifi_mac) {
	esp_read_mac(wifi_mac, ESP_MAC_WIFI_STA);
}
void print_auth_mode(int authmode) {
	switch (authmode) {
		case WIFI_AUTH_OPEN:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_OPEN");
			break;
		
		case WIFI_AUTH_WEP:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WEP");
			break;
		
		case WIFI_AUTH_WPA_PSK:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA_PSK");
			break;
		
		case WIFI_AUTH_WPA2_PSK:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA2_PSK");
			break;
		
		case WIFI_AUTH_WPA_WPA2_PSK:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
			break;
		
		case WIFI_AUTH_WPA2_ENTERPRISE:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA2_ENTERPRISE");
			break;

		case WIFI_AUTH_WPA3_PSK:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA3_PSK");
			break;

		case WIFI_AUTH_WPA2_WPA3_PSK:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
			break;
		
		default:
			ESP_LOGI(TAG_WIFI, "Authmode \tWIFI_AUTH_UNKNOWN");
			break;
	}
}
void print_cipher_type(int pairwise_cipher, int group_cipher) {
	switch (pairwise_cipher) {
		case WIFI_CIPHER_TYPE_NONE:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_NONE");
		break;

		case WIFI_CIPHER_TYPE_WEP40:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP40");
		break;
		
		case WIFI_CIPHER_TYPE_WEP104:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_WEP104");
		break;
		
		case WIFI_CIPHER_TYPE_TKIP:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP");
		break;
		
		case WIFI_CIPHER_TYPE_CCMP:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_CCMP");
		break;

		case WIFI_CIPHER_TYPE_TKIP_CCMP:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
		break;

		default:
			ESP_LOGI(TAG_WIFI, "Pairwise Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
		break;
	}

	switch (group_cipher) {
		case WIFI_CIPHER_TYPE_NONE:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_NONE");
		break;
		case WIFI_CIPHER_TYPE_WEP40:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_WEP40");
		break;
		case WIFI_CIPHER_TYPE_WEP104:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_WEP104");
		break;
		case WIFI_CIPHER_TYPE_TKIP:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP");
		break;
		case WIFI_CIPHER_TYPE_CCMP:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_CCMP");
		break;
		case WIFI_CIPHER_TYPE_TKIP_CCMP:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_TKIP_CCMP");
		break;
		default:
		ESP_LOGI(TAG_WIFI, "Group Cipher \tWIFI_CIPHER_TYPE_UNKNOWN");
		break;
	}
}
void wifi_scan(char* str) {
	uint16_t number = DEFAULT_SCAN_LIST_SIZE;
	wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
	uint16_t ap_count = 0;
	memset(ap_info, 0, sizeof(ap_info));

	esp_wifi_scan_start(NULL, true);
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
	ESP_LOGI(TAG_WIFI, "Total APs scanned = %u", ap_count);

	char buffer_temp[45];

	sprintf(str, "SSID\tCH\tRSSI\n");
	for (int i = 0; i<number; i++) {
		ESP_LOGI(TAG_WIFI, "SSID \t\t%s", ap_info[i].ssid);
		ESP_LOGI(TAG_WIFI, "RSSI \t\t%d", ap_info[i].rssi);
		ESP_LOGI(TAG_WIFI, "Channel \t\t%d\n", ap_info[i].primary);

		sprintf(buffer_temp, "%s\t%d\t%d\n", ap_info[i].ssid, ap_info[i].primary, ap_info[i].rssi);
		strcat(str, buffer_temp);
		// print_auth_mode(ap_info[i].authmode);
		// if (ap_info[i].authmode != WIFI_AUTH_WEP) { 
		// 	print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
		// }
	}
}
// ISR Handler
void wifi_connection_event_handler(void* handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	if (event_base == WIFI_EVENT) {
		if (event_id == WIFI_EVENT_WIFI_READY)
		{
			ESP_LOGI(TAG_WIFI,"WiFi Ready!");
		}
		else if(event_id == WIFI_EVENT_SCAN_DONE)
		{
			ESP_LOGI(TAG_WIFI, "ESP32 finish scanning AP");
		}
		else if(event_id == WIFI_EVENT_STA_START)
		{
			esp_wifi_connect();
			ESP_LOGI(TAG_WIFI, "ESP32 sta start event");
		}
		else if(event_id == WIFI_EVENT_STA_CONNECTED)
		{
			wifi_state = conn_states::connected;
			ESP_LOGI(TAG_WIFI, "connected to ap SSID:%s password:%s", WIFI_SSID_STA, WIFI_PASS);
			ESP_LOGI(TAG_WIFI, "ESP32 station connected to AP");
			led_wifi.set_duty(3);
			// httpd web socket start
		}
		else if(event_id == WIFI_EVENT_STA_DISCONNECTED)
		{
			// if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
			// {
			wifi_state = conn_states::disconnected;
			led_wifi.set_duty(50);
			ESP_LOGI(TAG_WIFI, "ESP32 station disconnected to AP");

			// if((s_retry_num%10) == 0)
			// {
			// 	ESP_LOGI(TAG_WIFI, "WiFi scan start\n");
			// 	wifi_scan();
			// }

			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG_WIFI, "connect retry: %d", s_retry_num);
			// }else
			// {
			// 	s_retry_num = 0;
			// 	xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
			// }
			// ESP_LOGI(TAG_WIFI,"connect to the AP fail");
		}
		else if (event_id == WIFI_EVENT_STA_STOP)
		{
			ESP_LOGI(TAG_WIFI, "ESP32 station stop");
		}
	}
	else if (event_base == IP_EVENT) {
		if (event_id == IP_EVENT_STA_GOT_IP)
		{
			ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
			ESP_LOGI(TAG_IP, "got ip0:" IPSTR, IP2STR(&event->ip_info.ip));
			s_retry_num = 0;
			// xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);

			ip_state = ip_states::ip_defined;

			// Start insecure http server
			httpd_server_start();

			// Start secure http server SSL;
			// httpd_ssl_server_start();
			
		}
		else if(event_id == IP_EVENT_STA_LOST_IP)
		{
			ESP_LOGI(TAG_IP,"lost ip!!!!");
			ip_state = ip_states::ip_not_defined;

			httpd_server_stop();
		}
	}
}
