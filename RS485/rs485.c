#include <stdio.h>
#include "rs485.h"
#include "serial.h"
#include "mytime.h"
#include "xml_parse.h"
#include "get_file_name.h"
#include "protocol_parse.h"
#include "xml_parse.h"
#include "link_list.h"

int fd_pio = -1;
int rs485_fd = -1;
at91_pio_arg arg;
extern DList*  timetable;

PNode timetable_index = NULL;
volatile unsigned char rs485_user = 0;

boolean rs485_init()
{
	if(fd_pio == -1)
		fd_pio = open(DEV_PIO, O_RDWR);
	if (fd_pio < 0) {
		printf("open PIO device error! %d\n", fd_pio);
		return FALSE;
	}
	printf("open PIO device successful!\n" );
	
	if(rs485_fd == -1)
		rs485_fd = serial_open(RS485_PORT, RS485_BAUD);
	if (rs485_fd < 0)
	{
		printf("Open serial failed\n");
		return FALSE;
	}
	printf("Open serial success\n");
	return TRUE;
}

boolean send_packet(cmd_packet* cmd)
{
	unsigned char buff[50]={cmd->PacketFlag0,cmd->PacketFlag1,cmd->NodeAddress,cmd->packetLen,cmd->CmdInfo,cmd->CheckCode};
	return serial_write(rs485_fd, buff, cmd->packetLen);
}

unsigned char get_packet_BBC(cmd_packet *cmd)
{	
	int i = 0 ,bbc = 0;	
	bbc = cmd->PacketFlag0^cmd->PacketFlag1^cmd->NodeAddress^cmd->packetLen^cmd->CmdInfo;
	return bbc;
}

