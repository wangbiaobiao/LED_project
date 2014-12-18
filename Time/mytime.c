#include "mytime.h"
#include <time.h>

int rtc_fd = -1;
boolean get_localtime(char * time_str)
{
    time_t timep;  
    struct tm *p_tm;  
    if((timep = time(NULL)) == -1)
    	return FALSE;;
    if((p_tm = localtime(&timep)) == NULL) /*获取GMT时间*/     
    	return FALSE;;
    sprintf(time_str, "%04d%02d%02d%02d%02d%02d", (p_tm->tm_year)+1900, (p_tm->tm_mon)+1, p_tm->tm_mday, p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);  
    printf("get_time:%s\n",time_str);
    return TRUE;
}

boolean setHwclock(struct tm* tm_temp)
{
	if(rtc_fd == -1)
		rtc_fd = open(RTC_NAME, O_RDWR);
	if (rtc_fd == -1) {
		printf("rtc fali\n");
		return FALSE;;   
	}
	int retval = ioctl(rtc_fd, RTC_SET_TIME, tm_temp);
	if (retval == -1) {
    	printf("RTC_SET_TIME ioctl\n");
    	return FALSE;;
	}
	return TRUE;
}

char* time_format( char *_time)
{
	_time[4] = _time[5];	
	_time[5] = _time[6];	
	_time[6] = _time[8];	
	_time[7] = _time[9];	
	_time[8] = _time[11];	
	_time[9] = _time[12];	
	_time[10] = _time[14];	
	_time[11] = _time[15];	
	_time[12] = _time[17];	
	_time[13] = _time[18];	
	_time[14] = '\0';
	printf("time_format:%s\n",_time);
	return _time;
}

void asc2tm(struct tm* tm_temp, char *_time)
{
       tm_temp->tm_sec = (_time[12]-'0')*10+(_time[13]-'0');         /* seconds */
       tm_temp->tm_min = (_time[10]-'0')*10+(_time[11]-'0');         /* minutes */
       tm_temp->tm_hour = (_time[8]-'0')*10+(_time[9]-'0');        /* hours */
       tm_temp->tm_mday = (_time[6]-'0')*10+(_time[7]-'0');        /* day of the month */
       tm_temp->tm_mon = (_time[4]-'0')*10+(_time[5]-'0')-1;         /* month */
       tm_temp->tm_year = (_time[0]-'0')*1000+(_time[1]-'0')*100+(_time[2]-'0')*10+(_time[3]-'0')-1900;        /* year */	
}
int str2sec(char* _time)
{
       struct tm tm_temp;
	   printf("time:%s\n",_time);
	   asc2tm(&tm_temp,_time);
       int sec_count = mktime(&tm_temp);
	  // printf("str2sec:%d,%d,%d,%d,%d,%d,%d\n",sec,tm_temp.tm_sec,tm_temp.tm_min,tm_temp.tm_hour,tm_temp.tm_mday,tm_temp.tm_mon,tm_temp.tm_year);
       if(sec_count == -1)
       		return -1;
       return sec_count;
}

boolean calibrateTime(char* currentTime)
{
	int sec_count = -1;
	if((sec_count = str2sec(currentTime)) == -1)
	{
	   	return FALSE;;	
	}
	   printf("+++++%d\n",sec_count);
	struct timeval tv;
	tv.tv_sec = sec_count;     /* seconds */
	tv.tv_usec = 0;    /* microseconds */
	if(settimeofday(&tv,NULL) == -1)
	 	return FALSE;;  
	struct tm tm_temp;
	asc2tm(&tm_temp,(char*)currentTime);
	return setHwclock(&tm_temp);   
}

void time_printf(int startTime, int endTime)
{
	struct tm* t_tm;
	t_tm = localtime((time_t*)(&startTime));
	printf("\nstarttime:%d-%02d-%02d %02d:%02d:%02d, endtime:",  
                    t_tm->tm_year + 1900,  
                    t_tm->tm_mon + 1,  
                    t_tm->tm_mday,    
                    t_tm->tm_hour,  
                    t_tm->tm_min,  
                    t_tm->tm_sec); 		
	t_tm = localtime((time_t*)(&endTime));
	printf("%d-%02d-%02d %02d:%02d:%02d\n",  
                    t_tm->tm_year + 1900,  
                    t_tm->tm_mon + 1,  
                    t_tm->tm_mday,    
                    t_tm->tm_hour,  
                    t_tm->tm_min,  
                    t_tm->tm_sec); 
}



