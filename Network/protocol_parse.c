#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "network.h"
#include "common.h"
#include "protocol_parse.h"
#include "xml_parse.h"
#include "get_file_name.h"
#include "mytime.h"
#include "rs485.h"
struct sockaddr_in message_server_addr; 

int message_sockfd = -1; 
extern volatile unsigned char message_isLogin;
int xml_length_type_100[128]={0};
int xml_length_type_301[128]={0};
int xml_length_type_100_size = 0;
int xml_length_type_301_size = 0;
char protocl_file_name[128] = "";
char strategy_file_name[128];
unsigned char protocol_version = 0;

int message_len = 0;
int regist_len = 0;
volatile int control_type = 0;
int volatile is_recieve_regist_packet = 0;
int volatile is_recieve_unregist_packet = 0;
int volatile is_recieve_message_packet = 0;
int volatile is_recieve_exception_packet = 0;


boolean xml_config(char* _protocl_file_name)
{
	if(!find_new_file(PROTOCOL_FILE_DIR,_protocl_file_name))
		return FALSE;
	printf("protocol_file_name:%s\n",_protocl_file_name);
	if(-1 == (xml_length_type_100_size = xml_parse(_protocl_file_name, 100, xml_length_type_100)))
		return FALSE;
	if(-1 == (xml_length_type_301_size = xml_parse(_protocl_file_name, 301, xml_length_type_301)))
		return FALSE;
	return TRUE;
}
void* message_init(void* arg)
{
	while(1)
	{
		if(message_isConnected == 0)
		{
			message_isLogin = 0;
			if(message_sockfd > 0)
			{
				close(message_sockfd);
				message_sockfd = -1;
			}
			message_isLogin = 0;
			
			boolean ret_value = FALSE;
			if((message_sockfd =  network_init(&message_server_addr, MESSAGE_SERVER_IP, MESSAGE_SERVER_PORT)) != -1)
				printf("network_init create socket successful.\n");
			else return FALSE;

			network_pthread_arg arg;
			arg.sock_fd = &message_sockfd;
			message_isConnected = 0;
			arg.is_connect = &message_isConnected;
			arg.server = &message_server_addr;	
			arg.ret_value = &ret_value;
			
			networt_connect((void *)(&arg));
			if(TRUE == ret_value)
			{
				printf("networt_connect success\n");
				if(!send_unregist_packet())
				{
					message_isConnected = 0;
					printf("============send_unregist_packet fail================\n");
					continue;
				}
				if(!send_regist_packet())
				{
					message_isConnected = 0;
					printf("============send_regist_packet fail================\n");
				}
				//message_isConnected = 1;
				//sleep(100);
			}
			else
				printf("networt_connect fail\n");
		}
	}
}

int get_message_len(unsigned char* info)
{
	return (info[4]<<24)+(info[5]<<16) \
	    	+(info[6]<<8)+info[7];
}

int get_message_type(unsigned char* info)
{
	return (info[8]<<8)+info[9];
}

boolean parse_exception_packet(unsigned char* info)
{

}

int parse_control_packet(unsigned char* info)
{
	int info_leek = 14, i =0, j = 0;
	unsigned char t_info[16],t_lightBox_info[66];
	memset(t_info,'\0',16);
	char t_split_info[10][64];
	int packet_len = parse_message_head(info);
	
	if(-1 == packet_len)
		return FALSE;
	
	for(i=0; i<(packet_len-MESSAGE_HEAD_LEN-2)/64; i++)
	{
		memset(t_lightBox_info,'\0',66);
		memset(t_split_info,'\0',sizeof(t_split_info));
		
		myUint8cpy(t_lightBox_info,info_leek+info,0,64);
		info_leek += 64;
		int split_len = mySplit(t_split_info, t_lightBox_info, ',' , 64);
		printf("t_lightBox_info:%s,lightBox number:%d\n",t_lightBox_info,t_lightBox_info[0]);
		if(t_lightBox_info[0] == 0)
			return i;
		control_type = atoi(t_split_info[1]);
		if(control_type == 0)
		{
		//perform_automatic_strategy_pid == -1说明此时没有perform_automatic_strategy线程，才去开启线程
			if(perform_automatic_strategy_pid == -1)
			{
				if(pthread_create(&perform_automatic_strategy_pid, NULL, perform_automatic_strategy, NULL))
				{
					printf("create pthread error .... \n");
					perform_automatic_strategy_pid = -1;
		//			return FALSE;
				}	
			}
			continue;
		//	return TRUE;
		}
		else if(control_type == 1)
		{
		//perform_automatic_strategy_pid != -1说明此时有线程，才需要去关闭它
			if(perform_automatic_strategy_pid != -1)
			{
				pthread_cancel(perform_automatic_strategy_pid);
   				pthread_join(perform_automatic_strategy_pid, NULL); //wait the thread stopped
				if(0 == get_rs485_sem_val())
				{
					if(!rs485_semaphore_v())
					{
						printf("rs485_semaphore_v fail\n");		
					}
				}
			}
			perform_automatic_strategy_pid = -1;	
		}
		int t_cmd = 0;
		cmd_packet return_packet;
		if(atoi(t_split_info[2]) == 0)
			t_cmd = 0x01;
		else if(atoi(t_split_info[2]) == 1)
			t_cmd = 0x02;
		else
		{
			 continue;
		}	
		if(!send_cmd(atoi(t_split_info[0]), t_cmd, (void*)(&return_packet)))
		{
			printf("shit you !!!!!!!! no work!!!!!!!!!\n");
		}	
	}
	
}