boolean send_cmd(unsigned char NodeAddress, unsigned char CmdInfo, void* return_packet)
{
	printf("=========send_cmd:%d========\n",get_rs485_sem_val());
	
	if(!rs485_semaphore_p()) 
	{
		printf("rs485_semaphore_p fail\n");
	}
	/*if(NodeAddress<64)//1016
		NodeAddress += 64;//1016*/
//	if(NodeAddress == -1)
//	{
//		if(!rs485_semaphore_v()) 
//		{
//			printf("rs485_semaphore_v fail\n");
//		}
//		return FALSE;
//	}
	printf("=========send_cmd:%d========\n",get_rs485_sem_val());
//	printf("=========send_cmd========\n");
	cmd_packet cmd, *rev_ctrl_packet;
	data_packet* rev_data_packet;
	int i = 0, j = 0, z = 0,index = 0, k = 0;
	int type_turn_len = 0, try_count = 4, rev_len = 0;
	char serialbuff[1024];
	cmd.PacketFlag0 = 'J';	
	cmd.PacketFlag1 = 'T';	
	cmd.NodeAddress = NodeAddress;
	cmd.packetLen = 0x6;
	cmd.CmdInfo = CmdInfo;
	cmd.CheckCode = get_packet_BBC(&cmd);
//	printf("=========send_cmd========\n");
	printf("cmd:%02x,%02x\n",cmd.NodeAddress,cmd.CmdInfo);
	if(cmd.CmdInfo < 0x20)
    {
		type_turn_len = 6;
		rev_ctrl_packet = (cmd_packet*)return_packet;
		memset(rev_ctrl_packet,0,sizeof(cmd_packet));
    }
    else if (cmd.CmdInfo > 0x20 && cmd.CmdInfo < 0x50 )
    {
		type_turn_len = 10;
		rev_data_packet = (data_packet*)return_packet;
		memset(rev_data_packet,0,sizeof(data_packet));
    }
	else 
    {
		if(!rs485_semaphore_v()) 
		{
			printf("rs485_semaphore_v fail\n");
		}
		return FALSE;
    }
	//printf("rev:%d\n",serial_read(_arg->fd, serialbuff, 1024));
	//usleep(1000*10);
	for(k=0; k<try_count ; k++)
	{
		memset(serialbuff,'\0',1024);
		usleep(1000*10);	
		RS485_SEND();
		usleep(1000*10);
		for(j=0; j<try_count ; j++)
		{
			if(send_packet(&cmd))
				break;
		}
		usleep(1000*10);
		RS485_RECV();
		usleep(1000*100);//usleep(1000*100);
		for(i = 0; i < 20; i++)
		{
		//由于读取到不定个数的0所以不管serial_read返回值是什么，只要
		//if(serialbuff[z] == 'J' && serialbuff[z+1] == 'T')  判断成功就可以
			serial_read(rs485_fd, serialbuff, 1024, &rev_len);
			if(rev_len >= 6)
			{
				for(z=0; z<rev_len-1; z++)//readLen-1 ----In order to not cross-border
				{
					if(serialbuff[z] == 'J' && serialbuff[z+1] == 'T')
					{		
						for(; z<rev_len; z++ )
						{
							serialbuff[index++] = serialbuff[z];
						}
						break;
					}
				}
				if(index == type_turn_len)
				{
					if(serialbuff[2] == cmd.NodeAddress && serialbuff[3] == type_turn_len  && \
						serialbuff[type_turn_len  - 1] == getUint8BCC(serialbuff, 0, type_turn_len  - 1))
					{
						printf("rs485 cmd send and recieve OK!\n");
						if(serialbuff[4] == (char)-1)
						{
							if(!rs485_semaphore_v()) 
							{
								printf("rs485_semaphore_v fail\n");
							}
							return FALSE;
						}
						if(type_turn_len  == 6)
						{
							rev_ctrl_packet->PacketFlag0 = serialbuff[0];
							rev_ctrl_packet->PacketFlag1 = serialbuff[1];
							rev_ctrl_packet->NodeAddress = serialbuff[2];
							rev_ctrl_packet->packetLen = serialbuff[3];
							rev_ctrl_packet->CmdInfo = serialbuff[4];
							rev_ctrl_packet->CheckCode = serialbuff[5];
						}
						else if(type_turn_len  == 10)
						{
							rev_data_packet->PacketFlag0 = serialbuff[0];
							rev_data_packet->PacketFlag1 = serialbuff[1];
							rev_data_packet->NodeAddress= serialbuff[2];
							rev_data_packet->packetLen = serialbuff[3];
							rev_data_packet->Status= serialbuff[4];
							rev_data_packet->SensorData = (serialbuff[5] << 24)+(serialbuff[6] << 16)\
								+(serialbuff[7] << 8)+serialbuff[8];
							rev_data_packet->CheckCode= serialbuff[9];
							printf("--------data%d----------\n",rev_data_packet->SensorData);
						}
						else
						{
						
							if(!rs485_semaphore_v()) 
							{
								printf("rs485_semaphore_v fail\n");
							}
							return FALSE;
						}
							
						if(!rs485_semaphore_v()) 
						{
							printf("rs485_semaphore_v fail\n");
						}
						return TRUE;
					}
				}
			}
			usleep(1);
		}
		printf("\n =====i=====:%d\n",i);
	}
	if(!rs485_semaphore_v()) 
	{
		printf("rs485_semaphore_v fail\n");
	}
	return FALSE;
	//serialbuff[10] = '\0';
	
	
//	if(readLen >=6 && last_arg != NULL)
//	{
//		char sendbuff[1024];
//		memset(sendbuff,'\0',1024);
//		if(cmd->CmdInfo == 0x23 || cmd->CmdInfo == 0x22)
//		{
//			strcat(sendbuff,"03x");
//		}
//		else
//		{
//			strcat(sendbuff,"02x");
//		}
//		printf("%d",serialbuff[3]);
//		for(z=0; z<serialbuff[3]; z++ )
//		{
//			sendbuff[z+3] = serialbuff[z];
//		}
//		int nBytes,err = -1;
//		if((nBytes = write(last_arg->newsock, sendbuff, serialbuff[3]+3)) == -1)
//		{
//			fprintf(stderr,"read error:%s\n",strerror(errno));
//			last_arg = NULL;
//			//pthread_exit((void *)err);
//		}						
//		printf("sendbuff,%d:%s\n",nBytes,sendbuff);
//		for(z=0; z<nBytes; z++ )
//		{
//			printf("%d\n",sendbuff[z]);
//		}
//	}
	////////////////////////else  send eror

}

void signal_outime(int signo)
{
	//signal(SIGALRM,signal_outime);
	if(!timing(TRUE))
		printf("signal_outime timing fail\n");
}
//void mysleep(int sec)
//{
//	struct timeval delay;
//	delay.tv_sec = sec;
//	delay.tv_usec = 0; // s
//	select(0, NULL, NULL, NULL, &delay);
//}

