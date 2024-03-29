#include "CRemoteTcpServer.h"

CRemoteTcpServer::CRemoteTcpServer() {
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	m_InitDate = Get_Current_Date();
	m_CurrentDate = m_InitDate;

        //建立Data/文件夹
        int isCreate = mkdir(InitDir.c_str(),S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
	if( !isCreate ) {
	        Print_Current_Date();
	        printf("Create Path:%s\n", InitDir.c_str());
	}
	else {
	        Print_Current_Date();
                printf("Create Path:%s Failed!\n", InitDir.c_str());
	}
        Make_Current_Dir();
}

CRemoteTcpServer::~CRemoteTcpServer() {
	close(m_New_Socket);
	close(m_Socket);
}

int CRemoteTcpServer::SetUpRemoteServer() {
	// 创建socket 
	if((m_Socket = socket(AF_INET, SOCK_STREAM, 0) ) == -1)
	{
	        Print_Current_Date();
		printf("Create Socket Error!\n");
		exit(1);
	}

        int flag = 1;
        if (setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0)
	{
	        Print_Current_Date();
	        printf("Socket Setsockopt Error=%d(%s)!!!\n", errno, strerror(errno));
	        exit(1);
	}

	//绑定socket和服务端(本地)地址 
	if (bind(m_Socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
	{
	        Print_Current_Date();
		printf("Server Bind Failed!\n");
		exit(1);
	}

	//监听 
	if (listen(m_Socket, 10) == -1)
	{
	        Print_Current_Date();
		printf("Server Listen Failed!\n");
		exit(1);
	}

	Print_Current_Date();
	printf("Listening To Client...\n");

	sockaddr_in client_addr;
	socklen_t sin_size = sizeof(struct sockaddr);

	m_New_Socket = accept(m_Socket, (struct sockaddr *)&client_addr, &sin_size);

	if (m_New_Socket == -1)
	{
		Print_Current_Date();
		printf("Server Accept Failed!\n");
		return -1;
	}
	Print_Current_Date();
	printf("Client Connect Successfully!\n");

	return 0;
}

int CRemoteTcpServer::RecFile() {
	char buffer[BUFFER_SIZE];

        //接收心跳包
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(m_New_Socket, buffer, BUFFER_SIZE, 0) < 0)
	{
            #ifndef NDEBUG
	        Print_Current_Date();
		printf("Server Connect Receive Failed!\n");
            #endif
	    close(m_New_Socket);
	    close(m_Socket);
	    return -1;
	}
	else {
	    if(buffer[0] != 0x55) {
                #ifndef NDEBUG
    	            Print_Current_Date();
    		    printf("Server Heart not 0x55!\n");
                #endif
    	        close(m_New_Socket);
	        close(m_Socket);
    	        return -1;
	    }
	}
	memset(buffer, 0, BUFFER_SIZE);
	buffer[0] = 0x55;
	if (send(m_New_Socket, buffer, BUFFER_SIZE, 0) < 0) {
            #ifndef NDEBUG
	        Print_Current_Date();
		printf("Server Connect Send Failed!\n");
            #endif
	    close(m_New_Socket);
	    close(m_Socket);
	    return -1;
	}
	Print_Current_Date();
	printf("Heart Beat Receive!\n");


        //接收文件名
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(m_New_Socket, buffer, BUFFER_SIZE, 0) < 0)
	{
                #ifndef NDEBUG
		    Print_Current_Date();
		    printf("Receive File Name Failed!\n");
                #endif
		close(m_New_Socket);
		close(m_Socket);
		return -1;
	}
	string tmpDate = Get_Current_Date();
	if(tmpDate == m_CurrentDate) {
	}
	else {
		m_CurrentDate = Get_Current_Date();
                Make_Current_Dir();
	}

	char file_name[FILE_NAME_MAX_SIZE + 1];
	memset(file_name, 0, FILE_NAME_MAX_SIZE + 1);
	strncpy(file_name, buffer, strlen(buffer) > FILE_NAME_MAX_SIZE ? FILE_NAME_MAX_SIZE : strlen(buffer));
	m_SaveFileName.clear();
	string strSubDir = Get_Current_Dir();
	m_SaveFileName = strSubDir + string(file_name);
	if(m_SaveFileName != strSubDir) {
	        Print_Current_Date();
	        printf("Save File: %s\n", m_SaveFileName.c_str());
	}

	//接收文件大小数据
	memset(buffer, 0, BUFFER_SIZE);
	if (recv(m_New_Socket, buffer, BUFFER_SIZE, 0) < 0)
	{
                #ifndef NDEBUG
		    Print_Current_Date();
		    printf("Server Receive Data Failed!\n");
		#endif
		close(m_New_Socket);
		close(m_Socket);
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
                #ifndef NDEBUG
		    Print_Current_Date();
		    printf("File: %s Can Not Open To Write\n", m_SaveFileName.c_str());
                #endif
		//close(m_New_Socket);
		//close(m_Socket);
		//return -1;
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
                                #ifndef NDEBUG
		    		    Print_Current_Date();
				    printf("File: %s Write Failed\n", m_SaveFileName.c_str());
				#endif
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
                        #ifndef NDEBUG
			    Print_Current_Date();
		            printf("Server Receive Data Failed!\n");
			#endif
		        close(m_New_Socket);
		        close(m_Socket);
			return -1;
		}
		Print_Current_Date();
		printf("Receive File: %s From Client Successful!\n", file_name);
        	fclose(fp);
        	//close(m_New_Socket);
        }
	//close(m_Socket);
	return 0;
}

int CRemoteTcpServer::Print_Current_Date() {
	time(&rawtime);
	ptminfo = localtime(&rawtime);
	printf("%02d-%02d-%02d %02d:%02d:%02d: ",
		ptminfo->tm_year + 1900, ptminfo->tm_mon + 1, ptminfo->tm_mday,
		ptminfo->tm_hour, ptminfo->tm_min, ptminfo->tm_sec);
	return 0;
}

string CRemoteTcpServer::Get_Current_Dir() {
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

string CRemoteTcpServer::Get_Current_Date() {
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

int CRemoteTcpServer::Make_Current_Dir() {
	Get_Current_Dir();

        int isCreate = mkdir(m_CurDir.c_str(),S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
	if( !isCreate ) {
	        Print_Current_Date();
	        printf("Create Path:%s\n", m_CurDir.c_str());
	}
	else {
	        Print_Current_Date();
                printf("Create Path:%s Failed!\n", m_CurDir.c_str());
		return -1;
	}
	return 0;
}
