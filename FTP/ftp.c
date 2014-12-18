#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include "network.h"
#include "ftp.h"
#include <sys/types.h>
#include <sys/stat.h>


extern volatile unsigned char ftp_isConnected ;
extern int ftp_sockfd;
//char ftp_server_ip[32];

struct sockaddr_in ftp_server_addr; 

char file_name[128] = "4.2.2";//must get for xml 

char ftp_path[256] = "/app/";

int ftp_sockfd = -1;

boolean ftp_init()
{
	char welcome_buff[1024];
	umask(0);
	memset(welcome_buff, '\0', 1024);
		
	if((ftp_sockfd =  network_init(&ftp_server_addr, FTP_SERVER_IP, FTP_SERVER_PORT)) != -1)
		PRINTF("ftp_init network_init create socket successful.\n")
	else return FALSE;
		
	if(networt_connect(ftp_sockfd, &ftp_server_addr))
	{
		PRINTF("networt_connect \n");
		if(-1 != read(ftp_sockfd, welcome_buff, 1024))
		{
			printf("welcome_buff :%s \n",welcome_buff);
			if(Service_ready_for_new_user == ascll2int(welcome_buff, 0, 2))
				return TRUE;
		}
	}
	else
		PRINTF("networt_connect fail\n");
	return FALSE;
}
	
int ftp_send_cmd(ftp_cmd_type type, void *arg)
{
	char send_buff[1024], rev_buff[1024];
	memset(send_buff, '\0', 1024);
	
	switch(type)
	{
		case USER:	
		{	
			sprintf(send_buff,"USER %s\r\n",USERNAME);
			if(TRUE == ftp_send_cmd_buff(send_buff, rev_buff))
			{
				//for example : ”331 User name okay, need password.”, so ascll2int rev_buff[0] to rev_buff[2];
				return ascll2int(rev_buff, 0, 2);
			}
			return  FTP_CMD_FAIL;
		}
		case PASS:	
		{	
			sprintf(send_buff,"PASS %s\r\n",PASSWORD);
			if(TRUE == ftp_send_cmd_buff(send_buff, rev_buff))
			{
				return ascll2int(rev_buff, 0, 2);
			}
			return  FTP_CMD_FAIL;
		}
		case SIZE:	
		{	
			char size_info[256];
			char* temp = NULL;
			memset(size_info,'\0',32);
			sprintf(send_buff,"SIZE %s\r\n",(char* )arg);
			if(TRUE == ftp_send_cmd_buff(send_buff, rev_buff))
			{
				//printf("======File_status :%d, strlen(rev_buff)%d\n",ascll2int(rev_buff, 0, 2), strlen(rev_buff));
				if(File_status == ascll2int(rev_buff, 0, 2))
				{
					temp = strchr(rev_buff,'\n');
					*temp = '\0';
					myStrcpy(size_info, rev_buff, 4, strlen(rev_buff) - 4); 
					//printf("====File_status :%s, strlen(rev_buff)%d\n=====",size_info,strlen(rev_buff));
					//printf("====File_status :%d\n",atoi(size_info));
					return atoi(size_info);
				}
			}
			return  FTP_CMD_FAIL;
		}
		case CWD:	
		{			
			sprintf(send_buff,"CWD %s\r\n",(char* )arg);
			if(TRUE == ftp_send_cmd_buff(send_buff, rev_buff))
			{
				return Requested_file_action_okay == ascll2int(rev_buff, 0, 2)?1:FTP_CMD_FAIL;
			}
			return  FTP_CMD_FAIL;
		}
		case PASV:	
		{	
			sprintf(send_buff,"PASV\r\n");
			if(1 == ftp_send_cmd_buff(send_buff, rev_buff))
			{
					if(Entering_Passive_Mode == ascll2int(rev_buff, 0, 2))
					{
						char ip_info[128];
						char ip_info_substring[16][512];
						char *ip_info_start = NULL;
						char *ip_info_end = NULL;
						int ftp_data_sock = 0;
						struct sockaddr_in ftp_data_addr;
						unsigned short port = 0;
						memset(ip_info, '\0', 128);
						memset(ip_info_substring, '\0', sizeof(ip_info_substring));
						if((ip_info_start = strchr(rev_buff,'(')) == NULL) return  FTP_CMD_FAIL;
						if((ip_info_end = strchr(rev_buff,')')) == NULL) return  FTP_CMD_FAIL;	
							
						myStrcpy(ip_info, ip_info_start+1, 0, ip_info_end - ip_info_start-1); 
						printf("ip_info:%s\n", ip_info);
						mySplit( ip_info_substring, ip_info, ',', strlen(ip_info));

						printf("ip_info:%s,%s\n", ip_info_substring[4],ip_info_substring[5]);
						port = atoi(ip_info_substring[4])*256+atoi(ip_info_substring[5]);
						printf("port :%d\n",port);
						if((ftp_data_sock =  network_init(&ftp_data_addr,FTP_SERVER_IP, port)) != -1)
							PRINTF("ftp_data_sock network_init create socket successful.\n")
						else return  FTP_CMD_FAIL;
							
						if(networt_connect(ftp_data_sock,&ftp_data_addr))
						{
							PRINTF("ftp_data_connect success\n");
							//while(1);
							return ftp_data_sock;
						}	
						return  FTP_CMD_FAIL;
					}
			}
			return  FTP_CMD_FAIL;
		}
		case PORT:	
		{	

		}
		case RETR:	
		{	
			sprintf(send_buff,"RETR %s\r\n",(char* )arg);
			if(TRUE == ftp_send_cmd_buff(send_buff, rev_buff))
			{
				return File_status_okay == ascll2int(rev_buff, 0, 2)?1:FTP_CMD_FAIL;
			}
			return  FTP_CMD_FAIL;
		}
		case STOR:	
		{	

		}
		case REST:	
		{	

		}
		case QUIT:	
		{	
			sprintf(send_buff,"QUIT\r\n");
			if(TRUE == ftp_send_cmd_buff(send_buff, rev_buff))
			{
				return Service_closing_control_onnection == ascll2int(rev_buff, 0, 2)?1:FTP_CMD_FAIL;
			}
			return  FTP_CMD_FAIL;
		}		
	}
	return  FTP_CMD_FAIL;
}

