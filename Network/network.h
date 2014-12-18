#ifndef ____NETWORK_H____
#define ____NETWORK_H____
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h> 
#include "common.h"


typedef enum network_init_type
{
 	FTP,
 	MESSAGE	
}network_init_type;
 
typedef struct network_pthread_arg
{
 	int* sock_fd;
 	struct sockaddr_in* server;
 	volatile unsigned char* is_connect ;
 	boolean* ret_value;		
}network_pthread_arg;

#define CONNECT_RETRY_TIME 60

extern struct sockaddr_in ftp_server_addr ; 
extern struct sockaddr_in message_server_addr ; 

extern int message_sockfd; 
extern int ftp_sockfd;

extern volatile int message_id;
extern volatile unsigned char isLogin ;

extern volatile unsigned char ftp_isConnected;
extern volatile unsigned char message_isConnected;


#define CREATE_DATA_TIME_START 0
#define CREATE_DATA_TIME_END 19

#define GATEWAY_VOLTAGE_START 20
#define GATEWAY_VOLTAGE_END 31

#define GATEWAY_POWER_STATUS_START 32
#define GATEWAY_POWER_STATUS_END 32

#define GATEWAY_TEMPERATURE_START 33
#define GATEWAY_TEMPERATURE_END 42

#define GATEWAY_ELECTRIC_START 43
#define GATEWAY_ELECTRIC_END 52
	
#define GATEWAY_VERSION_START 53
#define GATEWAY_VERSION_END 68

#define GATEWAY_STRATEGY_VERSION_START 69
#define GATEWAY_STRATEGY_VERSION_END 100

#define GATEWAY_ERROR_TYPE_START 101
#define GATEWAY_ERROR_TYPE_END 101

#define LIGHTBOX1_START 102
#define LIGHTBOX1_END 165

#define LIGHTBOX2_START 166
#define LIGHTBOX2_END 229

#define LIGHTBOX3_START 230
#define LIGHTBOX3_END 293

#define LIGHTBOX4_START 294
#define LIGHTBOX4_END	357

#define LIGHTBOX5_START 358
#define LIGHTBOX5_END 421

#define LIGHTBOX6_START 422
#define LIGHTBOX6_END 485

#define LIGHTBOX7_START 486
#define LIGHTBOX7_END 549

#define LIGHTBOX8_START 550
#define LIGHTBOX8_END 613

#define RCV_AND_SND_TIME_OUT 5

int network_init(struct sockaddr_in* _addr, char* ip_info, unsigned short port);

void* networt_connect(void* arg);

boolean network_write(int sockfd, void* regiset_info,int len);
boolean network_read(int sockfd, void* regiset_info,int len);


#endif
