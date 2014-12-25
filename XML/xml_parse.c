#include "xml_parse.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
char network_number[16] = "";

strategy_list * current_strategy_list = NULL; 
int strategy_list_size = 0;
char network_number[16];
DList* timetable = NULL;

void xml_open()
{   
    /* Init libxml */     
    xmlInitParser();
}

void xml_close()
{   

    /* Shutdown libxml */
    xmlCleanupParser();
    
    /*
     * this is to debug memory for regression tests
     */
    xmlMemoryDump();
}

int execute_xpath_expression(const char* filename, const xmlChar* xpathExpr, const xmlChar* Prop, char *find_str, int type) {
    xmlDocPtr doc;
    xmlXPathContextPtr xpathCtx; 
    xmlXPathObjectPtr xpathObj; 
    xmlNodeSetPtr  nodeset;
    char*  val = NULL;
    int size,i;

    assert(filename);
    assert(xpathExpr);
//	printf("\npoint1:%s\n",filename);
    /* Load XML document */
    doc = xmlParseFile(filename);
    if (doc == NULL) {
	fprintf(stderr, "Error: unable to parse file \"%s\"\n", filename);
	return(-1);
    }
//	printf("\npoint2\n");

    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(doc);
    if(xpathCtx == NULL) {
        fprintf(stderr,"Error: unable to create new XPath context\n");
        xmlFreeDoc(doc); 
        return(-1);
    }
    
    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression(xpathExpr, xpathCtx);
    if(xpathObj == NULL) {
        fprintf(stderr,"Error: unable to evaluate xpath expression \"%s\"\n", xpathExpr);
        xmlXPathFreeContext(xpathCtx); 
        xmlFreeDoc(doc); 
        return(-1);
    }

    /* Print results */
    //print_xpath_nodes(xpathObj->nodesetval, stdout);
    nodeset = xpathObj->nodesetval;
     if(xmlXPathNodeSetIsEmpty(nodeset))
    {
	printf("No such  nodes.\n");
        xmlXPathFreeObject(xpathObj);
        xmlXPathFreeContext(xpathCtx); 
        xmlFreeDoc(doc);
       return -4;
    }
	
     size = (nodeset) ? nodeset->nodeNr : 0;
     for(i = 0; i <size; i++)
    {
    	if(type ==0 )
			val=xmlGetProp(nodeset->nodeTab[i], BAD_CAST Prop);
		else
			val=xmlNodeGetContent(nodeset->nodeTab[i]);
		if(val != NULL){
			strcpy(find_str, val);
			printf("the results1 are:%s\n",val);
		}
    }
    xmlFree(val);

    /* Cleanup */
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx); 
    xmlFreeDoc(doc); 
    
    return(0);
}

int xml_parse(const char* filename, int type, int* xml_length) 
{
	xml_open();
	int index = 1;
	int result = -1;
	char xpathExpr[256];
	char find_str[64];
	char path[256] = "/app/protocol/";
	strcat(path,filename);
	memset(find_str,'\0',64);
	memset(xpathExpr,'\0',256);
	switch(type){
		case 100:
		{
			while(1)
			{	
				sprintf(xpathExpr,"/unifyMessage/messageDefinition[1]/field[%d]", index++);
				if((result = execute_xpath_expression(path, xpathExpr, "length", find_str, 0)) == -1)
				{
						xml_close();
						return -1;
				}
				if(result == -4)
				{
					xml_close();
					printf("result = -4\n");
					return index-2;
				}
				if(strlen(find_str)	!= 0)
						xml_length[index-2] = atoi(find_str);
			}
		}
		case 301:
		{
			while(1)
			{	
				sprintf(xpathExpr,"/unifyMessage/messageDefinition[6]/field[%d]", index++);
				if((result = execute_xpath_expression(path, xpathExpr, "length", find_str, 0)) == -1)
				{
					xml_close();
					return -1;
				}
				if(result == -4)
				{
					xml_close();
					printf("result = -4\n");
					return index-2;
				}
				if(strlen(find_str)	!= 0)
						xml_length[index-2] = atoi(find_str);
			}
		}
	}
	return -1;
}
boolean strategy_parse(const char* filename)
{
	if(current_strategy_list != NULL)
		free(current_strategy_list);
	if(timetable != NULL)
		DestroyList(timetable);
	timetable = InitList();
	int strategy_list_node_size = sizeof(strategy_list);
	current_strategy_list = (strategy_list*)malloc(strategy_list_node_size);
	xml_open();
	int strategy_id = 0, i =0, result = -1;
	char xpathExpr[256];
	char find_str[64];
	char path[256] = "/app/strategy/";
	strcat(path,filename);
	memset(xpathExpr,'\0',256);
	while(1)
	{
		strategy_id++;
		/*startDate*/
		memset(xpathExpr,'\0',256);
		sprintf(xpathExpr,"/StrategyConfig/strategy[%d]/startDate", strategy_id);
		if((result = execute_xpath_expression(path, xpathExpr, "", find_str, 1)) == -1)
		{
				xml_close();
				return FALSE;
		}
		if(result == -4)
		{
			xml_close();
			strategy_list_size = strategy_id-1;
			printf("result = -4, size:%d\n",strategy_list_size);
			return construct_timetable(current_strategy_list);
		}
		strcpy(current_strategy_list[strategy_id-1].startDate,find_str);
		/*endDate*/
		memset(xpathExpr,'\0',256);
		sprintf(xpathExpr,"/StrategyConfig/strategy[%d]/endDate", strategy_id);
		if((result = execute_xpath_expression(path, xpathExpr, "", find_str, 1)) == -1)
		{
				xml_close();
				return FALSE;
		}
		strcpy(current_strategy_list[strategy_id-1].endDate,find_str);
		/*startTime*/
		memset(xpathExpr,'\0',256);
		sprintf(xpathExpr,"/StrategyConfig/strategy[%d]/startTime", strategy_id);
		if((result = execute_xpath_expression(path, xpathExpr, "", find_str, 1)) == -1)
		{
				xml_close();
				return FALSE;
		}
		strcpy(current_strategy_list[strategy_id-1].startTime,find_str);
		/*endTime*/
		memset(xpathExpr,'\0',256);
		sprintf(xpathExpr,"/StrategyConfig/strategy[%d]/endTime", strategy_id);
		if((result = execute_xpath_expression(path, xpathExpr, "", find_str, 1)) == -1)
		{
				xml_close();
				return FALSE;
		}
		strcpy(current_strategy_list[strategy_id-1].endTime,find_str);
		printf("=======%s=======\n",current_strategy_list[0].startDate);

		
		//current_strategy_list = (strategy_list*)realloc(current_strategyy_list,strategy_list_node_size*(strategy_id+1));
		strategy_list* t_strategy_list  = NULL;
		t_strategy_list = (strategy_list*)realloc(current_strategy_list,5000);
		if(t_strategy_list == NULL)
		{
			xml_close();
			return FALSE;
		}
		current_strategy_list = t_strategy_list;	
		printf("======================");
	}
}

