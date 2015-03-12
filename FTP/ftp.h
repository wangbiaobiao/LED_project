#ifndef ____FTP_H___
#define ____FTP_H___
//#include "common.h"

typedef enum ftp_cmd_type
{
	USER,
	PASS,
	SIZE,
	CWD,
	PASV,
	PORT,
	RETR,
	STOR,
	REST,
	QUIT	
}ftp_cmd_type;

//typedef enum ftp_errno
//{
//	USER,
//	PASS,
//	SIZE,
//	CWD,
//	PASV,
//	PORT,
//	RETR,
//	STOR,
//	REST,
//	QUIT	
//}ftp_cmd_type;

typedef enum ftp_response_status
{
	Restart_marker_reply = 110,
	Service_ready_in_nnn_minutes = 120,
	Data_connection_already_open = 125,
	File_status_okay = 150,
	Command_okay = 200,
	Command_not_implemented_1 = 202,
	System_status = 211,
	Directory_status = 212,
	File_status = 213,
	Help_message = 214,
	Name_system_type = 215,
	Service_ready_for_new_user = 220,
	Service_closing_control_onnection = 221,
	Data_onnection_pen = 225,
	Closing_data_connection = 226,
	Entering_Passive_Mode = 227,
	User_logged_in = 230,
	Requested_file_action_okay = 250,
	Pathname_created = 257,
	User_name_okay = 331,
	Need_account_for_login = 332,
	Requested_file_action_pending_further_information = 350,
	Service_not_available = 421,
	Cant_open_data_connection = 425,
	Connection_closed = 426,
	Requested_file_action_not_taken = 450,
	Requested_action_aborted = 451,
	Insufficient_storage_space_in_system = 452,
	Syntax_error_command_unrecognized = 500,
	Syntax_error_in_parameters = 501,
	Command_not_implemented_2 = 502,
	Bad_sequence_of_commands = 503,
	Command_not_implemented_for_that_parameter = 504,
	Not_logged_in = 530,
	Need_account_for_storing_files = 532,
	File_unavailable  = 550,
	Requested_action_aborted_1 = 551,
	Requested_file_action_aborted = 552,
	File_name_not_allowed = 553
}ftp_stor_status;

#define USERNAME "zhengp"
#define PASSWORD "zhengp"
#define STRATEGY_FILE_NAME "4.2.2"
#define GETWAY_FILE_NAME "ledPro" 

//#define FTP_SERVER_IP "114.215.196.51"
#define FTP_SERVER_IP "192.168.80.70"
//#define FTP_SERVER_PORT 21
#define FTP_SERVER_PORT 11121

#define FTP_CMD_FAIL -1
#define FTP_RCV_TIMEOUT 30

extern char ftp_path[256];

boolean ftp_init();
boolean ftp_send_cmd_buff(const char* send_buff, char* ret_str);
int ftp_send_cmd(ftp_cmd_type type, void *arg);
boolean get_file_from_server(char (*dir_name)[128], char *file_name);

#endif