int parse_message_head(unsigned char* info)
{ 
	if(info[0]!= SXT_0 || info[1]!=SXT_1 )//|| info[2]!=APP_ID)
		return -1;
	if(info[3]!= protocol_version)
	{
		char dir_name_info[128][128] = {"protocol"}, file_name[128], t_cmd[128];
		memset(file_name, '\0', 128);
		memset(t_cmd, '\0', 128);
		sprintf(file_name,"led_v%d.xml",info[3]);
//		if(mySystem("mv /app/protocol/led_v1.xml /app/protocol/led_tmp"))
//		{
		if(get_file_from_server(dir_name_info, file_name))
		{
			sprintf(t_cmd,"mv /app/%s /app/protocol/%s", file_name,file_name);
			mySystem(t_cmd);
			sleep(3);
	
			memset(t_cmd, '\0', 128);
			sprintf(t_cmd,"rm /app/protocol/led_v%d.xml",protocol_version);
			mySystem(t_cmd);
			sleep(3);
			
			protocol_version = info[3];
			
			printf("update  led_v%d.xml  success\n", info[3]);
			
			if(!xml_config(file_name))
		  	{
		  		printf("construct_packet_head xml_config fail.\n");
		  	}
		}
		else
		{
//			mySystem("mv /app/protocol/led_tmp /app/protocol/led_v1.xml");
			printf("update  led_v%d.xml fail\n",info[3]);
		}
//		}
	}
	return get_message_len(info);
	//=========================================================
	//=========================================================
	//=========================================================
	//message id must be parsed
	//=========================================================
	//=========================================================
	//=========================================================
}

boolean parse_return_code(int type)
{
//	
//#define SUCESS 200;//			----成功
//#define SERVER_EXCEPTION 301;//		----服务器异常
//#define EMPTY_MSG_HANDLER 302;//	----无消息处理器
//#define NOT_ENOUGH_BYTES 401;//		----字节数不够
//#define ETX_ERROR 402;//		----ETX错误
//#define LRC_ERROR 403;//		----LRC错误
//#define CREATE_MSG_ERROR 501;//		----创建消息错误
//#define CREATE_MSG_FACTORY_ERROR 502;//	----创建消息工厂错误
//#define EMPTY_MSG_RETURN 503;//		----空消息返回
	printf("parse_return_code:%d", type);
	switch(type)
	{
		case SUCESS:
			printf("SUCESS \n");
			return TRUE;
			
		case SERVER_EXCEPTION:
			printf("SERVER_EXCEPTION \n");
			return FALSE;
			
		case EMPTY_MSG_HANDLER:
			printf("EMPTY_MSG_HANDLER \n");
			return FALSE;
		
		case NOT_ENOUGH_BYTES:
			printf("NOT_ENOUGH_BYTES \n");
			return FALSE;
		
		case ETX_ERROR:
			printf("ETX_ERROR \n");
			return FALSE;
		
		case LRC_ERROR:
			printf("LRC_ERROR \n");
			return FALSE;
		
		case CREATE_MSG_ERROR:
			printf("CREATE_MSG_ERROR \n");
			return FALSE;
		
		case CREATE_MSG_FACTORY_ERROR:
			printf("CREATE_MSG_ERROR \n");
			return FALSE;
		
		case EMPTY_MSG_RETURN:
			printf("EMPTY_MSG_RETURN \n");
			return FALSE;
		
		case _2_HAS_BEEN_REGISTERED :
			printf("_2_HAS_BEEN_REGISTERED \n");
			return FALSE;
		case _2_NO_REGISTER :
			printf("_2_NO_REGISTER \n");
			return TRUE;
		default:
			printf("parse_return_code default \n");
			return FALSE;
		
	}
}
boolean parse_heartbeat_packet(unsigned char * info)
{
	char t_info[10], t_time[20], t_version[16], current_version[16], t_strategy[32];
	char dir_name_info[128][128] = {"getway"};//, app_name[128] = "ledPro";
	struct tm t_temp;
	char t_cmd[128];
	int info_leek = 0;
	int packet_len = parse_message_head(info);
	if(-1 == packet_len)
		return FALSE;
	info_leek += MESSAGE_HEAD_LEN;
	if(-1 == packet_len)
		return FALSE;
	myUint8cpy(t_info, info+info_leek, 0, 8);
	if(parse_return_code(atoi(t_info)))
	{
		info_leek += 8;
		myUint8cpy(t_time, info+info_leek, 0, 20);
		if(!calibrateTime(t_time))
			printf("calibrateTime fail\n");
//#define STRATEGY_FILE_DIR "/app/strategy/"

//#define PROTOCOL_FILE_DIR "/app/protocol/"
		info_leek += 20;
		myUint8cpy(t_version, info+info_leek, 0, 16);
	//	strcat(app_name, t_version);
	//	if(!find_new_file(PROTOCOL_FILE_DIR,current_version))
	//		printf("find_new_file fail\n");
		char t_app_name[128];
		memset(t_app_name,'\0',128);
		strcat(t_app_name,"ledPro");
		strcat(t_app_name,t_version);
		if(strcmp(t_version,GETWAY_VERSION))
		{
			if(get_file_from_server(dir_name_info, t_app_name))
			{
				printf("update getway version %s success\n", t_version);
//				memset(t_cmd, '\0', 128);
//				sprintf(t_cmd,"rm -r /app/getway/ledPro");
//				mySystem(t_cmd);
//				sleep(2);

				memset(t_cmd, '\0', 128);
				sprintf(t_cmd,"mv -f /app/%s /app/update_program",t_app_name);
				mySystem(t_cmd);
				
				sleep(5);
				reboot();
					//sleep(1);
			}
			else
				printf("update getway version %s fail\n", t_version);
		}
		info_leek += 16;
		myUint8cpy(t_strategy, info+info_leek, 0, 32);
//		if(!find_new_file(STRATEGY_FILE_DIR,current_strategy))
//			printf("find_new_file fail\n");
		if(strcmp(t_strategy,strategy_file_name))
		{
			memset(dir_name_info[0], '\0', 128);
			strcpy(dir_name_info[0],"strategy");
			if(get_file_from_server(dir_name_info, t_strategy))
			{
				printf("update t_strategy version %s success\n", t_strategy);
				sprintf(t_cmd,"mv /app/%s /app/strategy/", t_strategy);
				mySystem(t_cmd);
				sleep(3);
					//sleep(1);
				memset(t_cmd, '\0', 128);
				sprintf(t_cmd,"rm /app/strategy/%s",strategy_file_name);
				mySystem(t_cmd);
				
				sleep(3);
				memset(strategy_file_name, '\0', 128);
				strcpy(strategy_file_name,t_strategy);
				timetable_index = 0;
				if(!strategy_parse(strategy_file_name))
				{
					printf("parse_heartbeat_packet strategy_parse fail\n");
				}
				if(control_type == 0)
				{
					if(perform_automatic_strategy_pid != -1)
					{
						pthread_cancel(perform_automatic_strategy_pid);
		   				pthread_join(perform_automatic_strategy_pid, NULL); //wait the thread stopped
					}
					perform_automatic_strategy_pid = -1;
				}
				int t_control_type = control_type;
				control_type = 0;	
				if(pthread_create(&perform_automatic_strategy_pid, NULL, perform_automatic_strategy, NULL))
				{
					printf("create pthread error .... \n");
					perform_automatic_strategy_pid = -1;
					control_type = t_control_type;	
					return FALSE;
				}	
			}
			else
				printf("update t_strategy version %s fail\n", t_strategy);
		}
	}
	else
		return FALSE;
	return TRUE;
}
boolean parse_unregist_packet(unsigned char * info)
{
	char t_info[8];
	int packet_len = parse_message_head(info);
	if(-1 == packet_len) 
		return FALSE;
	myUint8cpy(t_info, info+MESSAGE_HEAD_LEN, 0, SERVER_TO_GETWAY_RETCODE_LEN);
	return parse_return_code(atoi(t_info));
}