boolean ftp_send_cmd_buff(const char* send_buff, char* ret_str)
{
	int send_len  = strlen(send_buff), has_sended_len = 0;
	while(1)
	{
		has_sended_len += write(ftp_sockfd, send_buff,send_len - has_sended_len);
		if(has_sended_len == -1)
		{
			PRINTF("socket write fail \n");
			return FALSE;
		}
		if(has_sended_len == send_len)
		{
//			printf("socket write success:len = %d, %s\n",has_sended_len,send_buff);//1013
			break;
		}
	}
	memset(ret_str,'\0',1024);
	if(-1 == read(ftp_sockfd, ret_str, 1024))
	{
		PRINTF("socket read fail \n");
		return FALSE;	
	}
	else
	{
		_DEBUG_(printf("socket read success:%s\n",ret_str);)
		return TRUE;
	}
}

boolean get_file_from_server(char (*dir_name)[512], char *file_name)
{
	int ftp_data_sock = -1;
	int recv_len = -1;
	int try_times = 0;
	printf("get_file_from_server,dir:%s,file_name:%s\n",dir_name[0], file_name);
	if(TRUE == ftp_init())
	{
		PRINTF("connect ftp successful \n");	
		int ret_value = ftp_send_cmd(USER,NULL);
//		printf("ret val:%d\n",ret_value);
		if(User_logged_in == ret_value)
		{
			PRINTF("not password,login success \n");		 	
		}
		else if (User_name_okay == ret_value)
		{
			if(User_logged_in == ftp_send_cmd(PASS,NULL))
				PRINTF("login success \n")
			else
			{
				PRINTF("login fail \n");
				return FALSE;
			}
		}
		else
		{
			PRINTF("login fail! \n");
			return FALSE;
		}
		if((ftp_data_sock = ftp_send_cmd(PASV,NULL)) == FTP_CMD_FAIL)
		{

			PRINTF("PASV fail! \n");
			return FALSE;
		}
		int dir_count = 1, i = 0;//strlen((const char*)dir_name)
		for(i=0; i<dir_count; i++)
		{			
			printf("dir_count:%s\n",dir_name[i]);
			if(FTP_CMD_FAIL == ftp_send_cmd(CWD,dir_name[i]))
			{
				return FALSE;
			}
		}
		
		int file_size = ftp_send_cmd(SIZE, file_name);
		if(FTP_CMD_FAIL != file_size)
			printf("%s size:%d\n",file_name, file_size);
		else
			return FALSE;
		if(ftp_send_cmd(RETR, file_name)!=FTP_CMD_FAIL)
		{
			int has_rev_len = 0, has_write_len = 0;
			char * file_buff = (char *)malloc(4096);
			
			char open_file[256];
			memset(open_file,'\0',256);
			strcat(open_file, ftp_path);
			strcat(open_file, file_name);
//			int flags = fcntl(ftp_data_sock,F_GETFL,0); 
			fcntl(ftp_data_sock,F_GETFL,0); 
//			fcntl(ftp_data_sock, F_SETFL, flags | O_NONBLOCK);
			PRINTF("open save_file_fd\n");
			int save_file_fd = open(open_file,O_RDWR|O_CREAT|O_TRUNC,0777);
			if(save_file_fd < 0 ) 
			{
				PRINTF("open save_file_fd fail\n");
				free(file_buff);
//				fcntl(ftp_data_sock,F_SETFL,flags & ~O_NONBLOCK); 
				return FALSE;
			}
			
			while(1)
			{ 
				memset(file_buff,'\0',4096);
//				struct timeval tv; 
//			    //select等待I/o的最长时间 10s
//			    tv.tv_sec = FTP_RCV_TIMEOUT; 
//			    tv.tv_usec = 0; 
//			    fd_set wset; 
//			    FD_ZERO(&wset); 
//			    FD_SET(ftp_data_sock,&wset); 
//			    int ret_no = select(ftp_data_sock+1,NULL,&wset,NULL,&tv); 
//			    if(ret_no < 0) 
//			    { // select出错 
//			        printf("select fail \n"); 
//					free(file_buff);
//					fcntl(ftp_data_sock,F_SETFL,flags & ~O_NONBLOCK); 
//					if(close(save_file_fd) == -1) printf("close file fail\n");
//					return FALSE;
//			    } 
//			    else if (0 == ret_no) 
//			    { // 超时 
//			        printf("Timeout.\n" );
//					free(file_buff);
//					fcntl(ftp_data_sock,F_SETFL,flags & ~O_NONBLOCK); 
//					if(close(save_file_fd) == -1) printf("close file fail\n");
//					return FALSE;
//			    } 
//			    else 
//			    {
					recv_len = read(ftp_data_sock, file_buff, 4096);
					if(recv_len == -1)
					{
						PRINTF("socket read fail \n");
						free(file_buff);
//						fcntl(ftp_data_sock,F_SETFL,flags & ~O_NONBLOCK); 
						return FALSE;
					}
					has_rev_len += recv_len;
					while(1)
					{
						has_write_len += write(save_file_fd, file_buff, has_rev_len - has_write_len);
						if(has_write_len == -1)
						{
							PRINTF("file write fail \n");
							free(file_buff);
//							fcntl(ftp_data_sock,F_SETFL,flags & ~O_NONBLOCK); 
							if(close(save_file_fd) == -1) PRINTF("close file fail\n");
							return FALSE;
						}
						if(has_write_len == has_rev_len)
						{
							DEBUG(printf("file write success:len = %d, %s\n",has_rev_len,file_buff);)
							break;
						}
					}
					
					if(has_rev_len == file_size)
					{
						char recvbuff[128];
						memset(recvbuff, '\0', 128);
//						printf("socket read success:len = %d, %s\n",has_rev_len,file_buff);
						free(file_buff);
//						fcntl(ftp_data_sock,F_SETFL,flags & ~O_NONBLOCK); 
						if(close(save_file_fd) == -1) PRINTF("close file fail\n");
						
						close(ftp_data_sock);
//						printf("================\n");
//						fcntl(ftp_sockfd, F_SETFL, flags | O_NONBLOCK);
//						usleep(1000);
//						int read_len = read(ftp_sockfd, recvbuff, 128);
//						fcntl(ftp_sockfd,F_SETFL,flags & ~O_NONBLOCK);
						int read_len = network_read(ftp_sockfd, recvbuff, 19);
						if(read_len > 0)
						{
							printf("========%s========\n",recvbuff);
						  	if(Closing_data_connection != ascll2int(recvbuff, 0, 2))
								PRINTF("close connect fail\n");
						}
						break;
					}
					else 
					{

						if(recv_len == 0)
						{
							try_times++;
							if(try_times == 5000)
							{
								try_times = 0;
								free(file_buff);
								return FALSE;
							}
							printf("ftp recivece try_times:%d\n", try_times);
						}
					}
						
//			    }
			}	
		}
		else
		{	
			return FALSE;
		}
		if(ftp_send_cmd(QUIT,NULL) == FTP_CMD_FAIL)
		{
			PRINTF("QUIT fail! \n");
//			return FALSE;//11-01
		}
		else
		{
			PRINTF("QUIT success! \n");
		}
		close(ftp_sockfd);
		ftp_isConnected = 0;
		return TRUE;
	}
	else
	{
		PRINTF("connect ftp server fail \n");
		return FALSE;
	}
}
