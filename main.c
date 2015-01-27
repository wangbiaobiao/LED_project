#include <pthread.h>
#include <stdio.h>
#include "protocol_parse.h"
#include "rs485.h"
#include "ini_parse.h"
#include "ftp.h"

char gate_way_number[128];
//void* check_status(void *arg)
//{
//	while(1)
//	{
//		set_Time(startDate,endDate, startTime,endTime);
//		sleep(60);
//	}
//}
//void* new_pth(void *arg)
//{
//	char revbuff[1024],serialbuff[1024];
//	int myflag = pthread_flag, nBytes = 0;
//	cmd_packet cmd;
//	char splitBuff[5][100];
//	pthread_arg *_arg = (pthread_arg *)arg;
//	int err = -1, i = 0;
//	printf("new_pth\n");
//	while(pthread_flag == myflag)
//	{
//		memset(revbuff,'\0',1024);
//		if((nBytes = read(_arg->newsock, revbuff, 1024)) == -1)
//		{
//			fprintf(stderr,"read error:%s\n",strerror(errno));
//			last_arg = NULL;
//			pthread_exit((void *)err);
//		}
//		revbuff[nBytes] = '\0';
//		
//	//	unsigned char buff[100];
//	//	while(1)
//	//	{
//	//		RS485_RECV();
//	//		serial_read(_arg->fd,buff,100);
//		if(nBytes >  0)
//		{
//			printf("==revbuff : %s \n",revbuff);
//		  	int splitLen= mySplit(splitBuff,revbuff,'x',nBytes);
//			printf("===len:%d=====\n",splitLen);
//		   	for(i=0; i<splitLen; i++)
//			{
//				printf("splitLen%d:%s\n",i,splitBuff[i]);	
//			}
//			if(strcmp(splitBuff[0],"00") == 0)
//			{
//				printf("========\n");
//				if(calibrateTime(splitBuff[1]) == 1)
//				{
//					if((nBytes = write(_arg->newsock, "00x100", 6)) == -1)
//					{
//						fprintf(stderr,"read error:%s\n",strerror(errno));
//						last_arg = NULL;
//						pthread_exit((void *)err);
//					}						
//					printf("calibrateTime:%d\n",nBytes);
//				}
//				else
//				{
//					if((nBytes = write(_arg->newsock, "00x200", 6)) == -1)
//					{
//						fprintf(stderr,"read error:%s\n",strerror(errno));
//						last_arg = NULL;
//						pthread_exit((void *)err);
//					}					
//				}
//			}
//			else if(strcmp(splitBuff[0],"01") == 0)
//			{
//				memset(_set_time,'\0',sizeof(_set_time));
//				if(setTime_select(_set_time) == 0)
//				{
//					printf("setTime_select fail\n");	
//				}
//				myStrcpy(startDate, splitBuff[1], 0, 10);
//				myStrcpy(startTime, splitBuff[1], 11, 19);
//				myStrcpy(endDate, splitBuff[2], 0, 10);
//				myStrcpy(endTime, splitBuff[2], 11, 19);	
//				set_Time(startDate,endDate, startTime,endTime);
//				strcpy(_set_time[0].startTime, splitBuff[1]);
//				strcpy(_set_time[0].endTime, splitBuff[2]);
//				
//				if(setTime_update(_set_time[0].recordID,_set_time) == 0)
//				{
//					printf("setTime_update fail\n");	
//					if((nBytes = write(_arg->newsock, "01x200", 6)) == -1)
//					{
//						fprintf(stderr,"read error:%s\n",strerror(errno));
//						last_arg = NULL;
//						pthread_exit((void *)err);
//					}					
//				}else{
//					if((nBytes = write(_arg->newsock, "01x100", 6)) == -1)
//					{
//						fprintf(stderr,"read error:%s\n",strerror(errno));
//						last_arg = NULL;
//						pthread_exit((void *)err);
//					}					

