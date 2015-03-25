#include "xml_parse.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
strategy_list * current_strategy_list = NULL; 
int strategy_list_size = 0;
DList* timetable = NULL;
extern char gate_way_number[128];

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
	xmlDocPtr doc;
	xmlNodePtr curNode, dcurNode, ecurNode;
	xmlChar *szkey;
	int number;
	timetable = InitList();
	int strategy_list_node_size = sizeof(strategy_list);
	current_strategy_list = (strategy_list*)malloc(strategy_list_node_size);
	xml_open();
	char path[256] = "/app/strategy/";
	strcat(path,filename);
	doc = xmlReadFile(path, "UTF-8", XML_PARSE_RECOVER);
	if(NULL == doc)	
	{		
		printf("Document not parsed successfully\n");
		xmlFreeDoc(doc);
		return -1;
	}

	curNode = xmlDocGetRootElement(doc);
	if(NULL == curNode)
	{
		printf("empty document\n");
		xmlFreeDoc(doc);
		return -1;
	}
	curNode = curNode->children->next;
	while(curNode != NULL)
	{
		number++;
		szkey = xmlGetProp(curNode, BAD_CAST "node_address");
		strcpy(current_strategy_list[number-1].node_addr,szkey);
		printf("debug is %s %s\n",szkey,curNode->name);
		dcurNode = curNode->children->next;
		while(dcurNode != NULL)
		{
			printf("%s\n",dcurNode->name);
			ecurNode = dcurNode->children->next;
			while(ecurNode != NULL)
			{
				if(!xmlStrcmp(ecurNode->name, BAD_CAST "startDate"))
				{
					szkey = xmlGetProp(ecurNode, BAD_CAST "value");
					strcpy(current_strategy_list[number-1].startDate,szkey);
				}
				if(!xmlStrcmp(ecurNode->name, BAD_CAST "endDate"))
				{
					szkey = xmlGetProp(ecurNode, BAD_CAST "value");
					strcpy(current_strategy_list[number-1].endDate,szkey);
				}
				if(!xmlStrcmp(ecurNode->name, BAD_CAST "startTime"))
				{
					szkey = xmlGetProp(ecurNode, BAD_CAST "value");
					strcpy(current_strategy_list[number-1].startTime,szkey);
				}
				if(!xmlStrcmp(ecurNode->name, BAD_CAST "endTime"))
				{
					szkey = xmlGetProp(ecurNode, BAD_CAST "value");
					strcpy(current_strategy_list[number-1].endTime,szkey);
				}
				ecurNode = ecurNode->next->next;
				strategy_list* t_strategy_list  = NULL;
				t_strategy_list = (strategy_list*)realloc(current_strategy_list,strategy_list_node_size*10);
				current_strategy_list = t_strategy_list;
			}
			dcurNode = dcurNode->next->next;
		}
		curNode = curNode->next->next;
	}
	xmlFreeDoc(doc);
	return construct_timetable(current_strategy_list);
}



Position t_position;
boolean construct_timetable(strategy_list * list)
{
	int i, j;
	for(i=0; i<strategy_list_size; i++)
	{
		printf("start date is %s\n",current_strategy_list[i].startDate);
		printf("end date is %s\n",current_strategy_list[i].endDate);
		printf("start time is %s\n",current_strategy_list[i].startTime);
		printf("stop time is %s\n",current_strategy_list[i].endTime);
		printf("nod addr is %s\n",current_strategy_list[i].node_addr);

	}
	t_position = timetable->head;
	for(i=0; i<strategy_list_size; i++)
	{
		if(!one_strategy_timetable(current_strategy_list[i].startDate, current_strategy_list[i].endDate,\
			current_strategy_list[i].startTime, current_strategy_list[i].endTime,current_strategy_list[i].node_addr))
		{
			printf("one_strategy_timetable fail\n");
			return FALSE;
		}
	}
	#ifdef DEBUG
	PNode t_head = GetHead(timetable);
	PNode t_node = t_head;
	printf("GetSize:%d\n",GetSize(timetable));
	for(; t_node->next != NULL; t_node=t_node->next)
	{
		time_printf(t_node->data.startMoment, t_node->data.endMoment);
		printf("addr is %d\n",t_node->data.node_addr);
	}
	#endif
	return TRUE;
}

#define DEBUG

boolean one_strategy_timetable(char* startDate, char* endDate, char* startTime, char* endTime, char* nod_addr)
{
	printf("one_strategy_timetable:\nstartDate:%s, endDate:%s,\nstartTime:%s,endTime:%s\n",startDate,endDate,startTime, endTime);
   	int startData_startTime_count_second = -1, endDate_endTime_count_second = -1, endDate_startTime_count_second = -1, countSec = 0;
   	char startDate_startTime[50] = {0};
    char endDate_endTime[50] = {0};
	int addr;
   	sprintf(startDate_startTime,"%s %s",startDate, startTime);
   	sprintf(endDate_endTime,"%s %s",endDate, endTime);
	printf("startDate_startTime:%s, endDate_endTime:%s\n", startDate_startTime, endDate_endTime);

	addr = atoi(nod_addr);
	#ifdef DEBUG
	printf("the timetable's addr is %d\n",addr);
	#endif
   	if((startData_startTime_count_second = str2sec(time_format(startDate_startTime))) == -1)
	{
		return FALSE;
	}
	if((endDate_endTime_count_second = str2sec(time_format(endDate_endTime))) == -1)
	{
		return FALSE;
	}	
	
	strategy_Timetable * t_timeTable = NULL;
	
	int t_startTime = startData_startTime_count_second;
	strategy_Timetable t_timemoment;
	
	Position t_node;
	
	//t_position = ListTraverse(timetable, t_position, t_startTime);  
	t_timemoment.startMoment = startData_startTime_count_second;
	t_timemoment.endMoment= endDate_endTime_count_second;
	t_timemoment.node_addr = addr;
	t_timemoment.flag = 0;
	
	t_node = MakeNode(&t_timemoment);
	/*在链表中p位置之后插入新节点s*/  
	InsAfter(timetable, t_position, t_node);  
	t_position = t_position->next;

	PNode t_head = GetHead(timetable);
	PNode tt_node = t_head;
	
	#ifdef DEBUG
	printf("the timetable size is :%d\n",GetSize(timetable));
	for(; tt_node->next != NULL; tt_node=tt_node->next)
	{
		time_printf(tt_node->next->data.startMoment, tt_node->next->data.endMoment);
		printf("the node addr is %d\n",tt_node->next->data.node_addr);
	}
	#endif
	
	return TRUE;
}

boolean config_init()
{
	boolean t_return = TRUE;
	//excete xml
	memset(strategy_file_name,'\0',sizeof(strategy_file_name));
	if(!find_new_file(STRATEGY_FILE_DIR,strategy_file_name))
	{
			strcpy(strategy_file_name,"-1");
			printf("config_init find_new_file strategy_file_name fail\n");
			
			char dir_name_info[128][128] = {""}, t_cmd[256];
			memset(dir_name_info[0], '\0', 128);
			strcpy(dir_name_info[0],"strategy");
			
			if(get_file_from_server(dir_name_info, gate_way_number))
			{
				printf("update t_strategy version %s success\n", gate_way_number);
				sprintf(t_cmd,"mv /app/%s /app/strategy/", gate_way_number);
				mySystem(t_cmd);
				strcpy(strategy_file_name,gate_way_number);
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

