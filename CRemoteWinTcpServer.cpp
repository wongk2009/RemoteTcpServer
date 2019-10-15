#include "CRemoteWinTcpServer.h"

CRemoteWinTcpServer::CRemoteWinTcpServer(const char* ip = "127.0.0.1", const int port = 1502) {
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	server_addr.sin_port = htons(PORT);

	m_InitDate = Get_Current_Date();
	Get_Current_Dir();

	if (CreateDirectory(InitDir.c_str(), NULL) != 1) {
		Print_Current_Date();
		cout << "创建文件夹Data/失败！" << endl;
	}
	if (CreateDirectory(m_CurDir.c_str(), NULL) != 1) {
		Print_Current_Date();
		cout << "创建文件夹" << m_CurDir << "失败" << endl;
	}

	SetUpRemoteServer(ip, port);

}

CRemoteWinTcpServer::~CRemoteWinTcpServer() {
	closesocket(m_New_Socket);
	closesocket(m_Socket);
	//释放winsock库 
	WSACleanup();

}

int CRemoteWinTcpServer::SetUpRemoteServer(const char* ip = "127.0.0.1", const int port = 502) {

	// 初始化socket dll 
	wsaData;
	socketVersion = MAKEWORD(2, 0);
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		printf("Init socket dll error!");
		exit(1);
	}

	// 创建socket 
	m_Socket = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == m_Socket)
	{
		printf("Create Socket Error!");
		exit(1);
	}

	//绑定socket和服务端(本地)地址 
	if (SOCKET_ERROR == bind(m_Socket, (LPSOCKADDR)& server_addr, sizeof(server_addr)))
	{
		printf("Server Bind Failed: %d", WSAGetLastError());
		exit(1);
	}

	//监听 
	if (SOCKET_ERROR == listen(m_Socket, 10))
	{
		printf("Server Listen Failed: %d", WSAGetLastError());
		exit(1);
	}
	return 0;
}

int CRemoteWinTcpServer::RecFile() {
	while (1)
	{
		Print_Current_Date();
		printf("Listening To Client...\n");

		sockaddr_in client_addr;
		int client_addr_len = sizeof(client_addr);

		m_New_Socket = accept(m_Socket, (sockaddr*)& client_addr, &client_addr_len);

		if (SOCKET_ERROR == m_New_Socket)
		{
			Print_Current_Date();
			printf("Server Accept Failed: %d", WSAGetLastError());
			return -1;
		}

		char buffer[BUFFER_SIZE];
		memset(buffer, 0, BUFFER_SIZE);
		if (recv(m_New_Socket, buffer, BUFFER_SIZE, 0) < 0)
		{
			Print_Current_Date();
			printf("Server Receive Data Failed!");
			return -1;
		}

		char file_name[FILE_NAME_MAX_SIZE + 1];
		memset(file_name, 0, FILE_NAME_MAX_SIZE + 1);
		strncpy(file_name, buffer, strlen(buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));
		m_SaveFileName.clear();
		string strSubDir = Get_Current_Dir();
		m_SaveFileName = strSubDir + string(file_name);
		Print_Current_Date();
		printf("保存文件： %s\n", m_SaveFileName.c_str());

		//接收文件大小数据
		if (recv(m_New_Socket, buffer, BUFFER_SIZE, 0) < 0)
		{
			Print_Current_Date();
			printf("Server Receive Data Failed!");
			return -1;
		}
		char file_size[FILE_NAME_MAX_SIZE + 1];
		memset(file_size, 0, FILE_NAME_MAX_SIZE + 1);
		strncpy(file_size, buffer, strlen(buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));
		m_Full_Size = atoi(file_size);

		//打开文件，准备写入 
		FILE* fp = fopen(m_SaveFileName.c_str(), "wb"); //windows下是"wb",表示打开一个只写的二进制文件 
		if (NULL == fp)
		{
			Print_Current_Date();
			printf("File: %s Can Not Open To Write\n", m_SaveFileName.c_str());
			return -1;
		}
		else
		{
			memset(buffer, 0, BUFFER_SIZE);
			int length = 0;
			while ((length = recv(m_New_Socket, buffer, BUFFER_SIZE, 0)) > 0)
			{
				m_Received_Size += length;
				m_Remained_Size = m_Full_Size - m_Received_Size;
				if (fwrite(buffer, sizeof(char), length, fp) < length)
				{
					Print_Current_Date();
					printf("File: %s Write Failed\n", m_SaveFileName.c_str());
					break;
				}
				memset(buffer, 0, BUFFER_SIZE);
				//接收完毕后，退出接收
				if (m_Remained_Size == 0) {
					m_Received_Size = 0;
					m_Full_Size = 0;
					m_Remained_Size = 0;
					break;
				}
			}
			//如果接收失败
			if (length < 0)
			{
				Print_Current_Date();
				printf("Server Receive Data Failed!");
				return -1;
			}
			Print_Current_Date();
			printf("Receive File: %s From Client Successful!\n", file_name);
		}
		fclose(fp);
		//closesocket(m_New_Socket);
	}
	closesocket(m_Socket);
	//释放winsock库 
	WSACleanup();
	return 0;
}

int CRemoteWinTcpServer::Print_Current_Date() {
	time(&rawtime);
	ptminfo = localtime(&rawtime);
	printf("%02d-%02d-%02d %02d:%02d:%02d: ",
		ptminfo->tm_year + 1900, ptminfo->tm_mon + 1, ptminfo->tm_mday,
		ptminfo->tm_hour, ptminfo->tm_min, ptminfo->tm_sec);
	return 0;
}

string CRemoteWinTcpServer::Get_Current_Dir() {
	string tmp_CurDate = Get_Current_Date();
	if (tmp_CurDate == m_InitDate) {
		m_CurDir.clear();
		m_CurDir = "Data/" + tmp_CurDate + "/";
	}
	else {
		m_CurDir.clear();
		m_CurDir = "Data/" + tmp_CurDate + "/";
		m_InitDate = m_InitDate;
	}
	return m_CurDir;
}

string CRemoteWinTcpServer::Get_Current_Date() {
	time(&rawtime);
	ptminfo = localtime(&rawtime);
	string strYear = to_string(ptminfo->tm_year + 1900);
	string strMonth = to_string(ptminfo->tm_mon + 1);
	string strDay = to_string(ptminfo->tm_mday);
	while (strYear.size() < 2)
	{
		strYear = strYear.insert(0, "0");
	}
	while (strMonth.size() < 2)
	{
		strMonth = strMonth.insert(0, "0");
	}
	while (strDay.size() < 2)
	{
		strDay = strDay.insert(0, "0");
	}
	string strCurDate = strYear + "-" + strMonth + "-" + strDay;
	return strCurDate;
}