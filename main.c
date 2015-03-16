#include <pthread.h>
#include <stdio.h>
#include "protocol_parse.h"
#include "rs485.h"
#include "ini_parse.h"
#include "ftp.h"


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

int len_network_number = 4;
char gate_way_number[128] = {0};
extern char config_ini_name[128];

int ini_dir()
{
        if(opendir("/app/ini") == NULL)
        {   
                printf("init ini dir ok\n");
                mkdir("/app/ini",777);
        }   
        else
        {   
                printf("!!!init ini dir bad\n");
        }   
        return 0;

}

boolean my_parse_ini()
{
	int index = 0;

	boolean t_return = TRUE;
	//get info from cmdline of boot_args
	int cmdline_fd = open("/proc/cmdline",O_RDONLY);
	char dir_name_info[128][128];
	char t_cmdline_buff[256], cmdline_info[512];
	char path[128];
	char t_cmd[128];

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
	memset(network_number, '\0', 128);
	strcpy(network_number, cmdline_info+z+9);//¼ÓÏeth0:off:"  µĳ¤¶È
	memcpy(gate_way_number,gate_way_number+4,len_network_number);
	memcpy(network_number,network_number+4,len_network_number);
	network_number[len_network_number] = '\0';
	gate_way_number[len_network_number] = '\0';
	printf("network_number:%s\n",network_number);
	
	memset(dir_name_info[0], '\0', 128);
	strcpy(dir_name_info[0],"ledstationconfig");
	

	printf("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$gate_way_number$$$$$%s\n",gate_way_number);
	/*if(get_file_from_server(dir_name_info, gate_way_number))
	{
		printf("ftp get file success");
	}
	sprintf(t_cmd,"mv /app/%s /app/ini/",network_number);
	mySystem(t_cmd);
	sleep(2);
	sprintf(path,"/app/ini/%s",network_number);
	parse_ini(path);
	*/
	if(find_new_file(INI_FILE_DIR,config_ini_name))
	{
		sprintf(path,"/app/ini/%s",config_ini_name);
		parse_ini(path);
	}
	else{
		if(get_file_from_server(dir_name_info, gate_way_number))
		{
			printf("ftp get file success");
		sprintf(t_cmd,"mv /app/%s /app/ini/",network_number);
		mySystem(t_cmd);
		sleep(2);
		sprintf(path,"/app/ini/%s",network_number);
		parse_ini(path);
		}
		else
		{
			printf("ini init fail\n");
			exit(0);
		}
		
	}
}
pthread_t perform_automatic_strategy_pid = -1;


int  main(int argc, char * argv[])
{
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
	ini_dir();
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
	if(pthread_create(&perform_automatic_strategy_pid, NULL, perform_automatic_strategy, NULL))
	{
		printf("create pthread error .... \n");
		recieve_server_packet_pid = -1;
	}	
	printf("blue day\n");	
	//pthread_join(send_heartbeat_packet_pid,NULL);
	pthread_join(perform_automatic_strategy_pid,NULL);
	return 0;
}

/*int main(int argc, char * argv[])
{
	pid_t pid;
	pid = fork();
	int status;
	if(pid<0)	
	{		
		fprintf(stderr,"fork error");
	}	
	if(pid == 0)
	{
		l_main();
		
	}else
	{		
		pid=wait(&status);
		//sleep(2*60);
		system("reboot -f");
	}
	
}*/