//				}
//			}			
//			
//			else if(strcmp(splitBuff[0],"02") == 0)
//			{
//				reboot();
//			}
//			else 
//			{
//				sendCmd(&cmd, revbuff, _arg->fd);	
//			}
//		}
//	}
//}

boolean my_parse_ini()
{
	int index = 0;

	boolean t_return = TRUE;
	//get info from cmdline of boot_args
	int cmdline_fd = open("/proc/cmdline",O_RDONLY);
	char t_cmdline_buff[256], cmdline_info[512];

	memset(network_number, '\0', 16);
	if(cmdline_fd == -1)
		return FALSE;
	memset(cmdline_info, '\0', 512);
	memset(t_cmdline_buff, '\0', 256);
	if(read(cmdline_fd, t_cmdline_buff,256)>0)
	{
	
	strcat(cmdline_info,t_cmdline_buff);
		memset(t_cmdline_buff, '\0', 256);
	}
	printf("network_number:%s,%d\n",cmdline_info,strlen(cmdline_info));
	int t_len = strlen(cmdline_info), z = 0;
	for(z=0; z<t_len-15; z++)	
	{
		if(cmdline_info[z] == 'e' && cmdline_info[z+1] == 't'&&
		cmdline_info[z+2] == 'h' && cmdline_info[z+3] == '0')
		 break;
	}
	if(t_len-15 == z)
		return FALSE;
	printf("=========%c============",cmdline_info[z]);
	if( t_len < z+15)
		return FALSE;
	
	memset(gate_way_number, '\0', 128);
	strcpy(gate_way_number, cmdline_info+z+9);//¼ÓÏeth0:off:"  µĳ¤¶È
	gate_way_number[6] = '\0';
	printf("network_number:%s\n",network_number);
	
	printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$gate_way_number$$$$$%s\n",gate_way_number);
	/*if(get_file_from_server(dir_name_info, network_number))
	{
		printf("ftp get file success");
	}

	parse_ini(network_number);*/


}
pthread_t perform_automatic_strategy_pid = -1;

