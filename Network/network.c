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

    if (setsockopt (sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        printf("setsockopt SO_RCVTIMEO failed\n");

    if (setsockopt (sock_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
        printf("setsockopt SO_SNDTIMEO failed\n");
	struct linger t_linger = {.l_onoff=0,.l_linger=0};
	if (setsockopt( sock_fd, SOL_SOCKET, SO_LINGER, &t_linger, sizeof(t_linger)) < 0)
		printf("setsockopt SO_DONTLINGER failed\n");

	bzero(_addr,sizeof(_addr)); // 初始化,置0
	_addr->sin_family=AF_INET;          // IPV4
	_addr->sin_port=htons(port);  // (将本机器上的short数据转化为网络上的short数据)端口号
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
				//sleep(CONNECT_RETRY_TIME);
				//printf("sleep:%d,wait for retry!!!\n",CONNECT_RETRY_TIME);
				//return;
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

//void* networt_connect(void* arg)
//{
//	/* 客户程序发起连接请求 */
//	//printf("networt_connect -0\n");
//	network_pthread_arg* pthread_arg = (network_pthread_arg*)arg; 
//	*(pthread_arg->ret_value) = 0;
//	if(*(pthread_arg->is_connect)  == 0)
//	{	
//		int flags = fcntl(*(pthread_arg->sock_fd),F_GETFL,0); 
//		int fd = *(pthread_arg->sock_fd);
//		int error=-1, error_len = sizeof(int);
//		fcntl(fd,F_SETFL,flags | O_NONBLOCK); 
//	//	printf("networt_connect -1\n");
//		int max_fd = fd;
//		if(fd_pio > max_fd)
//			max_fd = fd_pio;
//		if(rs485_fd > max_fd)
//			max_fd = rs485_fd;
//		if(ftp_sockfd > max_fd)
//			max_fd = ftp_sockfd;
//		if(message_sockfd > max_fd)
//			max_fd = message_sockfd;
//		while(connect(fd, (struct sockaddr *)(pthread_arg->server), sizeof(struct sockaddr)) < 0) 
//		{  // EINPROGRESS表示connect正在尝试连接 
//		   // if(errno != EINPROGRESS && errno != EWOULDBLOCK) 			   
//		    fprintf(stderr,"Connect stauts:%s\a\n",strerror(errno)); 	 
//		    struct timeval tv; 
//		    //select等待I/o的最长时间 10s
//		    tv.tv_sec = 10; 
//		    tv.tv_usec = 0; 
//		    fd_set wset; 
//		    FD_ZERO(&wset); 
//		    FD_SET(fd,&wset); 

//		    int ret_no = select(max_fd+1,NULL,&wset,NULL,&tv); 
//			printf("select ret_no:%d",ret_no);
//		    if(ret_no < 0) 
//		    { // select出错 
//		        printf("networt_connect select fail \n"); 
//		    } 
//		    else if (0 == ret_no) 
//		    { // 超时 
//		        printf("networt_connect Timeout.\n" ); 
//		    } 
//		    else if(FD_ISSET(fd,&wset))
//		    {  
//				if(getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&error_len) == 0)
//				{
//					if(error == 0)
//					break;
//				}
//		    } 		    
//			sleep(CONNECT_RETRY_TIME);
//			printf("sleep:%d,wait for retry!!!\n",CONNECT_RETRY_TIME);
//		} 
//		*(pthread_arg->is_connect) = 1;
//		fcntl(fd,F_SETFL,flags & ~O_NONBLOCK);  // 设为阻塞模式 
//		      /* 连接成功了 */ 
//		printf("networt_connect success\n");
//		*(pthread_arg->ret_value) = 1;
//	}	
//}

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
			int i = 0;
			for(i=0; i<has_write_len ; i++)
				printf("%02x,",((unsigned char*)info)[i]);
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
			int i = 0;
			for(i=0; i<has_write_len ; i++)
				printf("%02x,",((unsigned char*)info)[i]);
			printf("\n");                                         //add by biaobiaoss
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
			int i = 0;
			for(i=0; i<has_read_len ; i++)
				printf("%02x,",((unsigned char*)info)[i]);
			return FALSE;
		}
		has_read_len += current_read_len;
		if(read_len == has_read_len)
		{
			printf("network_read read success:len = %d, %s\n",has_read_len,(char*)info);	
			int i = 0;
			for(i=0; i<has_read_len ; i++)
				printf("%02x,",((unsigned char*)info)[i]);
			return TRUE;
		}
	}
}

