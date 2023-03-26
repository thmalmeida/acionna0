#include "httpd_setup.hpp"

static const char *TAG_WS = "HTTPD";

// websocket ws_server handle parameters
async_resp_arg ws_server_sock0;									// pointer to connection socket
uint8_t ws_server_data[BUFFER_LEN];							// buffer received
uint8_t ws_server_data_len;										// buffer length received
uint8_t ws_server_data_flag = 0;								// flag to advise new buffer
conn_states ws_server_client_state = conn_states::disconnected;	// status connection

const httpd_uri_t ws = {
	"/ws",					// uri
	HTTP_GET,				// method
	ws_server_event_handler,// handler: esp_err_t (*handler)(httpd_req_t *r)
	NULL,					// user_ctx
	true,					// is_websocket
	true,					// handle_ws_control_frames
	NULL					// supported_subprotocol
};
httpd_handle_t ws_server = NULL;   // Server instance global declaration
httpd_config_t ws_server_config = {
	tskIDLE_PRIORITY+5,     // task_priority
	4096,                   // stack_size
	tskNO_AFFINITY,			// core_id
	CONFIG_WS_SERVER_PORT,	// server_port
	32768,                  // ctrl_port
	4,                      // max_open_sockets
	8,                      // max_uri_handlers
	8,                      // max_resp_headers
	5,                      //backlog_conn
	true,                 	// lru_purge_enable
	5,                      // recv_wait_timeout
	5,                      // send_wait_timeout
	NULL,                   // global_user_ctx
	NULL,                   // global_user_ctx_free_fn
	NULL,                   // global_transport_ctx
	NULL,                   // global_transport_ctx_free_fn
	true,					// enable/disable linger
	5,						// linger timeout in seconds
	NULL,                   // open_fn
	NULL,                   // close_fn
	NULL                    // uri_match_fn
};

esp_err_t ws_server_event_handler(httpd_req_t *req) {
	if (req->method == HTTP_GET) {
		ESP_LOGI(TAG_WS, "Handshake done, the new connection was opened");
		ESP_LOGI(TAG_WS, "req->method %u", req->method);

		ws_server_sock0.fd = httpd_req_to_sockfd(req);				// connection socket pointer
		ws_server_sock0.hd = req->handle;

		ws_server_client_state = conn_states::connected;

		return ESP_OK;
	}

	ESP_LOGI(TAG_WS, "req->method %u", req->method);
	httpd_ws_frame_t ws_pkt;								// create websocket packet
	uint8_t *buf = NULL;
	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));			// clear the buffer
	// ws_pkt.type = HTTPD_WS_TYPE_TEXT;
	/* Set max_len = 0 to get the frame len */
	esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);	// receive ws frame copy fill the packet
	if (ret != ESP_OK) {
		ESP_LOGE(TAG_WS, "httpd_ws_recv_frame failed to get frame len with %d", ret);
		return ret;
	}
	ESP_LOGI(TAG_WS, "Packet type: %d", ws_pkt.type);		// debug purpose
	ESP_LOGI(TAG_WS, "Packet data length: %d", ws_pkt.len);

	if (ws_pkt.len) {
		/* Alocate and clear buffer. ws_pkt.len + 1 is for NULL termination as we are expecting a string */
		buf = (uint8_t*)calloc(1, ws_pkt.len + 1);  // alocate memory for an array and initialize with zeros.
		if (buf == NULL)                            // Test if it is okay.
		{
			ESP_LOGE(TAG_WS, "Failed to calloc memory for buf");
			return ESP_ERR_NO_MEM;
		}
		ws_pkt.payload = buf;                       // Clear payload buffer

		/* Set max_len = ws_pkt.len to get the frame payload */
		// ESP_LOGI(TAG_WS, "Got packet with message: %s", ws_pkt.payload);
		ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
		if (ret != ESP_OK) {
			ESP_LOGE(TAG_WS, "httpd_ws_recv_frame failed with %d", ret);
			free(buf);
			return ret;
		}
		ESP_LOGI(TAG_WS, "Got packet with message: %s", ws_pkt.payload);

		memcpy(ws_server_data, ws_pkt.payload, ws_pkt.len);
		ws_server_data_len = ws_pkt.len;
		ws_server_data_flag = 1;
	}
	else {
		if((req->method == 0) && (ws_pkt.type == HTTPD_WS_TYPE_CLOSE))
		{
			ws_server_client_state = conn_states::disconnected;
		}
	}

	if(httpd_ws_get_fd_info(ws_server_sock0.hd, ws_server_sock0.fd) != HTTPD_WS_CLIENT_WEBSOCKET) {
		ESP_LOGI(TAG_WS, "Disconnected by get_fd_info()\n");
		ws_server_client_state = conn_states::disconnected;
	}

