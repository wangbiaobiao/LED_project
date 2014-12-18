#ifndef ____COMMON_H____
#define ____COMMON_H____

typedef enum boolean
{
	FALSE,
	TRUE	
}boolean;

boolean myStrcpy( char * des, char* src, int start,  int len); 

boolean myUint8cpy(unsigned char * des, unsigned char* src, int start,  int len);

unsigned char getUint8BCC(unsigned char * info,int start, int end);

boolean padding_string(char* pad_string, int start, int end, char pad_char);

int ascll2int(char* src_str, int start, int end);

int mySplit( char (*des_str)[64],const  char *src_str, char x , int len);

boolean mySystem(char * cmd);

#endif