boolean parse_regist_packet(unsigned char * info)
{
	char t_info[8];
	int packet_len = parse_message_head(info);
	if(-1 == packet_len) 
		return FALSE;
	myUint8cpy(t_info, info+MESSAGE_HEAD_LEN, 0, SERVER_TO_GETWAY_RETCODE_LEN);
	return parse_return_code(atoi(t_info));
}
boolean construct_packet_head(unsigned char* message_head, int type)
{
	message_head[0] = SXT_0;
	message_head[1] = SXT_1;
	message_head[3] = protocol_version;
	message_len = 0,regist_len = 0;
	int i = 0, t_message_len = 0;
	if(type == GETWAY_TO_SERVER_REGIST_TYPE)
	{
		message_head[2] = 0;
		message_head[8] = 0;
		message_head[9] = GETWAY_TO_SERVER_REGIST_TYPE;
		regist_len = 22;
		t_message_len = 22;
	}
	else if(type == GETWAY_TO_SERVER_UNREGIST_TYPE)
	{
		message_head[2] = 2;
		message_head[8] = 0;
		message_head[9] = GETWAY_TO_SERVER_UNREGIST_TYPE;
		t_message_len = 18;
	}	
	else if(type == GETWAY_TO_SERVER_HEARTBEAT_TYPE)
	{
		message_head[2] = 2;
		message_head[8] = 0;
		message_head[9] = GETWAY_TO_SERVER_HEARTBEAT_TYPE;
		for(i=0; i<xml_length_type_100_size; i++)
			message_len += xml_length_type_100[i];
		t_message_len = message_len;
	}	
	else if(type == GETWAY_TO_SERVER_CONTROL)
	{
		message_head[2] = 2;
		message_head[8] = (GETWAY_TO_SERVER_CONTROL>>8)& 0xff;
		message_head[9] = GETWAY_TO_SERVER_CONTROL& 0xff;;
/***************************************************************
		for(i=8; i<xml_length_type_100_size; i++)
			t_message_len += xml_length_type_100[i];
		t_message_len += 8;
****************************************************************/
		t_message_len = 8+64*8;
	}
	printf("message_len:%d\n",message_len);
	t_message_len = t_message_len+2;////ETX and LRC
	message_head[4] = (t_message_len >> 24) & 0xff;
	message_head[5] = (t_message_len >> 16) & 0xff;
	message_head[6] = (t_message_len >> 8) & 0xff;
	message_head[7] = t_message_len & 0xff;

	int current_message_id = ++message_id;
	 if(type == GETWAY_TO_SERVER_CONTROL)
		current_message_id = 0x87;
	message_head[10] = (current_message_id >> 24) & 0xff;
	message_head[11] = (current_message_id >> 16) & 0xff;
	message_head[12] = (current_message_id >> 8) & 0xff;
	message_head[13] = current_message_id & 0xff;
	return TRUE;
}

