#include "get_file_name.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

boolean find_new_file(char *basePath,char *file_name)
{
     DIR *dir;
	 struct dirent *ptr;
     //char base[128];
     boolean is_find = FALSE;
 
     if ((dir=opendir(basePath)) == NULL)
     {
         perror("Open dir error...");
         return FALSE;
     }
 	int file_name_max = 0;
	int file_name_num = 0;
     while ((ptr=readdir(dir)) != NULL)
     {
		memset(file_name, '\0', sizeof(file_name));
		if(!strcmp(ptr->d_name,"."))
			continue;	
		if(!strcmp(ptr->d_name,".."))
			continue;	
		strcpy(file_name, ptr->d_name);
		printf("file_name:%s\n",file_name);
		return TRUE;	 
//		file_name_num = atoi(ptr->d_name);
//         if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
//             continue;
//         else if(ptr->d_type == 8)    ///file
//             printf("d_name:%s/%s\n",basePath,ptr->d_name);
//         else if(ptr->d_type == 10)    ///link file
//             printf("d_name:%s/%s\n",basePath,ptr->d_name);
//         else if(ptr->d_type == 4)    ///dir
//         {
//             memset(base,'\0',sizeof(base));
//             strcpy(base,basePath);
//             strcat(base,"/");
//             strcat(base,ptr->d_name);
//             readFileList(base);
//         }
		
//		if(file_name_num > file_name_max)
//		{
//			file_name_max = file_name_num;
//			memset(file_name, '\0', sizeof(file_name));
//			strcpy(file_name, ptr->d_name);
//			is_find = TRUE;
//		}
//		if(!strcmp(ptr->d_name,find_file_name))
//		{
//     		closedir(dir);
//			return TRUE;
//		}
     }
     return FALSE;
}
 
//int main(void)
//{
//     DIR *dir;
//     char basePath[256];
// 	 char file_name[256];
//     ///get the current absoulte path
//     memset(basePath,'\0',sizeof(basePath));
//     getcwd(basePath, 255);
//     printf("the current dir is : %s\n",basePath);
// 
//     ///get the file list
//    // memset(basePath,'\0',sizeof(basePath));
//   //  strcpy(basePath,"./XL");
//     if(find_new_file(basePath, file_name))
//	 	printf("find \n");
//     return 0;
//}

