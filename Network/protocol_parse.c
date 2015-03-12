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
#include "ini_parse.h"
struct sockaddr_in message_server_addr; 

int message_sockfd = -1; 
extern volatile unsigned char message_isLogin;
int xml_length_type_100[128]={0};
int xml_length_type_301[128]={0};
int xml_length_type_100_size = 0;
int xml_length_type_301_size = 0;
char protocl_file_name[128] = "";
char strategy_file_name[128] ="";
char config_ini_name[128]="";
extern char gate_way_number[128];

unsigned char protocol_version = 0;

int message_len = 0;
int regist_len = 0;
volatile int control_type = 0;
int volatile is_recieve_regist_packet = 0;
int volatile is_recieve_unregist_packet = 0;
int volatile is_recieve_message_packet = 0;
int volatile is_recieve_exception_packet = 0;

extern boolean my_parse_ini();


/*boolean xml_config(char* _protocl_file_name)
{
	if(!find_new_file(PROTOCOL_FILE_DIR,_protocl_file_name))
		return FALSE;
	printf("protocol_file_name:%s\n",_protocl_file_name);
	if(-1 == (xml_length_type_100_size = xml_parse(_protocl_file_name, 100, xml_length_type_100)))
		return FALSE;
	if(-1 == (xml_length_type_301_size = xml_parse(_protocl_file_name, 301, xml_length_type_301)))
		return FALSE;
	return TRUE;
}*/
boolean xml_config(char* _protocl_file_name)
{
	xml_length_type_100_size = point_config.beiting.len+point_config.dapai.len+point_config.yuanhu.len - 3 + 8;
	xml_length_type_301_size = point_config.beiting.len+point_config.dapai.len+point_config.yuanhu.len - 3 + 1;
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
				printf("now already unregist\n");
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
		printf("#######################################%d\n",atoi(t_split_info[0]));
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
	
	char t_info[10], t_time[20], t_version[16], current_version[16], t_strategy[32],t_config[16];
	char dir_name_info[128][128] = {"ledrelay"};//, app_name[128] = "ledPro";
	char path[128]={0};
	printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$heaat_beat$$$$$$$$$$$$$$$$$\n");
	printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$heaat_beat$$$$$$$$$$$$$$$$$\n");
	printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$heaat_beat$$$$$$$$$$$$$$$$$\n");
	struct tm t_temp;
	char t_cmd[128];
	int info_leek = 0;
	int packet_len = parse_message_head(info);
	if(-1 == packet_len)
		return FALSE;
	//消息头
	info_leek += MESSAGE_HEAD_LEN;
	if(-1 == packet_len)
		return FALSE;
	myUint8cpy(t_info, info+info_leek, 0, 8);
	if(parse_return_code(atoi(t_info)))
	{
		//时间消息
		info_leek += 8;
		myUint8cpy(t_time, info+info_leek, 0, 20);
		if(!calibrateTime(t_time))
			printf("calibrateTime fail\n");
//#define STRATEGY_FILE_DIR "/app/strategy/"

//#define PROTOCOL_FILE_DIR "/app/protocol/"
		//中继器新版本
		info_leek += 20;
		myUint8cpy(t_version, info+info_leek, 0, 16);
	//	strcat(app_name, t_version);
	//	if(!find_new_file(PROTOCOL_FILE_DIR,current_version))
	//		printf("find_new_file fail\n");
		char t_app_name[128];
		memset(t_app_name,'\0',128);
		strcat(t_app_name,"ledPro");
		strcat(t_app_name,t_version);
		//发现网关的新版本
		printf("###########################################GETWAY_VERSION is %s t_version  is %s \n", GETWAY_VERSION,t_version);
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
				printf("!!!update getway version %s %s fail\n", t_version,t_app_name);
		}
		//发现策略的新版本
		info_leek += 16;
		myUint8cpy(t_strategy, info+info_leek, 0, 32);
		printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$STRATEGY$$$$$$$$$$$$%s$$$$$\n",t_strategy);
//		if(!find_new_file(STRATEGY_FILE_DIR,current_strategy))
//			printf("find_new_file fail\n");
		if((strlen(t_strategy) != 0)&&(strcmp(t_strategy,strategy_file_name)))
		{
			memset(dir_name_info[0], '\0', 128);
			strcpy(dir_name_info[0],"strategy");
			printf("###################%s###############%s\n",dir_name_info,gate_way_number);
			if(get_file_from_server(dir_name_info, gate_way_number))
			{
				printf("update t_strategy version %s success\n", t_strategy);
				printf("$$$$$$$$$$$%s$$$$$$$$$$$$$$$$$%s\n",gate_way_number,t_strategy);
				sprintf(t_cmd,"mv /app/%s /app/%s", gate_way_number,t_strategy);
				mySystem(t_cmd);
				sleep(2);
				sprintf(t_cmd,"mv /app/%s /app/strategy/", t_strategy);
				mySystem(t_cmd);
				sleep(2);
					//sleep(1);
				memset(t_cmd, '\0', 128);
				sprintf(t_cmd,"rm /app/strategy/%s",strategy_file_name);
				mySystem(t_cmd);
				sleep(2);
				memset(strategy_file_name, '\0', 128);
				strcpy(strategy_file_name,t_strategy);
				//timetable_index = 0;
				//current_strategy_list = 0;
				printf("-------------------------------------------------------\n");
				if(!strategy_parse(t_strategy))
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
				printf("!!!update t_strategy version %s fail\n", t_strategy);
		}
		//发现配置文件的新版本
		info_leek += 32;
		myUint8cpy(t_config, info+info_leek, 0, 16);
//		if(!find_new_file(STRATEGY_FILE_DIR,current_strategy))
//			printf("find_new_file fail\n");

		memset(dir_name_info[0], '\0', 128);
		strcpy(dir_name_info[0],"ledstationconfig");
		if(strcmp(t_config,config_ini_name))
		{
			if(get_file_from_server(dir_name_info, gate_way_number))
			{
				printf("ftp get file success");
				printf("gate_way_number%s************t_config%s\n",gate_way_number,t_config);
				
				memset(t_cmd, '\0', 128);
				sprintf(t_cmd,"mv /app/%s /app/%s", gate_way_number,t_config);
				mySystem(t_cmd);
				sleep(2);
				
				memset(t_cmd, '\0', 128);
				sprintf(t_cmd,"mv /app/%s /app/ini/", t_config);
				mySystem(t_cmd);
				sleep(2);
				memset(t_cmd, '\0', 128);
				sprintf(t_cmd,"rm /app/ini/%s",config_ini_name);
				mySystem(t_cmd);
				sleep(2);
				strcpy(config_ini_name,t_config);
				sprintf(path,"/app/ini/%s", t_config);
				printf("#########################################################################################################update config version %s good\n",t_config);
				if(!parse_ini(path))
				{
					printf("#########################################################################################################update config version %s good\n",t_config);
				}
				else
				{
					printf("!!!update config version %s fail\n", t_config);
				}
			}
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
		//what????
		/*for(i=0; i<xml_length_type_100_size; i++)
			message_len += xml_length_type_100[i];*/
		message_len = (xml_length_type_100_size - 8) * 64 + 102;
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
	int node_abnormal = 0, t_len = 0;
	int i = 0;
	char t_message_body[512], time_str[32] = "";

	int *NodeAddress = (int *)malloc(sizeof(int)*(xml_length_type_100_size - 8));
	for(i  = 1;i < point_config.beiting.len; i++)
	{
		*(NodeAddress + i - 1)  = point_config.beiting.point[i];
		printf("$$$$$$$$$$$$$$$$$$$$$$$$$$%d\n",point_config.beiting.point[i]);
	}
	NodeAddress  = NodeAddress + point_config.beiting.len-1;
	
	for(i  = 1;i < point_config.dapai.len; i++)
	{
		*(NodeAddress + i - 1 ) = point_config.dapai.point[i];
		printf("$$$$$$$$$$$$$$$$$$$$$$$$$$%d\n",point_config.dapai.point[i]);
	}
	NodeAddress  = NodeAddress + point_config.dapai.len-1;
	
	for(i  = 1;i < point_config.yuanhu.len; i++)
	{
		*(NodeAddress + i - 1 ) = point_config.yuanhu.point[i];
		printf("$$$$$$$$$$$$$$$$$$$$$$$$$$%d\n",point_config.yuanhu.point[i]);
	}
	NodeAddress  = NodeAddress - (point_config.dapai.len-1) - (point_config.beiting.len-1);
	
	for(i = 0;i < xml_length_type_100_size -8; i++)
	{
		printf("$$$$$$$$$$$$$$$$$$$$$$$$$$%d\n",*(NodeAddress+i));
	}
	
	memset(message_body, '\0', 4096);
	if(!get_localtime(time_str))
		return FALSE;
	sprintf(message_body,"%s",time_str);
	//time
	padding_string(message_body, CREATE_DATA_TIME_START+14 ,CREATE_DATA_TIME_END+1 , 0x00);
	printf("CREATE_DATA_TIME:%d,%s,\n",CREATE_DATA_TIME_END+1-strlen(time_str),message_body);	
	//voltage
	padding_string(message_body, GATEWAY_VOLTAGE_START ,GATEWAY_VOLTAGE_END+1 , 0x00);
	printf("GATEWAY_VOLTAGE:%s\n",GATEWAY_VOLTAGE_START+message_body);
	//power stat
	padding_string(message_body, GATEWAY_POWER_STATUS_START ,GATEWAY_POWER_STATUS_END+1 , 0x00);
	printf("GATEWAY_POWER_STATUS:%s\n",GATEWAY_POWER_STATUS_START+message_body);
	//temprature state
	padding_string(message_body, GATEWAY_TEMPERATURE_START ,GATEWAY_TEMPERATURE_END+1 , 0x00);
	printf("GATEWAY_TEMPERATURE_START:%s\n",GATEWAY_TEMPERATURE_START+message_body);	
	//electic stat
	padding_string(message_body, GATEWAY_ELECTRIC_START ,GATEWAY_ELECTRIC_END+1 , 0x00);
	printf("GATEWAY_ELECTRIC_START:%s\n",GATEWAY_ELECTRIC_START+message_body);
	//version stat
	//padding_string(message_body, GATEWAY_VERSION_START ,GATEWAY_VERSION_END+1 , 0x00);
	myUint8cpy(message_body,GETWAY_VERSION,GATEWAY_VERSION_START,strlen(GETWAY_VERSION));
	printf("GATEWAY_VERSION_START:%s\n",GATEWAY_VERSION_START+message_body);
	
	t_len = sprintf(message_body+GATEWAY_STRATEGY_VERSION_START,"%s","ledProv1");
	//stragey version stat
	padding_string(message_body, GATEWAY_STRATEGY_VERSION_START+t_len ,GATEWAY_STRATEGY_VERSION_END+1 , 0x00);
	printf("GATEWAY_STRATEGY_VERSION_START:%s\n",GATEWAY_STRATEGY_VERSION_START+message_body);
	//type stat
	padding_string(message_body, GATEWAY_ERROR_TYPE_START ,GATEWAY_ERROR_TYPE_END+1 , 0x00);
	printf("GATEWAY_ERROR_TYPE_START:%s\n",GATEWAY_ERROR_TYPE_START+message_body);

	//	sleep(10);
//	int has_message_len = 0;
//	for(i=0; i<8; i++)
//		has_message_len +=  xml_length_type_100[i];
	int t_start = LIGHTBOX1_START, t_end = LIGHTBOX1_START+64;
	int j = 0;
	void build_body(int len,char ch)
	{
		int i = 0;
		for(i=1; i<len; i++)
		{
	//			//strcat(message_body,"1100201");
			//灯箱号,控制类型(手动[1]或自动(策略[0])),开关状态(开[0]或关[1]),灯箱状态(正常[0]或异常[1]),电流
			//NodeAddress = i-7+64;//1016 backup:NodeAddress = i-7;
			printf("xml_len:%d,NodeAddress:%d\n",xml_length_type_100_size,*(NodeAddress+j));
			printf("============================");
			if(!send_cmd((*(NodeAddress+j)), CMD_GET_NODE_LED_STATUS,(void*)(&relay_packet)))
			{
				node_abnormal |= 1;
				relay_packet.SensorData = -1;
			}
			if(relay_packet.Status != CMD_GET_NODE_LED_STATUS)
				node_abnormal |= 1;
			if(!send_cmd(*(NodeAddress+j), CMD_GET_NODE_LED_VOLTAGE,(void*)(&voltage_packet)))
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
			t_len = sprintf(t_message_body,"%d,%d,%d,%d,%d,%c", *(NodeAddress+j), control_type, relay_packet.SensorData, node_abnormal, voltage_packet.SensorData,ch);
			//led stat
			myUint8cpy( message_body, t_message_body, t_start, t_len);
			printf("%d,LIGHTBOX1:%s,%c\n",t_end-t_start,t_start+message_body, message_body[t_start+1]);
			padding_string(message_body, t_start+t_len ,t_end , 0x00);
			printf("%d,LIGHTBOX1:%s\n",t_end-t_start,t_start+message_body);
			t_start = t_end;
			t_end = t_start + 64;
			j++;
		}
	}
	build_body(point_config.beiting.len,'b');
	build_body(point_config.dapai.len,'d');
	build_body(point_config.yuanhu.len,'y');
	free(NodeAddress);
	return TRUE;
}

boolean send_regist_packet()
{
	int j = 0, t_info_leek = 14, network_number_len = strlen(network_number);

	unsigned char terminalId_1 = (network_number_len+2)&0xff00 >> 16, terminalId_2 = (network_number_len+2)&0xff;
	unsigned char regist_send_info[256] = {SXT_0,SXT_1,REGIST_APP_ID,protocol_version,0x0,0x0,0x0,network_number_len+16,0x0,0x14,0x0,0x0,0x0,0x02};
	
	unsigned char  regist_send_info_tail[] = {0x0,0x08,0x31,0x32,0x33,0x34,0x35,0x36,0x0,0x04,0x30,0x30,0xcc};
	
	myUint8cpy(regist_send_info, &terminalId_1, t_info_leek, 1);
	t_info_leek += 1;
	myUint8cpy(regist_send_info, &terminalId_2, t_info_leek, 1);
	t_info_leek += 1;
	myUint8cpy(regist_send_info, network_number, t_info_leek, network_number_len);
	t_info_leek += network_number_len;
	myUint8cpy(regist_send_info, regist_send_info_tail, t_info_leek, sizeof(regist_send_info_tail));
	t_info_leek += sizeof(regist_send_info_tail);
	
	regist_send_info[t_info_leek] = getUint8BCC(regist_send_info,14,t_info_leek);
	t_info_leek += 1;
	printf("BBC:%02x\n", getUint8BCC(regist_send_info,14,t_info_leek));
	for(j=0; j<t_info_leek; j++)
	{
		printf("%02x,", regist_send_info[j]);
	}
	is_recieve_regist_packet = 0;
	if(network_write(message_sockfd,regist_send_info,t_info_leek) == FALSE)
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
	int j = 0, t_info_leek = 14, network_number_len = strlen(network_number);
	unsigned char terminalId_1 = (network_number_len+2)&0xff00 >> 16, terminalId_2 = (network_number_len+2)&0xff;
	
	unsigned char regist_send_info[256] = {SXT_0,SXT_1,REGIST_APP_ID,protocol_version,0x0,0x0,0x0,network_number_len+12,0x0,0x16,0x0,0x0,0x0,0x02};
	unsigned char  regist_send_info_tail[] = {0x0,0x08,0x31,0x32,0x33,0x34,0x35,0x36,0xcc};

	myUint8cpy(regist_send_info, &terminalId_1, t_info_leek, 1);
	t_info_leek += 1;
	myUint8cpy(regist_send_info, &terminalId_2, t_info_leek, 1);
	t_info_leek += 1;
	myUint8cpy(regist_send_info, network_number, t_info_leek, network_number_len);
	t_info_leek += network_number_len;
	myUint8cpy(regist_send_info, regist_send_info_tail, t_info_leek, sizeof(regist_send_info_tail));
	t_info_leek += sizeof(regist_send_info_tail);
	
	regist_send_info[t_info_leek] = getUint8BCC(regist_send_info,14,t_info_leek);
	t_info_leek += 1;
	printf("BBC:%02x\n", getUint8BCC(regist_send_info,14,t_info_leek));
	for(j=0; j<t_info_leek; j++)
	{
		printf("%02x,", regist_send_info[j]);
	}
	
	is_recieve_unregist_packet = 0;
	if(network_write(message_sockfd,regist_send_info,t_info_leek) == FALSE)
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
		int i = 0,t_end = t_start+64, node_abnormal = 0, t_len = 0;
		unsigned char t_led_status_info[512];


		int *NodeAddress = (int *)malloc(sizeof(int)*(xml_length_type_100_size - 8));
		for(i  = 1;i < point_config.beiting.len; i++)
		{
			*(NodeAddress + i - 1)  = point_config.beiting.point[i];
			printf("$$$$$$$$$$$$$$$$$$$$$$$$$$%d\n",point_config.beiting.point[i]);
		}
		NodeAddress  = NodeAddress + point_config.beiting.len-1;
		
		for(i  = 1;i < point_config.dapai.len; i++)
		{
			*(NodeAddress + i - 1 ) = point_config.dapai.point[i];
			printf("$$$$$$$$$$$$$$$$$$$$$$$$$$%d\n",point_config.dapai.point[i]);
		}
		NodeAddress  = NodeAddress + point_config.dapai.len-1;
		
		for(i  = 1;i < point_config.yuanhu.len; i++)
		{
			*(NodeAddress + i - 1 ) = point_config.yuanhu.point[i];
			printf("$$$$$$$$$$$$$$$$$$$$$$$$$$%d\n",point_config.yuanhu.point[i]);
		}
		NodeAddress  = NodeAddress - (point_config.dapai.len-1) - (point_config.beiting.len-1);
		
		for(i = 0;i < xml_length_type_100_size -8; i++)
		{
			printf("$$$$$$$$$$$$$$$$$$$$$$$$$$%d\n",*(NodeAddress+i));
		}
	
		data_packet relay_packet={.SensorData = -1}, voltage_packet = {.SensorData = -1};
		printf("\n===============nodes:%d=============\n",nodes);
		int j = 0;
		void build_body(int len,char ch)
		{
			int i = 0; 
			for(i=1; i<len; i++)
			{
	//				//strcat(message_body,"1100201");
				//灯箱号,控制类型(手动[1]或自动(策略[0])),开关状态(开[0]或关[1]),灯箱状态(正常[0]或异常[1]),电流
				if(!send_cmd((*(NodeAddress+j)), CMD_GET_NODE_LED_STATUS,(void*)(&relay_packet)))
				{
					node_abnormal |= 1;
					relay_packet.SensorData = -1;
				}
				if(relay_packet.Status != CMD_GET_NODE_LED_STATUS)
					node_abnormal |= 1;
				if(!send_cmd((*(NodeAddress+j)), CMD_GET_NODE_LED_VOLTAGE,(void*)(&voltage_packet)))
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
				t_len = sprintf(t_led_status_info,"%d,%d,%d,%d,%d,%c", (*(NodeAddress+j)), control_type, relay_packet.SensorData, node_abnormal, voltage_packet.SensorData, ch);
				myUint8cpy( led_status_info, t_led_status_info, t_start, t_len);
				printf("%d,LIGHTBOX1:%s,%c\n",t_end-t_start,t_start+led_status_info, led_status_info[t_start+1]);
				padding_string(led_status_info, t_start+t_len ,t_end , 0x00);
				printf("%d,LIGHTBOX1:%s\n",t_end-t_start,t_start+led_status_info);
				t_start = t_end;
				t_end = t_start + 64;
				j++;
			}
		}
		build_body(point_config.beiting.len,'b');
		build_body(point_config.dapai.len,'d');
		build_body(point_config.yuanhu.len,'y');
		for(; j<=8; j++)
		{
			printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@t_start is %d@@@@@@@t_start is %d\n",t_start,t_end);
			padding_string(led_status_info, t_start,t_end , 0x00);
			t_start = t_end;
			t_end = t_start + 64;
		}
		free(NodeAddress);
}


void* send_heartbeat_packet(void * arg)
{
	char time_str[128]="20140604101010";	
	unsigned char message_info[4096], message_body[4096];
	int j = 0;
	unsigned char message_head[64] = {SXT_0,SXT_1,MESSAGE_APP_ID,protocol_version,0x00,0x00,0x02,0x68,0x00,0x64,0x00,0x00,0x00,0x00,'\0'};
	
	while(1)
	{
		if(message_isConnected == 0)	
		{
			continue;
		}

		if(message_isLogin == 0)
		{
			continue;
		}
		memset(message_info, '\0', 4096);
		if(!construct_packet_head(message_head, GETWAY_TO_SERVER_HEARTBEAT_TYPE))
			continue;
		if(!construct_heartbeat_packet_body(message_body))
			continue;
		
		myUint8cpy(message_info, message_head, 0, MESSAGE_HEAD_LEN);
		myUint8cpy(message_info, message_body, MESSAGE_HEAD_LEN, message_len);
		message_info[message_len+MESSAGE_HEAD_LEN] = 0xcc;
		message_info[message_len+MESSAGE_HEAD_LEN+1] = getUint8BCC(message_info,14, message_len+MESSAGE_HEAD_LEN+1);
		int i;
		printf("************head****\n");
		for(i = 0;i < message_len+MESSAGE_HEAD_LEN+2; i++)
		{
			printf("%x ",message_info[i]);
		}
		printf("\n************body***\n");
		for(i = 0;i < message_len; i++)
		{
			printf("%x ",message_body[i]);
		}
		printf("\n");
		if(!network_write(message_sockfd, message_info, message_len+MESSAGE_HEAD_LEN+2))
		{
			message_isConnected = 0;
			continue;
		}
		else
		{
			printf("\n==================");
			j = 14;
			{
				int n=0,m=0;
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
		sleep(30);	
	}
}





boolean recieve_server_packet()
{
	char t_recieve_info[4096], message_temp[4096], stc_info[1024];
	int t_recieve_len = 0, t_recieve_total_len = 0, j = 0;

	memset(t_recieve_info, '\0', 4096);
	memset(message_temp, '\0', 4096);
	memset(stc_info, '\0', 1024);

	if(message_sockfd == -1)
		return FALSE;
	//read the head and get length
	if(network_read(message_sockfd, t_recieve_info, MESSAGE_HEAD_LEN))	
	{
		//控制包
		if(t_recieve_info[0] == STC)
		{
			for(j=0;j<MESSAGE_HEAD_LEN;j++)
				stc_info[j] = t_recieve_info[j];
			//read the info
			if(network_read(message_sockfd, stc_info+MESSAGE_HEAD_LEN, stc_info[2]-11))	
			{
				memset(t_recieve_info, '\0', 4096);
				//read the what
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
		//心跳包
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
					printf("-----------SERVER_TO_GETWAY_REGIST_TYPE------------%d------------------------------\n",t_return);
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
					printf("---------SERVER_TO_GETWAY_UNREGIST_TYPE---------------%d------------------------------\n",t_return);
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
					printf("---------SERVER_TO_GETWAY_HEARTBEAT_TYPE---------------%d------------------------------\n",t_return);
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
					printf("-------------SERVER_TO_GETWAY_CONTROL------------------------------------\n");
					int nodes = parse_control_packet(t_recieve_info);
					int t_packet_len = parse_message_head(t_recieve_info);
				//	if(!network_recieve_semaphore_v())
				//	{
				//		printf("network_recieve_semaphore_v fail\n");
				//	}
					if(-1 == t_packet_len)
						return FALSE;
					unsigned char led_status_info[4096];
					int info_leek = stc_info[2]+3;
					memset(led_status_info, '\0', 4096);
					myUint8cpy(led_status_info, stc_info, 0, info_leek);
					//控制消息头
					construct_packet_head(led_status_info+info_leek, GETWAY_TO_SERVER_CONTROL);
					led_status_info[info_leek+10] = t_recieve_info[10];
					led_status_info[info_leek+11] = t_recieve_info[11];
					led_status_info[info_leek+12] = t_recieve_info[12];
					led_status_info[info_leek+13] = t_recieve_info[13];
					info_leek += MESSAGE_HEAD_LEN;
					//ret code
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
					//led的信息 ************************there is bad**********************************
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
	printf("//////////////return////////////////////////////////\n");
	for(j = 0;j<122+64*8+2;j++)
	{
		printf("%02x,",_packet->return_info[j]);
	}
	printf("//////////////////////////////////////////////\n");
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