boolean construct_heartbeat_packet_body(unsigned char* message_body)
{
	data_packet relay_packet={.SensorData = -1}, voltage_packet = {.SensorData = -1};
	int NodeAddress = 1, node_abnormal = 0, t_len = 0;
	char t_message_body[512], time_str[32] = "";
	memset(message_body, '\0', 4096);
	if(!get_localtime(time_str))
		return FALSE;
	sprintf(message_body,"%s",time_str);
	padding_string(message_body, CREATE_DATA_TIME_START+14 ,CREATE_DATA_TIME_END+1 , 0x00);
	printf("CREATE_DATA_TIME:%d,%s,\n",CREATE_DATA_TIME_END+1-strlen(time_str),message_body);	
	
	padding_string(message_body, GATEWAY_VOLTAGE_START ,GATEWAY_VOLTAGE_END+1 , 0x00);
	printf("GATEWAY_VOLTAGE:%s\n",GATEWAY_VOLTAGE_START+message_body);
	
	padding_string(message_body, GATEWAY_POWER_STATUS_START ,GATEWAY_POWER_STATUS_END+1 , 0x00);
	printf("GATEWAY_POWER_STATUS:%s\n",GATEWAY_POWER_STATUS_START+message_body);

	padding_string(message_body, GATEWAY_TEMPERATURE_START ,GATEWAY_TEMPERATURE_END+1 , 0x00);
	printf("GATEWAY_TEMPERATURE_START:%s\n",GATEWAY_TEMPERATURE_START+message_body);	
	
	padding_string(message_body, GATEWAY_ELECTRIC_START ,GATEWAY_ELECTRIC_END+1 , 0x00);
	printf("GATEWAY_ELECTRIC_START:%s\n",GATEWAY_ELECTRIC_START+message_body);
	
	padding_string(message_body, GATEWAY_VERSION_START ,GATEWAY_VERSION_END+1 , 0x00);
	printf("GATEWAY_VERSION_START:%s\n",GATEWAY_VERSION_START+message_body);

	t_len = sprintf(message_body+GATEWAY_STRATEGY_VERSION_START,"%s","ledProv1");
	padding_string(message_body, GATEWAY_STRATEGY_VERSION_START+t_len ,GATEWAY_STRATEGY_VERSION_END+1 , 0x00);
	printf("GATEWAY_STRATEGY_VERSION_START:%s\n",GATEWAY_STRATEGY_VERSION_START+message_body);
	
	padding_string(message_body, GATEWAY_ERROR_TYPE_START ,GATEWAY_ERROR_TYPE_END+1 , 0x00);
	printf("GATEWAY_ERROR_TYPE_START:%s\n",GATEWAY_ERROR_TYPE_START+message_body);

	//	sleep(10);
//	int has_message_len = 0;
//	for(i=0; i<8; i++)
//		has_message_len +=  xml_length_type_100[i];
	int i = 0,t_start = LIGHTBOX1_START, t_end = LIGHTBOX1_START+xml_length_type_100[8];
	for(i=8; i<xml_length_type_100_size; i++)
	{
//			//strcat(message_body,"1100201");
		//灯箱号,控制类型(手动[1]或自动(策略[0])),开关状态(开[0]或关[1]),灯箱状态(正常[0]或异常[1]),电流
		NodeAddress = i-7+64;//1016 backup:NodeAddress = i-7;
		printf("xml_len:%d,NodeAddress:%d\n",xml_length_type_100_size,NodeAddress);
		printf("============================");
		if(!send_cmd(NodeAddress, CMD_GET_NODE_LED_STATUS,(void*)(&relay_packet)))
		{
			node_abnormal |= 1;
			relay_packet.SensorData = -1;
		}
		if(relay_packet.Status != CMD_GET_NODE_LED_STATUS)
			node_abnormal |= 1;
		if(!send_cmd(NodeAddress, CMD_GET_NODE_LED_VOLTAGE,(void*)(&voltage_packet)))
		{
			node_abnormal |= 1;
			voltage_packet.SensorData = -1;
		}
		if(voltage_packet.Status != CMD_GET_NODE_LED_VOLTAGE)
			node_abnormal |= 1;
		memset(t_message_body, '\0', 512);
		if(relay_packet.SensorData == 0)
			relay_packet.SensorData = 1;
		else if(relay_packet.SensorData == 1)
			relay_packet.SensorData = 0;
		if(voltage_packet.SensorData != -1)
			voltage_packet.SensorData = voltage_packet.SensorData*24/1000; 
		t_len = sprintf(t_message_body,"%d,%d,%d,%d,%d", NodeAddress, control_type, relay_packet.SensorData, node_abnormal, voltage_packet.SensorData);
		myUint8cpy( message_body, t_message_body, t_start, t_len);
		printf("%d,LIGHTBOX1:%s,%c\n",t_end-t_start,t_start+message_body, message_body[t_start+1]);
		padding_string(message_body, t_start+t_len ,t_end , 0x00);
		printf("%d,LIGHTBOX1:%s\n",t_end-t_start,t_start+message_body);
		t_start = t_end;
		t_end = t_start + xml_length_type_100[i];
	}
	return TRUE;
}

boolean send_regist_packet()
{
	int j = 0;
	unsigned char regist_send_info[256] = {SXT_0,SXT_1,REGIST_APP_ID,protocol_version,0x0,0x0,0x0,0x16,0x0,0x14,0x0,0x0,0x0,0x02,0x0,0x08,0x32,0x30,0x30,0x30,0x30,0x31,0x0,0x08,0x31,0x32,0x33,0x34,0x35,0x36,0x0,0x04,0x30,0x30,0xcc,'\0'};//35
	myUint8cpy(regist_send_info,network_number, 16, 6);
	regist_send_info[35] = getUint8BCC(regist_send_info,14,35);
	printf("BBC:%02x\n",getUint8BCC(regist_send_info,14,35));
	is_recieve_regist_packet = 0;
	if(network_write(message_sockfd,regist_send_info,36) == FALSE)
	{
		message_isConnected = 0;
		return FALSE;
	}
	for(j=0; j<RECIEVE_TIMEOUT; j++)
	{
		
		if(is_recieve_regist_packet == -1||is_recieve_regist_packet == 1)
			break;
		usleep(1000);
	}
	if(is_recieve_regist_packet == -1 || RECIEVE_TIMEOUT == j || is_recieve_regist_packet == 0)
	{
		printf("send_regist_packet fail\n");
		return FALSE;	
	}
	printf("send_regist_packet success\n");
	message_isLogin = 1;
	return TRUE;
}

