#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <pthread.h>
#include "network.h"
#include "ftp.h"

int ftp_port_mode = 0x00;;
#define FTP_PORT_MODE_LIMIT 250
extern volatile unsigned char ftp_isConnected ;
extern int ftp_sockfd;

struct sockaddr_in ftp_server_addr; 

char file_name[128] = "4.2.2";//must get for xml 

//char ftp_path[256] = "/home/zhengsh/test/led_ftp";
char ftp_path[256] = "/app/";

int ftp_sockfd = -1;

boolean ftp_init()
{
	char welcome_buff[1024];
	umask(0);
	memset(welcome_buff, '\0', 1024);
	boolean ret_value = FALSE;
	if(ftp_sockfd > 0)
		close(ftp_sockfd);
	if((ftp_sockfd =  network_init(&ftp_server_addr, FTP_SERVER_IP, FTP_SERVER_PORT)) != -1)
		printf("network_init create socket successful.\n");
	else return FALSE;
		
	network_pthread_arg arg;
	arg.sock_fd = &ftp_sockfd;
	ftp_isConnected = 0;
	arg.is_connect = &ftp_isConnected;
	arg.server = &ftp_server_addr;	
	arg.ret_value = &ret_value;
	
	networt_connect((void *)(&arg));
	if(TRUE == ret_value)
	{
		printf("networt_connect \n");
		if(-1 != read(ftp_sockfd, welcome_buff, 1024))
		{
			printf("welcome_buff :%s \n",welcome_buff);
			if(Service_ready_for_new_user == ascll2int(welcome_buff, 0, 2))
				return TRUE;
		}
	}
	else
		printf("networt_connect fail\n");
	return FALSE;
}

int accept_timeout(int fd, struct sockaddr *addr, int *addrlen, int time)
{
  int ret;
  if(time > 0) {
    fd_set rSet;
    FD_ZERO(&rSet);
    FD_SET(fd, &rSet);

    struct timeval timeout;
    timeout.tv_sec = time;
    timeout.tv_usec = 0;

    int selectRet;
    do {
      selectRet = select(fd + 1, &rSet, NULL, NULL, &timeout);
    }while(selectRet < 0 && selectRet == EINTR);
    if(selectRet < 0 ) {
      return -1;
    } else if(selectRet == 0) {
      errno = ETIMEDOUT;
      return -1;
    }	
  }
  if(addr) {
    ret = accept(fd, (struct sockaddr *)addr, addrlen);
  } else {
    ret = accept(fd, NULL, NULL);
  }
  return ret;
}
	
