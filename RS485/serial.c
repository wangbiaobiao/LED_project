#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>
#include <termios.h> /* POSIX terminal control definitions */
#include <errno.h>   /* Error number definitions */
#include <fcntl.h>   /* File control definitions */
#include <unistd.h>
#include "common.h"
#include "serial.h"

static void set_speed(int fd, speed_t  speed);

/*
 * Returns the file descriptor on success or -1 on error.
 */

int serial_open(const char *dev, unsigned int speed)
{
    int fd; /* File descriptor for the port */

    fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1)
    {
        /*
         * Could not open the port.
         */

        perror("open_port: Unable to open port ");

        return fd;
    }
    else
        fcntl(fd, F_SETFL, FNDELAY);


    switch (speed)
    {
    case 38400:
        speed = B38400;
        break;
  
    case 115200:
        speed = B115200;
        break;

    case 9600:
    default:
        speed = B9600;
        break;
    }

    set_speed(fd, (speed_t)speed);

    return (fd);
}

void set_speed(int fd, speed_t speed)
{
    struct termios options;
    /*
     * Get the current options for the port...
     */
    tcgetattr(fd, &options);

    /*
     * Set the baud rates to ...
     */
    cfsetispeed(&options, (speed_t)speed);
    cfsetospeed(&options, (speed_t)speed);

    /*
     * Enable the receiver and set local mode...
     */
    options.c_cflag |= (CLOCAL | CREAD);

    /*
     * Setting the Character Size
     */
    options.c_cflag &= ~CSIZE; /* Mask the character size bits */
    options.c_cflag |= CS8; /* Select 8 data bits */

    /*
     * Setting Parity Checking
     */
    options.c_cflag &= ~PARENB; /*No parity*/
    /*
     * Even parity
     options.c_cflag |= PARENB
     options.c_cflag &= ~PARODD
     */

    /*
     * Setting Stop Bit
     */
    options.c_cflag &= ~CSTOPB; /*One Stop Bit*/

    options.c_iflag = IGNPAR;
   // options.c_iflag &= ~(IXON | IXOFF | IXANY);

    /*
     * Setting Hardware Flow Control
     *
     */
#ifdef CNEW_RTSCTS
    options.c_cflag &= ~CNEW_RTSCTS; /* disable hardware flow control*/
#endif

    /*
     * Choosing Raw Input
     */
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /*
     * Setting Input Parity Options
     */
    //options.c_iflag |= (INPCK | ISTRIP);

    /*
     * Setting Software Flow Control
     *
     */
    //options.c_iflag &= ~(IXON | IXOFF | IXANY);

    /*
     * Choosing Raw Output
     */
    options.c_oflag &= ~OPOST;

    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 0;

    /*
     * Set the new options for the port...
     */
    tcsetattr(fd, TCSANOW, &options);
}

void serial_close(int fd)
{
    close(fd);
}

boolean serial_read (int fd, void *buf, int read_len,int* has_read_len)
{
	int current_read_len = 0, z = 0;
	for(z=0; z<SERIAL_RETRY; z++ )
	{
		current_read_len = read(fd, buf, read_len - *has_read_len);
		if(-1 == current_read_len)
			return FALSE;
		*has_read_len += current_read_len;
		if(*has_read_len == read_len)	
		{		
			for(z=0; z<read_len; z++ )
			{
				printf("%d,",((unsigned char *)buf)[z]);
			}	
			return TRUE;
		}
	}
    return FALSE;
}

boolean  serial_write(int fd, const void *buf, int write_len)
{
   	int has_write_len = 0, current_write_len = 0, z = 0;
	for(z=0; z<SERIAL_RETRY; z++ )
	{
		current_write_len = write(fd, buf, write_len - has_write_len);
		if(-1 == current_write_len)
			return FALSE;
		has_write_len += current_write_len;
		if(has_write_len == write_len)	
		{		
			for(z=0; z<write_len; z++ )
			{
				printf("%d,",((unsigned char *)buf)[z]);
			}	
			return TRUE;
		}
	}
    return FALSE;
}






