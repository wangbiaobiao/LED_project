#ifndef ____XML_PARSE_H____
#define ____XML_PARSE_H____
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include "common.h"
#include "get_file_name.h"
#include "protocol_parse.h"
#include "link_list.h"

#include "libxml/tree.h"
#include "libxml/parser.h"
#include "libxml/xpath.h"
#include "libxml/xpathInternals.h"

typedef struct strategy_list
{
	char startDate[12];
	char endDate[12];
	char startTime[12];
	char endTime[12];
}strategy_list;

struct _strategy_Timetable;

struct _DList;
extern struct _DList* timetable;
extern int timetable_size;
extern char network_number[16];

extern int strategy_list_size;
extern strategy_list * current_strategy_list; 

#define THE_NUMBER_OF_SECONDS_A_DAY 86400
#define DEFAULT_STRATEGY_VERSION "4.2"
#define DEFAULT_PROTOCOL_VERSION "led_v1.xml"

int execute_xpath_expression(const char* filename, const xmlChar* xpathExpr, const xmlChar* Prop,char *find_str, int type);

void xml_open();
void xml_close();
int xml_parse(const char* filename, int type, int* xml_length);
boolean strategy_parse(const char* filename);
boolean construct_timetable(strategy_list * list);
boolean one_strategy_timetable(char* startDate, char* endDate, char* startTime, char* endTime);	
boolean config_init();

#endif