int ftp_send_cmd(ftp_cmd_type type, void *arg)
{
	char send_buff[1024], rev_buff[1024], cmd_buff[4];
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
						char ip_info_substring[128][64];
						char *ip_info_start = NULL;
						char *ip_info_end = NULL;
						int ftp_data_sock = 0;
						volatile unsigned char ftp_data_isConnected = 0;
						struct sockaddr_in ftp_data_addr;
						unsigned short port = 0;
						boolean ret_value = FALSE;
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
							printf("network_init create socket successful.\n");
						else return  FTP_CMD_FAIL;
							
						network_pthread_arg arg;
						arg.sock_fd = &ftp_data_sock;
						arg.is_connect = &ftp_data_isConnected;
						arg.server = &ftp_data_addr;	
						arg.ret_value = &ret_value;
						
						networt_connect((void *)(&arg));
						
						if(TRUE == ret_value)
						{
							printf("ftp_data_connect success\n");
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
			int p1 = 0x76;//设置/* 客户端开始监听端口p1*256+p2 * =  3501
			int ftp_server_sock = -1;
			int x = 0;
			struct sockaddr_in ftp_data_addr;
			unsigned short client_port = 0;
			struct   ifreq ifr_ip;
			struct   sockaddr_in *sin; 
			struct  sockaddr_in client_name;
   			char ipaddr[128];  
   			int length = sizeof(struct  sockaddr_in);
			memset(&ifr_ip, 0, sizeof(ifr_ip));  
			memset(ipaddr, 0, sizeof(ipaddr));     
  			strncpy(ifr_ip.ifr_name, "eth0", sizeof(ifr_ip.ifr_name) - 1);    
			
			
			if((ftp_server_sock = socket(AF_INET, SOCK_STREAM, 0)) != -1)
				printf("ftp PORT create socket successful.\n");
			else
			{
				printf("ftp PORT create socket fail.\n");
				return FTP_CMD_FAIL;
			}
			
			if(ioctl(ftp_server_sock, SIOCGIFADDR, &ifr_ip) < 0 )     
		    {     
				printf("ftp PORT ioctl socket fail.\n");
				return FTP_CMD_FAIL;    
		    } 
		    sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;  
    		strcpy(ipaddr,inet_ntoa(sin->sin_addr));  
		    printf("local ip:%s \n", ipaddr);     
		    for(x=0; x<strlen(ipaddr); x++)
		    {
		    	if(ipaddr[x] == '.')
		    		ipaddr[x] = ',';
		    }
		    
		    
		    
			ftp_data_addr.sin_family = AF_INET;
			ftp_data_addr.sin_addr.s_addr = htons(INADDR_ANY);
			client_port = p1*256+ftp_port_mode;
  
			ftp_data_addr.sin_port = htons(client_port);
			
		/*	int option=1;
			setsockopt(ftp_data_sock,SOL_SOCKET,SO_REUSEADDR,(char *)&option,sizeof(option));
			struct linger li;
			li.l_onoff=1;
			li.l_linger=1;
			setsockopt(ftp_data_sock,SOL_SOCKET,SO_LINGER,(char *)&li,sizeof(li));*/
			
			if(-1 == bind(ftp_server_sock, (struct sockaddr *)&ftp_data_addr, sizeof(struct sockaddr_in)))
			{
				printf("ftp PORT bind fail.\n");
				close(ftp_server_sock);
				return FTP_CMD_FAIL; 
			}
			
			/* 客户端开始监听端口p1*256+p2 */
			if(-1 == listen(ftp_server_sock, 10))
			{
				printf("ftp PORT listen fail.\n");
				close(ftp_server_sock);
				return FTP_CMD_FAIL; 
			}
			
			sprintf(send_buff,"PORT %s,%d,%d\r\n", ipaddr, p1, ftp_port_mode);
			
			
			ftp_port_mode++;
			if(ftp_port_mode >  FTP_PORT_MODE_LIMIT)
				ftp_port_mode = 0;
				
			if(ftp_send_cmd_buff(send_buff, rev_buff))
			{
					if(Command_okay != ascll2int(rev_buff, 0, 2))
					{
						printf("ftp PORT server not answer 200.\n");
						close(ftp_server_sock);
						return  FTP_CMD_FAIL;
					}
					else
					{
						printf("ftp PORT server answer OK.\n");
						return  ftp_server_sock;
					}
			}
			close(ftp_server_sock);
			return  FTP_CMD_FAIL;

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
}

boolean ftp_send_cmd_buff(const char* send_buff, char* ret_str)
{
	int send_len  = strlen(send_buff), has_sended_len = 0;
	while(1)
	{
		has_sended_len += write(ftp_sockfd, send_buff,send_len - has_sended_len);
		if(has_sended_len == -1)
		{
			printf("socket write fail \n");
			return FALSE;
		}
		if(has_sended_len == send_len)
		{
			printf("socket write success:len = %d, %s\n",has_sended_len,send_buff);
			break;
		}
	}
	memset(ret_str,'\0',1024);
	if(-1 == read(ftp_sockfd, ret_str, 1024))
	{
		printf("socket read fail \n");
		return FALSE;	
	}
	else
	{
		printf("socket read success:%s\n",ret_str);
		return TRUE;
	}
}

boolean get_file_from_server(char (*dir_name)[128], char *file_name)
{
	int ftp_data_sock = -1;
	int ftp_server_sock = -1;
	printf("get_file_from_server\n");
	struct  sockaddr_in client_name;
	int length = sizeof(struct  sockaddr_in);
	if(TRUE == ftp_init())
	{
		printf("connect ftp successful \n");	
		int ret_value = ftp_send_cmd(USER,NULL);
		printf("ret val:%d\n",ret_value);
		if(User_logged_in == ret_value)
		{
			printf("not password,login success \n");		 	
		}
		else if (User_name_okay == ret_value)
		{
			if(User_logged_in == ftp_send_cmd(PASS,NULL))
				printf("login success \n");
			else
			{
				printf("login fail \n");
				return FALSE;
			}
		}
		else
		{
			printf("login fail! \n");
			return FALSE;
		}
		
//		if((ftp_data_sock = ftp_send_cmd(PASV,NULL)) == FTP_CMD_FAIL)//被动模式
		//主动模式
		if((ftp_server_sock = ftp_send_cmd(PORT,NULL)) == FTP_CMD_FAIL)
		{

			printf("PORT fail! %d\n", ftp_server_sock);
			return FALSE;
		}
		printf("====PORT success! ftp_server_socket:%d\n", ftp_server_sock);
		
		
		int dir_count = 1, i = 0;//strlen((const char*)dir_name)
		for(i=0; i<dir_count; i++)
		{			
			printf("dir_count:%s\n",dir_name[i]);
			if(FTP_CMD_FAIL == ftp_send_cmd(CWD,dir_name[i]))
			{
				close(ftp_server_sock);
				return FALSE;
			}
		}
		
		int file_size = ftp_send_cmd(SIZE, file_name);
		if(FTP_CMD_FAIL != file_size)
			printf("%s size:%d\n",file_name, file_size);
		else
		{
			close(ftp_server_sock);
			return FALSE;
		}
		
		if(ftp_send_cmd(RETR, file_name)!=FTP_CMD_FAIL)
		{
			int has_rev_len = 0, has_write_len = 0;
			char * file_buff = (char *)malloc(4096);
			
			char open_file[256];
			memset(open_file,'\0',256);
			strcat(open_file, ftp_path);
			strcat(open_file, file_name);
			
			if((ftp_data_sock = accept_timeout(ftp_server_sock, (struct sockaddr *)&client_name, &length, 10)) < 0)
			{
				printf("ftp accept fail:%s\n", strerror(errno));
				close(ftp_server_sock);
				return FALSE;
			}
			printf("ftp accept success\n");
			
//			int flags = fcntl(ftp_data_sock,F_GETFL,0); 
//			fcntl(ftp_data_sock, F_SETFL, flags | O_NONBLOCK);
			printf("open save_file_fd\n");
			int save_file_fd = open(open_file,O_RDWR|O_CREAT|O_TRUNC,0777);
			if(save_file_fd < 0 ) 
			{
				printf("open save_file_fd fail\n");
				free(file_buff);
				close(ftp_server_sock);
				close(ftp_data_sock);
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
					has_rev_len += read(ftp_data_sock, file_buff, 4096);
					if(has_rev_len == -1)
					{
						char recvbuff[128];
						memset(recvbuff, '\0', 128);
						printf("socket read fail: %s\n", strerror(errno));
						free(file_buff);
						close(ftp_server_sock);
						close(ftp_data_sock);
//						fcntl(ftp_data_sock,F_SETFL,flags & ~O_NONBLOCK); 

						return FALSE;
					}
					while(1)
					{
						has_write_len += write(save_file_fd, file_buff, has_rev_len - has_write_len);
						if(has_write_len == -1)
						{
							printf("file write fail \n");
							free(file_buff);
//							fcntl(ftp_data_sock,F_SETFL,flags & ~O_NONBLOCK); 
							if(close(save_file_fd) == -1) printf("close file fail\n");
							
							close(ftp_server_sock);
							close(ftp_data_sock);
							return FALSE;
						}
						if(has_write_len == has_rev_len)
						{
							printf("file write success:len = %d, %s\n",has_rev_len,file_buff);
							
							break;
						}
					}
					
					if(has_rev_len == file_size)
					{
						char recvbuff[128];
						memset(recvbuff, '\0', 128);
						printf("socket read success:len = %d, %s\n",has_rev_len,file_buff);
						free(file_buff);
//						fcntl(ftp_data_sock,F_SETFL,flags & ~O_NONBLOCK); 
						if(close(save_file_fd) == -1) printf("close file fail\n");
						
						close(ftp_data_sock);
						close(ftp_server_sock);
						printf("================\n");
//						fcntl(ftp_sockfd, F_SETFL, flags | O_NONBLOCK);
//						usleep(1000);
//						int read_len = read(ftp_sockfd, recvbuff, 128);
//						fcntl(ftp_sockfd,F_SETFL,flags & ~O_NONBLOCK);
						int read_len = network_read(ftp_sockfd, recvbuff, 24);						
						printf("========%s========\n",recvbuff);
						
						if(read_len != -1)
						{
						  	if(Closing_data_connection != ascll2int(recvbuff, 0, 2))
								printf("close connect fail\n");
						}
						break;
					}
//			    }
			}	
		}
		else
		{	
			close(ftp_server_sock);
			close(ftp_data_sock);
			return FALSE;
		}
		if(ftp_send_cmd(QUIT,NULL) == FTP_CMD_FAIL)
		{
			printf("QUIT fail! \n");
	//		return FALSE;
		}
		close(ftp_sockfd);
		close(ftp_server_sock);
		close(ftp_data_sock);
		ftp_isConnected = 0;
		return TRUE;
	}
	else
	{
		printf("connect ftp server fail \n");
		close(ftp_server_sock);
		close(ftp_data_sock);
		return FALSE;
	}
}