//	// Just for trigger for a specific message.
	// if (ws_pkt.type == HTTPD_WS_TYPE_TEXT && strcmp((char*)ws_pkt.payload,"Trigger async") == 0) {
	// 	free(buf);
	// 	ESP_LOGI(TAG_WS, "Trigger async if");
	// 	return trigger_async_send(req->handle, req);
	// }

	// for a copy
	// std::vector<uint8_t> myVector(myString.begin(), myString.end());
	// uint8_t *p = &myVector[0];
	// std::stringstream ss;
	// ss << "Hello, world, " << acionna0.pipe1_.pressure_mca << '\n';
	// std::string myString = ss.str();

	// ws_pkt.payload = reinterpret_cast<uint8_t*>(&myString[0]);
	// ws_pkt.len = myString.size();

	// uint8_t* command_str = new uint8_t[16];
	// int command_str_len = 0;
	// acionna0.parser_1(ws_pkt.payload, ws_pkt.len, command_str, command_str_len);

	// ESP_LOGI(TAG_WS, "%s", command_str);
	// ws_pkt.payload = command_str;
	// ws_pkt.len = command_str_len;

	// char* msg_back = NULL; //new char[40];
	// int msg_back_len = 0;
	// std::string msg_back;
	// msg_back = acionna0.handle_message(command_str);

	// ESP_LOGI(TAG_WS, "msg_back[%d]: %s", msg_back.length(), msg_back.c_str());

	// ws_pkt.payload = reinterpret_cast<uint8_t*>(&msg_back[0]);
	// ws_pkt.len = msg_back.length();
	// ESP_LOGI(TAG_WS, "msg_back length: %d", msg_back.length());

	// if(acionna0.signal_restart)
	// {
	// 	ESP_LOGI(TAG_WS, "ws_type_close");
	// 	ret = httpd_ws_send_frame(req, &ws_pkt);

	// 	ws_pkt.type = HTTPD_WS_TYPE_CLOSE;
	// }

	// ret = httpd_ws_send_frame(req, &ws_pkt);
	// if (ret != ESP_OK) {
	// 	ESP_LOGE(TAG_WS, "httpd_ws_send_frame failed with %d", ret);
	// }

	// delete[] command_str;
	// free(buf);
	return ret;
}
void httpd_server_start(void) {
	// Needs implementation for wss connection
	// Prepare keep-alive engine
	// wss_keep_alive_config_t keep_alive_config = KEEP_ALIVE_CONFIG_DEFAULT();

	// httpd_ssl_config

	// httpd_server_start(arg);
	// httpd_handle_t* ws_server = (httpd_handle_t*) arg;
	// httpd_handle_t ws_server = NULL; // moved to global
	ESP_LOGI(TAG_WS, "ws_server starting...");
	if (ws_server == NULL)
	{
		// *ws_server = start_webserver();
		// Start the httpd ws_server

		ESP_LOGI(TAG_WS, "using port: %u", ws_server_config.server_port);
		if (httpd_start(&ws_server, &ws_server_config) == ESP_OK) // httpd_start(&ws_server, &config)
		{
			// Registering the ws handler
			ESP_LOGI(TAG_WS, "registering URI handlers");
			httpd_register_uri_handler(ws_server, &ws);
			// return ws_server;
		}
		else
		{
			ESP_LOGI(TAG_WS, "error starting ws_server!");
			ws_server = NULL;
		}
	}
	ESP_LOGI(TAG_WS, "leaving httpd_server_start");
}
void httpd_server_stop(void) {
	if(ws_server != NULL) {
		httpd_stop(ws_server);
		ws_server = NULL;
	}
}
void ws_server_send(std::string data) {
	// build a websocket frame
	httpd_ws_frame_t ws_pkt;
	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
	// ws_pkt.payload = (uint8_t*)data;
	ws_pkt.payload = reinterpret_cast<uint8_t*>(&data[0]);
	// ws_pkt.len = strlen(data);
	ws_pkt.len = data.length();
	ws_pkt.type = HTTPD_WS_TYPE_TEXT;

	httpd_ws_send_frame_async(ws_server_sock0.hd, ws_server_sock0.fd, &ws_pkt);
}
