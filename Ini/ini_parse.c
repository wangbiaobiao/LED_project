/* Example: parse a simple configuration file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ini.h"
#include "ini_parse.h"

void tranform(configuration *pconfig)
{
	
    	char *saveptr1,*str1,*token;
    	int j;
	char *dapai,*yuanhu,*beiting;
	char *dapai_n,*yuanhu_n,*beiting_n;
	dapai_n  = strdup(pconfig->version);
	dapai  = strdup(pconfig->version);
	yuanhu_n  = strdup(pconfig->name);
	yuanhu  = strdup(pconfig->name);
	beiting_n  = strdup(pconfig->email);
	beiting  = strdup(pconfig->email);

	printf("******%s\n",dapai);

	for (j = 1, str1 = dapai_n; ; j++, str1 = NULL) {
               token = strtok_r(str1, ",", &saveptr1);
               if (token == NULL){
	 	   point_config.dapai.len = j-1;
                   break;
		}
               printf("%d: %s\n", j, token);
	}
	point_config.dapai.point = (int *)malloc(sizeof(int)*(j-1));

	for (j = 1, str1 = dapai; ; j++, str1 = NULL) {
               token = strtok_r(str1, ",", &saveptr1);
               if (token == NULL){
                   break;
		}
               printf("%d: %s\n", j, token);
	       point_config.dapai.point[j-1] = atoi(token);
	}
	printf("#######%d\n",point_config.dapai.len);
	for(j = 0; j < point_config.dapai.len; j++ )
	{
		printf("#######%d\n",point_config.dapai.point[j]);
	}

	
	for (j = 1, str1 = yuanhu_n; ; j++, str1 = NULL) {
               token = strtok_r(str1, ",", &saveptr1);
               if (token == NULL){
	 	   point_config.yuanhu.len = j-1;
                   break;
		}
               printf("%d: %s\n", j, token);
	}
	point_config.yuanhu.point = (int *)malloc(sizeof(int)*(j-1));

	for (j = 1, str1 = yuanhu; ; j++, str1 = NULL) {
               token = strtok_r(str1, ",", &saveptr1);
               if (token == NULL){
                   break;
		}
               printf("%d: %s\n", j, token);
	       point_config.yuanhu.point[j-1] = atoi(token);
	}
	printf("#######%d\n",point_config.yuanhu.len);
	for(j = 0; j < point_config.yuanhu.len; j++ )
	{
		printf("#######%d\n",point_config.yuanhu.point[j]);
	}


	for (j = 1, str1 = beiting_n; ; j++, str1 = NULL) {
               token = strtok_r(str1, ",", &saveptr1);
               if (token == NULL){
	 	   point_config.beiting.len = j-1;
                   break;
		}
               printf("%d: %s\n", j, token);
	}
	point_config.beiting.point = (int *)malloc(sizeof(int)*(j-1));

	for (j = 1, str1 = beiting; ; j++, str1 = NULL) {
               token = strtok_r(str1, ",", &saveptr1);
               if (token == NULL){
                   break;
		}
               printf("%d: %s\n", j, token);
	       point_config.beiting.point[j-1] = atoi(token);
	}
	printf("#######%d\n",point_config.beiting.len);
	for(j = 0; j < point_config.beiting.len; j++ )
	{
		printf("#######%d\n",point_config.beiting.point[j]);
	}
}

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* pconfig = (configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("NODE_INFORMATION", "dapai")) {
        pconfig->version = strdup(value);
    } else if (MATCH("NODE_INFORMATION", "yuanhu")) {
        pconfig->name = strdup(value);
    } else if (MATCH("NODE_INFORMATION", "beiting")) {
        pconfig->email = strdup(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}

int parse_ini(char* path)
{
    configuration config;

    if (ini_parse(path, handler, &config) < 0) {
        printf("Can't load the ini file\n");
        return 1;
    }
    tranform(&config);
    return 0;
}

