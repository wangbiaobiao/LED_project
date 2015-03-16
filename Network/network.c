#include "network.h"

volatile int message_id = 0;
volatile unsigned char message_isLogin = 0;

volatile unsigned char ftp_isConnected = 0;
volatile unsigned char message_isConnected = 0;

extern int fd_pio;
extern int rs485_fd;
extern int ftp_sockfd;
extern int message_sockfd;


int network_init(struct sockaddr_in* _addr, char* ip_info, unsigned short port)
{
	/* 客户程序开始建立 sockfd描述符 */ 
	printf("network_init \n");
	int sock_fd = -1;
	if((sock_fd = socket(AF_INET,SOCK_STREAM,0))==-1) // AF_INET:Internet;SOCK_STREAM:TCP
	{ 
		fprintf(stderr,"Message Socket Error:%s\a\n",strerror(errno)); 
		return  FALSE;
	} 	
	struct timeval timeout;      
    timeout.tv_sec = RCV_AND_SND_TIME_OUT;
    timeout.tv_usec = 0;
	/* 设置套接字描述符 */ 
    if (setsockopt (sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        printf("setsockopt SO_RCVTIMEO failed\n");

    if (setsockopt (sock_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        printf("setsockopt SO_SNDTIMEO failed\n");
	struct linger t_linger = {.l_onoff=0,.l_linger=0};
	if (setsockopt( sock_fd, SOL_SOCKET, SO_LINGER, &t_linger, sizeof(t_linger)) < 0)
		printf("setsockopt SO_DONTLINGER failed\n");

	bzero(_addr,sizeof(_addr)); // 初始化,置0
	_addr->sin_family=AF_INET;  // IPV4
	_addr->sin_port=htons(port);// (将本机器上的short数据转化为网络上的short数据)端口号
	_addr->sin_addr.s_addr=inet_addr(ip_info); // IP地址

	return sock_fd;
}

void* networt_connect(void* arg)
{
	network_pthread_arg* pthread_arg = (network_pthread_arg*)arg; 
	*(pthread_arg->ret_value) = 0;
	int fd = *(pthread_arg->sock_fd);
//	int flags = fcntl(fd,F_GETFL,0); 
//	fcntl(fd,F_SETFL,flags & ~O_NONBLOCK);  // 设为阻塞模式 
	printf("is_connect:%d\n",*(pthread_arg->is_connect));
	if(*(pthread_arg->is_connect)  == 0)
	{
		while(1)
		{
			if(connect(fd, (struct sockaddr *)(pthread_arg->server), sizeof(struct sockaddr)) == -1) 
			{
			}
			else
			{
				printf("connect\n");
				break;
			}
		}
		*(pthread_arg->is_connect) = 1; 
		      /* 连接成功了 */ 
		printf("networt_connect success\n");
		*(pthread_arg->ret_value) = 1;
	}
}


boolean network_write(int sockfd, void* info,int write_len)
{
	if(!network_send_semaphore_p())
	{
		printf("network_send_semaphore_p fail\n");
	}

	int has_write_len = 0, current_write_len = -1;
	while(1)
	{
		current_write_len = write(sockfd, info, write_len - has_write_len);
		if(current_write_len == -1)
		{
			printf("network_write fail:%s sockfd is %d\n", strerror(errno),sockfd);
			if(!network_send_semaphore_v())
			{
				printf("network_send_semaphore_p fail\n");
			}
			return FALSE;
		}
		has_write_len += current_write_len;
		if(write_len == has_write_len)
		{
			printf("network_write write success:len = %d, %s\n",write_len,(char*)info);	
			if(!network_send_semaphore_v())
			{
				printf("network_send_semaphore_p fail\n");
			}
			return TRUE;
		}
	} 
}
boolean network_read(int sockfd, void* info,int read_len)
{
	int has_read_len = 0, current_read_len = -1;
	while(1)
	{
		current_read_len = read(sockfd, info, read_len - has_read_len);
		if(current_read_len == -1)
		{
			printf("network_read fail,len:%d \n",has_read_len);
			return FALSE;
		}
		has_read_len += current_read_len;
		if(read_len == has_read_len)
		{
			printf("network_read read success:len = %d, %s\n",has_read_len,(char*)info);
			return TRUE;
		}
	}
}