boolean send_unregist_packet()
{
	int j = 0;
	unsigned char regist_send_info[256] = {SXT_0,SXT_1,REGIST_APP_ID,protocol_version,0x0,0x0,0x0,0x12,0x0,0x16,0x0,0x0,0x0,0x02,0x0,0x08,0x32,0x30,0x30,0x30,0x30,0x31,0x0,0x08,0x31,0x32
,0x33,0x34,0x35,0x36,0xcc,'\0'};//31
	myUint8cpy(regist_send_info,network_number, 16, 6);
	regist_send_info[31] = getUint8BCC(regist_send_info,14,31);
	printf("BBC:%02x\n",getUint8BCC(regist_send_info,14,31));
	is_recieve_unregist_packet = 0;
	if(network_write(message_sockfd,regist_send_info,32) == FALSE)
	{
		message_isConnected = 0;
		return FALSE;
	}
	for(j=0; j<RECIEVE_TIMEOUT; j++)
	{
		
		if(is_recieve_unregist_packet == -1||is_recieve_unregist_packet == 1)
			break;
		usleep(1000);
	}
	if(is_recieve_unregist_packet == -1 || RECIEVE_TIMEOUT == j || is_recieve_unregist_packet == 0)
	{
		printf("is_recieve_unregist_packet fail\n");
		return FALSE;	
	}
	printf("is_recieve_unregist_packet success\n");
	return TRUE;
}


