#ifndef CREMOTETCPSERVER_H
#define CREMOTETCPSERVER_H

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <iostream>  
#include <string>
#include <cstdio>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <error.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

#define PORT 1502 
#define SERVER_IP "122.51.1.204" 
#define BUFFER_SIZE 1024 
#define FILE_NAME_MAX_SIZE 512 

class CRemoteTcpServer {
public:
	CRemoteTcpServer();
	~CRemoteTcpServer();

	string InitDir = "Data";

	int SetUpRemoteServer();
	int RecFile();
	
private:
        struct sockaddr_in server_addr;
	int m_Socket;
	int m_New_Socket;

	time_t rawtime;
	struct tm* ptminfo;
	
	string m_InitDate;
	string m_CurDir;
	string m_SaveFileName;

	int Print_Current_Date();
	string Get_Current_Dir();
	string Get_Current_Date();

	unsigned int m_Received_Size = 0;
	unsigned int m_Full_Size = 0;
	unsigned int m_Remained_Size = 0;
};

#endif

