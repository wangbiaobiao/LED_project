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
	if(NodeAddress<64)//1016
		NodeAddress += 64;//1016
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
		//���ڶ�ȡ������������0���Բ���serial_read����ֵ��ʲô��ֻҪ
		//if(serialbuff[z] == 'J' && serialbuff[z+1] == 'T')  �жϳɹ��Ϳ���
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
	if(t_current_time == -1)
		return FALSE;
	cmd_packet return_packet;
	signal(SIGALRM,signal_outime);
	struct itimerval itv;
	PNode t_head = GetHead(timetable);
	PNode t_node = t_head;
	printf("GetSize:%d\n",GetSize(timetable));
	for(; t_node->next != NULL; t_node=t_node->next)
	{
		time_printf(t_node->data.startMoment, t_node->data.endMoment);
	}
	if(GetSize(timetable) == 0)
	{
		for(j=0; j<xml_length_type_301_size-1; j++)
		{
			send_cmd(j+1, 2, &return_packet);
		}
		return TRUE;
	}
	//��û�е����Ƶ�ʱ�䣬�ر����еĵ�
	/***************************************************************/
	timetable_index = t_head->next;
	/***************************************************************/
	if(timetable_index->data.startMoment > t_current_time)
	{
		printf("No time to turn on the lights, turn off all the lights\n");
		for(j=0; j<xml_length_type_301_size-1; j++)
		{
			send_cmd(j+1, 2, &return_packet);
		}
		itv.it_value.tv_sec = timetable_index->data.startMoment - t_current_time;
		itv.it_interval.tv_usec = 0;
		itv.it_interval.tv_sec =  0;
		itv.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL,&itv,NULL);
		return TRUE;
	}
	//t_node->next != NULLΪ�˲�Խ��
	//int t_timetable_size = GetSize(timetable);
	for(t_node=timetable_index; t_node->next != NULL; t_node=t_node->next)
	{
		t_moment_1 = t_current_time - t_node->data.startMoment;
		t_moment_2 = t_current_time - t_node->next->data.startMoment;
		if(t_moment_1 >= 0 && t_moment_2 <= 0)
			break;
	}
	//timetable_index ��¼���ҵ�ʱ�̱��λ��
	timetable_index = t_node;
	//û����Ҫ���Ƶ�ʱ��
	if(t_node == GetTail(timetable)->previous)
	{
		printf("No need to turn on the lights, turn off all the lights\n");
		//�����еĵƹر�
		for(j=0; j<xml_length_type_301_size-1; j++)
		{
			send_cmd(j+1, 2, &return_packet);
		}
		return TRUE;
	}
	time_printf(timetable_index->data.startMoment, timetable_index->data.endMoment);
	if(t_current_time > timetable_index->data.startMoment && t_current_time < timetable_index->data.endMoment)
	{
		printf("In the light of the time, turn on all the lights\n");
		for(j=0; j<xml_length_type_301_size-1; j++)
		{
			send_cmd(j+1, 1, &return_packet);
		}
		
		itv.it_value.tv_sec = timetable_index->next->data.startMoment- t_current_time;
		itv.it_interval.tv_usec = 0;
		itv.it_interval.tv_sec =  0;
		itv.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL,&itv,NULL);	
		return TRUE;
	}
	else 
	{
		printf("Not in the light of the time, turn off all the lights\n");
	time_printf(timetable_index->next->data.startMoment, timetable_index->next->data.endMoment);
		for(j=0; j<xml_length_type_301_size-1; j++)
		{
			send_cmd(j+1, 2, &return_packet);
		}
		
		itv.it_value.tv_sec = timetable_index->next->data.startMoment - t_current_time;
		itv.it_interval.tv_usec = 0;
		itv.it_interval.tv_sec =  0;
		itv.it_value.tv_usec = 0;
		setitimer(ITIMER_REAL,&itv,NULL);
		return TRUE;	
	}
	return FALSE;	
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
			sleep(60);
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

//reboot io ���� һ���������ļ�
void reboot() {
                arg.pin_idx = AT91_PIN_PC3;
                arg.pin_dir = AT91PIO_DIR_OUT;
                arg.pin_sta = 1;
                ioctl(fd_pio, IOCTL_PIO_SETDIR, &arg);
                ioctl(fd_pio, IOCTL_PIO_SETSTA, &arg);
}


