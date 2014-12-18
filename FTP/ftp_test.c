#include <stdio.h>
#include "ftp.h"

int main()
{
	char dir_name_info[128][128] = {"strategy"};
	int index = 0;
	while(get_file_from_server(dir_name_info, STRATEGY_FILE_NAME))
	{
		printf("ftp get file %s success%d\n",STRATEGY_FILE_NAME,index++);
		//sleep(1);
	}
	printf("ftp get file %s fail%d\n",STRATEGY_FILE_NAME,index++);
}

