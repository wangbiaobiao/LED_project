#ifndef ____PROTOCOL_PARSE_H____
#define ____PROTOCOL_PARSE_H____
#include "common.h"
#define GETWAY_VERSION "1.6"
#define MESSAGE_SERVER_IP "114.215.196.51" 
#define MESSAGE_SERVER_PORT 5084

#define STC 0x7f

#define SXT_0 0x7e
#define SXT_1 0x68
#define MESSAGE_APP_ID 0x02
#define REGIST_APP_ID 0x00
extern unsigned char protocol_version;

#define MESSAGE_HEAD_LEN 14


#define SERVER_TO_GETWAY_SYS_RESPONSE 1
#define GETWAY_TO_SERVER_REGIST_TYPE 20
#define SERVER_TO_GETWAY_REGIST_TYPE 21

#define GETWAY_TO_SERVER_UNREGIST_TYPE 22
#define SERVER_TO_GETWAY_UNREGIST_TYPE 23

#define GETWAY_TO_SERVER_HEARTBEAT_TYPE 100
#define SERVER_TO_GETWAY_HEARTBEAT_TYPE 101

#define SERVER_TO_GETWAY_CONTROL 300
#define GETWAY_TO_SERVER_CONTROL 301

#define GETWAY_TO_SERVER_EXCEPTION 400
#define SERVER_TO_GETWAY_EXCEPTION 401

#define SERVER_TO_GETWAY_RETCODE_LEN 6

#define CMD_GET_NODE_LED_STATUS 0X22
#define CMD_GET_NODE_LED_VOLTAGE 0X23

#define RECIEVE_TIMEOUT 5000


#define SUCESS 200//			----成功
#define SERVER_EXCEPTION 301//		----服务器异常
#define EMPTY_MSG_HANDLER 302//	----无消息处理器
#define NOT_ENOUGH_BYTES 401//		----字节数不够
#define ETX_ERROR 402//		----ETX错误
#define LRC_ERROR 403//		----LRC错误
#define CREATE_MSG_ERROR 501//		----创建消息错误
#define CREATE_MSG_FACTORY_ERROR 502//	----创建消息工厂错误
#define EMPTY_MSG_RETURN 503//		----空消息返回
#define _2_HAS_BEEN_REGISTERED 506
#define _2_NO_REGISTER 507

typedef struct _return_control_packet
{
	unsigned char return_info[4096];
	int info_len;
}_return_control_packet;

extern int xml_length_type_100[128];
extern int xml_length_type_301[128];
extern int xml_length_type_100_size;
extern int xml_length_type_301_size;
extern char protocl_file_name[128];
extern char strategy_file_name[128];
extern int message_len;
extern int regist_len;	

extern volatile int is_recieve_regist_packet;
extern volatile int is_recieve_unregist_packet;
extern volatile int is_recieve_message_packet;
extern volatile int is_recieve_exception_packet;

extern volatile int  control_type;
extern pthread_t perform_automatic_strategy_pid;

boolean xml_config(char* protocl_file_name);
void* message_init(void* arg);
int get_message_len(unsigned char* info);
int get_message_type(unsigned char* info);
int parse_message_head(unsigned char* info);
boolean send_regist_packet();
boolean send_unregist_packet();
boolean parse_exception_packet(unsigned char* info);
int parse_control_packet(unsigned char* info);
boolean parse_return_code(int type);
boolean parse_heartbeat_packet(unsigned char * info);
boolean parse_regist_packet(unsigned char * info);
boolean parse_unregist_packet(unsigned char * info);
boolean construct_packet_head(unsigned char* message_head, int type);
boolean construct_heartbeat_packet_body(unsigned char* message_body);
void* send_heartbeat_packet(void * arg);
boolean recieve_server_packet();
void* send_abnormal_packet(void * arg);
void* recieve_server_packet_pthread(void * arg);
void* return_control_packet(void * arg);


#endif