int main(int argc, char * argv[])
{
	//if(argc < 2)
//	{
//		printf("please input IP!!~~");
//		return -1;
//	}
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	printf("------I am version:%s---------\n",GETWAY_VERSION);
	
	my_parse_ini();
	if(!semaphore_init())
	{
		printf("semaphore_init fail\n");	
	}
	if(!set_semvalue())
		printf("set_semvalue fail\n");

	if(!rs485_init())
		printf("rs485_init failed\n");	
	
	if(!config_init())
		printf("config_init failed\n");	
	
	printf("\n\n==================GetSize:%d======================\n\n",GetSize(timetable));
	
	pthread_t send_heartbeat_packet_pid, recieve_server_packet_pid, message_init_pid; 
	if(pthread_create(&message_init_pid, NULL, message_init, NULL))
	{
		printf("create message_init pthread error .... \n");
	}
	
	if(pthread_create(&send_heartbeat_packet_pid, NULL, send_heartbeat_packet, NULL))
	{
		printf("create send_heartbeat_packet pthread error .... \n");
	}			
	if(pthread_create(&recieve_server_packet_pid, NULL, recieve_server_packet_pthread, NULL))
	{
		printf("create recieve_server_packet pthread error .... \n");
	}
	/*if(pthread_create(&perform_automatic_strategy_pid, NULL, perform_automatic_strategy, NULL))
	{
		printf("create pthread error .... \n");
		recieve_server_packet_pid = -1;
	}*/	
	printf("blue day\n");	
	pthread_join(send_heartbeat_packet_pid,NULL);
	pthread_join(perform_automatic_strategy_pid,NULL);
		
//	setTime_init();
//	memset(_set_time,'\0',sizeof(_set_time));
//	if(setTime_select(_set_time) == 0)
//	{
//		printf("setTime_select fail\n");	
//	}
//	if(0 == strlen(_set_time[0].startTime))
//	{
//		settime tmp;
//		memset(&tmp,'\0',sizeof(tmp));
//		strcpy(tmp.startTime, "2013-06-05-02:00:00");
//		strcpy(tmp.endTime, "2013-08-02-10:20:10");
//		setTime_insert(&tmp);
//	}
//	printf("startD:%s\n",_set_time[0].startTime);
//	myStrcpy(startDate, _set_time[0].startTime, 0, 10);
//	myStrcpy(startTime, _set_time[0].startTime, 11, 19);
//	myStrcpy(endDate, _set_time[0].endTime, 0, 10);
//	myStrcpy(endTime, _set_time[0].endTime ,11, 19);	
//	set_Time(startDate,endDate, startTime,endTime);
//	arg.pin_idx = 0;
//	printf("fd_pio:%d\n",fd_pio);
//	struct ifaddrs * ifAddrStruct=NULL;
//	void * tmpAddrPtr=NULL;
//	getifaddrs(&ifAddrStruct);
//	char addressBuffer[INET_ADDRSTRLEN];
//				            
//	while (ifAddrStruct!=NULL) {
//		memset(addressBuffer,'\0',sizeof(addressBuffer));
//		if (ifAddrStruct->ifa_addr->sa_family==AF_INET) { // check it is IP4
//			// is a valid IP4 Address
//			if(strcmp(ifAddrStruct->ifa_name,"eth0") == 0)
//			{
//				tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
//				inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
//				break;
//			}
//		}
//		ifAddrStruct=ifAddrStruct->ifa_next;
//	}
//	printf("address:%s\n",addressBuffer);
//	struct sockaddr_in client_addr;
//	int sin_size = sizeof(struct sockaddr_in);
//	int server_fd = network_init(addressBuffer);
//	int create_pthread_flag = 0;
//	pthread_t check_status_pth;
//	unsigned char buff[1024];
//	unsigned char b[1024] = {0x4a,0x54,0x01,0x06,0x02,0x1b};
//	int z = 0, len = 0;
//	while(1)
//	{
//		RS485_SEND();
//		for(z=0; z<6; z++)
//		{
//			serial_write(_fd,b+z,1);
//		}
//		usleep(1000*10);
//		RS485_RECV();
	//	while((len = serial_read(_fd,buff,1024))>0);
	//	{
	//		if(len > 0)
	//		{
	//			printf("len:%d\n",len);
	//			for(z=0; z<len; z++)
	//			{
	//				printf("%02X,",buff[z]);
	//			}
	//			printf("\n");
	//			break;
	//		}
	//	}
//		usleep(1000*50);
//		getchar();
//	}
//	while(1)
//	{
//		printf("start accept\n");
//		pthread_arg *arg = (pthread_arg *)malloc(sizeof(pthread_arg));
//		arg->fd = _fd;
//		
//		if((arg->newsock = accept(server_fd,(struct sockaddr *)(& client_addr),(socklen_t *)(&sin_size))) == -1)
//		{
//			fprintf(stderr,"accept error:%s\n",strerror(errno));
//			exit(1);
//		}
//		fprintf(stderr,"Server get connection from :%s\n", (char *)inet_ntoa(client_addr.sin_addr));
//		//write(arg->newsock,"hello",6);
//		pthread_flag = pthread_flag == 1?0:1;
//		if(last_arg != NULL)
//		{
//			printf("close clienSocket \n");
//			close(last_arg->newsock);
//			free(last_arg);
//			last_arg = NULL;
//		}
//		if(pthread_create(&arg->pth_id,NULL,new_pth,(void *)arg)!=0)
//  		{
//       			printf("create pthread error .... \n");
//			exit(1);
//   		}			
//		if(create_pthread_flag == 0)
//		{
//		if(pthread_create(&check_status_pth,NULL,check_status,NULL)!=0)
//  		{
//       			printf("create pthread error .... \n");
//			exit(1);
//   		}
//		create_pthread_flag = 1;
//		}
//		last_arg = arg;
//	}
	return 0;
}	
