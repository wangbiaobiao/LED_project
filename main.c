#include <regex.h>
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
//有些板子没有ini文件夹，兼容性的代码：
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
        int status,i;
        int cflags = REG_EXTENDED;
        regmatch_t pmatch[2];
        const size_t nmatch = 2;
        regex_t reg;
        const char * pattern = "eth0:off:([0-9]+)";
        char cmdline_buf[256];
        char cmd_str[256];
        FILE *cmdline_fd;

        cmdline_fd = fopen("/proc/cmdline", "r");
        if(cmdline_fd != NULL)
        {
                if(fgets(cmdline_buf,sizeof(cmdline_buf),cmdline_fd) == NULL)
                {
                        printf("cmdline is empty\n");
                }
        }
        fclose(cmdline_fd);

        regcomp(&reg, pattern, cflags);
        status = regexec(&reg, cmdline_buf, nmatch, pmatch, 0);
        if(status == REG_NOMATCH)
                printf("No Match\n");
        else if(status == 0)
        {
                printf("Match:\n");
                for(i = pmatch[1].rm_so; i < pmatch[1].rm_eo; ++i)
                putchar(cmdline_buf[i]);
                printf("\n");
                strcpy(cmd_str, cmdline_buf + pmatch[1].rm_so);
                printf("%s", cmd_str);
        }
        regfree(&reg);
	
	memset(gate_way_number, '\0', 128);
	strcpy(gate_way_number, cmd_str);
	memset(network_number, '\0', 128);
	strcpy(network_number, cmd_str);
	
	char dir_name_info[128][128];
	char t_cmd[128];
	char path[128];	
	memset(dir_name_info, '\0', 128);
	strcpy(dir_name_info,"ledstationconfig");
	printf("gate_way_number is %s\n",gate_way_number);

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
	printf("--------the app  version:%s---------\n",GETWAY_VERSION);
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

	printf("\n\n the time table size is %d====\n\n",GetSize(timetable));

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
	printf("app is runing\n");	
	pthread_join(send_heartbeat_packet_pid,NULL);
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
