#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "common.h"
#include <errno.h>
#include <sys/wait.h>   
#include <sys/types.h> 
boolean myStrcpy( char * des, char* src, int start,  int len)
{
	//printf("%d:%d ",start,len);
    int i = 0;
    if(des == NULL || src == NULL)
      return FALSE;
    for(i=0; i<len; i++)
    {
      *(des++) = *(src+start+i);
      //printf("%d:%c ",i,*(src-1));
    }
	*(des+start+i) = '\0';
	
	//printf("=====myStrcpy des:%s======\n",des);
    return TRUE;
}

boolean myUint8cpy(unsigned char * des, unsigned char* src, int start,  int len)
{
    int i = 0;
    if(des == NULL || src == NULL)
      return FALSE;
    for(i=0; i<len; i++)
    {
      *(des+start+i) = *(src++);
    }
      printf("%s ",src);
    return TRUE;
}
unsigned char getUint8BCC(unsigned char * info,int start, int end)
{
	int i =0;
	unsigned char bcc = 0;
	for(i=start; i<end; i++)
	{
		bcc ^= 	info[i];
		//if(i%16==0)
		//	printf("\n");
		//printf("%02x,",info[i]);
	}
//	printf("--%d,%d,%d,%d--",i,info[i-1],info[i-2],info[end]);
	return 	bcc;
}

boolean padding_string(char* pad_string, int start, int end, char pad_char)
{
	if(pad_string == NULL) return FALSE;
	int i = 0;
	for(i=start; i<end; i++)
	{
		pad_string[i] = pad_char;
	}
	return TRUE;
}

//Hexadecimal conversion
int ascll2int(char* src_str, int start, int end)
{
	if(src_str == NULL) return -1;
	int i = 0, figure = end - start, sum = 0, pow_data = 0;
	for(i=start; i<=end; i++)
	{
		sum += (src_str[i] - '0') * pow(10, figure--);
	}
	return sum;
}

int mySplit( char (*des_str)[64], const  char *src_str, char x , int len)
{
	int end = 0,index = 0, start = 0;
	if(des_str == NULL && src_str == NULL )
	{	
		return FALSE;	
	}
	for(end=0; end<len; end++)
	{
		if(src_str[end] == x)
		{
			myStrcpy(des_str[index++],(char*) src_str, start, end-start);
			start = end+1;	
			//printf("i:%d,%s\n",end,des_str[index-1]);	
		}	
	}
	myStrcpy(des_str[index++], (char*)src_str, start, end);
	return index;
}

boolean mySystem(char * cmdstring)
{
	int status, trycount = 3, i =0; 
	for(i=0; i<trycount; i++)
	{
		if(NULL == cmdstring) //如果cmdstring为空趁早闪退吧，尽管system()函数也能处理空指针 
		{ 
			return FALSE; 
		} 
		status = system(cmdstring); 
		if(status < 0) 
		{ 
			printf("cmd: %s\t error: %d", cmdstring, strerror(errno)); // 这里务必要把errno信息输出或记入Log 
			return FALSE; 
		} 

		if(WIFEXITED(status)) 
		{ 
			printf("normal termination, exit status = %d\n", WEXITSTATUS(status)); //取得cmdstring执行结果     
			return TRUE;
		} 
		else if(WIFSIGNALED(status)) 
		{ 
			printf("abnormal termination,signal number =%d\n", WTERMSIG(status)); //如果cmdstring被信号中断，取得信号值 
			return FALSE;
		} 
		else if(WIFSTOPPED(status)) 
		{ 
			printf("process stopped, signal number =%d\n", WSTOPSIG(status)); //如果cmdstring被信号暂停执行，取得信号值 
			return FALSE;
		} 
	}
	return FALSE;
}


