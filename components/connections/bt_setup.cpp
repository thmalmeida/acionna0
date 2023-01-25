#include "bt_setup.hpp"

const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

// Global Bluetooth handle parameters
uint32_t bt_sock0 = 0;							// sock handle connection
uint8_t bt_data[SPP_BT_DATA_LEN] = {0};			// buffer for data received
uint8_t bt_data_len = 0;						// pkt sock length
uint8_t bt_data_flag = 0;						// flag to advise new buffer
conn_states bt_state = conn_states::disconnected;	// status connection

void bt_init(void) {
	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
	esp_bt_controller_init(&bt_cfg);
	esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
	esp_bluedroid_init();
	esp_bluedroid_enable();
	esp_bt_gap_register_callback(esp_bt_gap_cb);
	esp_spp_register_callback(bt_event_handler);			// Function registration for event handler
	esp_spp_init(esp_spp_mode);

	#if (CONFIG_BT_SSP_ENABLED == true)
	/* Set default parameters for Secure Simple Pairing */
	esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
	esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
	esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
	#endif

	/*
	* Set default parameters for Legacy Pairing
	* Use variable pin, input pin code when pairing
	*/
	// esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
	// esp_bt_pin_code_t pin_code;
	// esp_bt_gap_set_pin(pin_type, 0, pin_code);
}
void bt_event_handler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
	switch (event)
	{
		case ESP_SPP_INIT_EVT: {		// When SPP is inited, the event comes
			ESP_LOGI(TAG_BT, "ESP_SPP_INIT_EVT");
			esp_spp_start_srv(sec_mask,role_slave, 0, SPP_SERVER_NAME);
			break;
		}
		case ESP_SPP_UNINIT_EVT: {
			ESP_LOGI(TAG_BT, "ESP_SPP_UNINIT_EVT");
			break;
		}
		case ESP_SPP_DISCOVERY_COMP_EVT: {
			ESP_LOGI(TAG_BT, "ESP_SPP_DISCOVERY_COMP_EVT");
			break;
		}
		case ESP_SPP_OPEN_EVT: {		// When SPP Client connection open, the event comes
			ESP_LOGI(TAG_BT, "ESP_SPP_OPEN_EVT");
//			esp_spp_write(param->open.handle, SPP_BT_DATA_LEN, bt_data); //thm
			break;
		}
		case ESP_SPP_CLOSE_EVT: {		// when close connection
			ESP_LOGI(TAG_BT, "ESP_SPP_CLOSE_EVT: Disconnected!");
			bt_state = conn_states::disconnected;
			break;
		}
		case ESP_SPP_START_EVT: {		// When SPP server started, the event comes
			ESP_LOGI(TAG_BT, "ESP_SPP_START_EVT");
			esp_bt_dev_set_device_name(DEVICE_NAME);
			esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
			break;
		}
		case ESP_SPP_CL_INIT_EVT: {		// When SPP client initiated a connection, the event comes
			ESP_LOGI(TAG_BT, "ESP_SPP_CL_INIT_EVT: Connected???");
			break;
		}
		case ESP_SPP_DATA_IND_EVT: {	// When SPP connection received data, the event comes, only for ESP_SPP_MODE_CB
			// #if (SPP_SHOW_MODE == SPP_SHOW_DATA)
			ESP_LOGI(TAG_BT, "ESP_SPP_DATA_IND_EVT len=%u handle=%lu", param->data_ind.len, param->data_ind.handle);
			//printf("Data: %c",param->data_ind);

			// Print buffer in hex format
			esp_log_buffer_hex("data:",param->data_ind.data,param->data_ind.len);

			// Print buffer in char text format
			esp_log_buffer_char("Received String Data",param->data_ind.data,param->data_ind.len);

			// Print received data on monitor screen string format
			ESP_LOGI(TAG_BT, "data: %s\n", param->data_ind.data);

			// Connection data length plus \r and \n code
			// bt_data_len = param->data_ind.len + 2 ;
			
			bt_data_len = param->data_ind.len;

			memcpy(bt_data, param->data_ind.data, bt_data_len);
			// memcpy(bt_data, param->data_ind.data, sizeof(param->data_ind.data));
			bt_data_flag = 1;
			// -----------------------------------------------------------------
		    // uint8_t* command_str = new uint8_t[16];
			// memset(command_str, 0, sizeof(*command_str));
			// int command_str_len = 0;
    		
			// acionna0.parser_1(param->data_ind.data, param->data_ind.len, command_str, command_str_len);
		    // ESP_LOGI(TAG_BT, "command_str: %s, lenght: %d", command_str, command_str_len);
		    // std::string msg_back;
		    // msg_back = acionna0.handle_message(command_str);
			// delete[] command_str;

		    // // ESP_LOGI(TAG_WS, "msg_back[%d]: %s", msg_back.length(), msg_back.c_str());
			// // uint8_t* spp_pkt_data = new uint8_t[100];
			// uint8_t* spp_pkt_data;
			// // *spp_pkt_data = &bt_data[0];
    		// spp_pkt_data = reinterpret_cast<uint8_t*>(&msg_back[0]);
			// for(int i=0; i<msg_back.length(); i++)
			// {
			// 	printf("%c", spp_pkt_data[i]);
			// }

		    // bt_sock0_len = msg_back.length();
    		// ESP_LOGI(TAG_BT, "msg_back length: %d", msg_back.length());
			// -----------------------------------------------------------------
			

			// ECHO BACK TEST	
			// // Copy data received to variable data to be send
			// int j;
			// for(j=0;j<param->data_ind.len;j++)
			// {
				// bt_data[j] = param->data_ind.data[j];
			// }

		    // bt_data[param->data_ind.len] = 0x0d; // /r
		    // bt_data[param->data_ind.len+1] = 0x0a; // /n




			// Send echo back
			// esp_spp_write(bt_sock0, bt_data_len, bt_data);	// send data;
			// delete[] spp_pkt_data;
			// #else
			// gettimeofday(&time_new, NULL);
			// data_num += param->data_ind.len;
			// if (time_new.tv_sec - time_old.tv_sec >= 3) {
			// print_speed();
			// }
			// #endif
			break;
		}
		case ESP_SPP_CONG_EVT: {
			ESP_LOGI(TAG_BT, "ESP_SPP_CONG_EVT");
			ESP_LOGI(TAG_BT, "ESP_SPP_CONG_EVT cong=%d", param->cong.cong);
			if (param->cong.cong == 0)
			{
				esp_spp_write(param->cong.handle, SPP_BT_DATA_LEN, bt_data);
			}
			break;
		}
		case ESP_SPP_WRITE_EVT: {		// Write something to socket
//			ESP_LOGI(TAG_BT, "ESP_SPP_WRITE_EVT"); acceptor code line only;

			// initiator code copy
			ESP_LOGI(TAG_BT, "ESP_SPP_WRITE_EVT len=%d cong=%d handle=%lu", param->write.len , param->write.cong, param->write.handle);
			esp_log_buffer_hex("write:",bt_data,SPP_BT_DATA_LEN);
//			if (param->write.cong == 0)
//			{
//				esp_spp_write(param->write.handle, SPP_BT_DATA_LEN, bt_data);
//			}
			break;
		}
		case ESP_SPP_SRV_OPEN_EVT: {	// Connection stablished
			ESP_LOGI(TAG_BT, "ESP_SPP_SRV_OPEN_EVT: Connected!");

			// Connection flags;
			bt_state = conn_states::connected;
			bt_sock0 = param->data_ind.handle;

			if (param->cong.cong == 0)
			{
				esp_spp_write(param->cong.handle, SPP_BT_DATA_LEN, bt_data);
			}

//			if (param->write.cong == 0)
//			{
//				ESP_LOGI(TAG_BT, "param->write.cong == 0");
//				esp_spp_write(param->write.handle, SPP_BT_DATA_LEN, bt_data);
//			}
			// gettimeofday(&time_old, NULL);
			break;
		}
		case ESP_SPP_SRV_STOP_EVT: {
			ESP_LOGI(TAG_BT, "ESP_SPP_SRV_STOP_EVT");
			break;
		}
		default:
		break;
	}
}
void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
   switch (event) {
   case ESP_BT_GAP_AUTH_CMPL_EVT:{
       if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
           ESP_LOGI(TAG_BT, "authentication success: %s", param->auth_cmpl.device_name);
           esp_log_buffer_hex(TAG_BT, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
       } else {
           ESP_LOGE(TAG_BT, "authentication failed, status:%d", param->auth_cmpl.stat);
       }
       break;
   }
   case ESP_BT_GAP_PIN_REQ_EVT:{
       ESP_LOGI(TAG_BT, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
       if (param->pin_req.min_16_digit) {
           ESP_LOGI(TAG_BT, "Input pin code: 0000 0000 0000 0000");
           esp_bt_pin_code_t pin_code = {0};
           esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
       } else {
           ESP_LOGI(TAG_BT, "Input pin code: 1234");
           esp_bt_pin_code_t pin_code;
           pin_code[0] = '1';
           pin_code[1] = '2';
           pin_code[2] = '3';
           pin_code[3] = '4';
           esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
       }
       break;
   }

#if (CONFIG_BT_SSP_ENABLED == true)
   case ESP_BT_GAP_CFM_REQ_EVT:
       ESP_LOGI(TAG_BT, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %lu", param->cfm_req.num_val);
       esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
       break;
   case ESP_BT_GAP_KEY_NOTIF_EVT:
       ESP_LOGI(TAG_BT, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%lu", param->key_notif.passkey);
       break;
   case ESP_BT_GAP_KEY_REQ_EVT:
       ESP_LOGI(TAG_BT, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
       break;
#endif

   case ESP_BT_GAP_MODE_CHG_EVT:
       ESP_LOGI(TAG_BT, "ESP_BT_GAP_MODE_CHG_EVT mode:%d", param->mode_chg.mode);
       break;

   default: {
       ESP_LOGI(TAG_BT, "event: %d", event);
       break;
   }
   }
   return;
}
void bt_send(std::string msg) {
	if(bt_state == conn_states::connected)
	{
		// bt_data = reinterpret_cast<uint8_t*>(&msg[0]);
		esp_spp_write(bt_sock0, msg.length(), reinterpret_cast<uint8_t*>(&msg[0]));	// send data;
	}

	// uint8_t* spp_pkt_data;
	// // *spp_pkt_data = &spp_data[0];
	// spp_pkt_data = reinterpret_cast<uint8_t*>(&buffer[0]);
	// // for(int i=0; i<strlen(buffer); i++)
	// // {
	// //     printf("%c", spp_pkt_data[i]);
	// // }
	// bt_sock0_len = strlen(buffer);
	// esp_spp_write(bt_sock0, bt_sock0_len, spp_pkt_data);	// send data;
}
