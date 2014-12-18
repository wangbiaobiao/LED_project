#ifndef ____MYTIME_H____
#define ____MYTIME_H____
#include "common.h"
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>

extern int rtc_fd; 

#define RTC_NAME "/dev/rtc"

boolean get_localtime(char * time_str);
boolean setHwclock(struct tm* tm_temp);
void asc2tm(struct tm* tm_temp, char *_time);
int str2sec(char* _time);
char* time_format( char *_time);
boolean calibrateTime(char* currentTime);
void time_printf(int startTime, int endTime);

#endif

