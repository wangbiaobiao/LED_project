#ifndef __RS485__
#define __RS485__
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "at91_ioctl.h"
#include "at91_gpio.h"
#include "common.h"
#include "link_list.h"

extern at91_pio_arg arg;

extern	int fd_pio;
extern int rs485_fd;

typedef struct cmd_packet
{
	unsigned char PacketFlag0;
	unsigned char PacketFlag1;
	unsigned char NodeAddress;
	unsigned char packetLen;
	unsigned char CmdInfo;
	unsigned char CheckCode;
}cmd_packet;

typedef struct data_packet
{
	unsigned char PacketFlag0;
	unsigned char PacketFlag1;
	unsigned char NodeAddress;
	unsigned char packetLen;
	unsigned char Status;
	int SensorData;
	unsigned char CheckCode;
}data_packet;

#define RS485_BAUD	115200
#define RS485_PORT  "/dev/ttyS3"

#define DEV_PIO "/dev/gpio0"

#define RS485_CTR  AT91_PIN_PA4

extern volatile control_type;
extern PNode timetable_index;
extern volatile unsigned char rs485_user;

void *perform_automatic_strategy(void * arg);


boolean timing();
void signal_outime(int signo);
boolean send_packet(cmd_packet* cmd);
unsigned char get_packet_BBC(cmd_packet *cmd);

boolean send_cmd(unsigned char NodeAddress, unsigned char CmdInfo, void* return_packet);
void  RS485_SEND() ;
void RS485_RECV() ;
boolean rs485_init();
//reboot io 控制 一起放在这个文件
void reboot();

#endif