boolean timing( )
{
	printf("===========timing=========\n");
	int i = 0, j = 0, t_moment_1 = 0, t_moment_2 = 0;
	time_t t_current_time = time(NULL);
	struct c_addr
	{
		int addr;
		int flag;
	};
	if(t_current_time == -1)
	{
		printf("can't get current time\n");
		return FALSE;
	}
	cmd_packet return_packet;
	signal(SIGALRM,signal_outime);                            //???what't the mean
	struct itimerval itv;
	PNode t_head = GetHead(timetable);
	PNode t_node = t_head;

	int node_start_day,node_end_day,node_start_time,node_end_time,current_day,current_time;
	
	struct c_addr * board_caddr= (struct c_addr *)malloc(GetSize(timetable)*sizeof(struct c_addr));

	printf("GetSize:%d\n",GetSize(timetable));
	for(; t_node->next != NULL; t_node=t_node->next)
	{
		time_printf(t_node->next->data.startMoment, t_node->next->data.endMoment);
		printf("addr is %d\n",t_node->next->data.node_addr);
	}
	t_node = t_head;
	/*解析策略，设置标记flag*/
	for(i = 0; t_node->next != NULL; t_node=t_node->next,i++)
	{
		node_start_day = t_node->next->data.startMoment / THE_NUMBER_OF_SECONDS_A_DAY;
		node_end_day = t_node->next->data.endMoment / THE_NUMBER_OF_SECONDS_A_DAY;
		node_start_time = t_node->next->data.startMoment - node_start_day * THE_NUMBER_OF_SECONDS_A_DAY;
		node_end_time = t_node->next->data.endMoment- node_end_day * THE_NUMBER_OF_SECONDS_A_DAY;

		current_day  = t_current_time / THE_NUMBER_OF_SECONDS_A_DAY;
		current_time  = t_current_time - current_day * THE_NUMBER_OF_SECONDS_A_DAY;

		printf("startMoment is %d,endMoment%d\n",t_node->next->data.startMoment,t_node->next->data.endMoment);
		
		printf("node_start_day is %d,node_end_day is %d,node_start_time is %d,node_end_time is %d\n",node_start_day,node_end_day,node_start_time,node_end_time);
		printf("current_day is %d,current_time%d\n",current_day,current_time);
		if(node_start_time > node_end_time)
		{
			node_end_time = node_end_time + 24*3600;
		}
		if((node_start_day <= current_day) && (node_end_day >= current_day) && 
		   (node_start_time <= current_time) && (node_end_time >= current_time))
		{
			printf("turn on is good\n");
			t_node->next->data.flag = 1;
			//send_cmd((unsigned char)t_node->next->data.node_addr, 1, &return_packet);
			board_caddr[i].addr = t_node->next->data.node_addr;
			board_caddr[i].flag = 1;
			
		}
		else
		{
			printf("turn off is good\n");
			//send_cmd((unsigned char)t_node->next->data.node_addr, 2, &return_packet);
			board_caddr[i].addr = t_node->next->data.node_addr;
			board_caddr[i].flag = 0;
		
		}
			
	}
	int n_size = GetSize(timetable);
	/*处理标记flag*/	
	for(i = 0; i < n_size; i++)
	{
		for(j = 0; j < n_size; j++)
		{
			if(board_caddr[i].addr == board_caddr[j].addr)
			{
				if(board_caddr[j].flag== 1)
				{
					board_caddr[i].flag = 1;
				}
			}
		}
	}
	for(i = 0; i < n_size; i++)
	{
		printf("#############################################################\n");
		printf("addr is %d, flag is %d\n",board_caddr[i].addr,board_caddr[i].flag);
	}
	for(i = 0; i < n_size; i++)
	{
		if(board_caddr[i].flag == 1)
		{
			send_cmd((unsigned char)board_caddr[i].addr, 1, &return_packet);
		}
		else
		{
			send_cmd((unsigned char)board_caddr[i].addr, 2, &return_packet);
		}
	}
	free(board_caddr);
	return TRUE;
}

void *perform_automatic_strategy(void * arg)
{
	printf("perform_automatic_strategy\n");
	int i = 0;
//	if(!rs485_semaphore_v()) 
//	{
//		printf("rs485_semaphore_v fail\n");
//	}	
	if(control_type == 0)
	{
		timetable_index = GetHead(timetable)->next;
		//timing(TRUE);
		while(1)
		{
			timing();
			sleep(10);
		}
	}
}

void  RS485_SEND() {
                arg.pin_idx = RS485_CTR;
                arg.pin_dir = AT91PIO_DIR_OUT;
                arg.pin_sta = 1;
                ioctl(fd_pio, IOCTL_PIO_SETDIR, &arg);
                ioctl(fd_pio, IOCTL_PIO_SETSTA, &arg);
}
void RS485_RECV() {
                arg.pin_idx = RS485_CTR;
                arg.pin_dir = AT91PIO_DIR_OUT;
                arg.pin_sta = 0;
                ioctl(fd_pio, IOCTL_PIO_SETDIR, &arg);
                ioctl(fd_pio, IOCTL_PIO_SETSTA, &arg);
}

//reboot io 控制 一起放在这个文件
void reboot() {
                arg.pin_idx = AT91_PIN_PC3;
                arg.pin_dir = AT91PIO_DIR_OUT;
                arg.pin_sta = 1;
                ioctl(fd_pio, IOCTL_PIO_SETDIR, &arg);
                ioctl(fd_pio, IOCTL_PIO_SETSTA, &arg);
}


