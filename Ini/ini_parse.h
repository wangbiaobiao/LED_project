#ifndef __INI_PARSE_H__
#define __INI_PARSE_H__

typedef struct
{
    const char* version;
    const char* name;
    const char* email;
} configuration;

struct sec{
	int *point;
	int len;
};

struct config_point{
	struct sec dapai;
	struct sec yuanhu;
	struct sec beiting;
};

struct config_point point_config;

int parse_ini(char* path);
#endif