void get_led_node_status(unsigned char* led_status_info,int t_start,int nodes)
{
		int i = 0,t_end = t_start+64, NodeAddress = 0, node_abnormal = 0, t_len = 0;
		unsigned char t_led_status_info[512];
		data_packet relay_packet={.SensorData = -1}, voltage_packet = {.SensorData = -1};
		printf("\n===============nodes:%d=============\n",nodes);
		for(i=0; i<nodes; i++)
		{
	//			//strcat(message_body,"1100201");
			//灯箱号,控制类型(手动[1]或自动(策略[0])),开关状态(开[0]或关[1]),灯箱状态(正常[0]或异常[1]),电流
			NodeAddress = i+1;
			if(!send_cmd(NodeAddress, CMD_GET_NODE_LED_STATUS,(void*)(&relay_packet)))
			{
				node_abnormal |= 1;
				relay_packet.SensorData = -1;
			}
			if(relay_packet.Status != CMD_GET_NODE_LED_STATUS)
				node_abnormal |= 1;
			if(!send_cmd(NodeAddress, CMD_GET_NODE_LED_VOLTAGE,(void*)(&voltage_packet)))
			{
				node_abnormal |= 1;
				voltage_packet.SensorData = -1;
			}
			if(voltage_packet.Status != CMD_GET_NODE_LED_VOLTAGE)
				node_abnormal |= 1;
			if(relay_packet.SensorData == 0)
				relay_packet.SensorData = 1;
			else if(relay_packet.SensorData == 1)
				relay_packet.SensorData = 0;
			if(voltage_packet.SensorData != -1)
				voltage_packet.SensorData = voltage_packet.SensorData*24/1000; 
			memset(t_led_status_info, '\0', 512);
			t_len = sprintf(t_led_status_info,"%d,%d,%d,%d,%d", NodeAddress, control_type, relay_packet.SensorData, node_abnormal, voltage_packet.SensorData);
			myUint8cpy( led_status_info, t_led_status_info, t_start, t_len);
			printf("%d,LIGHTBOX1:%s,%c\n",t_end-t_start,t_start+led_status_info, led_status_info[t_start+1]);
			padding_string(led_status_info, t_start+t_len ,t_end , 0x00);
			printf("%d,LIGHTBOX1:%s\n",t_end-t_start,t_start+led_status_info);
			t_start = t_end;
			t_end = t_start + 64;
		}
		for(; i<=8; i++)
		{
			padding_string(led_status_info, t_start,t_end , 0x00);
			t_start = t_end;
			t_end = t_start + 64;
		}
}
void* send_heartbeat_packet(void * arg)
{
	char time_str[128]="20140604101010";	
	unsigned char message_info[4096], message_body[4096];
	int j = 0;
	unsigned char message_head[64] = {SXT_0,SXT_1,MESSAGE_APP_ID,protocol_version,0x00,0x00,0x02,0x68,0x00,0x64,0x00,0x00,0x00,0x00,'\0'};
	
	while(1)
	{
	//	sleep(100);
	//	if(get_localtime(time_str) == 0)
	//	{
	//		sleep(1);
	//		continue;
	//	}
		if(message_isConnected == 0)	
		{
			continue;
		}

		if(message_isLogin == 0)
		{
			continue;
		}
			//message_head[6] = 0x00;//0xff & (current_message_id > 8);
			//message_head[7] = 0x14;//0xff & (current_message_id > 8);
			
			//message_head[8] = 0xff & (current_message_id > 8);
			//message_head[9] = 0xff & current_message_id;	
			//message_head[11] = 0x07;
			//unsigned char test[1024] = {0x7e,0x68,0x00,0x01,0x00,0x00,0x00,0x09,0x00,0x14,0x00,0x01,'2','0','0','8','8',0x00,0x00,0xcc,0xfe};
			
			//printf("getBCC1 %02x,%02x:%x\n",test[35],test[34],getUint8BCC(test,12,20));
			//strcat(registz_info,message_head);
			//memset(temp, '\0', 1024);
			//test[35] = getBCC1(regiset_info,14,35);
			//sprintf(temp,"20088%c",0xcc);	
			//strcat(registz_info,temp);
//			
//			memset(temp1, '\0', 1024);
//			sprintf(temp1,"%c",getBCC(temp));				
			//strcat(registz_info,temp1);
//			is_recieve_regist_packet = 0;
//			if(network_write(message_sockfd,regist_send_info,36) == FALSE)
//			{
//				message_isConnected = 0;
//				continue;
//			}
//			for(j=0; j<RECIEVE_TIMEOUT; j++)
//			{
//				
//if(is_recieve_regist_packet == -1||is_recieve_regist_packet == 1)
//					break;
//				usleep(100);
//			}
//			if(is_recieve_regist_packet == -1)
//				continue;
//				
//			message_isLogin = 1;
			//printf("\n send registz_info:%s\n",registz_info);	
//			memset(regist_rev_info, '\0', 256);
//			if(network_read(message_sockfd, regist_rev_info, MESSAGE_HEAD_LEN))	
//			{
//				int regist_rev_info_len = get_message_len(regist_rev_info);
//				memset(message_temp, '\0', 1024);
//				if(network_read(message_sockfd, message_temp, regist_rev_info_len))	
//				{
//					recv_len = regist_rev_info_len+MESSAGE_HEAD_LEN;
//					myUint8cpy(regist_rev_info, message_temp, MESSAGE_HEAD_LEN, regist_rev_info_len);
//					printf("recv_len:%d\n==================",recv_len);
//					for(j=0;j<recv_len;j++)
//					{
//						printf("%02x ",regist_rev_info[j]);
//					}
//					printf("==================\n");	
//					if(!parse_regist_packet(regist_rev_info))
//						continue;
//				}
//				else
//				{
//					message_isConnected = 0;
//					continue;
//				}
//				//while(1);
//			}
//			else
//			{
//				message_isConnected = 0;
//				continue;
//			}
//		else
//		{
//			printf("send suceess len:%d\n",send_len);
//		
//			printf("\n==================");
//			for(j=0;j<send_len;j++)
//			{
//				printf("%02x ",test[j]);
//			}
//			printf("==================\n");	
//		}
		
		
//sleep(10);
		memset(message_info, '\0', 4096);
		if(!construct_packet_head(message_head, GETWAY_TO_SERVER_HEARTBEAT_TYPE))
			continue;
		if(!construct_heartbeat_packet_body(message_body))
			continue;
//		current_message_id = ++message_id;
//		//message_head[6] = 0x02;//0xff & (current_message_id > 8);
//		//message_head[7] = 0x68;//0xff & (current_message_id > 8);
//		message_head[10] = 0xff & (current_message_id >> 24);
//		message_head[11] = 0xff & (current_message_id >> 16);
//		message_head[12] = 0xff & (current_message_id >> 8);
//		message_head[13] = 0xff & current_message_id;
		
		

		//strcat(message_body,"1100201");
//		myCopy(message_body, "1,1,0,0,201", LIGHTBOX1_START, 11);
//		padding_string(message_body, LIGHTBOX1_START+11 ,LIGHTBOX1_END+1 , 0x00);
//		printf("%d,LIGHTBOX1:%s\n",LIGHTBOX1_END-LIGHTBOX1_START+1,LIGHTBOX1_START+message_body);
//		
//		myCopy(message_body, "2,1,0,0,201", LIGHTBOX2_START, 11);
//		padding_string(message_body, LIGHTBOX2_START+11 ,LIGHTBOX2_END+1 , 0x00);
//		printf("LIGHTBOX2:%s\n",LIGHTBOX2_START+message_body);
//		
//		myCopy(message_body, "3,1,0,0,201", LIGHTBOX3_START, 11);
//		padding_string(message_body, LIGHTBOX3_START+11 ,LIGHTBOX3_END+1 , 0x00);
//		printf("LIGHTBOX3:%s\n",LIGHTBOX3_START+message_body);
//		
//		myCopy(message_body, "4,1,0,0,201", LIGHTBOX4_START, 11);
//		padding_string(message_body, LIGHTBOX4_START+11 ,LIGHTBOX4_END+1 , 0x00);
//		printf("LIGHTBOX4:%s\n",LIGHTBOX4_START+message_body);
//		
//		myCopy(message_body, "5,1,0,0,201", LIGHTBOX5_START, 11);
//		padding_string(message_body, LIGHTBOX5_START+11 ,LIGHTBOX5_END+1 , 0x00);
//		printf("%d,LIGHTBOX5:%s\n",LIGHTBOX5_END-LIGHTBOX5_START+1,LIGHTBOX5_START+message_body);
//		
//		myCopy(message_body, "6,1,0,0,201", LIGHTBOX6_START, 11);
//		padding_string(message_body, LIGHTBOX6_START+11 ,LIGHTBOX6_END+1 , 0x00);
//		printf("LIGHTBOX6:%s\n",LIGHTBOX6_START+message_body);
//		
//		myCopy(message_body, "7,1,0,0,201", LIGHTBOX7_START, 11);
//		padding_string(message_body, LIGHTBOX7_START+11 ,LIGHTBOX7_END+1 , 0x00);
//		printf("LIGHTBOX7:%s\n",LIGHTBOX7_START+message_body);
//		
//		myCopy(message_body, "8,1,0,0,201", LIGHTBOX8_START, 11);
//		padding_string(message_body, LIGHTBOX8_START+11 ,LIGHTBOX8_END+1 , 0x00);
//		printf("LIGHTBOX8:%s\n",LIGHTBOX8_START+message_body);
		myUint8cpy(message_info, message_head, 0, MESSAGE_HEAD_LEN);
		myUint8cpy(message_info, message_body, MESSAGE_HEAD_LEN, message_len);
		message_info[message_len+MESSAGE_HEAD_LEN] = 0xcc;
		message_info[message_len+MESSAGE_HEAD_LEN+1] = getUint8BCC(message_info,14, message_len+MESSAGE_HEAD_LEN+1);
		
		if(!network_write(message_sockfd, message_info, message_len+MESSAGE_HEAD_LEN+2))
		{
			message_isConnected = 0;
			continue;
		}
		else
		{
			printf("\n==================");
			j = 14;
		//	for(j=0;j<message_len+MESSAGE_HEAD_LEN+2;j++)
			{
				int n=0,m=0;
				for(n=0; n<xml_length_type_100_size; n++)
				{
					for(m=0; m<xml_length_type_100[n]; m++)
					{
						printf("%02x ",message_info[j++]);
					}
					printf("\n");
				}
			}
			printf("%02x,%02x\n",message_info[j],message_info[j+1]);
			printf("==================\n");
		}
		for(j=0; j<RECIEVE_TIMEOUT; j++)
		{
			if(is_recieve_message_packet == -1||is_recieve_message_packet == 1)
			{
				break;
			}
			usleep(1000);
		}
		if(is_recieve_message_packet == -1 || RECIEVE_TIMEOUT == j)
		{
			printf("send_heart_packet fail\n");
			message_isConnected = 0;
			continue;
		}
		printf("send_heart_packet success\n");
		sleep(60*2);
//		memset(message_rev_info, '\0', 1024);
//		if(network_read(message_sockfd, message_rev_info, MESSAGE_HEAD_LEN))	
//		{
//			int t_recieve_len = get_message_len(message_rev_info);
//			memset(message_temp, '\0', 1024);
//			if(network_read(message_sockfd, message_temp, message_rev_info_len))	
//			{
//				recv_len = message_rev_info_len+MESSAGE_HEAD_LEN;
//				myUint8cpy(message_rev_info, message_temp, MESSAGE_HEAD_LEN, message_rev_info_len);
//				printf("recv_len:%d\n==================",recv_len);
//				for(j=0;j<recv_len;j++)
//				{
//					printf("%02x ",message_rev_info[j]);
//				}
//				printf("==================\n");	
//				if(!parse_heartbeat_packet(message_rev_info))
//					continue;
//			}
//			else
//			{
//				message_isConnected = 0;
//				continue;
//			}
//			//while(1);
//		}
//		else
//		{
//			message_isConnected = 0;
//			continue;
//		}
//		//
//		while((recv_len = read(sockfd,recvbuff,1024)) > 0)	
//		{
//			printf("len:%d,rev:%s\n",strlen(recvbuff),recvbuff);
//				printf("recv_len:%d\n==================",recv_len);
//				for(j=0;j<recv_len;j++)
//				{
//					printf("%02x ",recvbuff[j]);
//				}
//				printf("==================\n");	
//				while(1);	
//		}
		//if(!parse_message_body())		
	}
}