boolean construct_timetable(strategy_list * list)
{
	int i, j;
	for(i=0; i<strategy_list_size; i++)
	{
		if(!one_strategy_timetable(current_strategy_list[i].startDate, current_strategy_list[i].endDate,\
			current_strategy_list[i].startTime, current_strategy_list[i].endTime))
		{
			printf("one_strategy_timetable fail\n");
			return FALSE;
		}
	}
	return TRUE;
}

boolean one_strategy_timetable(char* startDate, char* endDate, char* startTime, char* endTime)
{
	printf("one_strategy_timetable:\nstartDate:%s, endDate:%s,\nstartTime:%s,endTime:%s\n",startDate,endDate,startTime, endTime);
   	int startData_startTime_count_second = -1, endDate_endTime_count_second = -1, endDate_startTime_count_second = -1, countSec = 0;
   	char startDate_startTime[50] = {0};
   	char startDate_endTime[50] = {0};	
    char endDate_endTime[50] = {0};
	char endDate_startTime[50] = {0};
   	sprintf(startDate_startTime,"%s %s",startDate, startTime);
   	sprintf(startDate_endTime,"%s %s",startDate, endTime);
   	sprintf(endDate_endTime,"%s %s",endDate, endTime);
	sprintf(endDate_startTime,"%s %s",endDate, startTime);
	printf("startDate_startTime:%s, endDate_endTime:%s\n", startDate_startTime, endDate_endTime);
	
   	int startDate_endTime_count_second = str2sec(time_format(startDate_endTime));
	printf("%d\n",startDate_endTime_count_second );
   	if((startData_startTime_count_second = str2sec(time_format(startDate_startTime))) == -1)
	{
		return FALSE;
	}
	if((endDate_endTime_count_second = str2sec(time_format(endDate_endTime))) == -1)
	{
		return FALSE;
	}	
	if((endDate_startTime_count_second = str2sec(time_format(endDate_startTime))) == -1)
	{
		return FALSE;
	}	
	
	if(time(NULL) > endDate_endTime_count_second)
	{
		printf("stragety overtime,currenttime:%d,strategy_endTime%d\n",time(NULL),endDate_endTime_count_second);
		return TRUE;
	}
	strategy_Timetable * t_timeTable = NULL;
//	//timetable_size表示已经分配内存的天数
	int t_days = (endDate_startTime_count_second - startData_startTime_count_second)/THE_NUMBER_OF_SECONDS_A_DAY;
	printf("===========days%d============\n",t_days);
//	t_timeTable = (strategy_Timetable * )realloc(timetable, t_days*sizeof(strategy_Timetable));
//	if(t_timeTable == NULL)
//	{
//		printf("one_strategy_timetable realloc fail\n");
//		return FALSE;
//	}
//	timetable = t_timeTable;
    int start_end_sec = startDate_endTime_count_second - startData_startTime_count_second; 
	//if(start_end_sec < 0)代表startDate_endTime是第二天
	if(start_end_sec < 0)
	{
		printf("next day\n");
		start_end_sec = THE_NUMBER_OF_SECONDS_A_DAY + start_end_sec;
	}
	printf("endDate_endTime_count_second:%d,startDate_endTime_count_second:%d,startData_startTime_count_second:%d,start_end_sec:%d\n",endDate_endTime_count_second,startDate_endTime_count_second,startData_startTime_count_second,start_end_sec);
	int t_startTime = startData_startTime_count_second;
	strategy_Timetable t_timemoment;
	Position t_position = GetHead(timetable), t_node;
	printf("current time:%d\n",time(NULL));
	while(1)
	{
		if(t_startTime > endDate_startTime_count_second)
		{
			break;
		}
		t_position = ListTraverse(timetable, t_position, t_startTime);  
		t_timemoment.startMoment = t_startTime;
		t_timemoment.endMoment= t_startTime + start_end_sec;
		t_node = MakeNode(&t_timemoment);
		/*在链表中p位置之后插入新节点s*/  
		InsAfter(timetable, t_position, t_node);  
		t_position = t_position->next;
		t_startTime  += THE_NUMBER_OF_SECONDS_A_DAY;
	}
	printf("GetSize:%d\n",GetSize(timetable));
	return TRUE;
}

