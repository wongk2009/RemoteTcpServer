#pragma once
#ifndef CREMOTEWINTCPSERVER_H
#define CREMOTEWINTCPSERVER_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <WinSock2.h> 
#include <windows.h>    //Í·ÎÄ¼þ  
#include <iostream>  
#include <string>
#include <cstdio>
#include <ctime>

using namespace std;

#define PORT 1502 
#define SERVER_IP "192.168.2.103" 
#define BUFFER_SIZE 1024 
#define FILE_NAME_MAX_SIZE 512 
#pragma comment(lib, "WS2_32") 

class CRemoteWinTcpServer {
public:
	CRemoteWinTcpServer() = default;
	CRemoteWinTcpServer(const char*, const int);
	~CRemoteWinTcpServer();

	string InitDir = "Data";

	int SetUpRemoteServer(const char*, const int);
	int RecFile();
	
private:
	sockaddr_in server_addr;
	WSADATA wsaData;
	WORD socketVersion;
	SOCKET m_Socket;
	SOCKET m_New_Socket;

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

