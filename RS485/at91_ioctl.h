/*
**  SAM926x IOCTL Header
**  HangZhou QiYang Technology Co.Ltd. 
**  www.qiyangtech.com
*/

#ifndef __AT91_IOCTL_H__
#define __AT91_IOCTL_H__


// ==================================== PIO ============================================
typedef enum {
	AT91PIO_DIR_INP = 0,
	AT91PIO_DIR_OUT
} at91_pio_dir_t;

typedef struct {
	int  pin_idx;
	int  pin_dir;
	int  pin_sta;
} at91_pio_arg;

//IOCTL for PIO
#define	IOCTL_PIO_SETDIR	_IOW('Q', 0x01, int)   	//Set pio direct
#define	IOCTL_PIO_GETDIR 	_IOR('Q', 0x02, int)	//Get pio direct
#define	IOCTL_PIO_SETSTA	_IOW('Q', 0x03, int)	//Set pio status
#define	IOCTL_PIO_GETSTA	_IOR('Q', 0x04, int)	//Get pio status


// ==================================== EBI ============================================
typedef struct {
	int ebi_addr;
	int ebi_data;
} at91_ebi_arg;


// ==================================== PCX ============================================
typedef struct {
        int pcx_addr;
        int pcx_data;
} at91_pcx_arg;

// ==================================== TCB ============================================
typedef struct {
	int tc_freq;
} at91_tc_arg;

//IOCTL for TC
#define IOCTL_TC_START		_IOW('Q', 0x21, int)
#define	IOCTL_TC_STOP		_IOW('Q', 0x22, int)
#define IOCTL_TC_GETCNT		_IOR('Q', 0x23, int)

//IOCTL for AT91_CAN
#define IOCTL_CAN_SETBAND	_IOW('Q', 0x24, int)

//IOCTL for PC104
typedef struct {
        int     addr;
        int     data;
} pc104_arg;

enum {
        IOCTL_PC104_CHANGEWIDTH = 0x300,
        IOCTL_PC104_WAITINTERRUPT
};


// ==================================== PIO ============================================
//IOCTL for Buzzer
#define IOCTL_BZR_BEEP		_IOW('Q', 0x31, int)


// ==================================== PIO ============================================
typedef struct {
	int pwm_freq;		//frequency in Hz
	int pwm_duty;		//duty cycle in percent. ex. 50 is 50% 
	int pwm_pulse;		//1 for PWM positive pulse, 0 for negative pulse
} at91_pwm_arg;

#define IOCTL_PWM_START		_IOW('Q', 0x41, int)
#define IOCTL_PWM_STOP		_IOW('Q', 0x42, int)
#define IOCTL_PWM_SETARG	_IOW('Q', 0x43, at91_pwm_arg *)
#define IOCTL_PWM_GETARG	_IOR('Q', 0x44, at91_pwm_arg *)


#endif //__AT91_IOCTL_H__