boolean config_init()
{
	boolean t_return = TRUE;
	//get info from cmdline of boot_args
	int cmdline_fd = open("/proc/cmdline",O_RDONLY);
	char t_cmdline_buff[256], cmdline_info[512];
	memset(network_number, '\0', 16);
	if(cmdline_fd == -1)
		return FALSE;
	memset(cmdline_info, '\0', 512);
	memset(t_cmdline_buff, '\0', 256);
	if(read(cmdline_fd, t_cmdline_buff,256)>0)
	{
	
	strcat(cmdline_info,t_cmdline_buff);
		memset(t_cmdline_buff, '\0', 256);
	}
	printf("network_number:%s,%d\n",cmdline_info,strlen(cmdline_info));
	int t_len = strlen(cmdline_info), z = 0;
	for(z=0; z<t_len-15; z++)	
	{
		if(cmdline_info[z] == 'e' && cmdline_info[z+1] == 't'&&
		cmdline_info[z+2] == 'h' && cmdline_info[z+3] == '0')
		 break;
	}
	if(t_len-15 == z)
		return FALSE;
	printf("=========%c============",cmdline_info[z]);
	if( t_len < z+15)
		return FALSE;
	strcpy(network_number, cmdline_info+z+9);//加上"eth0:off:"  的长度
	network_number[6] = '\0';
	printf("network_number:%s\n",network_number);
	
	//excete xml
	memset(strategy_file_name,'\0',sizeof(strategy_file_name));
	if(!find_new_file(STRATEGY_FILE_DIR,strategy_file_name))
	{
			strcpy(strategy_file_name,"-1");
			printf("config_init find_new_file strategy_file_name fail\n");
			
			char dir_name_info[128][128] = {""}, t_cmd[256];
			memset(dir_name_info[0], '\0', 128);
			strcpy(dir_name_info[0],"strategy");
			
			if(get_file_from_server(dir_name_info, DEFAULT_STRATEGY_VERSION))
			{
				printf("update t_strategy version %s success\n", DEFAULT_STRATEGY_VERSION);
				sprintf(t_cmd,"mv /app/%s /app/strategy/", DEFAULT_STRATEGY_VERSION);
				mySystem(t_cmd);
				strcpy(strategy_file_name,DEFAULT_STRATEGY_VERSION);
				goto strategy_parse_point;
			}
			else
			{
				strcpy(strategy_file_name,"-1");
				t_return = FALSE;
			}
	}
	else
	{
strategy_parse_point:
		if(!strategy_parse(strategy_file_name))
		{
			printf("config_init strategy_parse fail\n");
			t_return = FALSE;
		}
		
	}
	memset(protocl_file_name, '\0', 128);
	//protocl document
	if(!find_new_file(PROTOCOL_FILE_DIR,protocl_file_name))
	{
			protocol_version = 'E';
			printf("config_init find_new_file protocl_file_name fail\n");
			
			char dir_name_info[128][128] = {""}, t_cmd[256];
			memset(dir_name_info[0], '\0', 128);
			strcpy(dir_name_info[0],"protocol");
			
			if(get_file_from_server(dir_name_info, DEFAULT_PROTOCOL_VERSION))
			{
				printf("update protocol version %s success\n", DEFAULT_PROTOCOL_VERSION);
				sprintf(t_cmd,"mv /app/%s /app/protocol/", DEFAULT_PROTOCOL_VERSION);
				mySystem(t_cmd);
				strcpy(protocl_file_name,DEFAULT_PROTOCOL_VERSION);
				goto xml_config_point;
			}
			else
			{
				strcpy(protocl_file_name,"-1");
				t_return = FALSE;
			}
	}
	else
	{
xml_config_point:
		protocol_version = protocl_file_name[5]-'0';
		if(!xml_config(protocl_file_name))
		{
			printf("config_init protocl_file_name fail\n");
			t_return = FALSE;
		}
		
	}	
	return t_return;
}