boolean recieve_server_packet()
{
	char t_recieve_info[4096], message_temp[4096], stc_info[1024];
	int t_recieve_len = 0, t_recieve_total_len = 0, j = 0;

	memset(t_recieve_info, '\0', 4096);
	memset(message_temp, '\0', 4096);
	memset(stc_info, '\0', 1024);
//	if(!network_recieve_semaphore_p())
//	{
//		printf("network_recieve_semaphore_p fail\n");
//	}

	if(message_sockfd == -1)
		return FALSE;
	if(network_read(message_sockfd, t_recieve_info, MESSAGE_HEAD_LEN))	
	{
		if(t_recieve_info[0] == STC)
		{
			for(j=0;j<MESSAGE_HEAD_LEN;j++)
				stc_info[j] = t_recieve_info[j];
	//		printf("++++++++++++++++++++++++++++++\n");
	//		network_read(message_sockfd, stc_info, 1024);
	//		printf("++++++++++++++++++++++++++++++\n");
			if(network_read(message_sockfd, stc_info+14, stc_info[1]-12))	
			{
				memset(t_recieve_info, '\0', 4096);
				if(!network_read(message_sockfd, t_recieve_info, MESSAGE_HEAD_LEN))	
				{
					message_isConnected = 0;
				}
			}
			else
			{
				message_isConnected = 0;
			}
		}
		printf("\n++%02x,%02x,%02x,%02x++\n",t_recieve_info[0],t_recieve_info[1],t_recieve_info[2],t_recieve_info[3]);
		t_recieve_len  = get_message_len(t_recieve_info);
		printf("===%d====",t_recieve_len);
		memset(message_temp, '\0', 4096);
		if(network_read(message_sockfd, message_temp, t_recieve_len))	
		{
		
			if(message_temp[t_recieve_len-2]!=0xcc || message_temp[t_recieve_len-1] != getUint8BCC(message_temp, 0, t_recieve_len-1))
			{
				printf("bcc or end_flag fail\n");
				return FALSE;
			}
			t_recieve_total_len = t_recieve_len + MESSAGE_HEAD_LEN;
			myUint8cpy(t_recieve_info, message_temp, MESSAGE_HEAD_LEN, t_recieve_len);
			printf("\nrecv_len:%d\n==================\n",t_recieve_total_len);
			
			for(j=0;j<t_recieve_total_len;j++)
			{
				printf("%02x ",t_recieve_info[j]);
			}
			printf("==================\n");	
			switch(get_message_type(t_recieve_info))
			{
				case SERVER_TO_GETWAY_SYS_RESPONSE: 
				case SERVER_TO_GETWAY_REGIST_TYPE:
				{
					boolean t_return  = parse_regist_packet(t_recieve_info);
				//	printf("--------------------------------------------------%d------------------------------\n",t_return);
					if(t_return)
						is_recieve_regist_packet = 1;
					else
						is_recieve_regist_packet = -1;
				//	if(!network_recieve_semaphore_v())
				//	{
				//		printf("network_recieve_semaphore_v fail\n");
				//	}
					return t_return;
						
				}
				case SERVER_TO_GETWAY_UNREGIST_TYPE:
				{
					boolean t_return  = parse_unregist_packet(t_recieve_info);
				//	printf("--------------------------------------------------%d------------------------------\n",t_return);
					if(t_return)
						is_recieve_unregist_packet = 1;
					else
						is_recieve_unregist_packet = -1;
				//	if(!network_recieve_semaphore_v())
				//	{
				//		printf("network_recieve_semaphore_v fail\n");
				//	}
					return t_return;
						
				}
				case SERVER_TO_GETWAY_HEARTBEAT_TYPE:
				{
					boolean t_return = parse_heartbeat_packet(t_recieve_info);
					if(t_return)
						is_recieve_message_packet = 1;
					else
						is_recieve_message_packet = -1;
				//	if(!network_recieve_semaphore_v())
				//	{
				//		printf("network_recieve_semaphore_v fail\n");
				//	}
					return t_return;
				}
				case SERVER_TO_GETWAY_CONTROL:
				{
					int nodes = parse_control_packet(t_recieve_info);
					int t_packet_len = parse_message_head(t_recieve_info);
				//	if(!network_recieve_semaphore_v())
				//	{
				//		printf("network_recieve_semaphore_v fail\n");
				//	}
					if(-1 == t_packet_len)
						return FALSE;
					unsigned char led_status_info[4096];
					int info_leek = stc_info[1]+2;
					memset(led_status_info, '\0', 4096);
					myUint8cpy(led_status_info, stc_info, 0, info_leek);
					construct_packet_head(led_status_info+info_leek, GETWAY_TO_SERVER_CONTROL);
					led_status_info[info_leek+10] = t_recieve_info[10];
					led_status_info[info_leek+11] = t_recieve_info[11];
					led_status_info[info_leek+12] = t_recieve_info[12];
					led_status_info[info_leek+13] = t_recieve_info[13];
					info_leek += MESSAGE_HEAD_LEN;
					if(nodes != 0)
					{
						led_status_info[info_leek] = '2';
						led_status_info[info_leek+1] = '0';
						led_status_info[info_leek+2] = '0';
					}
					else
					{	
						led_status_info[info_leek] = '2';
						led_status_info[info_leek+1] = '0';
						led_status_info[info_leek+2] = '1';
					}
					info_leek += 8;
					int t_info_leek = info_leek+t_packet_len-2;
					get_led_node_status(led_status_info+info_leek, 0, nodes);
					led_status_info[t_info_leek] = 0xcc;
					led_status_info[t_info_leek+1] = getUint8BCC(info_leek-8+led_status_info, 0, t_info_leek+1);
					pthread_t t_return_control_packet_pid; 
					_return_control_packet *_packet = (_return_control_packet *)malloc(sizeof(_return_control_packet));
					myUint8cpy(_packet->return_info, led_status_info, 0, t_info_leek+2);
					_packet->info_len = t_info_leek+2;
					if(pthread_create(&t_return_control_packet_pid, NULL, return_control_packet, (void*)(_packet)))
					{
						printf("create pthread error .... \n");
					}
					return nodes != 0;
				}
				case SERVER_TO_GETWAY_EXCEPTION:
				{
				//	if(t_return)
				//		is_recieve_exception_packet = 1;
				//	else
				//		is_recieve_exception_packet = -1;
				//	if(!network_recieve_semaphore_v())
				//	{
				//		printf("network_recieve_semaphore_v fail\n");
				//	}
				//	return t_return;
				}
			}
		}
//		else
//		{
//			message_isConnected = 0;
//		}
		//while(1);
	}
//	else
//	{
//		message_isConnected = 0;
//	}
	return FALSE;
}

	
void* send_abnormal_packet(void * arg)	
{
	
	
}

void* return_control_packet(void * arg)	
{
	_return_control_packet *_packet = (_return_control_packet *)arg;
	int j = 0, i = 0;
	printf("\n//////////////////////////////////////////////\n");
	for(j=0;j<100;j++)
	{
		printf("%02x,",_packet->return_info[j]);
	}
	printf("//////////////////////////////////////////////\n");
	printf("//////////////////////////////////////////////\n");
	for(j=100;j<114;j++)
	{
		printf("%02x,",_packet->return_info[j]);
	}
	printf("//////////////////////////////////////////////\n");
	printf("//////////////////////////////////////////////\n");
	for(j=114;j<122;j++)
	{
		printf("%02x,",_packet->return_info[j]);
	}
	printf("//////////////////////////////////////////////\n");
	printf("//////////////////////////////////////////////\n");
	for(i=1;i<9;i++)
	{
		printf("//////////////////////////////////////////////\n");
		for(j=122+64*(i-1);j<122+64*i;j++)
		{
			printf("%02x,",_packet->return_info[j]);
		}
		printf("//////////////////////////////////////////////\n");
	}
	printf("//////////////////////////////////////////////\n");
	for(j=122+64*8;j<122+64*8+2;j++)
	{
		printf("%02x,",_packet->return_info[j]);
	}
	printf("//////////////////////////////////////////////");
	if(!network_write(message_sockfd, _packet->return_info, _packet->info_len))
	{
			message_isConnected = 0;
	}
	free(_packet);
}

void* recieve_server_packet_pthread(void * arg)	
{
	while(1)
	{
		sleep(1);
		//if(message_isConnected == 0)	
	//	{
	//		if(message_sockfd > 0)
	//		{
	//			close(message_sockfd);
	//			message_sockfd = -1;
	//			message_isLogin = 0;
	//		}
	//		message_init();
	//		continue;
	//	}
		if(!recieve_server_packet())
		{
			printf("recieve_control_packet fail\n");
		}
			
	}
}

